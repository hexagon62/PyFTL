#include "Memory.hpp"
#include "GUI/GUI.hpp"
#include "State/Reader.hpp"
#include "Python/Bind.hpp"

#define GLEW_STATIC
#include <gl/glew.h>

#include <iostream>
#include <fstream>
#include <filesystem>

using wglSwapBuffers_t = BOOL(__stdcall*) (HDC hDc);
wglSwapBuffers_t wglSwapBuffers_orig = nullptr;

HWND g_hGameWindow;
WNDPROC g_hGameWindowProc;
WNDPROC g_hGameWindowProcOld;

struct ConsoleHandler
{
    static constexpr int DEFAULT_FOREGROUND_COLOR = 0x7;
    static constexpr int DEFAULT_BACKGROUND_COLOR = 0x0;

    ConsoleHandler()
    {
        init();
    }

    ~ConsoleHandler()
    {
        release();
    }

    operator bool()
    {
        return this->out && this->err && this->in && this->hConsole;
    }

    void init()
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

    void release()
    {
        if (!*this) return;

        FreeConsole();

        fclose(this->out);
        fclose(this->err);
        fclose(this->in);

        this->out = nullptr;
        this->err = nullptr;
        this->in = nullptr;
        this->hConsole = nullptr;
    }

    void setColor(int fg = DEFAULT_FOREGROUND_COLOR, int bg = DEFAULT_BACKGROUND_COLOR)
    {
        SetConsoleTextAttribute(this->hConsole, fg + bg * 16);
    }

    template<typename F>
    void withColor(int fg, int bg, F f)
    {
        setColor(fg + bg * 16);
        f();
        setColor();
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
    std::cout << std::flush;

    return std::cout;
}

template<typename... Ts>
std::ostream& PyFTLErr(ConsoleHandler& con, Ts&&... args)
{
    con.setColor(0xC, 0x0);
    ((std::cout << PYFTL_PREFIX) << ... << args);
    con.setColor();
    std::cout << std::flush;

    return std::cout;
}

void unrecoverable()
{
    throw std::runtime_error("see above; PyFTL will be quitting now.\n");
}

bool g_overlay = true;
bool g_active = false;
bool g_pause = false;
bool g_imguiInit = false;
bool g_quit = false, g_done = false;
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

        // Save old wndProc function
        g_hGameWindowProcOld =
            reinterpret_cast<WNDPROC>(GetWindowLongPtr(g_hGameWindow, GWLP_WNDPROC));

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

struct GLHookHandle
{
    bool hooked = false;
    wglSwapBuffers_t orig = nullptr;
    wglSwapBuffers_t gateway = nullptr;
    uintptr_t size = 0;
};

GLHookHandle g_glHookHandle;

bool hookRenderer(ConsoleHandler& con)
{
    g_glHookHandle.size = 7;

    g_glHookHandle.orig = wglSwapBuffers_orig =
        reinterpret_cast<wglSwapBuffers_t>(
            GetProcAddress(GetModuleHandle(L"opengl32.dll"), "wglSwapBuffers"));

    g_glHookHandle.gateway = wglSwapBuffers_orig =
        reinterpret_cast<wglSwapBuffers_t>(
            mem::trampHook32(
                reinterpret_cast<BYTE*>(wglSwapBuffers_orig),
                reinterpret_cast<BYTE*>(wglSwapBuffers_hook),
                g_glHookHandle.size));

    g_glHookHandle.hooked = true;
    PyFTLOut(con, "Hooked into OpenGL!\n");
    return true;
}

bool unhookRenderer()
{
    mem::removeHook32(
        reinterpret_cast<BYTE*>(g_glHookHandle.orig),
        reinterpret_cast<BYTE*>(g_glHookHandle.gateway),
        g_glHookHandle.size);

    wglSwapBuffers_orig = g_glHookHandle.orig;
    g_glHookHandle.orig = nullptr;
    g_glHookHandle.gateway = nullptr;
    g_glHookHandle.size = 0;

    // Return the old wndProc function
    g_hGameWindowProc =
        reinterpret_cast<WNDPROC>(
            SetWindowLongPtr(
                g_hGameWindow, GWLP_WNDPROC,
                reinterpret_cast<LONG_PTR>(g_hGameWindowProcOld)));

    g_glHookHandle.hooked = false;
    return true;
}

py::module initPython(ConsoleHandler& con)
{
    py::module result;

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

        con.withColor(0xA, 0x0, [&] {
            py::print(PYFTL_PREFIX + "Python initialized.");
        });

        auto path = std::filesystem::current_path() / PYTFTL_FOLDER;
        sys.attr("path").attr("insert")(0, path.string());
        result = py::module::import(PYTFTL_MAIN_MODULE);
        sys.attr("path").attr("remove")(path.string());
    }
    catch (const std::exception& e)
    {
        PyFTLErr(con, "Couldn't run python file: ", e.what(), '\n');
        unrecoverable();
    }

    // Game has to be running before anything else can happen
    if (!Reader::init() || !Reader::getState().running)
    {
        PyFTLOut(con, "Waiting for game to start...\n");

        while (!Reader::init());
        while (!Reader::getState().running) Reader::poll();

        PyFTLOut(con, "Game has started!\n");
    }

    try
    {
        result.attr("on_start")();
    }
    catch (const std::exception& e)
    {
        PyFTLErr(con, "Python exception: ", e.what(), '\n');
        PyFTLErr(con, "Since this exception happened in on_start, PyFTL will abort.\n");
        unrecoverable();
    }

    return result;
}

bool mainLoop(ConsoleHandler& con, py::module& pyMain)
{
    TimePoint last = Clock::now();

    if (!Reader::usingSeperateThread())
    {
        Reader::wait();
        Reader::poll();

        Duration timeEllapsed = Clock::now() - last;
        last = Clock::now();

        try
        {
            pyMain.attr("on_update")(timeEllapsed);
        }
        catch (const std::exception& e)
        {
            PyFTLErr(con, "Python exception: ", e.what(), '\n');
        }
    }

    if (Reader::reloadRequested())
    {
        PyFTLOut(con, "Reloading Python...\n");
        pyMain = initPython(con);
        Reader::finishReload();
    }

    return true;
}

DWORD WINAPI patcherThread(HMODULE hModule)
{
    ConsoleHandler con;

    try
    {
        hookRenderer(con);

        py::scoped_interpreter pyInterpreter{};
        py::module pyMain = initPython(con);

        while (!g_quit && mainLoop(con, pyMain));
    }
    catch (const std::exception& e)
    {
        PyFTLErr(con, "Unrecoverable error: ", e.what(), "\n");
        std::system("pause");
        g_done = true;
        return 1;
    }

    PyFTLOut(con, "Quitting. Goodbye!");
    g_done = true;
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
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        g_quit = true;
        if (g_glHookHandle.hooked) unhookRenderer();
        while (!g_done && Reader::getState().running); // wait for everything to finish
        break;
    }
    return TRUE;
}

