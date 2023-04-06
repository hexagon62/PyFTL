#include "Bind.hpp"
#include "../State/State.hpp"

namespace python_bindings
{

void bindMisc(py::module_& module)
{
	py::class_<PauseState>(module, "Pause")
		.def_readonly("any", &PauseState::any, "If the game is paused at all")
		.def_readonly("just_paused", &PauseState::justPaused, "If the game just paused")
		.def_readonly("just_unpaused", &PauseState::justUnpaused, "If the game just unpaused")
		.def_readonly("normal", &PauseState::normal, "If the player paused the game")
		.def_readonly("automatic", &PauseState::automatic, "If the game was automatically paused")
		.def_readonly("menu", &PauseState::menu, "If the game is paused due to a menu being open")
		.def_readonly("event", &PauseState::event, "If an event paused the game")
		.def_readonly("touch", &PauseState::touch, "If the game was paused through touch (not used on PC)")
		;

	py::class_<RandomAmount<int>>(module, "RandomAmountInt", "Used by FTL to generate random integers")
		.def_readonly("min", &RandomAmount<int>::min, "Minimum value")
		.def_readonly("max", &RandomAmount<int>::max, "Maximum value")
		.def_readonly("chance_none", &RandomAmount<int>::chanceNone, "Chance of nothing happening")
		;

	py::class_<RandomAmount<float>>(module, "RandomAmountFloat", "Used by FTL to generate random integers; casted to floating point")
		.def_readonly("min", &RandomAmount<float>::min, "Minimum value")
		.def_readonly("max", &RandomAmount<float>::max, "Maximum value")
		.def_readonly("chance_none", &RandomAmount<float>::chanceNone, "Chance of nothing happening")
		;

	py::class_<Game>(module, "Game", "The base game object")
		.def_readonly("just_loaded", &Game::justLoaded, "If the game just loaded")
		.def_readonly("game_over", &Game::gameOver, "If the game is over")
		.def_readonly("just_jumped", &Game::justJumped, "If the player just jumped")
		.def_readonly("pause", &Game::pause, "The pause state")
		.def_readonly("space", &Game::space, "The space state")
		.def_readonly("event", &Game::event, "The current event")
		.def_readonly("star_map", &Game::starMap, "The map of beacons & sectors")
		.def_readonly("player_ship", &Game::playerShip, "The player ship")
		.def_readonly("enemy_ship", &Game::enemyShip, "The enemy ship")
		.def_readonly("player_crew", &Game::playerCrew, "The player crew")
		.def_readonly("enemy_crew", &Game::enemyCrew, "The enemy crew")
		;

	py::class_<State>(module, "State", "The overall game state")
		.def_readonly("running", &State::running, "If the game is running")
		.def_readonly("game", &State::game, "Most game state data is here")
		.def_readonly("settings", &State::settings, "The settings")
		.def_readonly("blueprints", &State::blueprints, "The blueprint data")
		;
}

}

