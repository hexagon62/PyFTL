#pragma once

#include <vector>
#include <string>
#include <utility>
#include <algorithm>
#include <stdexcept>
#include <map>
#include <cstring>

// Oh no...
#include <Windows.h>
#include <Psapi.h>
#include <TlHelp32.h>

constexpr wchar_t FTLExecutable[] = L"FTLGame.exe";

struct Page
{
	std::vector<uint8_t> data;
	bool refreshing = false;
};

// This class handles all the weird Windows crap
class ProcessReader
{
public:
	ProcessReader()
	{}

	~ProcessReader()
	{
		this->closeHandle();
	}

	void hook()
	{
		if (this->handle)
			throw std::logic_error("Tried to open process when one is already open");

		PROCESSENTRY32 entry;
		entry.dwSize = sizeof(PROCESSENTRY32);

		HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

		if (Process32First(snapshot, &entry) == TRUE)
		{
			while (Process32Next(snapshot, &entry) == TRUE)
			{
				if (std::wstring(entry.szExeFile) == FTLExecutable)
				{
					this->openProcess(entry.th32ProcessID);
					break;
				}
			}
		}
	}

	void unhook()
	{
		this->closeProcess();
	}

	HWND getWindow() const
	{
		return this->window;
	}

	bool open() const
	{
		return this->handle;
	}

	bool running() const
	{
		if (!this->handle)
			return false;

		DWORD code = 0;
		GetExitCodeProcess(this->handle, &code);

		return code == STILL_ACTIVE;
	}

	bool is64Bit() const
	{
		if (!this->running())
			return false;

		USHORT pProcessMachine, pNativeMachine;
		IsWow64Process2(this->handle, &pProcessMachine, &pNativeMachine);

		bool wow64 = pProcessMachine != IMAGE_FILE_MACHINE_UNKNOWN;
		bool os64 = pNativeMachine != IMAGE_FILE_MACHINE_AMD64;

		return wow64 == os64;
	}

	uintptr_t baseAddress() const
	{
		return this->base;
	}

	// Full refresh will query the FTL process's pages again
	void refresh(bool full = false)
	{
		if (!this->handle)
			throw std::logic_error("Tried to refresh when no process is open.");

		if(full) this->getPageRegions();

		for (auto& [_, page] : this->pages)
			page.refreshing = true;
	}

	// Reads at a specific address
	template<typename T>
	bool read(T& object, uintptr_t address)
	{
		/*constexpr bool Small = sizeof(T) == 1;
		constexpr size_t Needed = Small ? 2 : sizeof(T);
		SIZE_T bytesRead = 0;
		BOOL succ = 0;

		if constexpr (Small)
		{
			char filler[2]{}; // For some reason the function refuses to read less than 2 bytes at a time
			succ = ReadProcessMemory(this->handle, reinterpret_cast<LPVOID>(address), filler, 2, &bytesRead);
			*reinterpret_cast<char*>(&object) = filler[0];
		}
		else
			succ = ReadProcessMemory(this->handle, reinterpret_cast<LPVOID>(address), &object, Needed, &bytesRead);

		int lastErr = GetLastError();

		if (lastErr != 0)
			throw std::runtime_error("Memory reading error. Code: " + std::to_string(lastErr));

		return succ && bytesRead == Needed;*/

		auto it = this->getPage(address);
		
		if (it == this->pages.end())
			return false;

		const auto& page = it->second;
		size_t relativeAddress = address-it->first;

		if (!this->inPage(address+sizeof(T), it->first, page.data.size()))
			return false;

		std::memcpy(&object, &page.data[relativeAddress], sizeof(T));

		return true;
	}

	int error()
	{
		return GetLastError();
	}

private:
	DWORD pID = 0;
	HWND window = nullptr;
	HANDLE handle = nullptr;
	UINT_PTR base = 0;

	std::map<UINT_PTR, Page> pages;
	std::vector<std::pair<UINT_PTR, UINT_PTR>> pageRegions;

	static bool inPage(UINT_PTR address, UINT_PTR base, UINT_PTR size)
	{
		return base <= address && address <= base + size;
	}

	auto findPage(UINT_PTR address)
	{
		auto it = this->pages.begin();
		for (; it != this->pages.end() && !this->inPage(address, it->first, it->second.data.size()); ++it);

		return it;
	}

	auto getPage(UINT_PTR address)
	{
		auto it = this->findPage(address);

		if (it == this->pages.end())
		{
			this->pollPages(address);
			it = this->findPage(address);
		}
		else if (it->second.refreshing)
		{
			this->pollPages(address);
		}

		return it;
	}

	// Reads all (potential) relevant memory
	void pollPages(UINT_PTR address)
	{
		MEMORY_BASIC_INFORMATION mbi{};
		unsigned char* addr = nullptr;

		for (auto&& [base, size] : this->pageRegions)
		{
			if (this->inPage(address, base, size))
			{
				if (!this->pages.count(base))
					this->pages.emplace(base, Page{ std::vector<uint8_t>(size, 0) });
				else
					this->pages.at(base).data.resize(size, 0);

				auto& page = this->pages.at(base);
				page.refreshing = false;

				SIZE_T bytesRead = 0;
				BOOL succ = ReadProcessMemory(this->handle, reinterpret_cast<LPVOID>(base), page.data.data(), size, &bytesRead);

				int lastErr = GetLastError();

				if (!succ || lastErr != 0)
					throw std::runtime_error("Memory reading error. Code: " + std::to_string(lastErr));

				if (bytesRead != size)
					throw std::runtime_error("Only read " + std::to_string(bytesRead) + " of " + std::to_string(size) + " bytes.");
			}
		}
	}

	void getPageRegions()
	{
		this->pageRegions.clear();

		MEMORY_BASIC_INFORMATION mbi{};
		unsigned char* addr = nullptr;

		while (VirtualQueryEx(this->handle, addr, &mbi, sizeof(mbi)))
		{
			auto base = reinterpret_cast<UINT_PTR>(mbi.BaseAddress);
			auto size = static_cast<size_t>(mbi.RegionSize);

			if (mbi.State == MEM_COMMIT && (mbi.Protect & (PAGE_NOACCESS | PAGE_GUARD)) == 0)
			{
				this->pageRegions.emplace_back(base, size);

				int lastErr = GetLastError();

				if (lastErr != 0)
					throw std::runtime_error("Page reading error. Code: " + std::to_string(lastErr));
			}

			addr += mbi.RegionSize;
		}

		// Happens at the end of the loop above
		if(GetLastError() == ERROR_INVALID_PARAMETER)
			SetLastError(0);
	}

	void findWindow()
	{
		for (HWND hwnd = GetTopWindow(nullptr); hwnd != nullptr; hwnd = GetNextWindow(hwnd, GW_HWNDNEXT))
		{
			if (!IsWindowVisible(hwnd))
				continue;

			DWORD pid = 0;
			GetWindowThreadProcessId(hwnd, &pid);
			if (pid == this->pID)
			{
				this->window = hwnd;
				break;
			}
		}
	}

	void openProcess(DWORD pid)
	{
		this->handle = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, pid);

		bool priv = setDebugPrivileges(true);

		if (!priv)
			this->closeHandle();
		else
		{
			if(!this->getBaseAddress())
				throw std::runtime_error("Somehow hooked into a process that isn't FTL?");

			this->pID = pid;
			this->findWindow();
			this->getPageRegions();
		}
	}

	void closeProcess()
	{
		if (!this->handle)
			throw std::logic_error("Tried to close process when one isn't open");

		this->base = 0;
		this->window = nullptr;
		this->pID = 0;
		this->closeHandle();
		this->pages.clear();
		this->pageRegions.clear();
	}

	void closeHandle()
	{
		if (this->handle)
			CloseHandle(this->handle);

		this->handle = nullptr;
	}

	bool getBaseAddress()
	{
		if (this->handle)
		{
			HMODULE hmod[1024]{};
			DWORD lpcbNeeded = 0;

			if (!EnumProcessModules(this->handle, hmod, sizeof(hmod), &lpcbNeeded))
				return false;

			TCHAR szModName[MAX_PATH];

			for (size_t i = 0; i < lpcbNeeded / sizeof(HMODULE); ++i)
			{
				if (GetModuleFileNameEx(this->handle, hmod[0], szModName, sizeof(szModName) / sizeof(TCHAR)))
				{
					std::wstring name = szModName;
					if (name.find(L"FTLGame.exe") != name.npos)
					{
						this->base = reinterpret_cast<UINT_PTR>(hmod[i]);
						return true;
					}
				}
			}
		}

		return false;
	}

	// TO DO: Determine if this is necessary
	static bool setDebugPrivileges(bool enable)
	{
		HANDLE proc = nullptr;

		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &proc))
			return false;

		TOKEN_PRIVILEGES tokenPrivileges{};
		LUID luid;

		if (!LookupPrivilegeValueA(nullptr, "SeDebugPrivilege", &luid))
			return false;

		tokenPrivileges.PrivilegeCount = 1;
		tokenPrivileges.Privileges[0].Luid = luid;
		tokenPrivileges.Privileges[0].Attributes = enable ? SE_PRIVILEGE_ENABLED : 0;

		if (!AdjustTokenPrivileges(proc, FALSE, &tokenPrivileges, sizeof(TOKEN_PRIVILEGES), nullptr, nullptr))
			return false;

		if (GetLastError() == ERROR_NOT_ALL_ASSIGNED)
			return false;

		return true;
	}
};