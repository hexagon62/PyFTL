#include <iostream>
#include <string>

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
    if (argc != 2)
    {
        std::cout << "Usage: " << std::string(argv[0]) << " <dll>\n";
        return 1;
    }

    DWORD pid = getPid();

    std::cout << "Injecting " << std::string(argv[1]) << " into ";
    std::wcout << FTLExecutable;
    std::cout << "...\n";

    HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pid);
    bool success = inject(argv[1], hProc);
    std::cout << (success ? "Success" : "Failed") << '\n';

    CloseHandle(hProc);
}
