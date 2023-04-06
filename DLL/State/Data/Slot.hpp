#pragma once

#include "Rect.hpp"
#include "Point.hpp"
#include "Crew.hpp"
#include "Repairable.hpp"

#include <vector>
#include <optional>

struct Slot
{
	int id = -1;
	Rect<int> rect;
	Point<int> position;
	bool occupiable = true;

	std::vector<Crew*> crewMoving;
	Crew* crew = nullptr;
	Crew* intruder = nullptr;
	std::optional<Fire> fire;
	std::optional<Breach> breach;
};
