#include <iostream>
#include <string>
#include <thread>
#include <filesystem>

// Oh no...
#include <windows.h>
#include <Psapi.h>
#include <TlHelp32.h>

constexpr wchar_t FTLExecutable[] = L"FTLGame.exe";

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

DWORD getPid()
{
    DWORD pid = 0;

    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    if (snapshot == INVALID_HANDLE_VALUE)
        return 0;

    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(snapshot, &entry))
    {
        do
        {
            if (_wcsicmp(entry.szExeFile, FTLExecutable) == 0)
            {
                pid = entry.th32ProcessID;
                break;
            }
        } while (Process32Next(snapshot, &entry));
    }

    CloseHandle(snapshot);

    return pid;
}

int main(int argc, char** argv)
{
    //if (argc != 1)
    //{
    //    std::cout << "Don't launch with any parameters!\n";
    //    return 1;
    //}

    std::cout << "Getting ID of the FTL process...\n";
    DWORD pid = getPid();

    if (!pid)
    {
        std::cout << "Couldn't find FTL! Make sure it's open.\n";
        std::system("pause");
        return 1;
    }

    std::cout << "Opening FTL process...\n";
    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);

    if (!hProc)
    {
        std::cout << "Couldn't open the process!\n";
        std::system("pause");
        return 1;
    }

    constexpr size_t DLL_COUNT = 2;
    constexpr size_t DLL_NAME_BUF = 50;

    constexpr char DLL_LIST[DLL_COUNT][DLL_NAME_BUF]
    {
        "python311.dll",
        "pyftl.dll"
    };

    auto pwd = std::filesystem::current_path();

    for (size_t i = 0; i < DLL_COUNT; i++)
    {
        auto path = pwd / DLL_LIST[i];
        std::string pathStr = path.string();

        if (!std::filesystem::exists(path))
        {
            std::cout << "Couldn't find " << pathStr << ".\n";
            std::cout << "Please ensure that the file exists and that your working directory is set correctly.\n";
            std::system("pause");
            return 1;
        }

        std::cout << "Injecting " << DLL_LIST[i] << " into ";
        std::wcout << FTLExecutable;
        std::cout << "...\n";

        bool success = inject(pathStr.c_str(), hProc);

        if (!success)
        {
            std::cout << "Couldn't inject the dll!\n";
            std::system("pause");
            return 1;
        }
    }

    std::cout << "Success! This window will close in a few seconds.\n";

    std::this_thread::sleep_for(std::chrono::seconds(3));

    CloseHandle(hProc);
}
