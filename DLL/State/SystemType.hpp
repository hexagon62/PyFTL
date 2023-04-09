#pragma once

#include <string>
#include <unordered_map>

enum class SystemType : int
{
	None = -1,
	Shields = 0,
	Engines,
	Oxygen,
	Weapons,
	Drones,
	Medbay,
	Piloting,
	Sensors,
	Doors,
	Teleporter,
	Cloaking,
	Artillery,
	Battery,
	Clonebay,
	MindControl,
	Hacking
};

inline const std::string& systemName(SystemType type)
{
	static const std::unordered_map<SystemType, std::string> map{
		{SystemType::None, "invalid"},
		{SystemType::Shields, "shields"},
		{SystemType::Engines, "engines"},
		{SystemType::Oxygen, "oxygen"},
		{SystemType::Weapons, "weapons"},
		{SystemType::Drones, "drones"},
		{SystemType::Medbay, "medbay"},
		{SystemType::Piloting, "piloting"},
		{SystemType::Sensors, "sensors"},
		{SystemType::Doors, "doors"},
		{SystemType::Teleporter, "teleporter"},
		{SystemType::Cloaking, "cloaking"},
		{SystemType::Artillery, "artillery"},
		{SystemType::Battery, "battery"},
		{SystemType::Clonebay, "clonebay"},
		{SystemType::MindControl, "mind control"},
		{SystemType::Hacking, "hacking"},
	};

	auto it = map.find(type);
	if (it == map.cend()) return map.at(SystemType::None);
	return it->second;
}

inline const std::string& systemPowerHotkey(SystemType type)
{
	static const std::unordered_map<SystemType, std::string> map{
		{SystemType::None, ""},
		{SystemType::Shields, "shields"},
		{SystemType::Engines, "engines"},
		{SystemType::Oxygen, "oxygen"},
		{SystemType::Weapons, "weapons"},
		{SystemType::Drones, "drones"},
		{SystemType::Medbay, "medbay"},
		{SystemType::Piloting, ""},
		{SystemType::Sensors, ""},
		{SystemType::Doors, ""},
		{SystemType::Teleporter, "teleporter"},
		{SystemType::Cloaking, "cloaking"},
		{SystemType::Artillery, "artillery"},
		{SystemType::Battery, ""},
		{SystemType::Clonebay, "medbay"},
		{SystemType::MindControl, "mind"},
		{SystemType::Hacking, "hacking"},
	};

	auto it = map.find(type);
	if (it == map.cend()) return map.at(SystemType::None);
	return it->second;
}

inline const std::string& systemUnpowerHotkey(SystemType type)
{
	static const std::unordered_map<SystemType, std::string> map{
		{SystemType::None, ""},
		{SystemType::Shields, "un_shields"},
		{SystemType::Engines, "un_engines"},
		{SystemType::Oxygen, "un_oxygen"},
		{SystemType::Weapons, "un_weapons"},
		{SystemType::Drones, "un_drones"},
		{SystemType::Medbay, "un_medbay"},
		{SystemType::Piloting, ""},
		{SystemType::Sensors, ""},
		{SystemType::Doors, ""},
		{SystemType::Teleporter, "un_teleporter"},
		{SystemType::Cloaking, "un_cloaking"},
		{SystemType::Artillery, "un_artillery"},
		{SystemType::Battery, ""},
		{SystemType::Clonebay, "un_medbay"},
		{SystemType::MindControl, "un_mind"},
		{SystemType::Hacking, "un_hacking"},
	};

	auto it = map.find(type);
	if (it == map.cend()) return map.at(SystemType::None);
	return it->second;
}
