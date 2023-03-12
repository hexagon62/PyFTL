#pragma once

#include "State.hpp"

#include <iostream>

inline std::ostream& operator<<(std::ostream& o, const SystemID& sys)
{
	switch (sys)
	{
	case SystemID::Invalid: o << "invalid"; break;
	case SystemID::Shields: o << "shields"; break;
	case SystemID::Engines: o << "engines"; break;
	case SystemID::Oxygen: o << "oxygen"; break;
	case SystemID::Weapons: o << "weapons"; break;
	case SystemID::Drones: o << "drones"; break;
	case SystemID::Medbay: o << "medbay"; break;
	case SystemID::Pilot: o << "piloting"; break;
	case SystemID::Sensors: o << "sensors"; break;
	case SystemID::Doors: o << "doors"; break;
	case SystemID::Teleporter: o << "teleporter"; break;
	case SystemID::Cloaking: o << "cloaking"; break;
	case SystemID::Artillery: o << "artillery"; break;
	case SystemID::Battery: o << "battery"; break;
	case SystemID::Clonebay: o << "clonebay"; break;
	case SystemID::Mind: o << "mind"; break;
	case SystemID::Hacking: o << "hacking"; break;
	}

	return o;
}
