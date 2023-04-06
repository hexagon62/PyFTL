#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <filesystem>

// Oh no...
#include <windows.h>
#include <Psapi.h>
#include <TlHelp32.h>

constexpr wchar_t FTLExecutable[] = L"FTLGame.exe";

struct DLL
{
    wchar_t name[128] = L"\0";
    bool reload = true;
};

constexpr std::array DLL_LIST{
    DLL{ L"python311.dll" },
    DLL{ L"pyftl.dll" }
};

bool inject(const char* dll, HANDLE hProc)
{
    void* injectDLLPathRemote = VirtualAllocEx(hProc, 0x00, MAX_PATH, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

    if (!injectDLLPathRemote)
        return false;

    if (!WriteProcessMemory(hProc, injectDLLPathRemote, dll, strlen(dll) + 1, nullptr))
        return false;

    HANDLE hRemoteThread = CreateRemoteThread(
        hProc, nullptr, 0,
        (LPTHREAD_START_ROUTINE)LoadLibraryA,
        injectDLLPathRemote,
        0, nullptr);

    if (hRemoteThread && hRemoteThread != INVALID_HANDLE_VALUE)
        CloseHandle(hRemoteThread);
    else
        return false;

    return true;
}

bool unload(const HMODULE& dll, HANDLE hProc)
{
    HANDLE hRemoteThread = CreateRemoteThread(
        hProc, nullptr, 0,
        (LPTHREAD_START_ROUTINE)FreeLibrary,
        dll,
        0, nullptr);

    if (hRemoteThread && hRemoteThread != INVALID_HANDLE_VALUE)
        CloseHandle(hRemoteThread);
    else
        return false;

    return true;
}

struct ProcessInfo
{
    ProcessInfo()
    {
        std::fill(this->injected.begin(), this->injected.end(), (HMODULE)nullptr);
    }

    DWORD pid = 0;
    std::array<HMODULE, DLL_LIST.size()> injected;
};

ProcessInfo getProcessInfo()
{
    ProcessInfo info;

    // Process part
    {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

        if (snapshot == INVALID_HANDLE_VALUE) return {};

        PROCESSENTRY32 entry;
        entry.dwSize = sizeof(PROCESSENTRY32);

        if (Process32First(snapshot, &entry))
        {
            do
            {
                // Found process
                if (_wcsicmp(entry.szExeFile, FTLExecutable) == 0)
                {
                    info.pid = entry.th32ProcessID;
                    break;
                }

            } while (Process32Next(snapshot, &entry));
        }

        CloseHandle(snapshot);
    }

    // Modules part
    if (info.pid > 0)
    {
        HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, info.pid);

        if (snapshot == INVALID_HANDLE_VALUE) return {};

        MODULEENTRY32 entry;
        entry.dwSize = sizeof(MODULEENTRY32);
        if (Module32First(snapshot, &entry))
        {
            do
            {
                for (size_t i = 0; i < DLL_LIST.size(); i++)
                {
                    auto&& dll = DLL_LIST[i];

                    // Found dll
                    if (_wcsicmp(entry.szModule, dll.name) == 0)
                    {
                        info.injected[i] = entry.hModule;
                    }
                }

            } while (Module32Next(snapshot, &entry));
        }

        CloseHandle(snapshot);
    }

    return info;
}

int main(int argc, char** argv)
{
    std::vector<std::string> args;

    for (int i = 0; i < argc; i++)
        args.push_back(argv[i]);

    bool detach = false;
    bool pause = false;

    for (auto&& arg : args)
    {
        if (arg == "?" || arg == "help")
        {
            std::cout << "Specify -d or -detach to detach all dlls." << std::endl;
            std::cout << "Specify -p or -pause to wait for a keypress before terminating." << std::endl;
            std::cout << "Specify ? or help to print this message and quit." << std::endl;
            return 0;
        }

        if (arg == "-d" || arg == "-detach") detach = true;
        if (arg == "-p" || arg == "-pause") pause = true;
    }

    std::cout << "Getting ID of the FTL process..." << std::endl;
    ProcessInfo info = getProcessInfo();

    if (!info.pid)
    {
        std::cout << "Couldn't find FTL! Make sure it's open." << std::endl;
        if (pause) std::system("pause");
        return 1;
    }

    std::cout << "Opening FTL process..." << std::endl;
    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, info.pid);

    if (!hProc)
    {
        std::cout << "Couldn't open the process!" << std::endl;
        if (pause) std::system("pause");
        return 1;
    }

    auto pwd = std::filesystem::current_path();
    bool workDone = false;

    // Detach DLLs
    for (int i = DLL_LIST.size()-1; i >= 0; i--)
    {
        auto& dll = DLL_LIST[i];
        if (info.injected[i])
        {
            if (dll.reload || detach)
            {
                std::wcout << L"Unloading " << dll.name << L" from ";
                std::wcout << FTLExecutable;
                std::cout << "..." << std::endl;

                bool success = unload(info.injected[i], hProc);

                if (!success)
                {
                    std::cout << "Couldn't unload the dll!" << std::endl;
                    if (pause) std::system("pause");
                    return 1;
                }

                workDone = true;
            }
            else
            {
                std::wcout << L"Skipping " << dll.name << L", it's loaded and doesn't need reloading." << std::endl;
                continue;
            }
        }
    }

    // Attach DLLs
    if (!detach)
    {
        for (size_t i = 0; i < DLL_LIST.size(); i++)
        {
            auto& dll = DLL_LIST[i];

            auto path = pwd / dll.name;
            std::string pathStr = path.string();

            if (!std::filesystem::exists(path))
            {
                std::cout << "Couldn't find " << pathStr << "." << std::endl;
                std::cout << "Please ensure that the file exists and that your working directory is set correctly." << std::endl;
                if (pause) std::system("pause");
                return 1;
            }

            std::wcout << L"Injecting " << dll.name << L" into ";
            std::wcout << FTLExecutable;
            std::cout << "..." << std::endl;

            bool success = inject(pathStr.c_str(), hProc);

            if (!success)
            {
                std::cout << "Couldn't inject the dll!" << std::endl;
                if (pause) std::system("pause");
                return 1;
            }

            workDone = true;
        }
    }

    CloseHandle(hProc);

    if (pause)
    {
        if (workDone) std::cout << "Success! ";
        else std::cout << "Nothing to be done. No DLLs need loading/unloading." << std::endl;
        std::cout << "The launcher will terminate in a few seconds." << std::endl;
        std::system("pause");
    }
    else
    {
        if (workDone) std::cout << "Success!" << std::endl;
        else std::cout << "Nothing to be done. No DLLs need loading/unloading." << std::endl;
    }
}
