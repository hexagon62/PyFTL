#pragma once

enum class DroneType
{
	Invalid = -1,
	Combat, // COMBAT
	Defense, // DEFENSE
	AntiBoarder, // BATTLE
	Boarder, // BOARDER
	HullRepair, // SHIP_REPAIR
	SystemRepair, // REPAIR
	Hacking, // HACKING
	Shield // SHIELD
};
