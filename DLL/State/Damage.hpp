#pragma once

struct Damage
{
	static constexpr int HARDCODED_CREW_DAMAGE_FACTOR = 15;

	int normal = 0;
	int ion = 0;
	int system = 0;
	int crew = 0;
	int fireChance = 0;
	int breachChance = 0;
	int stunChance = 0;
	int pierce = 0;
	int stunTime = 0;
	bool hullBonus = false;
	bool friendlyFire = false;
};
