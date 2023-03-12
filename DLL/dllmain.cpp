#include "Memory.hpp"
#include "Player/Player.hpp"
#include "GUI/GUI.hpp"

#define GLEW_STATIC
#include <gl/glew.h>
#include <windowsx.h>

#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>

using wglSwapBuffers_t = BOOL(__stdcall*) (HDC hDc);
wglSwapBuffers_t wglSwapBuffers_orig = nullptr;

HWND g_hGameWindow;
WNDPROC g_hGameWindowProc;

bool g_active = false;
GUIHelper g_gui;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK windowProc_hook(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (~uMsg & WM_FTLAI) // not AI generated
    {
        auto& io = ImGui::GetIO();

        if (uMsg == WM_KEYDOWN && wParam == VK_OEM_3) // ~ key
        {
            g_overlay = !g_overlay;
            ImGui::CaptureMouseFromApp(false);
            ImGui::CaptureKeyboardFromApp(false);
            return false;
        }

        CallWindowProc(
            reinterpret_cast<WNDPROC>(
                ImGui_ImplWin32_WndProcHandler),
            hWnd, uMsg, wParam, lParam);

        bool imguiWants =
            g_overlay && (io.WantCaptureMouse || io.WantCaptureKeyboard);

        bool ignorable =
            uMsg == WM_KEYUP ||
            uMsg == WM_KEYDOWN ||
            uMsg == WM_MOUSEMOVE ||
            uMsg == WM_LBUTTONDOWN ||
            uMsg == WM_LBUTTONUP ||
            uMsg == WM_LBUTTONDBLCLK ||
            uMsg == WM_MBUTTONDOWN ||
            uMsg == WM_MBUTTONUP ||
            uMsg == WM_MBUTTONDBLCLK ||
            uMsg == WM_RBUTTONDOWN ||
            uMsg == WM_RBUTTONUP ||
            uMsg == WM_RBUTTONDBLCLK ||
            uMsg == WM_XBUTTONDOWN ||
            uMsg == WM_XBUTTONUP ||
            uMsg == WM_XBUTTONDBLCLK ||
            false;

        if ((g_active && g_gui.inGame()) || imguiWants)
        {
            ShowCursor(true);

            if (ignorable)
                return true;
        }

    }
    else
    {
        uMsg &= ~WM_FTLAI;
    }

    return CallWindowProc(g_hGameWindowProc, hWnd, uMsg, wParam, lParam);
}

BOOL __stdcall wglSwapBuffers_hook(HDC hDc)
{
    static bool init = false;

    if (!init)
    {
        // Get the window handle
        g_hGameWindow = WindowFromDC(hDc);

        // Overwrite the wndProc function
        g_hGameWindowProc =
            reinterpret_cast<WNDPROC>(
                SetWindowLongPtr(
                    g_hGameWindow, GWLP_WNDPROC,
                    reinterpret_cast<LONG_PTR>(windowProc_hook)));

        glewInit();
        ImGui::CreateContext();
        ImGui_ImplWin32_Init(g_hGameWindow);
        ImGui_ImplOpenGL3_Init();
        ImGui::CaptureMouseFromApp();

        init = true;
    }

    if (g_overlay && init)
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        g_gui.render();
        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    return wglSwapBuffers_orig(hDc);
}

DWORD WINAPI patcherThread(HMODULE hModule)
{
    constexpr char PointerMapFile[] = "pointermap.cfg";
    std::ifstream map(PointerMapFile);
    if (!map)
    {
        AllocConsole();
        FILE* f = new FILE();
        freopen_s(&f, "CONOUT$", "w", stdout);

        std::cout <<
            "Couldn't open " << PointerMapFile << ".\n"
            "Make sure it's present and make sure the application has appropriate permissions.\n";

        FreeConsole();

        return 1;
    }

    AddressData addresses;

    try
    {
        addresses.parse(map);
    }
    catch (const std::exception& e)
    {
        AllocConsole();
        FILE* f = new FILE();
        freopen_s(&f, "CONOUT$", "w", stdout);

        std::cout << "Couldn't parse " << PointerMapFile << ".\n";
        std::cout << e.what() << '\n';

        FreeConsole();

        return 1;
    }

    StateReader reader(addresses);
    Player player(reader);

    g_gui.setReader(reader);
    g_gui.setPlayer(player);

    // Hook OpenGL stuff
    wglSwapBuffers_orig =
        reinterpret_cast<wglSwapBuffers_t>(
            GetProcAddress(GetModuleHandle(L"opengl32.dll"), "wglSwapBuffers"));

    wglSwapBuffers_orig =
        reinterpret_cast<wglSwapBuffers_t>(
            mem::trampHook32(
                reinterpret_cast<BYTE*>(wglSwapBuffers_orig),
                reinterpret_cast<BYTE*>(wglSwapBuffers_hook),
                7));

    while (true)
    {
        constexpr int POLL_RATE = 16;

        reader.poll();
        player.think(POLL_RATE);
        std::this_thread::sleep_for(std::chrono::milliseconds(POLL_RATE));
    }

    return 0;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        HANDLE hThread = CreateThread(
            nullptr, 0,
            (LPTHREAD_START_ROUTINE)patcherThread, hModule, 0, 0);

        if (hThread)
            CloseHandle(hThread);

        break;
    }
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

