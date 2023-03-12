#pragma once

#include "Thinker.hpp"

#include <set>
#include <array>
#include <functional>

namespace Priority
{

constexpr int Maximum = IntMax;
constexpr int High = 500;
constexpr int MediumHigh = 400;
constexpr int Medium = 300;
constexpr int MediumLow = 200;
constexpr int Low = 100;
constexpr int LeastConcern = 0;

}

struct PowerPriorityRange
{
	int min = IntMin;
	int max = IntMax;
};

class RootThinker;

class Reactor : public Thinker
{
public:
	Reactor(ThinkerParams params);

	void think() override;

	int add(SystemID system, int amount = 1, int priority = Priority::Medium);
	int remove(SystemID system, int amount, int priority);
	int removeGroup(SystemID system, int amount = IntMax, PowerPriorityRange range = {});

	// Sets the amount of bars in a given priority class
	int set(SystemID system, int amount, int priority = Priority::Medium);

	// Removes all bars in the given priority range and replaces them with a group of the given amount and priority
	int setGroup(SystemID system, int amount, int priority = Priority::Medium, PowerPriorityRange removal = {});

	int toggleWeapon(WeaponState& weapon, bool on = true, int priority = Priority::Medium);
	int toggleDrone(DroneState& drone, bool on = true, int priority = Priority::Medium);

	int systemDesire(SystemID system, PowerPriorityRange range = {}) const;
	int systemDesire(SystemID system, int priority) const;
	int allocatedToSystem(SystemID system) const;
	int available() const;
	int usable() const;
	bool batteryReady() const;

private:
	struct PowerDesire
	{
		SystemID system = SystemID::Invalid;
		int amount = 1, priority = Priority::Medium;
		int multiple = 1; // the allocated power must be a multiple of this

		bool operator<(const PowerDesire& o) const
		{
			if (this->priority == o.priority)
				return this->amount > o.amount;

			return this->priority > o.priority;
		}
	};

	std::multiset<PowerDesire> desires;
	std::array<int, SystemTypes> allocated;

	void maintainPower();
};
