#include "Reactor.hpp"

#include <cmath>

Reactor::Reactor(ThinkerParams params)
	: Thinker(params)
	, allocated() // Even though I zero this out, VS complains about initialization
				  // Yet, it's more than happy to accept this even though it's UB!
{
	for (auto& a : this->allocated)
		a = 0;
}

void Reactor::think()
{
	this->maintainPower();
}

int Reactor::removeGroup(SystemID system, int amount, PowerPriorityRange range)
{
	if (amount <= 0)
		return 0;

	auto* s = this->state.player.systems.get(system);
	if (!s)
		return 0;

	int orig = amount;

	// Go in reverse order to remove the lowest priority desires first
	for (auto desire = this->desires.rbegin(); desire != this->desires.rend();)
	{
		if (amount == 0)
			break;

		bool del = false;

		if (desire->system != system)
		{
			++desire;
			continue;
		}

		if (desire->priority >= range.min && desire->priority <= range.max)
		{
			int d = std::min(amount, desire->amount);
			amount -= d;

			int r = desire->amount - d;
			auto it = desire;
			if (r > 0)
			{
				it = std::make_reverse_iterator(this->desires.insert({
					.system = desire->system,
					.amount = r,
					.priority = desire->priority
				}));
			}

			desire++;
			auto eraseIt = this->desires.erase(desire.base());
			desire = std::make_reverse_iterator(eraseIt);
			if (r > 0) desire = it;

			del = true;
		}

		if (!del) ++desire;
	}

	return orig - amount;
}

int Reactor::remove(SystemID system, int amount, int priority)
{
	return this->removeGroup(system, amount, { .min = priority, .max = priority });
}

int Reactor::add(SystemID system, int amount, int priority)
{
	if (amount <= 0)
		return 0;

	auto* s = this->state.player.systems.get(system);
	if (!s)
		return 0;

	int max = s->level.second;
	amount = std::min(amount, max);

	int des = this->systemDesire(system);
	int diff = des + amount - max;
	if (diff > 0)
		this->removeGroup(system, diff, { .max = priority - 1 });

	this->desires.insert({
		.system = system,
		.amount = amount,
		.priority = priority
	});

	return amount;
}

int Reactor::set(SystemID system, int amount, int priority)
{
	return this->setGroup(system, amount, priority, { .min = priority, .max = priority });
}

int Reactor::setGroup(SystemID system, int amount, int priority, PowerPriorityRange removal)
{
	this->removeGroup(system, IntMax, removal);
	return this->add(system, amount, priority);
}

int Reactor::systemDesire(SystemID system, PowerPriorityRange range) const
{
	int result = 0;

	for (auto& desire : this->desires)
	{
		if (desire.system != system)
			continue;

		if (desire.priority >= range.min && desire.priority <= range.max)
			result += desire.amount;
	}

	return result;
}

int Reactor::systemDesire(SystemID system, int priority) const
{
	return this->systemDesire(system, { .min = priority, .max = priority });
}

int Reactor::allocatedToSystem(SystemID system) const
{
	return this->allocated[size_t(system)];
}

int Reactor::available() const
{
	auto& used = this->state.player.reactor.first;

	return this->usable() - used;
}

int Reactor::usable() const
{
	static const std::string ZoltanSpecies = "energy";

	auto& reactorMax = this->state.player.reactor.second;
	auto& reactorLimit = this->state.player.reactorLimit;

	int zoltanBars = 0;
	for (auto& crew : this->state.crew)
		if (crew.shipId == 0 && crew.species == ZoltanSpecies && crew.shipAffiliation == 0)
			zoltanBars++;

	int batteryBars = 0;
	auto& battery = this->state.player.systems.battery;
	if (this->systemReady(battery) && battery.on)
		batteryBars = battery.power.total * 2;

	int lockedBars = 0;
	for (auto id : SystemOrder)
	{
		if (id == SystemID::Pilot)
			break;

		auto* s = this->state.player.systems.get(id);
		if (!s) continue;

		if (this->systemLocked(id))
			lockedBars += s->power.total - s->power.zoltan;
	}

	return std::min(reactorMax, reactorLimit)+zoltanBars+batteryBars-lockedBars;
}

bool Reactor::batteryReady() const
{
	auto& battery = this->state.player.systems.battery;
	return
		this->systemReady(battery) && !battery.on;
}

void Reactor::maintainPower()
{
	for (auto& a : this->allocated)
		a = 0;

	int max = this->usable();

	for (auto& desire : this->desires)
	{
		int d = 0;
		auto i = size_t(desire.system);
		auto& al = this->allocated[i];

		auto* s = this->state.player.systems.get(desire.system);
		if (!s)
		{
			this->allocated[i] = -1;
			continue;
		}

		if (max < desire.amount && this->batteryReady())
		{
			this->input.battery();
			max += this->state.player.systems.battery.power.total * 2;
		}

		d = this->clampLevel(desire.system, al+std::min(desire.amount, max))-al;
		al += d;
		max -= d;
	}

	for (auto id : SystemOrder)
	{
		if (id == SystemID::Pilot)
			break;

		auto* s = this->state.player.systems.get(id);
		if (!s) continue;

		auto i = int(id);
		auto& al = this->allocated[i];

		// Shields can only be powered in increments of 2
		if (id == SystemID::Shields)
		{
			al /= 2;
			al *= 2;
		}

		// Unless ioned or zoltan powered, anyways
		if (this->systemLocked(id))
			al = s->power.reactor + s->power.battery;

		if (al < s->power.zoltan)
			al = s->power.zoltan;

		if (s->power.total != al)
			this->input.changePower(id, al);
	}
}