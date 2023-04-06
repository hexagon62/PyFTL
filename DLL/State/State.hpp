#pragma once

#include "Data/Key.hpp"
#include "Data/Systems.hpp"
#include "Data/Ship.hpp"
#include "Data/Space.hpp"
#include "Data/StarMap.hpp"
#include "Data/Settings.hpp"
#include "Data/Blueprints.hpp"

#include <vector>
#include <optional>

struct PauseState
{
	bool any = false, justPaused = false, justUnpaused = false;
	bool normal = false, automatic = false, menu = false, event = false, touch = false;
};

struct Game
{
	bool justLoaded = false;
	bool gameOver = false;
	bool justJumped = false;

	PauseState pause;
	Space space;
	LocationEvent event;
	StarMap starMap;
	std::optional<Ship> playerShip, enemyShip;
	std::vector<Crew> playerCrew, enemyCrew;
};

struct State
{
	bool running = false;
	std::optional<Game> game;
	Settings settings;
	Blueprints blueprints;
};
