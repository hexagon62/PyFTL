#pragma once

#include "WindowsButWithoutAsMuchCancer.hpp"

namespace mem
{

// Minimum jump size is 5 bytes
constexpr uintptr_t MIN_JUMP = 5;

template<typename T = uintptr_t>
T& get(uintptr_t addr)
{
	return *reinterpret_cast<T*>(addr);
}

// A handle for a hook
class Hook
{
public:
	// Constructs the hook in an inactive state
	Hook() = default;

	// Construct a hook and activate it
	Hook(BYTE* target, const BYTE* user, uintptr_t size = MIN_JUMP, bool replace = false);

	// Move a hook object into another one
	Hook(Hook&& other) noexcept;
	Hook& operator=(Hook&& other) noexcept;

	// Hooks cannot be copied; they have unique ownership
	Hook(const Hook&) = delete;
	Hook& operator=(const Hook&) = delete;

	// Unhooks and destroys the object
	~Hook();

	// Activates the hook
	// target is the code to be hooked into
	// user is a pointer to the user's code
	// size is the amount of bytes to write; must be >= MIN_JUMP if not replacing
	// if replace is true, then we directly replace the opcodes of the target
	// If already hooked into something, it will unhook first
	// unless replacing, user should point to executable memory
	void hook(BYTE* target, const BYTE* user, uintptr_t size = MIN_JUMP, bool replace = false);

	// Deactivates the hook
	// Does nothing if not already active
	void unhook();

	// Pointer to the original function stuff
	// which is reallocated while hooked
	BYTE* original() const;

	// Check if the hook is active
	bool hooked() const;
	operator bool() const;

	// Check if this is a replacing hook
	bool replacing() const;

private:
	bool active = false;
	bool replace = false;
	BYTE* target = nullptr;
	const BYTE* user = nullptr;
	BYTE* gateway = nullptr;
	uintptr_t size = 0;
};

}
