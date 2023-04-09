#pragma once

#include <utility>

struct Power
{
	std::pair<int, int> total{ 0, 0 };
	int required = 0;
	int normal = 0;
	int zoltan = 0;
	int battery = 0;
	int cap = 0;
	int ionLevel = 0;
	std::pair<float, float> ionTimer{ 0.f, 0.f };
	int restoreTo = 0;
};
