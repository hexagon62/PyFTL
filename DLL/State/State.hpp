#pragma once

#include <utility>
#include <string>
#include <vector>
#include <array>
#include <optional>
#include <map>
#include <memory>

enum class Key : int
{
	Unknown = 0,
	Backspace = 8,
	Tab = 9,
	Clear = 12,
	Return = 13,
	Pause = 19,
	Escape = 27,
	Space = 32,
	Exclaim = 33,
	DoubleQuote = 34,
	Hash = 35,
	Dollar = 36,
	Ampersand = 38,
	Quote = 39,
	LeftParenthesis = 40,
	RightParenthesis = 41,
	Asterisk = 42,
	Plus = 43,
	Comma = 44,
	Minus = 45,
	Period = 46,
	Slash = 47,
	Num0 = 48,
	Num1 = 49,
	Num2 = 50,
	Num3 = 51,
	Num4 = 52,
	Num5 = 53,
	Num6 = 54,
	Num7 = 55,
	Num8 = 56,
	Num9 = 57,
	Colon = 58,
	Semicolon = 59,
	Less = 60,
	Equals = 61,
	Greater = 62,
	Question = 63,
	At = 64,

	// skip uppercase

	LeftBracket = 91,
	BackSlash = 92,
	RightBracket = 93,
	Caret = 94,
	Underscore = 95,
	Backquote = 96,
	A = 97,
	B = 98,
	C = 99,
	D = 100,
	E = 101,
	F = 102,
	G = 103,
	H = 104,
	I = 105,
	J = 106,
	K = 107,
	L = 108,
	M = 109,
	N = 110,
	O = 111,
	P = 112,
	Q = 113,
	R = 114,
	S = 115,
	T = 116,
	U = 117,
	V = 118,
	W = 119,
	X = 120,
	Y = 121,
	Z = 122,
	Delete = 127,

	// numpad

	Numpad0 = 256,
	Numpad1 = 257,
	Numpad2 = 258,
	Numpad3 = 259,
	Numpad4 = 260,
	Numpad5 = 261,
	Numpad6 = 262,
	Numpad7 = 267,
	Numpad8 = 268,
	Numpad9 = 269,
	NumpadPeriod = 266,
	NumpadDivide = 267,
	NumpadMultiply = 268,
	NumpadMinus = 269,
	NumpadPlus = 270,
	NumpadEnter = 271,

	// arrow keys
	Up = 273,
	Down = 274,
	Right = 275,
	Left = 276,
	
	// home/end
	Insert = 277,
	Home = 278,
	End = 279,
	PageUp = 280,
	PageDown = 281,

	// function keys
	F1 = 282,
	F2 = 283,
	F3 = 284,
	F4 = 285,
	F5 = 286,
	F6 = 287,
	F7 = 288,
	F8 = 289,
	F9 = 290,
	F10 = 291,
	F11 = 292,
	F12 = 293,
	F13 = 294,
	F14 = 295,
	F15 = 296,

	// modifier keys
	NumLock = 300,
	CapsLock = 301,
	ScrollLock = 302
};

template<typename T>
struct Point
{
	T x = T(0), y = T(0);

	Point& operator+=(const Point& other)
	{
		this->x += other.x;
		this->y += other.y;
		return *this;
	}

	Point& operator-=(const Point& other)
	{
		this->x -= other.x;
		this->y -= other.y;
		return *this;
	}

	Point& operator*=(const T& factor)
	{
		this->x *= factor;
		this->y *= factor;
		return *this;
	}

	Point& operator/=(const T& factor)
	{
		this->x /= factor;
		this->y /= factor;
		return *this;
	}

	Point operator+(const Point& other) const
	{
		return { this->x + other.x, this->y + other.y };
	}

	Point operator-(const Point& other) const
	{
		return { this->x - other.x, this->y - other.y };
	}

	Point operator*(const T& factor) const
	{
		return { this->x * factor, this->y * factor };
	}

	Point operator/(const T& factor) const
	{
		return { this->x / factor, this->y / factor };
	}
};

template<typename T>
using Capped = std::pair<T, T>;

template<typename T>
struct Rect
{
	T x = T(0), y = T(0), w = T(0), h = T(0);

	template<typename U>
	bool contains(const Point<U>& p) const
	{
		return
			p.x >= U(this->x) && p.x <= U(this->x + this->w) &&
			p.y >= U(this->y) && p.y <= U(this->y + this->h);
	}

	Point<T> center() const
	{
		return { this->x + this->w / T(2), this->y + this->h / T(2) };
	}
};

template<typename T>
struct Ellipse
{
	Point<T> center;
	T a = T(0), b = T(0);

	template<typename U>
	bool contains(const Point<U>& p) const
	{
		U term1 = p.x - U(center.x);
		U term2 = p.y - U(center.y);

		return
			(term1 * term1) / (a * a) + (term2 * term2) / (b * b) <= T(0);
	}
};

enum class SystemType : int
{
	None = -1,
	Shields = 0,
	Engines,
	Oxygen,
	Weapons,
	Drones,
	Medbay,
	Piloting,
	Sensors,
	Doors,
	Teleporter,
	Cloaking,
	Artillery,
	battery,
	Clonebay,
	MindControl,
	Hacking
};

enum class HackLevel : int
{
	Invalid = -1,
	None = 0,
	Passive = 1,
	Active = 2
};

struct Blueprint
{
	std::string name;
	int cost = 0;
	int rarity = 0, baseRarity = 0;
};

struct SystemBlueprint : Blueprint
{
	int powerStart = 0, powerMax = 0;
	std::vector<int> upgradeCosts;
};

struct Augment : Blueprint
{
	float value = 0.f;
	bool stacking = false;
};

struct BoostPower
{
	int type = 0;
	float amount = 0.f;
	int count = 0;
};

struct Damage
{
	static constexpr int HARDCODED_CREW_DAMAGE_FACTOR = 15;

	int normal = 0;
	int ion = 0;
	int system = 0;
	int crew = 0;
	int fireChance = 0;
	int breachChance = 0;
	int stunChance = 0;
	int pierce = 0;
	int stunTime = 0;
	bool hullBonus = false;
	bool friendlyFire = false;
};

enum class WeaponType
{
	Invalid = -1,
	Laser, // LASER
	Beam, // BEAM
	Burst, // BURST
	Missiles, // MISSILES
	Bomb // BOMB
};

struct WeaponBlueprint : Blueprint
{
	Damage damage;
	WeaponType type = WeaponType::Invalid;
	int shots = 0;
	int missiles = 0;
	float cooldown = 0.f;
	int power = 0;
	int beamLength = -1;
	int burstRadius = 0;
	int chargeLevels = 0;
	BoostPower boost;
	int projectilesFake = 0, projectiles = 0;
	int projectilesTotal = 0;
};

struct Weapon
{
	Capped<float> cooldown{ 0.f, 0.f };
	WeaponBlueprint blueprint;
	bool autoFire = false;
	bool fireWhenReady = false;
	bool powered = false;
	bool artillery = false;
	bool targetingPlayer = false;
	float firingAngle = 0.f;
	float entryAngle = 0.f;
	Point<int> mount;
	int zoltanPower = 0;
	HackLevel hackLevel = HackLevel::None;
	Capped<int> boost{ 0, 0 };
	Capped<int> charge{ 0, 0 };
	Capped<float> shotTimer{ 0.f, 0.f };
	std::vector<Point<float>> targetPoints;
};

enum class DroneType
{
	Invalid = -1,
	Combat, // COMBAT
	Defense, // DEFENSE
	AntiBoarder, // BATTLE
	Boarder, // BOARDER
	HullRepair, // SHIP_REPAIR
	SystemRepair, // REPAIR
	Hacking, // HACKING
	Shield // SHIELD
};

struct DroneBlueprint : Blueprint
{
	static constexpr std::array<float, 5> HARDCODED_SHIELD_COOLDOWNS{ 8.f, 10.f, 13.f, 16.f, 20.f };

	DroneType type = DroneType::Invalid;
	int power = 0;
	float cooldown = 0.f;
	int speed = 0;
};

struct SpaceDroneMovementExtra
{
	Point<float> destinationLast;
	float progress = 0.f;
	float heading = 0.f, headingLast = 0.f;
};

struct SpaceDroneInfo
{
	bool playerSpace = false, playerSpaceIsDestination = false;

	bool moving = false;
	Point<float> position, positionLast, destination;
	Point<float> speed;

	float pause = 0.f;
	Capped<float> cooldown{ 0.f, 0.f };
	float angle = 0.f, angleDesired = 0.f, angleMalfunction = 0.f;
	float ionTime = 0.f;

	std::optional<WeaponBlueprint> weapon;
	std::optional<SpaceDroneMovementExtra> extraMovement;
};

struct Drone
{
	static constexpr float HARDCODED_REBUILD_TIME = 10.f;
	static constexpr float HARDCODED_SAFE_MALFUNCTION_TIME = 2.f;

	DroneBlueprint blueprint;
	bool player = false;
	bool powered = false;
	bool dead = false;
	int zoltanPower = 0;
	HackLevel hackLevel = HackLevel::None;
	float hackTime = 0.f;
	Capped<float> destroyTimer{ 0.f, 0.f };

	std::optional<SpaceDroneInfo> space;
};

struct Path
{
	Point<int> start, finish;
	std::vector<int> doors;
	float distance = -1.f;
};

struct CrewBlueprint : Blueprint
{
	std::string name, nameLong, species;
	bool male = false;
	Capped<int> skillPiloting{ 0, 0 };
	Capped<int> skillEngines{ 0, 0 };
	Capped<int> skillShields{ 0, 0 };
	Capped<int> skillWeapons{ 0, 0 };
	Capped<int> skillRepair{ 0, 0 };
	Capped<int> skillCombat{ 0, 0 };
};

struct Crew
{
	int uiBox = -1, selectionState = 0;
	CrewBlueprint blueprint;
	Point<float> position, goal;
	Capped<float> health{ 0.f, 0.f };
	Point<float> speed;
	Path path;
	bool player = false;
	bool onPlayerShip = false;
	bool newPath = false;
	bool suffocating = false;
	bool repairing = false;
	bool intruder = false;
	bool fighting = false;
	bool dead = false;
	bool manning = false;
	bool moving = false;
	bool healing = false;
	int onFire = 0;
	int room = 0, slot = 0; // slot is set when reading room slots... FTL moment
	SystemType mannedSystem = SystemType::None;
	int roomGoal = 0, slotGoal = 0;
	int roomSaved = 0, slotSaved = 0;
	int cloneQueuePosition = -1; // will be set when reading clonebay status
	int deathId = -1; // used to sort the clone queue
	float cloneDeathProgress = 0.f;
	bool readyToClone = false;
	bool mindControlled = false;
	int mindControlHealthBoost = 0;
	float mindControlDamageMultiplier = 1.f;
	float stunTime = 0.f;
	Capped<float> teleportTimer{ 0.f, 0.f };
	bool teleporting = false;
	bool leaving = false;
	bool arriving = false;
	bool drone = false;
};

struct Power
{
	Capped<int> total{ 0, 0 };
	int normal = 0;
	int zoltan = 0;
	int battery = 0;
	int cap = 0;
	int ionLevel = 0;
	Capped<float> ionTimer{ 0.f, 0.f };
	int restoreTo = 0;
};

struct Repairable
{
	float repairProgress = 0.f;
	Point<int> position;
	int room = -1, slot = -1;
};

struct Fire : Repairable
{
	float deathTimer = 0.f;
};

struct Breach : Repairable
{

};

struct Slot
{
	int id = -1;
	Rect<int> rect;
	Point<int> position;
	bool occupiable = true;

	std::vector<Crew*> crewMoving;
	Crew* crew = nullptr;
	Crew* intruder = nullptr;
	std::optional<Fire> fire;
	std::optional<Breach> breach;
};

struct Room
{
	static constexpr int HARDCODED_TILE_SIZE = 35;

	SystemType system = SystemType::None;
	int id = -1;
	int primarySlot = 0, primaryDirection = 0;
	
	Rect<int> rect;
	Point<int> tiles;

	bool player = false;
	bool stunning = false;
	float oxygen = 0.f;
	HackLevel hackLevel = HackLevel::None;

	std::vector<Crew*> crewMoving;
	std::vector<Crew*> crew;
	std::vector<Crew*> intruders;
	float fireRepair = 0, breachRepair = 0;
	int slotsOccupiable = 0;
	std::vector<Slot> slots;

	Slot& slotAt(const Point<int>& position)
	{
		return this->slots[this->slotIdAt(position)];
	}

	const Slot& slotAt(const Point<int>& position) const
	{
		return this->slots[this->slotIdAt(position)];
	}

	int slotIdAt(const Point<int>& position) const
	{
		Point<int> relPos = position;
		relPos.x -= rect.x;
		relPos.y -= rect.y;
		relPos /= HARDCODED_TILE_SIZE;

		return relPos.x + this->tiles.x * relPos.y;
	}
};

struct Door
{
	// 0 - normal doors
	// 1 - blast doors
	// 2 - super doors
	static constexpr std::array<std::array<int, 3>, 3> HARDCODED_HEALTH{ {
		{ 12, 16, 20 }, // easy
		{ 8, 12, 18 },  // normal
		{ 6, 10, 15 }   // hard
	} };

	int id = -1;
	std::pair<int, int> rooms{ -1, -1 };
	int level = 0;
	Capped<int> health{ 0, 0 };
	HackLevel hackLevel = HackLevel::None;
	bool open = false, openFake = false;
	bool ioned = false;
	bool vertical = false;
	bool airlock = false;

	Point<int> position, dimensions;
};

struct System
{
	SystemType type = SystemType::None;
	SystemBlueprint blueprint;
	int room = -1;
	Power power;
	Capped<int> health{ 0, 0 }, level{ 0, 0 };
	int manningLevel = 0;
	bool needsManning = false;
	bool occupied = false;
	bool onFire = false;
	bool breached = false;
	bool boardersAttacking = false;
	float damageProgress = 0.f;
	float repairProgress = 0.f;
};

struct ShieldSystem : System
{
	Ellipse<int> boundary;
	Capped<int> bubbles{ 0, 0 };
	Capped<float> charge{ 0.f, 0.f };
};

struct EngineSystem : System
{
	static constexpr std::array<int, 9> HARDCODED_EVASION_VALUES{
		0, 5, 10, 15, 20, 25, 28, 31, 35
	};

	static constexpr std::array<int, 4> HARDCODED_EVASION_SKILL_BOOST{
		0, 5, 7, 10
	};

	bool boostFTL = false;
};

struct MedbaySystem : System
{
	int slot = -1; // bay takes up a slot
};

struct ClonebaySystem : System
{
	static constexpr float HARDCODED_DEATH_TIME = 3.f;

	std::vector<Crew*> queue;
	Capped<float> cloneTimer{ 0.f, 0.f };
	Capped<float> deathTimer{ 0.f, 0.f };
	int slot = -1; // bay takes up a slot
};

struct OxygenSystem : System
{
	// Probably no extra fields?
};

struct TeleporterSystem : System
{
	int slots = 0;
	int targetRoom = -1;
	std::vector<Crew*> crewPresent;
	bool sending = false;
	bool receiving = false;
};

struct CloakingSystem : System
{
	static constexpr int HARDCODED_EVASION_BONUS = 60;

	bool on = false;
	Capped<float> timer{ 0.f, 0.f };
};

struct ArtillerySystem : System
{
	Weapon weapon;
};

struct MindControlSystem : System
{
	bool on = false;
	Capped<float> timer{ 0.f, 0.f };
	int targetRoom = -1;
	bool targetingPlayerShip = false;
};

struct HackingDrone : Drone
{
	Point<float> start, goal;
	bool arrived = false, setUp = false;
	int room = -1;
};

struct HackingSystem : System
{
	bool on = false;
	Capped<float> timer{ 0.f, 0.f };
	SystemType target = SystemType::None, queued = SystemType::None;
	HackingDrone drone;
};

struct WeaponSystem : System
{
	int slotCount = 0;
	std::vector<Weapon> weapons;
	std::vector<bool> userPowered, repower;
};

struct DroneSystem : System
{
	int slotCount = 0;
	std::vector<Drone> drones;
	std::vector<bool> userPowered, repower;
};

struct PilotingSystem : System
{
	static constexpr std::array<int, 4> HARDCODED_EVASION_SKILL_BOOST{
		0, 5, 7, 10
	};
};

struct SensorSystem : System
{
	// Probably no extra fields?
};

struct DoorSystem : System
{
	// Probably no extra fields?
};

struct BatterySystem : System
{
	bool on = false;
	Capped<float> timer{ 0.f, 0.f };
	int provides = 0, providing = 0;
};

struct Cargo
{
	int scrap = 0, fuel = 0, missiles = 0, droneParts = 0;

	std::vector<Weapon> weapons;
	std::vector<Drone> drones;
	std::vector<Augment> augments;
};

struct Reactor
{
	Capped<int> total{ 0, 0 };
	Capped<int> normal{ 0, 0 };
	Capped<int> battery{ 0, 0 };
	int level = 0, cap = 0;
};

struct Ship
{
	bool player = false;
	bool destroyed = false;
	bool automated = false;
	bool jumping = false;
	bool canJump = false;
	bool canInventory = false;

	Capped<float> jumpTimer{ 0.f, 0.f };

	Capped<int> hull{ 0, 0 };
	Capped<int> superShields{ 0, 0 };

	int evasion = 0;
	float totalOxygen = 0.f;

	std::vector<Room> rooms;
	std::vector<Door> doors;

	Reactor reactor;
	std::optional<ShieldSystem> shields;
	std::optional<EngineSystem> engines;
	std::optional<MedbaySystem> medbay;
	std::optional<ClonebaySystem> clonebay;
	std::optional<OxygenSystem> oxygen;
	std::optional<TeleporterSystem> teleporter;
	std::optional<CloakingSystem> cloaking;
	std::vector<ArtillerySystem> artillery;
	std::optional<MindControlSystem> mindControl;
	std::optional<HackingSystem> hacking;
	std::optional<WeaponSystem> weapons;
	std::optional<DroneSystem> drones;
	std::optional<PilotingSystem> piloting;
	std::optional<SensorSystem> sensors;
	std::optional<DoorSystem> doorControl;
	std::optional<BatterySystem> battery;

	Cargo cargo;
};

struct PauseState
{
	bool any = false, justPaused = false, justUnpaused = false;
	bool normal = false, automatic = false, menu = false, event = false, touch = false;
};

enum class ProjectileType : int
{
	Invalid = -1,
	Miscellaneous = 0,
	Laser = 1,
	Asteroid = 2,
	Missile = 3,
	Bomb = 4,
	Beam = 5,
	ABS = 6,
};

struct Beam
{
	Point<float> begin, end;
	float length = 0.f;
	bool pierced = false;
	bool damagedSuperShield = false;
};

struct Bomb
{
	float explosionTimer = -1.f;
	bool damagedSuperShield = false;
	bool bypassedSuperShield = false;
};

struct Projectile
{
	ProjectileType type = ProjectileType::Invalid;
	Point<float> position, positionLast, target, speed;
	float lifespan = 0.f;
	float heading = -1.f, entryAngle = -1.f;
	float angle = 0.f, spinSpeed = 0.f;
	bool player = false;
	bool playerSpace = false;
	bool playerSpaceIsDestination = false;
	bool dead = false;
	bool missed = false;
	bool hit = false;
	bool passed = false;
	std::optional<Beam> beam;
	std::optional<Bomb> bomb;
};

template<typename T>
struct RandomAmount
{
	T min = T(0), max = T(0);
	float chanceNone = 0.f;
};

struct AsteroidInfo
{
	std::array<RandomAmount<float>, 3> spawnRates, stateLengths;
	int shipCount = 0;
	int state = 0;
	int playerSpace = 0;
	int nextDirection = 0;
	float stateTimer = 0.f, timer = 0.f;
	bool running = false;
	int shieldLevel = 0;
};

enum class EnvironmentType
{
	Invalid = -1,
	Normal = 0,
	Asteroids = 1,
	CloseToSun = 2,
	Nebula = 3,
	IonStorm = 4,
	Pulsar = 5,
	ASB = 6
};

struct Space
{
	std::vector<Projectile> projectiles;
	EnvironmentType environment = EnvironmentType::Normal;
	std::optional<AsteroidInfo> asteroids;
	bool environmentTargetingEnemy = false;
	Capped<float> hazardTimer;
};

struct ShipEvent
{
	//ShipBlueprint ship;
	bool hostile = false;
	RandomAmount<int> surrenderThreshold, escapeThreshold;
};

struct ResourceEvent
{
	int missiles = 0, fuel = 0, droneParts = 0, scrap = 0, crew = 0;
	bool traitor = false, cloneable = false, steal = false, intruders = false;
	std::optional<WeaponBlueprint> weapon;
	std::optional<DroneBlueprint> drone;
	std::optional<Augment> augment;
	std::string crewType;
	int fleetDelay = 0, hullDamage = 0;
	SystemType system = SystemType::None;
	int upgradeAmount = 0;
	std::string removeAugment;
};

struct BoardingEvent
{
	std::string crewType;
	int min = 0, max = 0, amount = 0;
	bool breach = false;
};

enum class StoreBoxType
{
	Invalid = -1,
	Weapon = 0,
	Drone = 1,
	Augment = 2,
	Crew = 3,
	System = 4,
	Item = 5
};

struct StoreBox
{
	StoreBoxType type = StoreBoxType::Invalid;
	int actualPrice = 0; // may differ from blueprint!
	int id = 0;
	bool page2 = false;

	std::optional<WeaponBlueprint> weapon;
	std::optional<DroneBlueprint> drone; // may be present with drone system!
	std::optional<Augment> augment;
	std::optional<CrewBlueprint> crew;
	std::optional<SystemBlueprint> system;
};

struct Store
{
	static constexpr int HARDCODED_BOXES_PER_SECTION = 3;

	std::vector<StoreBox> boxes;
	std::vector<StoreBoxType> sections;
	int fuel = 0, fuelCost = 0;
	int missiles = 0, missileCost = 0;
	int droneParts = 0, dronePartCost = 0;
	int repairCost = 0, repairCostFull = 0;
	bool page2 = false;
};

struct EventDamage
{
	SystemType system;
	int amount = 0, effect = 0;
};

struct LocationEvent;

struct Choice
{
	std::shared_ptr<LocationEvent> event; // would've used unique but idk how to with pybind
	std::string requiredObject;
	int levelMin = 0, levelMax = std::numeric_limits<int>::max();
	int maxGroup = std::numeric_limits<int>::max();
	bool blue = false;
	bool hiddenReward = false;
};

namespace raw
{

struct Store;

}

struct LocationEvent
{
	EnvironmentType environment = EnvironmentType::Normal;
	bool environmentTargetsEnemy = false;
	bool exit = false;
	bool distress = false;
	bool revealMap = false;
	bool repair = false;
	int unlockShip = -1;

	std::optional<ShipEvent> ship;
	ResourceEvent resources, reward;
	BoardingEvent boarders;
	std::optional<Store> store;

	std::vector<EventDamage> damage;
	std::vector<Choice> choices;

	// Some implementation stuffs
	raw::Store* _storePtr = nullptr;
};

struct Location
{
	int id = -1;
	Point<float> position;
	std::vector<Location*> neighbors;
	int visits = 0;
	bool known = false;
	bool exit = false;
	bool hazard = false;
	bool nebula = false;
	bool flagshipPresent = false;
	bool quest = false;
	bool fleetOvertaking = false;
	bool enemyShip = false;
	LocationEvent event;
};

enum class SectorType
{
	Invalid = -1,
	Civilian, Hostile, Nebula
};

struct Sector
{
	int id = -1;
	SectorType type = SectorType::Invalid;
	std::string name;
	bool visited = false;
	bool reachable = false;
	std::vector<Sector*> neighbors;
	Point<int> position;
	int level = -1;
	bool unique = false;
};

struct StarMap
{
	std::vector<Location> locations;
	bool lastStand = false;
	bool flagshipJumping = false;
	bool mapRevealed = false;
	bool secretSector = false;
	std::vector<Location*> flagshipPath;
	Point<float> translation;
	Ellipse<float> dangerZone; // actually a circle
	int pursuitDelay = 0;
	int turnsLeft = -1;
	std::vector<Sector> sectors;
	Location* currentLocation = nullptr;
	Sector* currentSector = nullptr;
	bool choosingNewSector = false;
	bool infiniteMode = false;
	bool nebulaSector = false;
	bool distressBeacon = false;
	int sectorNumber = -1;
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

struct Settings
{
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
	bool advancedEditionEnabled = false;
	std::string language = "en";
	Point<int> screenSize;
	EventChoiceSelection eventChoiceSelection = EventChoiceSelection::DisableHotkeys;
	std::map<std::string, Key> hotkeys;
};

struct Blueprints
{
	//int rarityTotal = 0;
	//gcc::map<gcc::string, ShipBlueprint> shipBlueprints;
	std::map<std::string, WeaponBlueprint> weaponBlueprints;
	std::map<std::string, DroneBlueprint> droneBlueprints;
	std::map<std::string, Augment> augmentBlueprints;
	std::map<std::string, CrewBlueprint> crewBlueprints;
	//gcc::map<gcc::string, bool> nameList;
	//gcc::map<gcc::string, gcc::string> shortNames;
	//gcc::map<gcc::string, gcc::map<gcc::string, bool>> languageNameLists;
	//gcc::map<gcc::string, ItemBlueprint> itemBlueprints;
	std::map<std::string, SystemBlueprint> systemBlueprints;
	//gcc::map<gcc::string, gcc::vector<gcc::string>> blueprintLists;
	//gcc::vector<gcc::string> currentNames;
};

struct State
{
	bool running = false;
	std::optional<Game> game;
	Settings settings;
	Blueprints blueprints;
};
