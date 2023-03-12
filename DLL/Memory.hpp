#pragma once

#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

namespace mem
{

// Minimum jump size is 5 bytes
constexpr uintptr_t MIN_JUMP = 5;

template<typename T = uintptr_t>
T& get(uintptr_t addr)
{
	return *reinterpret_cast<T*>(addr);
}

void patch(BYTE* src, BYTE* dest, size_t size);
void patchEx(BYTE* src, BYTE* dest, size_t size, HANDLE hProcess);
void nop(BYTE* dest, size_t size);
void nopEx(BYTE* dest, size_t size, HANDLE hProcess);
uintptr_t getFinalAddress(uintptr_t ptr, const std::vector<size_t>& offsets);

bool detour32(BYTE* src, BYTE* dest, uintptr_t size);
BYTE* trampHook32(BYTE* src, BYTE* dest, uintptr_t size);

}
