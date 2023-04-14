#include "Utility/Memory.hpp"
#include "GUI.hpp"

#include <fstream>
#include <filesystem>
#include <mutex>
#include <condition_variable>
#include <algorithm>
#include <cctype>

using wglSwapBuffers_t = BOOL(__stdcall*) (HDC hDc);
wglSwapBuffers_t wglSwapBuffers_orig = nullptr;

HWND g_hGameWindow;
WNDPROC g_hGameWindowProc;
WNDPROC g_hGameWindowProcOld;

bool g_imguiMainMenu = true;
bool g_imguiInit = false;
bool g_quit = false;
GUIHelper g_gui;

class PyWriter
{
public:
    void write(const std::string& str)
    {
        for (auto&& c : str)
        {
            if (c == '\n' || c == '\r')
            {
                this->flush();
            }
            else
            {
                this->buf.push_back(c);
            }
        }
    }

    void flush()
    {
        if (this->buf.empty()) return;
        g_gui.output(this->buf, TextOutputType::Default);
        this->buf.clear();
    }

private:
    std::string buf;
};

class PyReader
{
public:
    std::string readline()
    {
        throw std::runtime_error("cannot use stdin in your python code");
    }
};

PYBIND11_EMBEDDED_MODULE(ftl_io_redirect, module)
{
    py::class_<PyWriter>(module, "Writer")
        .def("write", &PyWriter::write)
        .def("flush", &PyWriter::flush);

    py::class_<PyReader>(module, "Reader")
        .def("readline", &PyReader::readline);
}

template<typename... Ts>
void PyFTLOut(Ts&&... args)
{
    static std::ostringstream ss;
    (ss << ... << args);
    g_gui.output(ss.str(), TextOutputType::Internal);
    ss.str("");
}

template<typename... Ts>
void PyFTLErr(Ts&&... args)
{
    static std::ostringstream ss;
    (ss << ... << args);
    g_gui.output(ss.str(), TextOutputType::InternalError);
    ss.str("");
}

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
        auto&& state = Reader::getState();

        CallWindowProc(
            reinterpret_cast<WNDPROC>(
                ImGui_ImplWin32_WndProcHandler),
            hWnd, uMsg, wParam, lParam);

        bool mouse =
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

        bool keyboard =
            uMsg == WM_KEYUP ||
            uMsg == WM_KEYDOWN ||
            false;

        if (g_gui.wantInput())
        {
            return true;
        }

        if (!Input::humanMouseAllowed() && mouse)
        {
            return true;
        }

        if (!Input::humanKeyboardAllowed() && keyboard)
        {
            return true;
        }

    }
    else
    {
        uMsg &= ~WM_FTLAI;
    }

    return CallWindowProc(g_hGameWindowProc, hWnd, uMsg, wParam, lParam);
}

mem::Hook g_glHook;
std::mutex g_readerMutex;
std::condition_variable g_readerCV;

BOOL __stdcall wglSwapBuffers_hook(HDC hDc)
{
    std::lock_guard lock(g_readerMutex);

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

        Input::setReady();

        glewInit();
        static auto* context = ImGui::CreateContext();
        ImGui::SetCurrentContext(context);
        ImGui_ImplWin32_Init(g_hGameWindow);
        ImGui_ImplOpenGL3_Init();
        ImGui::CaptureMouseFromApp();

        g_imguiInit = true;
    }

    if (g_imguiInit && ImGui::GetCurrentContext())
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        g_gui.render();
        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    try
    {
        Reader::poll();
    }
    catch (const std::exception& e)
    {
        PyFTLErr("Unhandled exception: ", e.what());
    }

    g_readerCV.notify_all();

    return reinterpret_cast<wglSwapBuffers_t>(g_glHook.original())(hDc);
}

bool hookRenderer()
{
    void* addr = GetProcAddress(GetModuleHandle(L"opengl32.dll"), "wglSwapBuffers");

    g_glHook.hook((BYTE*)addr, (BYTE*)wglSwapBuffers_hook, 7);

    PyFTLOut("Hooked into OpenGL!");
    return true;
}

bool unhookRenderer()
{
    std::lock_guard lock(g_readerMutex);
    g_glHook.unhook();
    Input::setReady(false);

    // Return the old wndProc function
    g_hGameWindowProc =
        reinterpret_cast<WNDPROC>(
            SetWindowLongPtr(
                g_hGameWindow, GWLP_WNDPROC,
                reinterpret_cast<LONG_PTR>(g_hGameWindowProcOld)));

    return true;
}

py::module initPython()
{
    py::module result;
    py::gil_scoped_acquire gil;

    try
    {
        py::module::import("ftl_io_redirect");

        py::module sys = py::module::import("sys");

        sys.attr("stdout") = PyWriter{};
        sys.attr("stderr") = PyWriter{};
        sys.attr("stdin") = PyReader{};

        PyFTLOut("Python initialized.");

        auto path = std::filesystem::current_path() / PYTFTL_FOLDER;
        sys.attr("path").attr("insert")(0, path.string());
        result = py::module::import(PYTFTL_MAIN_MODULE);
        sys.attr("path").attr("remove")(path.string());
        g_gui.setScope(result.attr("__dict__"));
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error("Couldn't run python file: " + std::string(e.what()));
    }

    // Game has to be running before anything else can happen
    if (!Reader::init() || !Reader::getState().running)
    {
        PyFTLOut("Waiting for game to start...");

        while (!Reader::init());
        while (!Reader::getState().running) Reader::poll();

        PyFTLOut("Game has started!");
    }

    try
    {
        if (py::hasattr(result, "on_start"))
        {
            result.attr("on_start")();
        }
    }
    catch (const std::exception& e)
    {
        PyFTLErr("Python exception: ", e.what());
    }

    return result;
}

void mainLoop(py::module& pyMain)
{
    static double last = Reader::now();

    try
    {
        py::gil_scoped_acquire gil;

        double now = Reader::now();
        double timeEllapsed = now - last;
        last = now;

        if (g_gui.requestedPythonCode())
        {
            g_gui.runPythonCode();
        }

        if (py::hasattr(pyMain, "on_update"))
        {
            pyMain.attr("on_update")(timeEllapsed);
        }
    }
    catch (const std::exception& e)
    {
        PyFTLErr("Python exception: ", e.what());
    }

    if (Reader::reloadRequested())
    {
        PyFTLOut("Reloading Python...");
        pyMain = initPython();
        Reader::finishReload();
    }
}

void patcherThread(std::stop_token stop)
{
    try
    {
        hookRenderer();

        py::scoped_interpreter pyInterpreter{};
        py::gil_scoped_release gil;
        py::module pyMain = initPython();

        while (!stop.stop_requested() && !g_quit)
        {
            std::unique_lock lock(g_readerMutex);
            g_readerCV.wait(lock);

            mainLoop(pyMain);
        }
    }
    catch (const std::exception& e)
    {
        g_gui.setUnrecoverable(e.what());
    }

    g_quit = true;
}

std::jthread g_patcher;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
    {
        g_patcher = std::jthread(patcherThread);
        break;
    }
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        g_quit = true;
        if (g_glHook.hooked()) unhookRenderer();
        if (g_patcher.joinable())
        {
            g_patcher.request_stop();
            g_patcher.join();
        }
        break;
    }
    return TRUE;
}

