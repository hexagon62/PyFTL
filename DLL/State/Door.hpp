#pragma once

#include "HackLevel.hpp"
#include "Point.hpp"

#include <array>
#include <utility>

struct Door
{
	// 0 - normal doors
	// 1 - blast doors
	// 2 - super doors
	static constexpr std::array<std::array<int, 3>, 3> HARDCODED_HEALTH{ {
		{ 12, 16, 20 }, // easy
		{ 8, 12, 18 },  // normal
		{ 6, 10, 15 }   // hard
	} };

	int id = -1;
	std::pair<int, int> rooms{ -1, -1 };
	int level = 0;
	std::pair<int, int> health{ 0, 0 };
	HackLevel hackLevel = HackLevel::None;
	bool open = false, openFake = false;
	bool ioned = false;
	bool vertical = false;
	bool airlock = false;
	bool player = false;

	Rect<int> rect;
};
