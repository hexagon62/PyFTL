#pragma once

#include "Bind.hpp"
#include "../State/UI.hpp"

namespace python_bindings
{

void bindUI(py::module_& module)
{

	py::class_<StoreUIState>(module, "StoreUIState", "The state of the store menu.")
		.def_readonly("close", &StoreUIState::close, "The button to exit the store")
		.def_readonly("buy", &StoreUIState::buy, "The buy tab")
		.def_readonly("sell", &StoreUIState::sell, "The sell tab")
		.def_readonly("pages", &StoreUIState::pages, "The tabs to change pages.")
		.def_readonly("fuel", &StoreUIState::fuel, "The button to buy fuel. Valid only if store has it.")
		.def_readonly("missiles", &StoreUIState::missiles, "The button to buy missiles. Valid only if store has them.")
		.def_readonly("drone_parts", &StoreUIState::droneParts, "The button to buy drone parts. Valid only if store has them.")
		.def_readonly("repair", &StoreUIState::repair, "The button to repair 1 hull damage")
		.def_readonly("repair_all", &StoreUIState::repairAll, "The button to repair all hull damage")
		.def_readonly("boxes", &StoreUIState::boxes, "The boxes for each store item")
		.def_readonly("confirm", &StoreUIState::confirm, "The confirmation window that pops up when filling your system slots.\nValid only if present.")
		;

	py::class_<SystemUpgradeUIState>(module, "SystemUpgradeUIState", "The state of a particular system's upgrade UI")
		.def_readonly("box", &SystemUpgradeUIState::box, "The hitbox of the upgrade button")
		.def_readonly("upgrade", &SystemUpgradeUIState::box, "A pair of levels arranged as from-to")
		;

	py::class_<UpgradesUIState>(module, "UpgradesUIState", "The state of the ship upgrade menu.")
		.def_readonly("shields", &UpgradesUIState::shields, "The button to upgrade shields.\nValid only if the system is present.")
		.def_readonly("engines", &UpgradesUIState::shields, "The button to upgrade engines.\nValid only if the system is present.")
		.def_readonly("oxygen", &UpgradesUIState::oxygen, "The button to upgrade oxygen.\nValid only if the system is present.")
		.def_readonly("weapons", &UpgradesUIState::weapons, "The button to upgrade weapons.\nValid only if the system is present.")
		.def_readonly("drones", &UpgradesUIState::drones, "The button to upgrade drones.\nValid only if the system is present.")
		.def_readonly("medbay", &UpgradesUIState::medbay, "The button to upgrade medbay.\nValid only if the system is present.")
		.def_readonly("piloting", &UpgradesUIState::piloting, "The button to upgrade piloting.\nValid only if the system is present.")
		.def_readonly("sensors", &UpgradesUIState::sensors, "The button to upgrade sensors.\nValid only if the system is present.")
		.def_readonly("doors", &UpgradesUIState::doorControl, "The button to upgrade doors.\nValid only if the system is present.")
		.def_readonly("teleporter", &UpgradesUIState::teleporter, "The button to upgrade the teleporter.\nValid only if the system is present.")
		.def_readonly("cloaking", &UpgradesUIState::cloaking, "The button to upgrade cloaking.\nValid only if the system is present.")
		.def_readonly("artillery", &UpgradesUIState::shields, "The buttons to upgrade artillery systems.")
		.def_readonly("battery", &UpgradesUIState::shields, "The button to upgrade the backup battery.\nValid only if the system is present.")
		.def_readonly("clonebay", &UpgradesUIState::clonebay, "The button to upgrade the clonebay.\nValid only if the system is present.")
		.def_readonly("mind_control", &UpgradesUIState::mindControl, "The button to upgrade mind control.\nValid only if the system is present.")
		.def_readonly("hacking", &UpgradesUIState::hacking, "The button to upgrade hacking.\nValid only if the system is present.")
		.def_readonly("reactor", &UpgradesUIState::reactor, "The button to upgrade the reactor")
		.def_readonly("undo", &UpgradesUIState::undo, "The button to undo any changes while the menu is still open")
		.def_readonly("accept", &UpgradesUIState::accept, "The button to close the menu")
		.def("has_system", &UpgradesUIState::hasSystem, "Checks if the specified system is present")
		.def("get_system", &UpgradesUIState::getSystem, py::return_value_policy::reference, "Gets the specified system's upgrade button")
		;

	py::class_<CrewManifestUIState>(module, "CrewManifestUIState", "The state of the ship crew menu.")
		.def_readonly("boxes", &CrewManifestUIState::boxes, "The boxes for each crewmember.\nThese each have multiple sub-boxes.")
		.def_readonly("confirm", &CrewManifestUIState::confirm, "The window that pops up to confirm you want to dismiss crew.\nValid only if present.")
		.def_readonly("accept", &CrewManifestUIState::accept, "The button to close the menu")
		;

	py::class_<CargoUIState>(module, "CargoUIState", "The state of the ship inventory menu.")
		.def_readonly("weapons", &CargoUIState::weapons, "The boxes defining the weapon system slots")
		.def_readonly("drones", &CargoUIState::drones, "The boxes defining the drone system slots")
		.def_readonly("augments", &CargoUIState::augments, "The boxes defining the augment slots")
		.def_readonly("storage", &CargoUIState::storage, "The boxes defining the weapon/drone storage slots")
		.def_readonly("discard", &CargoUIState::discard, "The box where you sell or discard items.\nValid if in the store menu, or there's over-capacity.")
		.def_readonly("accept", &CargoUIState::accept, "The button to close the menu")
		;

	py::class_<CrewBoxUIState>(module, "CrewBoxUIState", "The state of a crewmember's UI box.")
		.def_readonly("box", &CrewBoxUIState::box, "The main box for selecting the crew")
		.def_readonly("skills", &CrewBoxUIState::skills, "The box where skills and special abilities pop up if hovered")
		;

	py::class_<StarMapUIState>(module, "StarMapUIState", "The state of jump menu.")
		.def_readonly("back", &StarMapUIState::back, "The button that closes the sector jump menu, or the star map altogether")
		.def_readonly("wait", &StarMapUIState::wait, "The button to wait.\nValid only if waiting is possible.")
		.def_readonly("distress", &StarMapUIState::distress, "The button to toggle the distress beacon.\nValid only if out of fuel.")
		.def_readonly("next_sector", &StarMapUIState::nextSector, "The button to go to the next sector.\nValid only if at an exit beacon.")
		;

	py::class_<EventChoiceUIState>(module, "EventChoiceUIState", "The state of choice in the event UI.")
		.def_readonly("text", &EventChoiceUIState::text, "The text")
		.def_readonly("box", &EventChoiceUIState::box, "The box bounding the choice")
		;

	py::class_<EventUIState>(module, "EventUIState", "The state of the event UI")
		.def_readonly_static("HARDCODED_BRIEF_DELAY_TIME", &EventUIState::HARDCODED_BRIEF_DELAY_TIME, "The delay used if event hotkeys are set to 'brief delay'")
		.def_readonly("text", &EventUIState::text, "The main text of the event")
		.def_readonly("choices", &EventUIState::choices, "The choices that can be made")
		.def_readonly("open_time", &EventUIState::openTime, "Timer for progress towards when hotkey presses will work")
		;

	py::class_<GameOverUIState>(module, "GameOverUIState", "The state of the game over UI")
		.def_readonly("stats", &GameOverUIState::stats, "The button to go to the stats menu")
		.def_readonly("restart", &GameOverUIState::restart, "The button to restart the game")
		.def_readonly("hangar", &GameOverUIState::hangar, "The button to go to the hangar")
		.def_readonly("main_menu", &GameOverUIState::mainMenu, "The button to go to the main menu")
		.def_readonly("quit", &GameOverUIState::quit, "The button to quit the game")
		;

	py::class_<MenuUIState>(module, "MenuUIState", "The state of the pause menu UI")
		.def_readonly("continue_button", &MenuUIState::continueButton, "The button to continue the game")
		.def_readonly("main_menu", &MenuUIState::mainMenu, "The button to go to the main menu")
		.def_readonly("hangar", &MenuUIState::hangar, "The button to go to the hangar")
		.def_readonly("restart", &MenuUIState::restart, "The button to restart the game")
		.def_readonly("options", &MenuUIState::options, "The button to go to the options menu")
		.def_readonly("controls", &MenuUIState::controls, "The button to bring up the controls overview")
		.def_readonly("quit", &MenuUIState::quit, "The button to save and quit the game")
		.def_readonly("difficulty", &MenuUIState::difficulty, "The box containing the difficulty level")
		.def_readonly("ae_enabled", &MenuUIState::aeEnabled, "The box containing if AE is enabled")
		.def_readonly("achievements", &MenuUIState::achievements, "The box containing the ship achievements")
		.def_readonly("show_controls", &MenuUIState::showControls, "If the controls overview is currently being shown")
		.def_readonly("confirm", &MenuUIState::confirm, "The button to confirm restarting")
		;

	py::class_<GameUIState>(module, "GameUIState", "The state of in-game UI stuff. Mostly button positions.")
		.def_readonly("player_ship", &GameUIState::playerShip, "The base position of the player ship")
		.def_readonly("enemy_ship", &GameUIState::enemyShip, "The base position of the enemy ship")
		.def_readonly("ftl", &GameUIState::ftl, "The FTL button")
		.def_readonly("ship_button", &GameUIState::shipButton, "The ship button")
		.def_readonly("menu_button", &GameUIState::menuButton, "The button for the pause menu")
		.def_readonly("store_button", &GameUIState::storeButton, "The button to open the store.\nValid only if a store is presnet.")
		.def_readonly("crew_boxes", &GameUIState::crewBoxes, "The boxes for crewmembers.\nThis consists of two parts, the main part, and the skills part.")
		.def_readonly("save_stations", &GameUIState::saveStations, "The button to save crew stations")
		.def_readonly("load_stations", &GameUIState::loadStations, "The button to return crew to their stations")
		.def_readonly("reactor", &GameUIState::reactor, "The box containing all reactor power bars")
		.def_readonly("shields", &GameUIState::shields, "The box containing the shields icon and power bars.\nValid only if the system is present.")
		.def_readonly("engines", &GameUIState::engines, "The box containing the engines icon and power bars.\nValid only if the system is present.")
		.def_readonly("oxygen", &GameUIState::oxygen, "The box containing the oxygen icon and power bars.\nValid only if the system is present.")
		.def_readonly("weapons", &GameUIState::weapons, "The box containing the weapons icon and power bars.\nValid only if the system is present.")
		.def_readonly("drones", &GameUIState::drones, "The box containing the drones icon and power bars.\nValid only if the system is present.")
		.def_readonly("medbay", &GameUIState::medbay, "The box containing the medbay icon and power bars.\nValid only if the system is present.")
		.def_readonly("piloting", &GameUIState::piloting, "The box containing the piloting icon and power bars.\nValid only if the system is present.")
		.def_readonly("sensors", &GameUIState::sensors, "The box containing the sensors icon and power bars.\nValid only if the system is present.")
		.def_readonly("doors", &GameUIState::doorControl, "The box containing the doors icon and power bars.\nValid only if the system is present.")
		.def_readonly("teleporter", &GameUIState::teleporter, "The box containing the teleporter icon and power bars.\nValid only if the system is present.")
		.def_readonly("cloaking", &GameUIState::cloaking, "The box containing the cloaking icon and power bars.\nValid only if the system is present.")
		.def_readonly("artillery", &GameUIState::artillery, "The boxes containing the artillery icons and power bars.\nValid only if the system is present.")
		.def_readonly("battery", &GameUIState::battery, "The box containing the backup battery icon and power bars.\nValid only if the system is present.")
		.def_readonly("clonebay", &GameUIState::clonebay, "The box containing the clonebay icon and power bars.\nValid only if the system is present.")
		.def_readonly("mind_control", &GameUIState::mindControl, "The box containing the mind control icon and power bars.\nValid only if the system is present.")
		.def_readonly("hacking", &GameUIState::hacking, "The box containing the hacking icon and power bars, if present")
		.def_readonly("weapon_boxes", &GameUIState::weaponBoxes, "The boxes for each weapon currently equipped.\nValid only if the weapom system is present")
		.def_readonly("autofire", &GameUIState::autofire, "The button to toggle auto-fire.\nValid only if the weapom system is present")
		.def_readonly("open_all_doors", &GameUIState::openAllDoors, "The button to open all doors.\nValid only if the door system is present.")
		.def_readonly("drone_boxes", &GameUIState::droneBoxes, "The boxes for each drone currently equipped.\nValid only if the drone system is present")
		.def_readonly("open_all_doors", &GameUIState::openAllDoors, "The button to open all doors.\nValid only if the door system is present.")
		.def_readonly("close_all_doors", &GameUIState::closeAllDoors, "The button to close all doors.\nValid only if the door system is present.")
		.def_readonly("teleport_send", &GameUIState::teleportSend, "The button to teleport crew away.\nValid only if the teleporter system is present.")
		.def_readonly("teleport_return", &GameUIState::teleportReturn, "The button to teleport crew back.\nValid only if the teleporter system is present.")
		.def_readonly("start_cloak", &GameUIState::startCloak, "The button to activate cloaking.\nValid only if the cloaking system is present.")
		.def_readonly("start_battery", &GameUIState::startBattery, "The button to activate the backup battery.\nValid only if the backup battery system is present.")
		.def_readonly("start_mind_control", &GameUIState::startMindControl, "The button to activate mind control.\nValid only if the mind control system is present.")
		.def_readonly("start_hack", &GameUIState::startHack, "The button to star hacking.\nValid only if the hacking system is present.")
		.def_readonly("upgrades", &GameUIState::upgrades, "The state of the upgrades menu. Valid only if opened.")
		.def_readonly("crew_menu", &GameUIState::crewMenu, "The state of the crew manifest. Valid only if opened.")
		.def_readonly("leave_crew", &GameUIState::leaveCrew, "The window that pops up if leaving crew behind on the enemy ship.")
		.def_readonly("cargo", &GameUIState::cargo, "The state of the inventory menu. Valid only if opened.")
		.def_readonly("star_map", &GameUIState::starMap, "The state of the jump menu. Valid only if opened.")
		.def_readonly("event", &GameUIState::event, "The state of the event menu.\nValid only if there is an event.")
		.def_readonly("store", &GameUIState::store, "The state of the store menu.\nValid only if opened.")
		.def_readonly("game_over", &GameUIState::store, "The state of the game over menu.\nValid only if opened.")
		.def_readonly("menu", &GameUIState::menu, "The state of the pause menu.\nValid only if opened.")
		.def("has_system", &GameUIState::hasSystem, "Checks if the specified system is present")
		.def("get_system", &GameUIState::getSystem, py::return_value_policy::reference, "Gets the specified system's box containing its icon and power bars")
		;

	py::class_<MainMenuUIState>(module, "MainMenuUIState", "The state of the main menu UI")
		.def_readonly("continue_button", &MainMenuUIState::continueButton, "The button to continue the game")
		.def_readonly("new_game", &MainMenuUIState::newGame, "The button to start a new game")
		.def_readonly("tutorial", &MainMenuUIState::tutorial, "The button to start the tutorial")
		.def_readonly("stats", &MainMenuUIState::stats, "The button to go to the stats menu")
		.def_readonly("options", &MainMenuUIState::options, "The button to go to the options menu")
		.def_readonly("credits", &MainMenuUIState::credits, "The button to go to the credits")
		.def_readonly("quit", &MainMenuUIState::quit, "The button to quit the game")
		.def_readonly("confirm", &MainMenuUIState::confirm, "The button to confirm restarting")
		;

	py::class_<MouseState>(module, "MouseState", "The state of the mouse")
		.def_readonly("position", &MouseState::position, "The current position of the mouse")
		.def_readonly("position_last", &MouseState::positionLast, "The last position of the mouse")
		.def_readonly("drag_from", &MouseState::dragFrom, "The starting position of the box when clicking+dragging to select crew")
		.def_readonly("aiming", &MouseState::aiming, "The system/weapon currently being aimed")
		.def_readonly("autofire", &MouseState::autofire, "If autofiring. Is None if not aiming a weapon.")
		;

	py::class_<UIState>(module, "UIState", "The state of the overall UI")
		.def_readonly("mouse", &UIState::mouse, "The state of the mouse")
		.def_readonly("game", &UIState::game, "The state of in-game UI stuff.\nValid only if in-game.")
		;
}

}
