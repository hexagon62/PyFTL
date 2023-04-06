#include "Memory.hpp"

#include "Utility/WindowsButWithoutAsMuchCancer.hpp"

namespace mem
{

bool detour32(BYTE* src, BYTE* dest, uintptr_t size)
{
	// Can't jump if below minimum
	if (size < MIN_JUMP)
		return false;

	DWORD oldProtect;
	VirtualProtect(src, size, PAGE_EXECUTE_READWRITE, &oldProtect);

	uintptr_t relativeAddress = dest - src - MIN_JUMP;

	// Change the instructions to jump to our code
	*src = 0xE9; // opcode
	*reinterpret_cast<uintptr_t*>(src + 1) = relativeAddress; // address to jump to

	VirtualProtect(src, size, oldProtect, &oldProtect);

	return true;
}

BYTE* trampHook32(BYTE* src, BYTE* dest, uintptr_t size)
{
	if (size < MIN_JUMP) return nullptr;

	// Create gateway
	constexpr DWORD flags = MEM_COMMIT | MEM_RESERVE;
	constexpr DWORD protect = PAGE_EXECUTE_READWRITE;
	BYTE* gateway = static_cast<BYTE*>(VirtualAlloc(nullptr, size + MIN_JUMP, flags, protect));

	// Write stolen bytes to gateway
	memcpy_s(gateway, size, src, size);

	// Get gateway to the destination address
	uintptr_t gatewayRelativeAddress = src - gateway - MIN_JUMP;

	// Add jump to end of the gateway
	*(gateway + size) = 0xE9;

	// Write address of gateway to jump
	*reinterpret_cast<uintptr_t*>(reinterpret_cast<uintptr_t>(gateway) + size + 1) = gatewayRelativeAddress;

	// Perform detour
	detour32(src, dest, size);

	return gateway;
}

bool removeHook32(BYTE* src, BYTE* gateway, uintptr_t size)
{
	// Size must be below minimum for jump instruction
	if (size < MIN_JUMP)
		return false;

	DWORD oldProtect;
	VirtualProtect(src, size, PAGE_EXECUTE_READWRITE, &oldProtect);

	// Write stolen bytes back to source
	memcpy_s(src, size, gateway, size);

	VirtualProtect(src, size, oldProtect, &oldProtect);

	// Free memory at gateway
	bool freed = bool(VirtualFree(gateway, 0, MEM_RELEASE));

	return true;
}

}
