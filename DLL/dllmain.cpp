#include "Memory.hpp"
#include "GUI/GUI.hpp"
#include "GameState/CleanReader.hpp"

#define GLEW_STATIC
#include <gl/glew.h>

#include <pybind11/embed.h>
#include <pybind11/chrono.h>
namespace py = pybind11;

#include <iostream>
#include <fstream>
#include <mutex>
#include <condition_variable>
#include <filesystem>

using wglSwapBuffers_t = BOOL(__stdcall*) (HDC hDc);
wglSwapBuffers_t wglSwapBuffers_orig = nullptr;

HWND g_hGameWindow;
WNDPROC g_hGameWindowProc;

bool g_overlay = true;
bool g_active = false;
bool g_pause = false;
bool g_imguiInit = false;
GUIHelper g_gui;

constexpr char PYTFTL_FOLDER[] = "pyftl";
constexpr char PYTFTL_MAIN_MODULE[] = "main";
constexpr char PYTFTL_EXTENSION[] = ".py";

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#define WM_FTLAI 0x800 // temporary pls remove

LRESULT CALLBACK windowProc_hook(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (~uMsg & WM_FTLAI && g_imguiInit) // not AI generated
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

        bool allowedKeys = // allow pausing by human
            wParam == VK_ESCAPE ||
            wParam == VK_SPACE ||
            wParam == 0x49 || // I
            wParam == 0x4A || // J
            wParam == 0x55; // U

        if (wParam == VK_F9) g_active = !g_active; // F9 to (de)activate AI

        if ((g_active && !g_pause && g_gui.inGame()) || imguiWants)
        {
            ShowCursor(true);

            if (ignorable && !allowedKeys)
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
    if (!g_imguiInit)
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
        static auto* context = ImGui::CreateContext();
        ImGui::SetCurrentContext(context);
        ImGui_ImplWin32_Init(g_hGameWindow);
        ImGui_ImplOpenGL3_Init();
        ImGui::CaptureMouseFromApp();

        g_imguiInit = true;
    }

    if (g_overlay && g_imguiInit && ImGui::GetCurrentContext())
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

struct ConsoleHandler
{
    static constexpr int DEFAULT_FOREGROUND_COLOR = 0x7;
    static constexpr int DEFAULT_BACKGROUND_COLOR = 0x0;

    ConsoleHandler()
    {
        AllocConsole();

        this->out = new FILE();
        this->err = new FILE();
        this->in = new FILE();

        freopen_s(&this->out, "CONOUT$", "w", stdout);
        freopen_s(&this->err, "CONERR$", "w", stderr);
        freopen_s(&this->in, "CONIN$", "r", stdin);

        this->hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        this->setColor();
    }

    void setColor(int fg = DEFAULT_FOREGROUND_COLOR, int bg = DEFAULT_BACKGROUND_COLOR)
    {
        SetConsoleTextAttribute(this->hConsole, fg + bg*16);
    }

    template<typename F>
    void withColor(int fg, int bg, F f)
    {
        setColor(fg + bg * 16);
        f();
        setColor();
    }

    ~ConsoleHandler()
    {
        FreeConsole();

        fclose(this->out);
        fclose(this->err);
        fclose(this->in);

        this->out = nullptr;
        this->err = nullptr;
        this->in = nullptr;
        this->hConsole = nullptr;
    }

    FILE* out = nullptr;
    FILE* err = nullptr;
    FILE* in = nullptr;
    HANDLE hConsole = nullptr;
};

class PyWriter
{
public:
    PyWriter(std::ostream& stream)
        : stream(stream)
    {}

    void write(std::string str)
    {
        stream << str;
    }

    void flush()
    {
        stream << std::flush;
    }

private:
    std::ostream& stream;
};

class PyReader
{
public:
    PyReader(std::istream& stream)
        : stream(stream)
    {}

    std::string readline()
    {
        std::string str;
        std::getline(stream, str);
        return str;
    }

private:
    std::istream& stream;
};

PYBIND11_EMBEDDED_MODULE(ftl_io_redirect, module)
{
    py::class_<PyWriter>(module, "Writer")
        .def("write", &PyWriter::write)
        .def("flush", &PyWriter::flush);

    py::class_<PyReader>(module, "Reader")
        .def("readline", &PyReader::readline);
}

const std::string PYFTL_PREFIX = "[PyFTL] ";

template<typename... Ts>
std::ostream& PyFTLOut(ConsoleHandler& con, Ts&&... args)
{
    con.setColor(0xA, 0x0);
    ((std::cout << PYFTL_PREFIX) << ... << args);
    con.setColor();

    return std::cout;
}

template<typename... Ts>
std::ostream& PyFTLErr(ConsoleHandler& con, Ts&&... args)
{
    con.setColor(0xC, 0x0);
    ((std::cout << PYFTL_PREFIX) << ... << args);
    con.setColor();

    return std::cout;
}

DWORD WINAPI patcherThread(HMODULE hModule)
{
    ConsoleHandler con;

    py::scoped_interpreter pyInterpreter{};
    py::module pyMainModule;

    try
    {
        py::module::import("ftl_io_redirect");

        py::module sys = py::module::import("sys");

        PyWriter out{ std::cout };
        PyWriter err{ std::cout };
        PyReader in{ std::cin };

        sys.attr("stdout") = out;
        sys.attr("stderr") = err;
        sys.attr("stdin") = in;

        con.setColor(0xA, 0x0);
        py::print(PYFTL_PREFIX + "Python initialized.");
        con.setColor();

        auto path = std::filesystem::current_path() / PYTFTL_FOLDER;
        sys.attr("path").attr("insert")(0, path.string());
        pyMainModule = py::module::import(PYTFTL_MAIN_MODULE);
        sys.attr("path").attr("remove")(path.string());
    }
    catch (const std::exception& e)
    {
        PyFTLErr(con, "Couldn't run python file: ", e.what(), '\n');
        std::system("pause");
        return 1;
    }

    // Game has to be running before anything else can happen
    if (!Reader::init() || !Reader::getState().running)
    {
        PyFTLOut(con, "Waiting for game to start...\n");

        while (!Reader::init());
        while (!Reader::getState().running) Reader::poll();

        PyFTLOut(con, "Game has started!\n");
    }

    bool quit = false;

    //pybind11::gil_scoped_release gilRelease;
    //std::mutex mut;
    //std::condition_variable cv;

    std::jthread pollerThread([&] {
        while (!quit)
        {
            Reader::poll();
            Reader::wait();
        }
    });

    //std::jthread pyUpdateThread([&] {
    //    TimePoint last = Clock::now();
    //
    //    while (!quit)
    //    {
    //        Duration timeEllapsed = Clock::now() - last;
    //        last = Clock::now();
    //
    //        try
    //        {
    //            pybind11::gil_scoped_acquire gilAcquire;
    //            std::unique_lock<std::mutex> lock(mut);
    //            cv.wait(lock);
    //            pyMainModule.attr("on_update")(timeEllapsed);
    //        }
    //        catch (const std::exception& e)
    //        {
    //            PyFTLErr(con, "Python exception: ", e.what(), '\n');
    //        }
    //    }
    //});

    try
    {
        //std::unique_lock<std::mutex> lock(mut);
        pybind11::gil_scoped_acquire gilAcquire;
        pyMainModule.attr("on_start")();
    }
    catch (const std::exception& e)
    {
        PyFTLErr(con, "Python exception: ", e.what(), '\n');
        PyFTLErr(con, "Since this exception happened in on_start, PyFTL will abort.\n");
        std::system("pause");
        return 1;
    }

    // Now that everything else worked, we can hook the OpenGL stuff!
    wglSwapBuffers_orig =
        reinterpret_cast<wglSwapBuffers_t>(
            GetProcAddress(GetModuleHandle(L"opengl32.dll"), "wglSwapBuffers"));

    wglSwapBuffers_orig =
        reinterpret_cast<wglSwapBuffers_t>(
            mem::trampHook32(
                reinterpret_cast<BYTE*>(wglSwapBuffers_orig),
                reinterpret_cast<BYTE*>(wglSwapBuffers_hook),
                7));

    PyFTLOut(con, "Hooked into the game's rendering loop!\n");

    // And then this thread will get blocked by the polling thread
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

