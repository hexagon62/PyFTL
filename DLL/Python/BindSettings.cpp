#include "Bind.hpp"
#include "../State/Settings.hpp"

namespace python_bindings
{

void bindSettings(py::module_& module)
{
	py::enum_<FullscreenMode>(module, "FullscreenMode", "A fullscreen mode the game can be set to")
		.value("Off", FullscreenMode::Off, "Windowed")
		.value("Stretch", FullscreenMode::Stretch, "Borderless window")
		.value("Borders", FullscreenMode::Borders, "Fullscreen that is not upscaleed")
		.value("Native", FullscreenMode::Native, "Native fullscreen")
		;

	py::enum_<Difficulty>(module, "Difficulty", "A difficulty level")
		.value("Easy", Difficulty::Easy)
		.value("Normal", Difficulty::Normal)
		.value("Hard", Difficulty::Hard)
		;

	py::enum_<EventChoiceSelection>(module, "EventChoiceSelection", "Options for how event choices can be made")
		.value("DisableHotkeys", EventChoiceSelection::DisableHotkeys)
		.value("NoDelay", EventChoiceSelection::NoDelay)
		.value("BriefDelay", EventChoiceSelection::BriefDelay)
		;

	py::class_<Settings>(module, "Settings", "Game settings")
		.def_readonly("fullscreen", &Settings::fullscreen, "The fullscreen mode")
		.def_readonly("sound_volume", &Settings::soundVolume, "The sound volume")
		.def_readonly("music_volume", &Settings::musicVolume, "The music volume")
		.def_readonly("difficulty", &Settings::difficulty, "The current difficulty level")
		.def_readonly("console_enabled", &Settings::consoleEnabled, "If the console is enabled")
		.def_readonly("pause_on_focus_loss", &Settings::pauseOnFocusLoss, "Pause if focus is lost")
		.def_readonly("touch_pause", &Settings::touchPause, "Pause on touch (unused on PC?)")
		.def_readonly("no_dynamic_backgrounds", &Settings::noDynamicBackgrounds, "Lowend mode, no dynamic backgrounds")
		.def_readonly("achievement_popups", &Settings::achievementPopups, "Popups for achievements")
		.def_readonly("vertical_sync", &Settings::verticalSync, "Vertical sync")
		.def_readonly("frame_limit", &Settings::frameLimit, "Frame limit")
		.def_readonly("show_breacon_paths_on_hover", &Settings::showBeaconPathsOnHover, "Show beacon paths on hover")
		.def_readonly("colorblind_mode", &Settings::colorblindMode, "Colorblind mode")
		.def_readonly("ae_enabled", &Settings::aeEnabled, "If Advanced Edition is currently enabled")
		.def_readonly("language", &Settings::language, "The current language")
		.def_readonly("screeen_size", &Settings::screenSize, "The resolution of the monitor")
		.def_readonly("event_choice_selection", &Settings::eventChoiceSelection, "How event choices are made")
		.def_readonly("hotkeys", &Settings::hotkeys, "The current hotkeys")
		;
}

}
