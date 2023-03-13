#pragma once
#pragma once

#include <cstdint>
#include <utility>

namespace raw
{

template<size_t N>
struct Padding
{
	uint8_t bytes[N];
};

template<typename T>
struct Misaligned
{
	Padding<sizeof(T)> data;

	template<typename... Ts>
	Misaligned(Ts&&... args)
		: data(std::forward<Ts>(args)...)
	{}

	operator T& ()
	{
		return reinterpret_cast<T>(this->data);
	}
};

using String = Padding<4>;

template<typename T>
struct Vector
{
	T* begin = nullptr;
	T* end = nullptr;
	T* storageEnd = nullptr;

	size_t size() const
	{
		return (reinterpret_cast<size_t>(this->end) - reinterpret_cast<size_t>(this->begin)) / sizeof(T);
	}

	T& operator[](size_t i)
	{
		return this->begin[i];
	}

	const T& operator[](size_t i) const
	{
		return this->begin[i];
	}
};

struct BitIterator
{
	Padding<1>* bits = nullptr;
	unsigned offset = 0;
};

template<>
struct Vector<bool>
{
	BitIterator start, end;
	Padding<1>* storageEnd = nullptr;
};

using vptr = void*;

struct GL_Color
{
	float r = 0.f, g = 0.f, b = 0.f, a = 0.f;
};

struct Point
{
	int x = 0, y = 0;
};

struct Pointf
{
	float x = 0.f, y = 0.f;
};

struct Rect
{
	int x = 0, y = 0, w = 0, h = 0;
};

struct Ellipse
{
	Point center;
	float a = 0.f, b = 0.f;
};

struct Animation
{
	Padding<144> _u0;
};

struct AnimationTracker
{
	vptr _vptr = nullptr;
	float time = 0.f;
	bool loop = false;
	Padding<3> _u0;
	float current_time = 0.f;
	bool running = false;
	bool reverse = false;
	bool done = false;
	Padding<1> _u1;
	float loopDelay = 0.f;
	float currentDelay = 0.f;
};

struct CachedImage
{
	Padding<56> _u0;
};

struct TextString
{
	String data;
	bool isLiteral = false;
	Padding<3> _u0;
};

struct WarningMessage
{
	vptr _vptr = nullptr;
	AnimationTracker tracker;
	Point position;
	bool isImage = false;
	Padding<3> _u0;
	TextString text;
	bool centerText = false;
	Padding<3> _u1;
	GL_Color textColor;
	bool useWarningLine = false;
	Padding<3> _u2;
	CachedImage image;
	String imageName;
	bool flash = false;
	Padding<3> _u3;
	String sound;
	AnimationTracker flashTracker;
};

struct WarningWithLines : WarningMessage
{
	Padding<4> linePrimitive; // GL_Primitive
	Point textOrigin;
	TextString topText;
	TextString bottomText;
	int topTextLimit = -1;
	int bottomTextLimit = -1;
};

struct DamageMessage
{
	AnimationTracker tracker;
	Pointf position;
	GL_Color color;
	bool bFloatDown = false;
	Padding<3> _u0;
	Vector<void*> primitives;
};

struct ShipObject
{
	vptr _vptr = nullptr;
	int iShipId = -1;
};

struct Targetable
{
	vptr _vptr = nullptr;
	int type = -1;
	bool hostile = false;
	bool targeted = false;
	Padding<2> _u0;
};

struct Collideable
{
	vptr _vptr = nullptr;
};

struct ShipSystem
{

};

struct OxygenSystem
{

};

struct TeleportSystem
{

};

struct CloakingSystem
{

};

struct BatterySystem
{

};

struct MindSystem
{

};

struct CloneSystem
{

};

struct HackingSystem
{

};

struct Shields
{

};

struct WeaponSystem
{

};

struct DroneSystem
{

};

struct MedbaySystem
{

};

struct EngineSystem
{

};

struct ArtillerySystem
{

};

struct CrewMember
{

};

template<typename T>
struct Spreader : ShipObject
{
	int count = 0;
	Vector<int> roomCount;
	Vector<T> grid;
};

struct Selectable
{
	vptr _vptr = nullptr;
	int selectedState = 0;
};

struct Repairable : Selectable, ShipObject
{
	float fDamage = 0.f;
	Point pLoc;
	float fMaxDamage = 0.f;
	char* name = nullptr;
	int roomId = 0;
	int iRepairCount = 0;
};

struct Spreadable
{
	char* soundName = nullptr;
};

struct Fire : Spreadable
{
	float fDeathTimer = 0.f;
	float fStartTimer = 0.f;
	float fOxygen = 0.f;
	uint8_t u0[288];
	bool bWasOnFire;
};

struct SpaceManager
{

};

struct Ship
{
	Padding<768> _u0;
};

struct ShipBlueprint
{
	Padding<340> _u0;
};

struct Projectile
{

};

struct ParticleEmitter
{

};

struct WeaponControl
{
	Padding<764> _u0;
};

struct DroneControl
{
	Padding<656> _u0;
};

struct Drone
{

};

struct SpaceDrone
{

};

struct ShipManager : ShipObject, Targetable, Collideable
{
	Vector<ShipSystem*> vSystemList;
	OxygenSystem* oxygenSystem = nullptr;
	TeleportSystem* teleportSystem = nullptr;
	CloakingSystem* cloakSystem = nullptr;
	BatterySystem* batterySystem = nullptr;
	MindSystem* mindSystem = nullptr;
	CloneSystem* cloneSystem = nullptr;
	HackingSystem* hackingSystem = nullptr;
	bool showNetwork = false;
	bool addedSystem = false;
	Padding<2> _u0;
	Shields* shieldSystem = nullptr;
	WeaponSystem* weaponSystem = nullptr;
	DroneSystem* droneSystem = nullptr;
	MedbaySystem* medbaySystem = nullptr;
	EngineSystem* engineSystem = nullptr;
	Vector<ArtillerySystem*> artillerySystems;
	Vector<CrewMember*> vCrewList;
	Spreader<Fire> fireSpreader;
	Ship ship;
	Padding<40> statusMessages;
	bool bGameOver = false;
	Padding<3> _u1;
	ShipManager* current_target = nullptr;
	std::pair<float, float> jump_timer{ 0.f, 0.f };
	int fuel_count = -1;
	bool hostile_ship = false;
	bool bDestroyed = false;
	Padding<2> _u2;
	int iLastDamage = -1;
	AnimationTracker jumpAnimation;
	Vector<DamageMessage*> damMessages;
	Vector<int> systemKey;
	int currentScrap = -1;
	bool bJumping = false;
	bool bAutomated = false;
	Padding<2> _u3;
	int shipLevel = -1;
	ShipBlueprint myBlueprint;
	bool lastEngineStatus = false;
	bool lastJumpReady = false;
	bool bContainsPlayerCrew = false;
	Padding<1> _u4;
	int iIntruderCount = -1;
	Vector<Vector<int>> crewCounts;
	int tempDroneCount = -1;
	int tempMissileCount = -1;
	Vector<Animation> explosions;
	Vector<bool> tempVision;
	bool bHighlightCrew = false;
	Padding<3> _u5;
	Vector<Drone*> droneTrash;
	Vector<SpaceDrone*> spaceDrones;
	Vector<SpaceDrone*> newDroneArrivals;
	int bpCount = -1;
	int iCustomizeMode = -1;
	bool bShowRoom = false;
	Padding<3> _u6;
	Vector<Projectile*> superBarrage;
	bool bInvincible = false;
	Padding<3> _u7;
	Vector<SpaceDrone*> superDrones;
	Padding<sizeof(void*)> _glStuff;
	int failedDodgeCounter = -1;
	Vector<float> hitByBeam;
	bool heavyDamagedUncloaked = false;
	Padding<3> _u8;
	int damageCloaked = -1;
	Padding<24> killedByBeam; // std::map<int,int>
	int minBeaconHealth = -1;
	Vector<ParticleEmitter*> fireExtinguishers;
	bool bWasSafe = false;
	Padding<3> _u9;
};

struct CompleteShip
{
	vptr _vptr = nullptr;
	int iShipId = -1;
	ShipManager* shipManager = nullptr;
	SpaceManager* spaceManager = nullptr;
	bool bPlayerShip = false;
	Padding<3> _u0;

};

struct CombatControl;

struct ShipStatus
{
	Point location;
	float size = 0.f;
	ShipManager* ship = nullptr;
	CombatControl* combat = nullptr;
	Padding<sizeof(void*) * 46> _glStuff;
	int lastHealth = -1;
	Ellipse baseShield;
	int currentHover = -1;
	Point evadeOxygenBoxLocation;
	int lastFuel = -1;
	int lastDrones = -1;
	int lastScrap = -1;
	int lastMissiles = -1;
	int lastHull = -1;
	WarningWithLines* hullMessage = nullptr;
	WarningMessage* shieldMessage = nullptr;
	WarningMessage* oxygenMessage = nullptr;
	WarningMessage* boardingMessage = nullptr;
	Vector<DamageMessage*> resourceMessages;
	AnimationTracker noMoneyTracker;
	AnimationTracker flashTracker;
	bool bBossFight = false;
	bool bEnemyShip = false;
	Padding<2> _u0;
	Point noShipShift;
	Point intruerShift;
	Point energyShieldPos;
	Point intruderPos;
};

struct CrewBox
{

};

struct Door
{

};

struct Button
{
	Padding<108> _u0;
};

struct FTLButton
{
	Padding<308> _u0;
};

struct TextButton
{
	Padding<220> _u0;
};

struct CrewControl
{
	ShipManager* shipManager = nullptr;
	Vector<CrewMember*> selectedCrew;
	Vector<CrewMember*> potentialSelectedCrew;
	Door* selectedDoor = nullptr;
	Repairable* selectedRepair = nullptr;
	Point selectedGrid;
	int selectedRoom;
	bool selectedPlayerShip = false;
	Padding<3> _u0;
	Point availablePosition;
	Vector<CrewBox*> crewBoxes;
	Point firstMouse;
	Point currentMouse;
	Point worldFirstMouse;
	Point worldCurrentMouse;
	bool mouseDown = false;
	bool bUpdated = false;
	Padding<2> _u1;
	int activeTouch = false;
	bool selectingCrew = false;
	bool selectingCrewOnPlayerShip = false;
	Padding<2> _u2;
	Misaligned<double> selectingCrewStartTime = 0.0;
	bool doorControlMode = false;
	bool doorControlOpen = false;
	bool doorControlOpenSet = false;
	Padding<1> _u3;
	CombatControl* combatControl = nullptr;
	unsigned selectedCrewBox;
	AnimationTracker crewMessage;
	String message;
	Button saveStations;
	Button returnStations;
	Padding<8> _u4;
	int stationsLastY = -1;
};

struct SystemBox
{

};

struct SystemControl
{
	ShipManager* shipManager = nullptr;
	CombatControl* combatControl = nullptr;
	Vector<SystemBox*> sysBoxes;
	Rect SystemPower;
	bool bSystemPowerHover = false;
	Padding<3> _u0;
	Point position;
	Point systemPowerPosition;
	Point subSystemPosition;
	Padding<sizeof(void*) * 10> _glStuff;
	int sub_spacing = -1;
	WarningMessage* notEnoughPower = nullptr;
	AnimationTracker flashBatteryPower;
	AnimationTracker flashTracker;
};

struct CommandGui;

struct WindowFrame
{
	Rect rect;
	Padding<12> _glStuff;
};

struct HandAnimation
{
	Padding<4> _glStuff;
	Point start;
	Point finish;
	Pointf location;
	bool bRunning = false;
	Padding<3> _u0;
	float pause = 0.f;
};

struct CombatControl
{
	CommandGui* gui = nullptr;
	ShipManager* shipManager = nullptr;
	Point playerShipPosition;
	SpaceManager* space = nullptr;
	WeaponControl weapControl;
	DroneControl droneControl;
	Vector<SystemBox*> sysBoxes;
	Vector<CompleteShip*> enemyShips;
	CompleteShip* currentTarget;
	SpaceDrone* currentDrone;
	Point position;
	int selectedRoom = -1;
	int selectedSelfRoom = -1;
	Point targetPosition;
	Point boxPosition;
	WindowFrame* hostileBoxFrame = nullptr;
	CachedImage _images0[24];
	bool open = false;
	Padding<3> _u0;
	float shipIconSize = 0.f;
	Pointf potentialAiming;
	Vector<Pointf> aimingPoints;
	Pointf lastMouse;
	bool mouseDown = false;
	bool isAimingTouch = false;
	bool movingBeam = false;
	Padding<1> _u1;
	Point beamMoveLast;
	bool invalidBeamTouch;
	Padding<3> _u2;
	Point screenReposition;
	std::pair<int, int> teleportCommand{-1, -1};
	int iTeleportArmed = -1;
	CachedImage _images1[4];
	AnimationTracker ftl_timer;
	WarningMessage ftlWarning;
	AnimationTracker hacking_timer;
	Vector<String> hackingMessages_doNotUse;
	bool boss_visual = false;
	bool bTeachingBeam = false;
	Padding<2> _u3;
	WindowFrame* tipBox = nullptr;
	HandAnimation hand;
};

struct SpaceStatus
{
	Padding<104> _u0;
};

struct StarMap
{

};

struct FocusWindow
{
	vptr _vptr = nullptr;
	bool bOpen = false;
	bool bFullFocus = false;
	Padding<2> _u0;
	Point close;
	bool bCloseButtonSelected = false;
	Padding<3> _u1;
	Point position;
};

struct ChoiceBox
{
	Padding<340> _u0;
};

struct MenuScreen
{
	Padding<816> _u0;
};

struct GameOver
{
	Padding<148> _u0;
};

struct OptionsScreen
{
	Padding<3340> _u0;
};

struct InputBox
{
	Padding<60> _u0;
};

struct TabbedWindow
{
	Padding<304> _u0;
};

struct Upgrades
{
	Padding<540> _u0;
};

struct CrewManifest
{
	Padding<784> _u0;
};

struct Equipment
{
	Padding<508> _u0;
};

struct Location
{

};

struct Store
{

};

struct TimerHelper
{
	int maxTime = 0, minTime = 0;
	float currTime = 0.f, currGoal = 0.f;
	bool loop = false, running = false;
	Padding<2> _u0;
};

struct ConfirmWindow
{
	Padding<520> _u0;
};

struct CommandGui
{
	ShipStatus shipStatus;
	CrewControl crewControl;
	SystemControl sysControl;
	CombatControl combatControl;
	FTLButton ftlButton;
	SpaceStatus spaceStatus;
	StarMap* starMap = nullptr;
	CompleteShip* shipComplete = nullptr;
	Vector<FocusWindow*> focusWindows;
	Point pauseTextLoc;
	Padding<sizeof(void*) * 13> _glStuff;
	Point shipPosition;
	String locationText;
	String loadEvent;
	int loadSector = -1;
	ChoiceBox choiceBox;
	bool gameover = false;
	bool alreadyWon = false;
	bool outOfFuel = false;
	Padding<1> _u0;
	MenuScreen menuBox;
	GameOver gmaeOverScreen;
	OptionsScreen optionsBox;
	bool bPaused = false;
	bool bAutoPaused = false;
	bool menu_pause = false;
	bool event_pause = false;
	bool touch_pause = false;
	Padding<3> _u1;
	int touchPauseReason = -1;
	InputBox inputBox;
	float fShakeTimer = 0.f;
	TabbedWindow shipScreens;
	TabbedWindow storeScreens;
	Upgrades upgradeScreen;
	CrewManifest crewScreen;
	Equipment equipScreen;
	Location* newLocation = nullptr;
	SpaceManager* space = nullptr;
	Button upgradeButton;
	WarningMessage upgradeWarning;
	TextButton storeButton;
	Button optionsButton;
	float pause_anim_time = 0.f;
	float pause_animation = 0.f;
	Vector<Store*> storeTrash;
	TimerHelper flickerTimer;
	TimerHelper showTimer;
	bool bHideUI = false;
	Padding<3> _u2;
	CompleteShip* enemyShip;
	bool waitLocation = false;
	bool lastLocationWait = false;
	bool dangerLocation = false;
	Padding<1> _u3;
	Vector<int> commandKey;
	bool jumpComplete = false;
	Padding<3> _u4;
	int mapId = -1;
	ConfirmWindow leaveCrewDialog;
	bool secretSector = false;
	Padding<3> _u5;
	int activeTouch = -1;
	bool activeTouchIsButton = false;
	bool activeTouchIsCrewBox = false;
	bool activeTouchIsShip = false;
	bool activeTouchIsNull = false;
	Vector<int> extraTouches;
	bool bTutorialWasRunning = false;
	bool focusAteMouse = false;
	bool choiceBoxOpen = false;
	Padding<1> _u6;
	int systemDetailsWidth = -1;
	ChoiceBox writeErrorDialog;
	bool suppressWriteError = false;
	Padding<3> _u7;
};

struct WorldManager
{

};

struct ShipBuilder
{
	Padding<6032> _u0;
};

struct CreditScreen
{
	Padding<56> _u0;
};

struct MainMenu
{
	bool open = false;
	Padding<3> _u0;
	int activeTouch = -1;
	Padding<sizeof(void*)*2> _glStuff0;
	AnimationTracker glowTracker;
	Button continueButton;
	Button startButton;
	Button helpButton;
	Button statButton;
	Button optionsButton;
	Button creditsButton;
	Button quitButton;
	Vector<Button*> buttons;
	int finalChoice = -1;
	ShipBuilder shipBuilder;
	bool bScoreScreen = false;
	Padding<3> _u1;
	OptionsScreen optionScreen;
	bool bSelectSave = false;
	Padding<3> _u2;
	ConfirmWindow confirmNewGame;
	ChoiceBox changelog;
	bool bCreditScreen = false;
	Padding<3> _u3;
	CreditScreen credits;
	bool bChangedLogin = false;
	Padding<3> _u4;
	Vector<CrewMember*> testCrew;
	bool bChangedScreen = false;
	bool bSyncScreen = false;
	Padding<2> _u5;
	String error;
};

struct CEvent
{
	vptr _vptr = nullptr;
};

struct LanguageChooser : FocusWindow
{
	Vector<TextButton*> buttons;
	int iChoice = -1;
};

struct CApp
{
	CEvent super_CEvent;
	bool Running = false;
	bool shift_held = false;
	Padding<2> _u0;
	CommandGui* gui = nullptr;
	WorldManager* world = nullptr;
	MainMenu menu;
	LanguageChooser langChooser;
	int screen_x = -1;
	int screen_y = -1;
	int modifier_x = -1;
	int modifier_y = -1;
	bool fullScreenLastState = false;
	bool minimized = false;
	bool minLastState = false;
	bool focus = false;
	bool focusLastState = false;
	bool steamOverlay = false;
	bool steamOverlayLastState = false;
	bool rendering = false;
	bool gameLogic = false;
	Padding<3> _u1;
	float mouseModifier_x = 0.f;
	float mouseModifier_y = 0.f;
	Padding<sizeof(void*)> _glStuff;
	bool fboSupport = false;
	Padding<3> _u2;
	int x_bar = -1;
	int y_bar = -1;
	bool lCtrl = false;
	bool useFrameBuffer = false;
	bool manualResolutionError = false;
	Padding<1> _u3;
	int manualResErrorX = -1;
	int manualResErrorY = -1;
	bool nativeFullScreenError = false;
	bool fbStretchError = false;
	Padding<2> _u4;
	String lastLanguage;
	bool inputFocus = false;
	Padding<3> _u5;
};

struct State
{
	CApp* app = nullptr;
};

constexpr size_t __SIZE_TEST = sizeof(ShipManager);
constexpr uintptr_t CAppBase = 0x4C5020;

}
