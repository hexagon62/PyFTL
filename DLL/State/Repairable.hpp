#pragma once

#include "Point.hpp"

struct Repairable
{
	float repairProgress = 0.f;
	Point<int> position;
	int room = -1, slot = -1;
};

struct Fire : Repairable
{
	float deathTimer = 0.f;
};

struct Breach : Repairable
{

};
