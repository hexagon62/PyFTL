#pragma once

#include "../../GameState/Reader.hpp"
#include "../Input.hpp"

constexpr int IntMin = std::numeric_limits<int>::min();
constexpr int IntMax = std::numeric_limits<int>::max();

struct ThinkerParams
{
	const State& state;
	const KnownBlueprints& blueprints;
	Input& input;
};

class Thinker
{
public:
	Thinker(ThinkerParams params)
		: state(params.state)
		, blueprints(params.blueprints)
		, input(params.input)
	{}

	virtual void think() = 0;

protected:
	const State& state;
	const KnownBlueprints& blueprints;
	Input& input;

	bool targetAlive() const;

	static bool systemReady(const SystemState& system);
	bool offensiveSystemReady(const SystemState& system) const;
	static float systemIonTime(const SystemState& system);
	bool systemLocked(SystemID system) const;

	static int clampLevel(const SystemState& system, int lvl = IntMax);
	int clampLevel(SystemID system, int lvl = IntMax) const;
};
