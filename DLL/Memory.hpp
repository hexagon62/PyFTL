#pragma once

#include "Utility/WindowsButWithoutAsMuchCancer.hpp"

namespace mem
{

// Minimum jump size is 5 bytes
constexpr uintptr_t MIN_JUMP = 5;

template<typename T = uintptr_t>
T& get(uintptr_t addr)
{
	return *reinterpret_cast<T*>(addr);
}

bool detour32(BYTE* src, BYTE* dest, uintptr_t size);
BYTE* trampHook32(BYTE* src, BYTE* dest, uintptr_t size);
bool removeHook32(BYTE* src, BYTE* gateway, uintptr_t size);

}
