#pragma once

#include "SystemType.hpp"
#include "SystemBlueprint.hpp"
#include "Power.hpp"
#include "HackLevel.hpp"

#include <array>
#include <utility>
#include <algorithm>
#include <stdexcept>

struct System
{
	int uiBox = -1, discriminator = 0;
	SystemType type = SystemType::None;
	SystemBlueprint blueprint;
	int room = -1;
	Power power;
	std::pair<int, int> health{ 0, 0 }, level{ 0, 0 };
	int manningLevel = 0;
	HackLevel hackLevel = HackLevel::None;
	bool player = false;
	bool needsManning = false;
	bool occupied = false;
	bool onFire = false;
	bool breached = false;
	bool boardersAttacking = false;
	float damageProgress = 0.f;
	float repairProgress = 0.f;

	virtual ~System() = default;

	bool subsystem() const
	{
		return
			this->type == SystemType::Piloting ||
			this->type == SystemType::Doors ||
			this->type == SystemType::Sensors ||
			this->type == SystemType::Battery;
	}

	// Returns what the power can currently be changed to
	std::pair<int, int> powerRange() const
	{
		// If subsystem, or locked, can't be changed from what it currently is
		if (this->subsystem() || this->hackLevel == HackLevel::Active || this->power.ionLevel > 0)
			return { this->power.total.first, this->power.total.first };

		int mul = std::max(1, this->power.required);
		int cap = (this->power.total.second / mul) * mul;

		// cap from events/environment is already stored in total.second
		return { std::min(this->power.zoltan, cap), cap };
	}

	virtual bool operable() const
	{
		return
			this->power.total.first >= this->power.required &&
			this->hackLevel != HackLevel::Active;
	}
};

class SystemNotInstalled final : public std::out_of_range
{
public:
	SystemNotInstalled(SystemType type)
		: std::out_of_range(
			"tried to use the " + systemName(type) +
			" system, which the ship doesn't have"
		)
	{}

	SystemNotInstalled(SystemType type, int which, int cap)
		: std::out_of_range(
			"tried to use " + systemName(type) + " #" +
			std::to_string(which) +
			" when the ship only has " + std::to_string(cap) + " such systems"
		)
	{}
};

template<typename T>
inline bool hasSystem(const T& obj, SystemType type, int which = 0)
{
	if (type != SystemType::Artillery && which != 0)
	{
		return false;
	}

	switch (type)
	{
	case SystemType::Shields: return obj.shields.has_value();
	case SystemType::Engines: return obj.engines.has_value();
	case SystemType::Oxygen: return obj.oxygen.has_value();
	case SystemType::Weapons: return obj.weapons.has_value();
	case SystemType::Drones: return obj.drones.has_value();
	case SystemType::Medbay: return obj.medbay.has_value();
	case SystemType::Piloting: return obj.piloting.has_value();
	case SystemType::Sensors: return obj.sensors.has_value();
	case SystemType::Doors: return obj.doorControl.has_value();
	case SystemType::Teleporter: return obj.teleporter.has_value();
	case SystemType::Cloaking: return obj.cloaking.has_value();
	case SystemType::Artillery: return size_t(which) < obj.artillery.size();
	case SystemType::Battery: return obj.battery.has_value();
	case SystemType::Clonebay: return obj.clonebay.has_value();
	case SystemType::MindControl: return obj.mindControl.has_value();
	case SystemType::Hacking: return obj.hacking.has_value();
	}

	return false;
}


template<typename Ret, typename T>
inline Ret getSystem(const T& obj, SystemType type, int which = 0)
{
	if (!hasSystem(obj, type, which))
	{
		if (type == SystemType::Artillery || which > 0)
		{
			throw SystemNotInstalled(type, which + 1, int(hasSystem(obj, type)));
		}
	}
	else
	{
		switch (type)
		{
		case SystemType::Shields: return *obj.shields;
		case SystemType::Engines: return *obj.engines;
		case SystemType::Oxygen: return *obj.oxygen;
		case SystemType::Weapons: return *obj.weapons;
		case SystemType::Drones: return *obj.drones;
		case SystemType::Medbay: return *obj.medbay;
		case SystemType::Piloting: return *obj.piloting;
		case SystemType::Sensors: return *obj.sensors;
		case SystemType::Doors: return *obj.doorControl;
		case SystemType::Teleporter: return *obj.teleporter;
		case SystemType::Cloaking: return *obj.cloaking;
		case SystemType::Artillery: return obj.artillery.at(which);
		case SystemType::Battery: return *obj.battery;
		case SystemType::Clonebay: return *obj.clonebay;
		case SystemType::MindControl: return *obj.mindControl;
		case SystemType::Hacking: return *obj.hacking;
		}
	}

	throw SystemNotInstalled(type);
}
