#include "Thinker.hpp"

#include <cmath>
#include <limits>

bool Thinker::targetAlive() const
{
	return this->state.targetPresent && this->state.target.hull.first > 0;
}

bool Thinker::systemReady(const SystemState& system)
{
	return
		system.present &&
		Thinker::clampLevel(system) > 0 &&
		system.hackLevel == 0 &&
		Thinker::systemIonTime(system) <= 0.f;
}

bool Thinker::offensiveSystemReady(const SystemState& system) const
{
	return this->state.targetPresent && this->systemReady(system);
}

float Thinker::systemIonTime(const SystemState& system)
{
	return float(system.ion.first) * 5.f + system.ion.second;
}

bool Thinker::systemLocked(SystemID system) const
{
	auto* s = this->state.player.systems.get(system);
	if (!s) return true;

	bool locked = s->ion.first > 0 || s->ion.second > 0.f;

	switch (system)
	{
	case SystemID::Cloaking:
	{
		locked = locked || static_cast<const CloakingState*>(s)->on;
		break;
	}
	case SystemID::Mind:
	{
		const auto& [tCurr, tMax] = static_cast<const MindState*>(s)->timer;
		locked = locked || (tCurr > 0.f && tCurr < tMax);
	}
	case SystemID::Hacking:
	{
		const auto& [tCurr, tMax] = static_cast<const HackingState*>(s)->timer;
		locked = locked || (tCurr > 0.f && tCurr < tMax);
	}
	}

	return locked;
}

int Thinker::clampLevel(const SystemState& system, int lvl)
{
	if (lvl < 0)
		return 0;

	int l = std::min(system.level.first, system.health);
	l = std::min(l, system.power.limit);
	l = std::min(l, lvl);

	return l;
}

int Thinker::clampLevel(SystemID id, int lvl) const
{
	auto* s = this->state.player.systems.get(id);
	if (!s)
		return 0;

	return Thinker::clampLevel(*s, lvl);
}