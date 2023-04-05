#include "Player.hpp"

#include <algorithm>
#include <iostream>

Player::Player(const StateReader& reader)
	: reader(reader)
	, input(reader)
{

}

void Player::iterate(int dt)
{
	auto&& s = this->reader.getState();
	
	if (s.inGame && s.focused && !s.pause.any && !s.player.jumping && g_active && !g_pause)
	{
		this->think(dt);

		if (!this->input.idle())
		{
			this->input.iterate();
			return;
		}

		this->act(dt);
	}
}

void Player::think(int dt)
{
	auto&& s = this->reader.getState();
	auto&& shields = s.player.systems.shields;
	auto&& oxygen = s.player.systems.oxygen;

	if (shields.present)
	{
		this->state.desiredPower.shields = (shields.level.first / 2) * 2;
	}

	if (oxygen.present)
	{
		if (oxygen.total >= this->state.oxygenBreakpoints[0])
		{
			this->state.desiredPower.oxygen = 0;
		}

		if (oxygen.total <= this->state.oxygenBreakpoints[1])
		{
			this->state.desiredPower.oxygen = 1;
		}

		if (oxygen.total <= this->state.oxygenBreakpoints[2])
		{
			this->state.desiredPower.oxygen = std::min(oxygen.level.first, 2);
		}

		if (oxygen.total <= this->state.oxygenBreakpoints[3])
		{
			this->state.desiredPower.oxygen = std::min(oxygen.level.first, 3);
		}
	}

	if (s.player.jumping || s.targetPresent) this->state.preppingForJump = false;
}

void Player::act(int dt)
{
	auto&& s = this->reader.getState();

	if (!s.targetPresent && s.player.ftl.first == s.player.ftl.second && !s.player.jumping && !this->state.preppingForJump)
	{
		std::cout << "No target. Please jump and navigate the events for me!\n";
		this->state.preppingForJump = true;
	}

	if(this->state.preppingForJump)
		this->input.openJumpMenu();

	this->processReactorInputs();
}

void Player::processReactorInputs()
{
	auto&& sys = this->reader.getState().player.systems;
	auto&& desired = this->state.desiredPower;

	if (desired.shields != sys.shields.power.total)
		this->input.changePower(SystemType::Shields, desired.shields, true);

	if (desired.oxygen != sys.oxygen.power.total)
		this->input.changePower(SystemType::Oxygen, desired.oxygen, true);
}
