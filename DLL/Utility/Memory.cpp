#include "Memory.hpp"

#include <cassert>

namespace mem
{

namespace
{

bool replace32(BYTE* what, const BYTE* with, uintptr_t size)
{
	DWORD oldProtect;
	VirtualProtect(what, size, PAGE_EXECUTE_READWRITE, &oldProtect);

	memcpy_s(what, size, with, size);

	VirtualProtect(what, size, oldProtect, &oldProtect);

	return true;
}

BYTE* replaceHook32(BYTE* what, const BYTE* with, uintptr_t size)
{
	constexpr DWORD flags = MEM_COMMIT | MEM_RESERVE;
	constexpr DWORD protect = PAGE_EXECUTE_READWRITE;
	BYTE* gateway = static_cast<BYTE*>(VirtualAlloc(nullptr, size, flags, protect));

	// Store stolen bytes in gateway to restore later
	// We don't add a jump since it's just unused
	memcpy_s(gateway, size, what, size);

	// Replace bytes
	replace32(what, with, size);

	return gateway;
}

bool removeReplaceHook32(BYTE* what, BYTE* gateway, uintptr_t size)
{
	return replace32(what, gateway, size);
}

bool detour32(BYTE* src, const BYTE* dest, uintptr_t size)
{
	assert(size > MIN_JUMP || "size must be big enough to hold a jmp operation");

	DWORD oldProtect;
	VirtualProtect(src, size, PAGE_EXECUTE_READWRITE, &oldProtect);

	uintptr_t relativeAddress = dest - src - MIN_JUMP;

	// Change the instructions to jump to our code
	*src = 0xE9; // opcode
	*reinterpret_cast<uintptr_t*>(src + 1) = relativeAddress; // address to jump to

	VirtualProtect(src, size, oldProtect, &oldProtect);

	return true;
}

BYTE* trampHook32(BYTE* src, const BYTE* dest, uintptr_t size)
{
	assert(size > MIN_JUMP || "size must be big enough to hold a jmp operation");

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
	assert(size > MIN_JUMP || "size must be big enough to hold a jmp operation");

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

Hook::Hook(BYTE* target, const BYTE* user, uintptr_t size, bool replace)
{
	this->hook(target, user, size, replace);
}

Hook::Hook(Hook&& other) noexcept
	: active(other.active)
	, target(other.target)
	, user(other.user)
	, gateway(other.gateway)
	, size(other.size)
{
	other.active = false;
	other.target = nullptr;
	other.user = nullptr;
	other.gateway = nullptr;
	other.size = 0;
}

Hook& Hook::operator=(Hook&& other) noexcept
{
	this->active = other.active;
	this->target = other.target;
	this->user = other.user;
	this->gateway = other.gateway;
	this->size = other.size;
	other.active = false;
	other.target = nullptr;
	other.user = nullptr;
	other.gateway = nullptr;
	other.size = 0;

	return *this;
}

Hook::~Hook()
{
	this->unhook();
}

void Hook::hook(BYTE* target, const BYTE* user, uintptr_t size, bool replace)
{
	this->unhook();

	this->target = target;
	this->user = user;

	if (replace)
	{
		this->gateway = replaceHook32(this->target, this->user, size);
		this->replace = true;
	}
	else
	{
		this->gateway = trampHook32(this->target, this->user, size);
	}

	this->active = true;
	this->size = size;
}

void Hook::unhook()
{
	if (!this->active) return;

	if (this->replace)
	{
		removeReplaceHook32(this->target, this->gateway, this->size);
		this->replace = false;
	}
	else
	{
		removeHook32(this->target, this->gateway, this->size);
	}

	this->active = false;
	this->target = nullptr;
	this->user = nullptr;
	this->gateway = nullptr;
	this->size = 0;
}

BYTE* Hook::original() const
{
	return this->gateway;
}

bool Hook::hooked() const
{
	return this->active;
}

Hook::operator bool() const
{
	return this->active;
}

bool Hook::replacing() const
{
	return this->replace;
}

}
