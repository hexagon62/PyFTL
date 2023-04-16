#pragma once

#include "Blueprint.hpp"

struct Augment : Blueprint
{
	float value = 0.f;
	int slot = -1; // slot in cargo
	bool stacking = false;
};
