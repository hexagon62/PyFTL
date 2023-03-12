#include "Player.hpp"

Player::Player(const StateReader& reader)
	: reader(reader)
	, input(reader)
{

}

void Player::think(int dt)
{
	auto&& state = this->reader.getState();
	auto&& shields = state.player.systems.shields;

    if (state.inGame && state.focused && !state.pause.any && !state.player.jumping && g_active)
    {
        if (!this->input.idle())
            this->input.iterate();



		if (shields.present && shields.power.total == 0)
			input.changePower(SystemID::Shields, 2, true);
    }
}

int Player::shieldsNeeded(const State& state) const
{
	// placeholder
	return 5;
}