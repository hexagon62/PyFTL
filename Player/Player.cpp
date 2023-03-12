#include "Player.hpp"
#include "Thinker/Root.hpp"

#include <cmath>

Player::Player(const StateReader& reader)
	: reader(reader)
	, input(reader)
{

}

void Player::onLoop(int dt)
{
	auto& curr = this->reader.getState();

	if (curr.inGame && curr.focused && !curr.pause.any && !curr.player.jumping)
	{
		if (!this->thinker)
		{
			this->thinker.reset(new RootThinker{
				ThinkerParams{
					this->reader.getState(),
					this->reader.getBlueprints(),
					input
				}
			});
		}

		if (this->input.idle())
			this->thinker->think();

		this->input.iterate();
	}
	else if (!curr.inGame)
		this->thinker.reset();
}
