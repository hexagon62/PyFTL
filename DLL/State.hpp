#pragma once

#include "State/Key.hpp"
#include "State/Systems.hpp"
#include "State/Ship.hpp"
#include "State/Space.hpp"
#include "State/StarMap.hpp"
#include "State/Settings.hpp"
#include "State/Blueprints.hpp"
#include "State/UI.hpp"

#include <vector>
#include <optional>

struct PauseState
{
	bool any = false, justPaused = false, justUnpaused = false;
	bool normal = false, automatic = false, menu = false, event = false;
};

struct Game
{
	bool justLoaded = false;
	bool gameOver = false;
	bool justJumped = false;

	PauseState pause;
	Space space;
	StarMap starMap;
	std::optional<LocationEvent> event;
	std::optional<Ship> playerShip, enemyShip;
	std::vector<Crew> playerCrew, enemyCrew;
};

struct State
{
	bool running = false;
	std::optional<Game> game;
	UIState ui;
	Settings settings;
	Blueprints blueprints;
};
