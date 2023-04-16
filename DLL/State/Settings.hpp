#pragma once

#include "Point.hpp"
#include "Key.hpp"

#include <string>
#include <map>

enum class FullscreenMode : int
{
	Off, Stretch, Borders, Native
};

enum class Difficulty : int
{
	Easy, Normal, Hard
};

enum class EventChoiceSelection : int
{
	DisableHotkeys, NoDelay, BriefDelay
};

struct Settings
{
	FullscreenMode fullscreen = FullscreenMode::Off;
	int soundVolume = 0;
	int musicVolume = 0;
	Difficulty difficulty = Difficulty::Easy;
	bool consoleEnabled = false;
	bool pauseOnFocusLoss = false;
	bool touchPause = false;
	bool noDynamicBackgrounds = false;
	bool achievementPopups = false;
	bool verticalSync = false;
	bool frameLimit = false;
	bool showBeaconPathsOnHover = false;
	bool colorblindMode = false;
	bool aeEnabled = false;
	std::string language = "en";
	Point<int> screenSize;
	EventChoiceSelection eventChoiceSelection = EventChoiceSelection::DisableHotkeys;
	std::map<std::string, Key> hotkeys;
};
