#pragma once

#include "SystemType.hpp"
#include "Rect.hpp"
#include "Point.hpp"
#include "HackLevel.hpp"
#include "Slot.hpp"

#include <vector>

struct Room
{
	static constexpr int HARDCODED_TILE_SIZE = 35;

	SystemType system = SystemType::None;
	int id = -1;
	int primarySlot = 0, primaryDirection = 0;

	Rect<int> rect;
	Point<int> tiles;

	bool player = false;
	bool stunning = false;
	float oxygen = 0.f;
	HackLevel hackLevel = HackLevel::None;

	std::vector<Crew*> crewMoving;
	std::vector<Crew*> crew;
	std::vector<Crew*> intruders;
	float fireRepair = 0, breachRepair = 0;
	int slotsOccupiable = 0;
	std::vector<Slot> slots;

	Slot& slotAt(const Point<int>& position)
	{
		return this->slots[this->slotIdAt(position)];
	}

	const Slot& slotAt(const Point<int>& position) const
	{
		return this->slots[this->slotIdAt(position)];
	}

	int slotIdAt(const Point<int>& position) const
	{
		Point<int> relPos = position;
		relPos.x -= rect.x;
		relPos.y -= rect.y;
		relPos /= HARDCODED_TILE_SIZE;

		return relPos.x + this->tiles.x * relPos.y;
	}
};
