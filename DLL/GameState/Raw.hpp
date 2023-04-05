#pragma once
#pragma once

#include <cstdint>
#include <cstddef>
#include <utility>
#include <array>
#include <stdexcept>

namespace raw
{

template<size_t N>
struct Pad
{
	std::byte bytes[N];
};

using vptr = void**;

// recreations of the memory layout of gcc's stdlib implementations
// not full-fledged recreations of their implementation
namespace gcc
{

struct string
{
	static constexpr size_t SMALL_STRING_OPTIMIZATION_LEN = 15;

	char* str = nullptr;
	size_t len = 0;

	union
	{
		char local_data[SMALL_STRING_OPTIMIZATION_LEN + 1];
		size_t allocated_capacity;
	};
};

template<typename T>
struct vector
{
	T* begin = nullptr;
	T* end = nullptr;
	T* capacity = nullptr;

	size_t size() const
	{
		return (reinterpret_cast<size_t>(this->end) - reinterpret_cast<size_t>(this->begin)) / sizeof(T);
	}

	bool empty() const
	{
		return this->size() == 0;
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

template<>
struct vector<bool>
{
	char* begin = nullptr;
	size_t beginBit = 0;
	char* end = nullptr;
	size_t endBit = 0;
	char* capacity = nullptr;

	size_t size() const
	{
		size_t bytesUsed = reinterpret_cast<size_t>(this->end) - reinterpret_cast<size_t>(this->begin);

		return bytesUsed * CHAR_BIT + endBit - beginBit - 1;
	}

	bool empty() const
	{
		return this->size() == 0;
	}

	bool operator[](size_t i)
	{
		size_t byte = i / CHAR_BIT;
		size_t bit = i % CHAR_BIT;

		return (this->begin[byte] >> bit) & 1;
	}

	const bool operator[](size_t i) const
	{
		size_t byte = i / CHAR_BIT;
		size_t bit = i % CHAR_BIT;

		return (this->begin[byte] >> bit) & 1;
	}
};

template<typename T>
struct queue
{
	Pad<40> _u0; // dunno how queue works yet
};

namespace impl
{

enum class rb_color { red = 0, black = 1 };

template<typename Key, typename Value>
struct map_node
{
	using value_type = std::pair<const Key, Value>;
	rb_color color = rb_color::red;
	map_node* parent = nullptr;
	map_node* left = nullptr;
	map_node* right = nullptr;
	value_type data;
};

}

template<typename Key, typename Value, typename Compare = std::less<Key>>
class map
{
	using node_type = impl::map_node<Key, Value>;

	size_t _allocatorStuffDunnoWhatThisIsForExactly = 0;
	impl::rb_color color = impl::rb_color::red;
	node_type* root = nullptr;
	node_type* leftMost = nullptr;
	node_type* rightRight = nullptr;
	size_t count = 0;

public:
	using value_type = std::pair<const Key, Value>;
	using key_compare = Compare;

	size_t size() const
	{
		return count;
	}

	bool empty() const
	{
		return this->size() == 0;
	}

	template<typename F>
	void dfs(F callback) const
	{
		this->dfsImpl(callback, this->root);
	}

	Value& operator[](const Key& key)
	{
		return this->extract(key);
	}

	const Value& operator[](const Key& key) const
	{
		return this->extract(key);
	}

private:
	template<typename F>
	void dfsImpl(F f, node_type* node = nullptr) const
	{
		if (!node) return;
		this->dfsImpl(f, node->left);
		f(node->data.first, node->data.second);
		this->dfsImpl(f, node->right);
	}

	Value& extract(const Key& key) const
	{
		key_compare cmp;
		node_type* node = this->root;

		while (node)
		{
			if (node->data.first == key)
			{
				break;
			}
			else if (cmp(key, node->data.first))
			{
				node = node->left;
			}
			else
			{
				node = node->right;
			}
		}

		if (!node) throw std::out_of_range("map does not have key");
		return node->data.second;
	}
};

};

struct GL_FrameBuffer
{

};

struct GL_Texture
{
	int id_ = 0;
	int width_ = 0;
	int height_ = 0;
	bool isLogical_ = 0;
	Pad<3> _u0;
	float u_base_ = 0.f;
	float v_base_ = 0.f;
	float u_size_ = 0.f;
	float v_size_ = 0.f;
};

struct GL_Color
{
	float r = 0.f, g = 0.f, b = 0.f, a = 0.f;
};

struct GL_Primitive
{
	int type = 0;
	float lineWidth = 0.f;
	bool hasTexture = false;
	Pad<3> _u0;
	GL_Texture* texture = nullptr;
	bool textureAntialias = false;
	bool hasColor = false;
	Pad<2> _u1;
	GL_Color color;
	int id = 0;
};

struct Point
{
	int x = 0, y = 0;
};

struct Pointf
{
	float x = 0, y = 0;
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

struct CachedPrimitive
{
	vptr _vptr = nullptr;
	GL_Primitive* primitive = nullptr;
};

struct CachedRect : CachedPrimitive
{
	int x = 0, y = 0, w = 0, h = 0;
};

struct CachedRectOutline : CachedPrimitive
{
	int x = 0, y = 0, w = 0, h = 0;
	int thickness = 0;
};

struct CachedImage : CachedPrimitive
{
	gcc::string imageName;
	GL_Texture* texture = nullptr;
	int x = 0, y = 0;
	float wScale = 0.f, hScale = 0.f;
	float x_start = 0.f, y_start = 0.f;
	float x_size = 0.f, y_size = 0.f;
	float rotation = 0.f;
	bool mirrored = false;
	Pad<3> _u0;
};

struct ImageDesc
{
	GL_Primitive* tex = nullptr;
	int resId = 0;
	int w = 0, h = 0, x = 0, y = 0, rot = 0;
};

struct AnimationTracker
{
	vptr _vptr = nullptr;
	float time = 0.f;
	bool loop = false;
	Pad<3> _u0;
	float current_time = 0.f;
	bool running = false;
	bool reverse = false;
	bool done = false;
	Pad<1> _u1;
	float loopDelay = 0.f;
	float currentDelay = 0.f;
};

struct TimerHelper
{
	int maxTime = 0, minTime = 0;
	float currTime = 0.f, currGoal = 0.f;
	bool loop = false;
	bool running = false;
	Pad<2> _u0;
};

struct AnimationDescriptor
{
	int numFrames = 0;
	int imageWidth = 0, imageHeight = 0;
	int stripStartY = 0, stripStartX = 0;
	int frameWidth = 0, frameHeight = 0;
};

struct Animation
{
	GL_Texture* animationStrip = nullptr;
	AnimationDescriptor info;
	AnimationTracker tracker;
	Pointf position;
	gcc::string soundForward;
	gcc::string soundReverse;
	bool randomizeFrames = false;
	Pad<3> _u0;
	float fScale = 0.f;
	float fYStretch = 0.f;
	int currentFrame = 0;
	bool bAlwaysMirror = false;
	gcc::vector<gcc::vector<gcc::string>> soundQueue;
	float fadeOut = 0.f;
	float startFadeOut = 0.f;
	gcc::string animName;
	int mask_x_pos = 0, mask_x_size = 0;
	int mask_y_pos = 0, mask_y_size = 0;
	GL_Primitive* primitive = nullptr;
	GL_Primitive* mirroredPrimitive = nullptr;
};

struct CEvent
{
	vptr _vptr = nullptr;
};

struct GenericButton
{
	vptr _vptr = nullptr;
	Point position;
	Rect hitbox;
	bool allowAnyTouch = false;
	bool touchSelectable = false;
	bool bRenderOff = false;
	bool bRenderSelected = false;
	bool bFlashing = false;
	Pad<3> _u0;
	AnimationTracker flashing;
	bool bActive = false;
	bool bHover = false;
	bool bActivated = false;
	bool bSelected = false;
	int activeTouch = -1;
};

struct Button : GenericButton
{
	GL_Texture* images[3]{ nullptr, nullptr, nullptr };
	GL_Primitive* primitives[3]{ nullptr, nullptr, nullptr };
	Point imageSize;
	bool bMirror = false;
	Pad<3> _u0;
};

struct TextString
{
	gcc::string data;
	bool isLiteral = false;
	Pad<3> _u0;
};

struct TextButton : GenericButton
{
	GL_Primitive* primitives[3]{ nullptr, nullptr, nullptr };
	GL_Texture* baseImage = nullptr;
	Point baseImageOffset;
	GL_Primitive* basePrimitive = nullptr;
	bool colorsSet = false;
	Pad<3> _u0;
	GL_Color colors[3]{};
	GL_Color textColor;
	Point buttonSize;
	int cornerInset = 0;
	bool autoWidth = false;
	Pad<3> _u1;
	int autoWidthMargin = 0;
	int autoWidthMin = 0;
	bool autoRightAlign = false;
	Pad<3> _u2;
	TextString label;
	int font = 0;
	int lineHeight = 0;
	int textYOffset = 0;
	bool autoShrink = false;
	Pad<3> _u3;
};

struct TextInput
{
	gcc::string prompt;
	gcc::vector<int> text, oldText;
	int pos = 0, lastPos = 0;
	bool bActive = false;
	Pad<3> _u0;
	int allowedChars = 0, maxChars = 0;
	TimerHelper blinker;
};

struct SlideBar
{
	Rect box;
	bool hovering = false, holding = false;
	Pad<2> _u0;
	Rect marker;
	Point mouseStart, rectStart;
	std::pair<int, int> minMax;
};

struct FocusWindow
{
	vptr _vptr = nullptr;
	bool bOpen = false;
	bool bFullFocus = false;
	Pad<2> _u0;
	Point close;
	bool bCloseButtonSelected = false;
	Pad<3> _u1;
	Point position;
};

struct ConfirmWindow : FocusWindow
{
	TextString text;
	int textHeight = 0;
	int minWidth = 0;
	int windowWidth = 0;
	TextString yesText, noText;
	bool autoCenter;
	Pad<3> _u0;
	GL_Texture* windowImage = nullptr;
	GL_Primitive* window = nullptr;
	TextButton yesButton, noButton;
	bool result = false;
	Pad<3> _u1;
};

struct WindowFrame
{
	Rect rect;
	GL_Primitive* outline = nullptr;
	GL_Primitive* mask = nullptr;
	GL_Primitive* pattern = nullptr;
};

struct TabbedWindow : FocusWindow
{
	gcc::vector<Button*> buttons;
	gcc::vector<FocusWindow*> windows;
	gcc::vector<gcc::string> names;
	unsigned currentTab = 0;
	int buttonType = 0;
	TextButton doneButton;
	Point move;
	bool bBlockClose = false;
	bool bTutorialMode = false;
	bool bWindowLock = false;
	Pad<1> _u0;
};

struct Description
{
	TextString title, shortTitle, description;
	int cost = 0;
	int rarity = 0, baseRarity = 0;
	int bp = 0;
	bool locked = false;
	Pad<3> _u0;
	TextString tooltip;
	gcc::string tip;
};

struct InputBox : FocusWindow
{
	WindowFrame* textBox = nullptr;
	gcc::string mainText;
	bool bDone = false;
	bool bInvertCaps = false;
	Pad<2> _u0;
	gcc::string inputText;
	gcc::vector<std::string> lastInputs;
	int lastInputIndex = 0;
};

struct DamageMessage
{
	AnimationTracker tracker;
	Pointf position;
	GL_Color color;
	bool bFloatDown = false;
	Pad<3> _u0;
	gcc::vector<GL_Primitive*> primitives;
};

struct Particle
{
	float position_x = 0.f, position_y = 0.f;
	float speed_x = 0.f, speed_y = 0.f;
	float acceleration_x = 0.f, acceleration_y = 0.f;
	float lifespan = 0.f;
	bool alive = false;
	Pad<3> _u0;
};

struct ParticleEmitter
{
	Particle particles[64];
	float birthRate = 0.f;
	float birthCounter = 0.f;
	float lifespan = 0.f;
	float speedMag = 0.f;
	float position_x = 0.f, position_y = 0.f;
	float max_dx = 0.f, min_dx = 0.f, max_dy = 0.f, min_dy = 0.f;
	int image_x = 0, image_y = 0;
	GL_Primitive* primitive = nullptr;
	float emitAngle = 0.f;
	bool randAngle = false;
	bool running = false;
	Pad<2> _u0;
	float maxAlpha = 0.f;
	float minSize = 0.f, maxSize = 0.f;
	int currentCount = 0;
};

struct ShipObject
{
	vptr _vptr = nullptr;
	int iShipId = 0;
};

struct Targetable
{
	vptr _vptr = nullptr;
	int type = 0;
	bool hostile = false;
	bool targeted = false;
	Pad<2> _u0;
};

struct Collideable
{
	vptr _vptr = nullptr;
};

struct Blueprint
{
	vptr _vptr = nullptr;
	gcc::string name;
	Description desc;
	int type = -1;
};

struct EffectsBlueprint
{
	gcc::vector<gcc::string> launchSounds;
	gcc::vector<gcc::string> hitShipSounds;
	gcc::vector<gcc::string> hitShieldSounds;
	gcc::vector<gcc::string> missSounds;
	gcc::string image;
};

struct CrewBlueprint : Blueprint
{
	TextString crewName, crewNameLong;
	gcc::vector<TextString> powers;
	bool male = false;
	Pad<3> _u0;
	gcc::vector<std::pair<int, int>> skillLevel;
	gcc::vector<gcc::vector<GL_Color>> colorLayers;
	gcc::vector<int> colorChoices;
};

struct MiniProjectile
{
	gcc::string image;
	bool fake;
	Pad<3> _u0;
};

struct BoostPower
{
	int type = 0;
	float amount = 0.f;
	int count = 0;
};

struct Damage
{
	int iDamage = 0;
	int iShieldPiercing = 0;
	int fireChance = 0;
	int breachChance = 0;
	int stunChance = 0;
	int iIonDamage = 0;
	int iSystemDamage = 0;
	int iPersDamage = 0;
	bool bHullBuster = false;
	Pad<3> _u0;
	int ownerId = 0;
	int selfId = 0;
	bool bLockdown = false;
	bool crystalShard = false;
	bool bFriendlyFire = false;
	Pad<1> _u1;
	int iStun = 0;
};

struct WeaponBlueprint : Blueprint
{
	gcc::string typeName;
	Damage damage;
	int shots = 0;
	int missiles = 0;
	float cooldown = 0.f;
	int power = 0;
	int length = 0;
	float speed = 0.f;
	int miniCount = 0;
	EffectsBlueprint effects;
	gcc::string weaponArt;
	gcc::string combatIcon;
	gcc::string explosion;
	int radius = 0;
	gcc::vector<MiniProjectile> miniProjectiles;
	BoostPower boostPower;
	int drone_targetable = 0;
	int spin = 0;
	int chargeLevels = 0;
	TextString flavorType;
	GL_Color color;
};

struct DroneBlueprint : Blueprint
{
	gcc::string typeName;
	int level = 0;
	int targetType = 0;
	int power = 0;
	float cooldown = 0.f;
	int speed = 0;
	int dodge = 0;
	gcc::string weaponBlueprint, droneImage, combatIcon;
};

struct AugmentBlueprint : Blueprint
{
	float value = 0.f;
	bool stacking = false;
	Pad<3> _u0;
};

struct ItemBlueprint : Blueprint
{

};

struct WeaponMount
{
	Point position;
	bool mirror = false;
	bool rotate = false;
	Pad<2> _u0;
	int slide = 0;
	int gib = 0;
};

struct WeaponAnimation
{
	Animation anim;
	bool bFireShot = false;
	bool bFiring = false;
	Pad<2> _u0;
	float fChargeLevel = 0.f;
	int iChargedFrame = 0;
	int iFireFrame = 0;
	bool bMirrored = false;
	bool bRotation = false;
	Pad<2> _u1;
	Point fireLocation;
	bool bPowered = false;
	Pad<3> _u2;
	Point mountPoint;
	Point renderPoint;
	Point fireMountVector;
	AnimationTracker slideTracker;
	int slideDirection = 0;
	CachedImage iChargeImage;
	Animation explosionAnim;
	WeaponMount mount;
	float fDelayChargeTime = 0.f;
	Animation boostAnim;
	int boostLevel = -1;
	bool bShowcharge = false;
	Pad<3> _u3;
	float fActualChargeLevel = 0.f;
	int iChargeOffset = 0;
	int iChargeLevels = 0;
	int currentOffset = 0;
	CachedImage chargeBox, chargeBar;
	int iHackLevel = 0;
	Animation hackSparks;
	bool playerShip = false;
	Pad<3> _u4;
};

struct Projectile : Collideable, Targetable
{
	Pointf position, last_position;
	float speed_magnitude;
	Pointf target;
	float heading;
	int ownerId = 0;
	unsigned selfId = 0;
	Damage damage;
	float lifespan = 0.f;
	int destinationSpace = 0, currentSpace = 0;
	int targetId = 0;
	bool dead = false;
	Pad<3> _u0;
	Animation death_animation;
	Animation flight_animation;
	Pointf speed;
	bool missed = false;
	bool hitTarget = false;
	Pad<2> _u1;
	gcc::string hitSolidSound, hitShieldSound, missSound;
	float entryAngle = 0.f;
	bool startedDeath = false;
	bool passedTarget = false;
	bool bBroadcastTarget = false;
	Pad<1> _u2;
	AnimationTracker flashTracker;
	GL_Color color;

	int getType() const
	{
		if (!this->Collideable::_vptr) return -1;

		auto func = reinterpret_cast<int(*)(const Projectile*)>(this->Collideable::_vptr[31]);

		return (*func)(this);
	}
};

struct LaserBlast : Projectile
{
	Targetable* movingTarget = nullptr;
	float spinAngle = 0.f, spinSpeed = 0.f;
};

struct Asteroid : Projectile
{
	GL_Texture* imageId = nullptr;
	float angle = 0.f;
};

struct Missile : Projectile
{

};

struct BombProjectile : Projectile
{
	bool bMissed = false;
	Pad<3> _u0;
	DamageMessage* missMessage = nullptr;
	float explosiveDelay = 0.f;
	bool bSuperShield = false;
	bool superShieldBypass = false;
	Pad<2> _u1;
};

struct CollisionResponse
{
	int collision_type = 0;
	Pointf point;
	int damage = 0, superDamage = 0;
};

struct BeamWeapon : Projectile
{
	Pointf sub_end, sub_start, shield_end, final_end, target2, target3;
	float lifespan = 0.f;
	float length = 0.f;
	float dh = 0.f;
	CollisionResponse collision;
	int soundChannel = 0;
	gcc::vector<Animation> contactAnimations;
	float animationTimer = 0.f;
	int lastDamage = 0;
	Targetable* movingTarget = nullptr;
	float start_heading = 0.f;
	float timer = 0.f;
	WeaponAnimation* weapAnimation = nullptr;
	bool piercedShield = false;
	bool oneSpace = false;
	bool bDamageSuperShield = false;
	Pad<1> _u0;
	int movingTargetId = 1;
	bool checkedCollision = false;
	Pad<3> _u1;
	gcc::vector<Animation> smokeAnims;
	Pointf lastSmokeAnim;
};

struct PDSFire : LaserBlast
{
	Pointf startPoint;
	bool passedTarget = false;
	Pad<3> _u0;
	float currentScale = 1.f;
	bool missed = false;
	Pad<3> _u1;
	Animation explosionAnimation;
};

struct CloakingSystem;

struct ProjectileFactory : ShipObject
{
	std::pair<float, float> cooldown, subCooldown;
	float baseCooldown = 0.f;
	WeaponBlueprint* blueprint = nullptr;
	Point localPosition;
	Animation flight_animation;
	bool autoFiring = false;
	bool fireWhenReady = false;
	bool powered = false;
	Pad<1> _u0;
	int requiredPower = 0;
	gcc::vector<Pointf> targets, lastTargets;
	int targetId = -1;
	int iAmmo = 0;
	gcc::string name;
	int numShots = 0;
	float currentFiringAngle = 0.f, currentEntryAngle = 0.f;
	Targetable* currentShipTarget = nullptr;
	CloakingSystem* cloakingSystem = nullptr;
	WeaponAnimation weaponVisual;
	WeaponMount mount;
	gcc::vector<Projectile*> queuedProjectiles;
	int iBonusPower = 0;
	bool bFiredOnce = false;
	Pad<3> _u1;
	int iSpendMissile = 0;
	float cooldownModifier = 0.f;
	int shotsFiredAtTarget = 0;
	int radius = 0;
	int boostLevel = 0;
	int lastProjectileId = -1;
	int chargeLevel = 0;
	int iHackLevel = 0;
	int goalChargeLevel = 0;
	bool isArtillery = false;
	Pad<3> _u2;
};

struct Drone
{
	vptr _vptr = nullptr;
	int iShipId = 0;
	int selfId = 0;
	bool powered = false;
	Pad<3> _u0;
	int powerRequired = 0;
	bool deployed = false;
	Pad<3> _u1;
	int type = 0;
	DroneBlueprint* blueprint = nullptr;
	bool bDead = false;
	Pad<3> _u2;
	int iBonusPower = 0;
	bool poweredAtLocation = false;
	Pad<3> _u3;
	float destroyedTimer = 0.f;
	int iHackLevel = 0;
	float hackTime = 0.f;
};

struct SpaceDrone : Drone, Targetable, Collideable
{
	int currentSpace = 0, destinationSpace = 0;
	Pointf currentLocation, lastLocation, destinationLocation;
	Pointf pointTarget;
	Animation explosion;
	Targetable* weaponTarget = nullptr;
	Pointf targetLocation;
	Pointf targetSpeed;
	Targetable* movementTarget = nullptr;
	Pointf speedVector;
	bool poweredLastFrame = false;
	bool deployedLastFrame = false;
	bool bFire = false;
	Pad<1> _u0;
	float pause = 0.f, additionalPause = 0.f;
	float weaponCooldown = 0.f;
	float current_angle = 0.f;
	float aimingAngle = 0.f, lastAimingAngle = 0.f, desiredAimingAngle = 0.f;
	DamageMessage* message = nullptr;
	Animation weapon_animation;
	WeaponBlueprint* weaponBlueprint;
	int lifespan = 0;
	bool bLoadedPosition = false;
	bool bDisrupted = false;
	Pad<2> _u1;
	float hackAngle = 0.f;
	float ionStun = 0.f;
	Pointf beamCurrentTarget, beamFinalTarget;
	float beamSpeed;
	Animation hackSparks;
};

struct CombatDrone : SpaceDrone
{
	Pointf lastDestination;
	float progressToDestination;
	float heading;
	float oldHeading;
	CachedImage drone_image_off;
	CachedImage drone_image_charging;
	CachedImage drone_image_on;
	CachedImage engine_image;
};

struct SystemBlueprint : Blueprint
{

};

struct InfoBox
{
	Point location;
	SystemBlueprint* blueprint = nullptr;
	Description desc;
	int tempUpgrade = 0;
	int powerLevel = 0;
	int maxPower = 0;
	int systemId = 0;
	int systemWidth = 0;
	int yShift = 0;
	Point descBoxSize;
	CrewBlueprint* pCrewBlueprint = nullptr;
	gcc::string warnining;
	bool bDetailed = false;
	Pad<3> _u0;
	gcc::string additionalTip, additionalWarning;
	WindowFrame* primaryBox = nullptr;
	int primaryBoxOffset = 0;
	WindowFrame* secondaryBox = nullptr;
	gcc::string droneBlueprint;
};

struct ResourceEvent
{
	int missiles = 0;
	int fuel = 0;
	int drones = 0;
	int scrap = 0;
	int crew = 0;
	bool traitor = false;
	bool cloneable = false;
	Pad<2> _u0;
	TextString cloneText;
	gcc::string crewType;
	WeaponBlueprint* weapon = nullptr;
	DroneBlueprint* drone = nullptr;
	AugmentBlueprint* augment = nullptr;
	CrewBlueprint crewBlue;
	int systemId = -1;
	int weaponCount = 0;
	int droneCount = 0;
	bool steal = false;
	bool intruders = false;
	Pad<2> _u1;
	int fleetDelay = 0;
	int hullDamage = 0;
	int upgradeAmount = 0;
	int upgradeId = 0;
	int upgradeSuccessFlag = 0;
	gcc::string removeItem;
};

struct ShipSystem
{
	vptr _vptr = nullptr;
	int selectedState = 0;
	ShipObject _shipObj;
	float fDamage = 0.f;
	Point pLoc;
	float fMaxDamage = 0.f;
	gcc::string name;
	int roomId = -1;
	int iRepairCount = 0;
	int iSystemType = -1;
	bool bNeedsManned = false;
	bool bManned = false;
	Pad<2> _u0;
	int iActiveManned = 0;
	bool bBoostable = false;
	Pad<3> _u1;
	std::pair<int, int> powerState{ 0, 0 };
	int iRequiredPower = 0;
	GL_Texture* imageIcon = nullptr;
	GL_Primitive* iconPrimitive = nullptr;
	GL_Primitive* iconBorderPrimitive = nullptr;
	GL_Primitive* iconPrimitives[20]{
		nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr
	};
	CachedRect partialDamageRect;
	CachedRectOutline lockOutline;
	Rect roomShape;
	bool bOnFire = false;
	bool bBreached = false;
	Pad<2> _u2;
	std::pair<int, int> healthState{ 0, 0 };
	float fDamageOverTime = 0.f;
	float fRepairOverTime = 0.f;
	bool damagedLastFrame = false;
	bool repairedLastFrame = false;
	Pad<2> _u3;
	int originalPower = 0;
	bool bNeedsPower = false;
	int iTempPowerCap = 0;
	int iTempPowerLoss = 0;
	int iTempDividePower = 0;
	int iLockCount = 0;
	TimerHelper lockTimer;
	bool bExploded = false;
	bool bOccupied = false;
	bool bFriendlies = false;
	Pad<1> _u4;
	gcc::string interiorImageName;
	GL_Primitive* interiorImage = nullptr;
	GL_Primitive* interiorImageOn = nullptr;
	GL_Primitive* interiorImageManned = nullptr;
	GL_Primitive* interiorImageFancy = nullptr;
	int lastUserPower = 0;
	int iBonusPower = 0;
	int iLastBonusPower = 0;
	Pointf location;
	int bpCost = 0;
	AnimationTracker flashTracker;
	int maxLevel = 0;
	int iBatteryPower = 0;
	int iHackEffect = 0;
	bool bUnderAttack = false;
	bool bLevelBoostable = false;
	bool bTriggerIon = false;
	Pad<1> _u5;
	gcc::vector<Animation> damagingEffects;
	int computerLevel = -1;
};

struct OxygenSystem : ShipSystem
{
	float max_oxygen = 0.f;
	gcc::vector<float> oxygenLevels;
	float fTotalOxygen = 0.f;
	bool bLeakingO2 = false;
	Pad<3> _u0;
};

struct TeleportSystem : ShipSystem
{
	float chargeLevel = 0.f;
	bool bCanSend = false;
	bool bCanReceive = false;
	Pad<2> _u0;
	int iArmed = 0;
	gcc::vector<bool> crewSlots;
	int iPreparedCrew = 0;
	int iNumSlots = 0;
	bool bSuperShields = false;
	Pad<3> _u1;
};

struct CloakingSystem : ShipSystem
{
	bool bTurnedOn = false;
	Pad<3> _u0;
	TimerHelper timer;
	gcc::string soundeffect;
	AnimationTracker glowTracker;
	GL_Primitive* glowImage = nullptr;
};

struct BatterySystem : ShipSystem
{
	bool bTurnedOn = false;
	Pad<3> _u0;
	TimerHelper timer;
	gcc::string soundeffect;
};

struct CrewMember;

struct MindSystem : ShipSystem
{
	std::pair<float, float> controlTimer{ 0.f, 0.f };
	bool bCanUse = false;
	Pad<3> _u0;
	int iArmed = 0;
	gcc::vector<CrewMember*> controlledCrew;
	bool bSuperShields = false;
	bool bBlocked = false;
	Pad<2> _u1;
	int iQueuedTarget = 0;
	int iQueuedShip = 0;
	gcc::vector<CrewMember*> queuedCrew;
};

struct CloneSystem : ShipSystem
{
	float fTimeToClone = 0.f;
	CrewMember* clone = nullptr;
	float fTimeGoal = 0.f;
	float fDeathTime = 0.f;
	GL_Texture* bottom = nullptr;
	GL_Texture* top = nullptr;
	GL_Texture* gas = nullptr;
	int slot = 0;
	Animation* currentCloneAnimation = nullptr;
	gcc::map<gcc::string, Animation> cloneAnimations;
};

struct HackingDrone : SpaceDrone
{
	Pointf startingPosition;
	GL_Texture* droneImage_on = nullptr;
	GL_Texture* droneImage_off = nullptr;
	GL_Texture* lightImage = nullptr;
	Pointf finalDestination;
	bool arrive = false;
	bool finishedSetup = false;
	Pad<2> _u0;
	AnimationTracker flashTracker;
	Animation flying;
	Animation extending;
	Animation explosion;
	int prefRoom = 0;
};

struct HackingSystem : ShipSystem
{
	bool bHacking = false;
	Pad<3> _u0;
	HackingDrone drone;
	bool bBlocked = false;
	bool bArmed = false;
	Pad<2> _u1;
	ShipSystem* currentSystem = nullptr;
	std::pair<float, float> effectTimer{ 0.f, 0.f };
	bool bCanHack = false;
	Pad<3> _u2;
	ShipSystem* queuedSystem = nullptr;
	int spendDrone = 0;
};

struct ShieldPower
{
	int first = 0, second = 0;
	std::pair<int, int> super{ 0,0 };
};

struct Shields : ShipSystem
{
	struct Shield
	{
		float charger = 0.f;
		ShieldPower power;
		float superTime = -1.f;
	};

	struct ShieldAnimation
	{
		Point location;
		float current_size = 0.f, end_size = 0.f;
		float current_thickness = 0.f, end_thickness = 0.f;
		float length = 0.f;
		float dx = 0.f;
		int size = -1;
		int ownerId = -1;
		int damage = 0;
	};

	float ellipseRatio = 0.f;
	Point center;
	Ellipse baseShield;
	int iHighlightedSide = -1;
	float debug_x = 0.f, debug_y = 0.f;
	Shield shields;
	bool shields_shutdown = false;
	Pad<3> _u0;
	gcc::vector<ShieldAnimation> shieldHits;
	AnimationTracker shieldsDown;
	bool superShieldDown = false;
	Pad<3> _u1;
	Pointf shieldsDownPoint;
	AnimationTracker shieldsup;
	GL_Texture* shieldImage = nullptr;
	GL_Primitive* shieldPrimitive = nullptr;
	gcc::string shieldImageName;
	bool bEnemyPresent = false;
	Pad<3> _u2;
	gcc::vector<DamageMessage*> damMessages;
	bool bBarrierMode = false;
	Pad<3> _u3;
	float lastHitTimer = 0.f;
	float chargeTime = 0.f;
	int lastHitShieldLevel = 0;
	AnimationTracker superShieldUp;
	Point superUpLoc;
	bool bExcessChargeHack = false;
	Pad<3> _u4;
};

struct WeaponSystem : ShipSystem
{
	Pointf target;
	gcc::vector<ProjectileFactory*> weapons;
	gcc::vector<ProjectileFactory*> weaponsTrashList;
	float shot_timer = 0.f;
	int shot_count = 0;
	int missile_count = 0;
	int missile_start = 0;
	CloakingSystem* cloakingSystem = nullptr;
	gcc::vector<bool> userPowered;
	int slot_count = 0;
	int iStartingBatteryPower = 0;
	gcc::vector<bool> repowerList;
};

struct DroneSystem : ShipSystem
{
	gcc::vector<Drone*> drones;
	int drone_count = 0;
	int drone_start = 0;
	Targetable* targetShip = nullptr;
	gcc::vector<bool> userPowered;
	int slot_count = 0;
	int iStartingBatteryPower = 0;
	gcc::vector<bool> repowerList;
};

struct EngineSystem : ShipSystem
{
	bool bBoostFTL = false;
	Pad<3> _u0;
};

struct MedbaySystem : ShipSystem
{

};

struct ArtillerySystem : ShipSystem
{
	ProjectileFactory* projectileFactory = nullptr;
	Targetable* target = nullptr;
	bool bCloaked = false;
	Pad<3> _u0;
};

struct Selectable
{
	vptr _vptr = nullptr;
	int selectedState = 0;
};

struct Repairable : Selectable
{
	ShipObject shipObj;
	float fDamage = 0;
	Point pLoc;
	float fMaxDamage = 0.f;
	gcc::string name;
	int roomId = 0;
	int iRepairCount = 0;
};

struct Spreadable : Repairable
{
	gcc::string soundName;
};

struct Fire : Spreadable
{
	float fDeathTimer = 0.f;
	float fStartTimer = 0.f;
	float fOxygen = 0.f;
	Animation fireAnimation;
	Animation smokeAnimation;
	bool bWasOnFire = false;
	Pad<3> _u0;
};

template<typename T>
struct Spreader : ShipObject
{
	int count = 0;
	gcc::vector<int> roomCount;
	gcc::vector<gcc::vector<T>> grid;
};

struct Room : Selectable, ShipObject
{
	Rect rect;
	int iRoomId = 0;
	bool bBlackedOut = false;
	Pad<3> _u0;
	gcc::vector<int> filledSlots;
	gcc::vector<gcc::vector<bool>> slots;
	bool bWarningLight = false;
	Pad<3> _u1;
	AnimationTracker lightTracker;
	int iFireCount = 0;
	gcc::vector<Animation> fires;
	int primarySlot = 0;
	int primaryDirection = 0;
	float lastO2 = 0.f;
	GL_Primitive* floorPrimitive = nullptr;
	GL_Primitive* blackoutPrimitive = nullptr;
	GL_Primitive* highlightPrimitive = nullptr;
	GL_Primitive* highlightPrimitive2 = nullptr;
	GL_Primitive* o2LowPrimitive = nullptr;
	GL_Primitive* computerPrimitive = nullptr;
	GL_Primitive* computerGlowPrimitive = nullptr;
	GL_Primitive* computerGlowYellowPrimitive = nullptr;
	GL_Primitive* lightPrimitive = nullptr;
	GL_Primitive* lightGlowPrimitive = nullptr;
	Animation stunSparks;
	Animation consoleSparks;
	bool bStunning = false;
	Pad<3> _u2;
	float fHacked = 0.f;
	int currentSparkRotation = 0;
	gcc::vector<Animation> sparks;
	float sparkTimer = 0.f;
	int sparkCount = 0;
	int iHackLevel = 0;
	Animation roomTapped;
};

struct CrewTarget
{
	vptr _vptr = nullptr;
	int iShipId = 0;
};

struct Door : CrewTarget, Selectable
{
	int iRoom1 = 0, iRoom2 = 0;
	bool bOpen = false;
	Pad<3> _u0;
	int iBlast = 0;
	bool bFakeOpen = false;
	Pad<3> _u1;
	int width = 0, height = 0;
	GL_Primitive* outlinePrimitive = nullptr;
	GL_Primitive* highlightPrimitive = nullptr;
	Animation doorAnim;
	Animation doorAnimLarge;
	int iDoorId = 0;
	int baseHealth = 0;
	int health = 0;
	AnimationTracker forcedOpen;
	AnimationTracker gotHit;
	int doorLevel = 0;
	bool bIoned = false;
	Pad<3> _u2;
	float fakeOpenTimer = 0.f;
	AnimationTracker lockedDown;
	float lastbase = 0.f;
	int iHacked = 0;
	int x = 0, y = 0;
	bool bVertical = false;
	Pad<3> _u3;
};

struct OuterHull : Repairable
{
	Animation breach, heal;
};

struct BoardingGoal
{
	float fHealthLimit = 0.f;
	int causedDamage = 0;
	int targetsDestroyed = 0;
	int target = 0;
	int damageType = 0;
};

struct CrewTask
{
	int taskId = 0, room = 0, system = 0;
};

struct SCrewStats
{
	gcc::vector<int> stat;
	gcc::string species, name;
	bool male = false;
	Pad<3> _u0;
};

struct Slot
{
	int roomId = 0, slotId = 0;
	Point worldLocation;
};

struct Path
{
	Point start;
	gcc::vector<Door*> doors;
	Point finish;
	float distance = -1.f;
};

struct CrewLaser : Projectile
{
	int r = 0, g = 0, b = 0;
};

struct CrewAnimation
{
	vptr _vptr = nullptr;
	int iShipId = 0;
	gcc::vector<gcc::vector<Animation>> anims;
	GL_Texture* baseStrip = nullptr;
	GL_Texture* colorStrip = nullptr;
	gcc::vector<GL_Texture*> layerStrips;
	Pointf lastPosition;
	int direction = 0, sub_direction = 0;
	int status = 0;
	int moveDirection = 0;
	ParticleEmitter smokeEmitter;
	bool bSharedSpot = false;
	Pad<3> _u0;
	gcc::vector<CrewLaser> shots;
	TimerHelper shootTimer;
	TimerHelper punchTimer;
	Pointf target;
	float damageDone;
	bool bPlayer = false;
	bool bFrozen = false;
	bool bDrone = false;
	bool bGhost = false;
	bool bExactShooting = false;
	Pad<3> _u1;
	Animation projectile;
	bool btyping = false;
	Pad<3> _u2;
	gcc::string race;
	int currentShip = 0;
	bool bMale = false;
	bool colorblind = false;
	Pad<2> _u3;
	gcc::vector<GL_Color> layerColors;
	int forcedAnimation = -1;
	int forcedDirection = -1;
	GL_Color projectileColor;
	bool bStunned = false;
	bool bDoorTarget = false;
	bool uniqueBool1 = false, uniqueBool2 = false;
};

struct Ship;

struct CrewMember
{
	vptr _vptr = nullptr;
	int iShipId = 0;
	float x = 0.f, y = 0.f;
	float size = 0.f;
	float scale = 0.f;
	float goal_x = 0.f, goal_y = 0.f;
	int width = 0, height = 0;
	std::pair<float, float> health{ 0.f, 0.f };
	float speed_x = 0.f, speed_y = 0.f;
	Path path;
	bool new_path = false;
	Pad<3> _u0;
	float x_destination = 0.f, y_destination = 0.f;
	Door* last_door = nullptr;
	Repairable* currentRepair = nullptr;
	bool bSuffocating = false;
	Pad<3> _u1;
	int moveGoal = 0;
	int selectionState = 0;
	int iRoomId = 0;
	int iManningId = 0;
	int iRepairId = 0;
	int iStackId = 0;
	Slot currentSlot;
	bool intruder = false;
	bool bFighting = false;
	bool bSharedSpot = false;
	Pad<1> _u2;
	CrewAnimation* crewAnim;
	GL_Texture* selectionImage = nullptr;
	CachedImage healthBox, healthBoxRed;
	CachedRect healthBar;
	float fMedbay = 0.f;
	float lastDamageTimer = 0.f;
	float lastHealthChange = 0.f;
	int currentShipId = 0;
	AnimationTracker flashHealthTracker;
	Pointf curerntTarget;
	CrewTarget* crewTarget = nullptr;
	BoardingGoal boardingGoal;
	bool bFrozen = false, bFrozenLocation = false;
	Pad<2> _u3;
	CrewTask task;
	gcc::string type;
	Ship* ship = nullptr;
	Slot finalGoal;
	Door* blockingDoor = nullptr;
	bool bOutOfGame = false;
	Pad<3> _u4;
	gcc::string species;
	bool bDead = false;
	Pad<3> _u5;
	int iOnFire = 0;
	bool bActiveManning = false;
	Pad<3> _u6;
	ShipSystem* currentSystem = nullptr;
	int usingSkill = -1;
	CrewBlueprint blueprint;
	Animation healing;
	Animation stunned;
	AnimationTracker levelUp;
	int lastLevelUp = 0;
	SCrewStats stats;
	gcc::vector<gcc::vector<bool>> skillsEarned;
	bool clone_ready = false;
	bool bMindControlled = false;
	Pad<2> _u7;
	int iDeathNumber = -1;
	Animation mindControlled;
	Animation stunIcon;
	gcc::vector<gcc::vector<AnimationTracker>> skillUp;
	int healthBoost = 0;
	float fMindDamageBoost = 1.f;
	float fCloneDying = 0.f;
	bool bResisted = false;
	Pad<3> _u8;
	Slot savedPosition;
	float fStunTime = 0.f;
	CachedImage movementTarget;
	bool bCloned = false;
	Pad<3> _u9;
};

struct CrewDrone : CrewMember, Drone
{
	int droneRoom = 0;
	Animation powerUp, powerDown;
	GL_Texture* lightLayer = nullptr;
	GL_Texture* baseLayer = nullptr;
};

struct ExplosionAnimation : AnimationTracker
{
	ShipObject shipObj;
	gcc::vector<Animation> explosions;
	gcc::vector<GL_Texture> pieces;
	gcc::vector<gcc::string> pieceNames;
	gcc::vector<float> rotationSpeed;
	gcc::vector<float> rotation;
	gcc::vector<std::pair<float, float>> rotationSpeedMinMax;
	gcc::vector<Pointf> movementVector;
	gcc::vector<Pointf> position;
	gcc::vector<Pointf> startingPosition;
	float explosionTimer = 0.f;
	float soundTimer = 0.f;
	bool bFinalBoom = false;
	bool bJumpOut = false;
	Pad<2> _u0;
	gcc::vector<WeaponAnimation> weaponAnims;
	Point pos;
};

struct LockdownShard
{
	Animation shard;
	Pointf position, goal;
	float speed = 0.f;
	bool bArrive = false;
	bool bDone = false;
	Pad<2> _u0;
	float lifeTime = 0.f;
	bool superFreeze = false;
	Pad<3> _u1;
	int lockingRoom = 0;
};

struct TouchTooltip
{

};

struct WarningMessage
{
	vptr _vptr = nullptr;
	AnimationTracker tracker;
	Point position;
	bool isImage = false;
	Pad<3> _u0;
	TextString text;
	bool centerText = false;
	Pad<3> _u1;
	GL_Color textColor;
	bool useWarningLine = false;
	Pad<3> _u2;
	CachedImage image;
	gcc::string imageName;
	bool flash = false;
	Pad<3> _u3;
	gcc::string sound;
	AnimationTracker flashTracker;
};

struct WarningWithLines : WarningMessage
{
	GL_Primitive* linePrimitive = nullptr;
	Point textOrigin;
	TextString topText, buttomText;
	int topTextLimit = 0, bottomTextLimit = 0;
};

struct TapBoxFrame
{
	Point location;
	bool useWideBox = false;
	Pad<3> _u0;
	int boxHeight = 0;
	gcc::vector<int> buttonHeights;
	gcc::vector<GL_Primitive*> primitives;
	Rect hitBox;
};

struct SystemBox
{
	vptr _vptr = nullptr;
	Point location;
	GL_Primitive* timerCircle[10]{
		nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr
	};
	GL_Primitive* timerLines = nullptr;
	GL_Primitive* timerStencil = nullptr;
	int lastTimerStencilCount = 0;
	GL_Primitive* brokenIcon = nullptr;
	GL_Primitive* lockIcon = nullptr;
	GL_Primitive* hackIcon = nullptr;
	ShipSystem* pSystem = nullptr;
	bool bShowPower = false;
	Pad<3> _u0;
	float powerAlpha = 0.f;
	bool mouseHover = false;
	Pad<3> _u1;
	int activeTouch = 0;
	Point touchInitialOffset;
	bool tapped = false;
	bool draggingPower = false;
	Pad<2> _u2;
	int dragInitialPower = 0;
	float lastDragSpeed = 0.f;
	int lastDragY = 0;
	double lastDragTime = 0.0;
	WarningMessage warning;
	int topPower = 0;
	Rect hitBox;
	int hitBoxTop = 0;
	bool hitBoxTopWasSet = false;
	Pad<3> _u3;
	GL_Texture* wireImage = nullptr;
	bool bSimplePower = false;
	bool bPlayerUI = false;
	bool useLargeTapIcon = false;
	Pad<1> _u4;
	Point largeTapIconOffset;
	gcc::vector<int> tapButtonHeights;
	int tapButtonOffsetY = 0;
	int cooldownOffsetY = 0;
	float keyPressed = 0.f;
	TouchTooltip* touchTooltip = nullptr;
	TapBoxFrame tapBoxFrame;
	bool lockedOpen = false;
	Pad<3> _u5;
};

struct ChoiceText
{
	int type = 0;
	gcc::string text;
	ResourceEvent rewards;
};

struct ChoiceBox : FocusWindow
{
	GL_Texture* textBox = nullptr;
	WindowFrame* box = nullptr;
	gcc::string mainText;
	gcc::vector<ChoiceText> choices;
	unsigned columnSize = 0;
	gcc::vector<Rect> choiceBoxes;
	int potentialChoice = -1, selectedChoice = -1;
	int fontSize = 0;
	bool center = false;
	Pad<3> _u0;
	int gap_size = 0;
	float openTime = 0.f;
	ResourceEvent rewards;
	GL_Color currentTextColor;
	Pointf lastChoice;
};

struct ShipBlueprint : Blueprint
{
	struct SystemTemplate
	{
		int systemId = 0;
		int powerLevel = 0;
		gcc::vector<int> location;
		int bp = 0;
		int maxPower = 0;
		gcc::string image;
		int slot = 0;
		int direction = 0;
		gcc::vector<gcc::string> weapon;
	};

	Description desc;
	gcc::string blueprintName;
	TextString name;
	TextString shipClass;
	gcc::string layoutFile, imgFile, cloakFile, shieldFile, floorFile;
	gcc::map<int, SystemTemplate> systemInfo;
	gcc::vector<int> systems;
	int droneCount = 0, originalDroneCount = 0;
	int droneSlots = 0;
	gcc::string loadDrones;
	gcc::vector<gcc::string> drones;
	gcc::vector<gcc::string> augments;
	int weaponCount = 0, originalWeaponCount = 0;
	int weaponSlots = 0;
	gcc::string loadWeapons;
	gcc::vector<gcc::string> weapons;
	int missiles = 0;
	int drone_count = 0;
	int health = 0;
	int originalCrewCount = 0;
	gcc::vector<gcc::string> defaultCrew;
	gcc::vector<CrewBlueprint> customCrew;
	int maxPower = 0;
	int boardingAI = 0;
	int bp_count = 0;
	int maxCrew = 0;
	int maxSector = 0;
	int minSector = 0;
	TextString unlock;
};

struct Ship : ShipObject
{
	struct DoorState
	{
		int state = 0;
		bool hacked = false;
		Pad<3> _u0;
		int level = -1;
	};

	gcc::vector<Room*> vRoomList;
	gcc::vector<Door*> vDoorList;
	gcc::vector<OuterHull*> vOuterWalls;
	gcc::vector<Door*> vOuterAirlocks;
	std::pair<int, int> hullIntegrity{ 0, 0 };
	gcc::vector<WeaponMount> weaponMounts;
	gcc::string floorImageName;
	ImageDesc shipFloor;
	GL_Primitive* floorPrimitive = nullptr;
	gcc::string shipImageName;
	ImageDesc shipImage;
	Point glowOffset;
	GL_Primitive* shipImagePrimitive = nullptr;
	gcc::string cloakImageName;
	ImageDesc shipImageCloak;
	GL_Primitive* cloakPrimitive = nullptr;
	GL_Primitive* gridPrimitive = nullptr;
	GL_Primitive* wallsPrimitive = nullptr;
	GL_Primitive* doorsPrimitive = nullptr;
	gcc::vector<DoorState> doorState;
	bool lastDoorControlMode = false;
	Pad<3> _u0;
	GL_Texture* thrustersImage = nullptr;
	GL_Texture* jumpGlare = nullptr;
	int vertical_shift = 0, horizontal_shift = 0;
	gcc::string shipName;
	ExplosionAnimation explosion;
	bool bDestroyed = false;
	Pad<3> _u1;
	Ellipse baseEllipse;
	Animation engineAnim[2]{};
	AnimationTracker cloakingTracker;
	bool bCloaked = false;
	bool bExperiment = false;
	bool bShowEngines = false;
	Pad<1> _u2;
	gcc::vector<LockdownShard> lockdowns;
};

struct ShipManager : ShipObject, Targetable, Collideable
{
	gcc::vector<ShipSystem*> vSystemList;
	OxygenSystem* oxygenSystem = nullptr;
	TeleportSystem* teleportSystem = nullptr;
	CloakingSystem* cloakSystem = nullptr;
	BatterySystem* BatterySystem = nullptr;
	MindSystem* mindSystem = nullptr;
	CloneSystem* cloneSystem = nullptr;
	HackingSystem* hackingSystem = nullptr;
	bool showNetwork = false;
	bool addedSystem = false;
	Pad<2> _u0;
	Shields* shieldSystem = nullptr;
	WeaponSystem* weaponSystem = nullptr;
	DroneSystem* droneSystem = nullptr;
	EngineSystem* engineSystem = nullptr;
	MedbaySystem* medbaySystem = nullptr;
	gcc::vector<ArtillerySystem*> artillerySystems;
	gcc::vector<CrewMember*> vCrewList;
	Spreader<Fire> fireSpreader;
	Ship ship;
	char statusMessages[40]{};
	bool bGameOver = false;
	Pad<3> _u1;
	ShipManager* current_target = nullptr;
	std::pair<float, float> jump_timer{ 0.f, 0.f };
	int fuel_count = 0;
	bool hostile_ship = false;
	bool bDestroyed = false;
	Pad<2> _u2;
	int iLastDamage = 0;
	AnimationTracker jumpAnimation;
	gcc::vector<DamageMessage*> damMessages;
	gcc::vector<int> systemKey;
	int currentScrap = 0;
	bool bJumping = false;
	bool bAutomated = false;
	int shipLevel = 0;
	ShipBlueprint myBlueprint;
	bool lastEngineStatus = false;
	bool lastJumpReady = false;
	bool bContainsPlayerCrew = false;
	Pad<1> _u3;
	int iIntruderCount = 0;
	gcc::vector<gcc::vector<int>> crewCounts;
	int tempDroneCount = 0;
	int tempMissileCount = 0;
	gcc::vector<Animation> explosions;
	gcc::vector<bool> tempVision;
	bool bHighlightCrew = false;
	Pad<3> _u4;
	gcc::vector<Drone*> droneTrash;
	gcc::vector<SpaceDrone*> spaceDrones;
	gcc::vector<SpaceDrone*> newDroneArrivals;
	int bpCount = 0;
	int iCustomizeMode = 0;
	bool bShowRoom = false;
	Pad<3> _u5;
	gcc::vector<Projectile*> superBarrage;
	bool bInvincible = false;
	Pad<3> _u6;
	gcc::vector<SpaceDrone*> superDrones;
	GL_Primitive* highlight = nullptr;
	int failedDodgeCounter = 0;
	gcc::vector<float> hitByBeam;
	bool enemyDamagedUncloaked = false;
	Pad<3> _u7;
	int damageCloaked = 0;
	std::pair<int, int> killedByBeam{ 0, 0 };
	int minBeaconHealth = 0;
	gcc::vector<ParticleEmitter*> fireExtinguishers;
	bool bWasSafe = false;
	Pad<3> _u8;
};

struct EquipmentBoxItem
{
	ProjectileFactory* pWeapon = nullptr;
	Drone* pDrone = nullptr;
	CrewMember* pCrew = nullptr;
	const AugmentBlueprint* augment = nullptr;
};

struct EquipmentBox
{
	vptr _vptr = nullptr;
	GL_Primitive* blocked_overlay = nullptr;
	GL_Color overlayColor;
	gcc::string imageName;
	GL_Primitive* empty = nullptr;
	GL_Primitive* full = nullptr;
	GL_Primitive* selected_empty = nullptr;
	GL_Primitive* selected_full = nullptr;
	WeaponSystem* weaponSys;
	DroneSystem* droneSys;
	Point location;
	Rect hitBox;
	EquipmentBoxItem item;
	bool bMouseHovering = false;
	bool bGlow = false;
	bool bBlocked = false;
	Pad<1> _u0;
	int slot = 0;
	bool bLocked = false;
	Pad<3> _u1;
	int value = 0;
	bool bPermanentLock = false;
	bool blockDetailed = false;
	Pad<2> _u2;
};

struct CrewEquipBox : EquipmentBox
{
	ShipManager* ship = nullptr;
	bool bDead = false;
	Pad<3> _u0;
	TextButton deleteButton,renameButton;
	bool bShowDelete;
	bool bShowRename;
	bool bQuickRenaming;
	Pad<1> _u1;
	TextInput nameInput;
	GL_Primitive* box = nullptr;
	GL_Primitive* box_on = nullptr;
	bool bConfirmDelete;
	Pad<3> _u2;
};

struct AugmentEquipBox : EquipmentBox
{
	ShipManager* ship = nullptr;
};

struct DropBox
{
	Point position;
	bool isSellBox = false;
	Pad<3> _u0;
	GL_Texture* boxImage[2]{ nullptr, nullptr };
	int selectedImage = 0;
	TextString titleText;
	TextString bodyText;
	int bodySpace = 0;
	TextString lowerText;
	TextString sellText;
	gcc::string sellCostText;
	int textWidth = 0;
	int insertHeight = 0;
	int titleInsert = 0;
};

struct Equipment : FocusWindow
{
	GL_Texture* box = nullptr;
	GL_Texture* storeBox = nullptr;
	DropBox overBox;
	DropBox overAugImage;
	DropBox sellBox;
	bool bSellingItem = false;
	Pad<3> _u0;
	ShipManager* shipManager = nullptr;
	gcc::vector<EquipmentBox*> vEquipmentBoxes;
	gcc::vector<ProjectileFactory*> weaponsTrashList;
	EquipmentBox* overcapacityBox = nullptr;
	AugmentEquipBox* overAugBox = nullptr;
	int selectedEquipBox = 0;
	int draggingEquipBox = 0;
	int potentialDraggingBox = 0;
	bool bDragging = false;
	Pad<3> _u1;
	Point firstMouse;
	Point currentMouse;
	Point dragBoxCenter;
	Point dragBoxOffset;
	InfoBox infoBox;
	gcc::string sellCostText;
	bool bOverCapacity = false;
	bool bOverAugCapacity = false;
	bool bStoreMode = false;
	Pad<1> _u2;
	int cargoId = 0;
	Point infoBoxLoc;
};

struct CrewCustomizeBox : CrewEquipBox
{
	TextButton customizeButton;
	bool bCustomizing = false;
	Pad<3> _u0;
	Point customizeLocation;
	TextButton acceptButton, bigRenameButton;
	Button leftButton, rightButton;
	bool bRenaming;
	bool haveCustomizeTouch;
	bool customizeActivated;
	GL_Primitive* box = nullptr;
	GL_Primitive* box_on = nullptr;
	GL_Texture* bigBox = nullptr;
};

struct SystemCustomBox : SystemBox
{
	ShipManager* shipManager = nullptr;
	Button button;
};

struct CAchievement
{
	gcc::string name_id;
	std::pair<int, int> progress;
	bool unlocked = false;
	Pad<3> _u0;
	TextString name, description, header;
	bool newAchievement = false;
	bool multiDifficulty = false;
	Pad<2> _u1;
	int difficulty = 0;
	gcc::string ship;
	int shipDifficulties[3]{ 0, 0, 0 };
	int dimension = 0;
	CachedImage icon, miniIcon, miniIconLocked, lockImage, dotOn, dotOff;
	GL_Primitive* outline = nullptr;
	GL_Primitive* mini_outline = nullptr;
	GL_Primitive* lockOverlay = nullptr;
};

struct ShipAchievementInfo
{
	CAchievement* achievement = nullptr;
	Point position;
	int dimension = 0;
};

struct CombatControl;

struct ShipStatus
{
	Point location;
	float size = 0.f;
	ShipManager* ship = nullptr;
	CombatControl* combat = nullptr;
	GL_Primitive* hullBox = nullptr;
	GL_Primitive* hullBox_Red = nullptr;
	GL_Primitive* shieldBox_On = nullptr;
	GL_Primitive* shieldBox_Off = nullptr;
	GL_Primitive* shieldBox_Red = nullptr;
	GL_Primitive* shieldCircleCharged[4]{ nullptr, nullptr, nullptr, nullptr };
	GL_Primitive* shieldCircleUncharged[4]{ nullptr, nullptr, nullptr, nullptr };
	GL_Primitive* shieldCircleHacked[4]{ nullptr, nullptr, nullptr, nullptr };
	GL_Primitive* shieldCircleHackedCharged[4]{ nullptr, nullptr, nullptr, nullptr };
	GL_Primitive* energyShieldBox = nullptr;
	GL_Primitive* energyShieldBar[5]{ nullptr, nullptr, nullptr, nullptr, nullptr };
	GL_Texture* hullLabel = nullptr;
	GL_Texture* hullLabel_Red = nullptr;
	GL_Primitive* shieldBoxPurple = nullptr;
	GL_Primitive* oxygenPurple = nullptr;
	GL_Primitive* evadePurple = nullptr;
	GL_Primitive* evadeOxygenBox = nullptr;
	GL_Primitive* evadeOxygenBox_topRed = nullptr;
	GL_Primitive* evadeOxygenBox_bottomRed = nullptr;
	GL_Primitive* evadeOxygenBox_bothRed = nullptr;
	GL_Primitive* fuelIcon = nullptr;
	GL_Primitive* missilesIcon = nullptr;
	GL_Primitive* dronesIcon = nullptr;
	GL_Primitive* scrapIcon = nullptr;
	GL_Primitive* fuelIcon_red = nullptr;
	GL_Primitive* missilesIcon_red = nullptr;
	GL_Primitive* dronesIcon_red = nullptr;
	GL_Primitive* scrapIcon_red = nullptr;
	GL_Primitive* healthMask = nullptr;
	GL_Texture* healthMaskTexture = nullptr;
	int lastHealth = 0;
	Ellipse baseShield;
	int currentHover = -1;
	Point evadeOxygenBoxLocation;
	int lastFuel = 0;
	int lastDrones = 0;
	int lastScrap = 0;
	int lastMissiles = 0;
	int lastHull = 0;
	WarningWithLines* hullMessage = nullptr;
	WarningMessage* shieldMessage = nullptr;
	WarningMessage* oxygenMessage = nullptr;
	WarningMessage* boardingMessage = nullptr;
	gcc::vector<DamageMessage*> resourceMessages;
	AnimationTracker noMoneyTracker;
	AnimationTracker flashTracker;
	bool bBossFight = false;
	bool bEnemyShip = false;
	Pad<2> _u0;
	Point noShipShift, intruderShift;
	Point energyShieldPos;
	Point intruderPos;
};

struct CrewBox
{
	Rect box, skillBox;
	CrewMember* pCrew = nullptr;
	bool mouseHover = false;
	Pad<3> _u0;
	TextButton powerButton;
	int number = 0;
	bool bSelectable = false;
	Pad<3> _u1;
	AnimationTracker flashHealthTracker;
	GL_Primitive* boxBackground = nullptr;
	GL_Primitive* boxOutline = nullptr;
	GL_Primitive* skillBoxBackground = nullptr;
	GL_Primitive* skillBoxOutline = nullptr;
	GL_Primitive* cooldownBar = nullptr;
	CachedImage healthWarning;
	int lastCooldownHeight = -1;
	GL_Primitive* healthBar = nullptr;
	int lastHealthWidth = 0;
	Animation mindControlled;
	Animation stunned;
	bool hideExtra = false;
	Pad<3> _u2;
	gcc::string sTooltip;
};

struct CrewControl
{
	ShipManager* shipManger = nullptr;
	gcc::vector<CrewMember*> selectedCrew, potentialSelectedCrew;
	Door* selectedDoor = nullptr;
	Repairable* selectedRepair = nullptr;
	Point selectedGrid;
	int selectedRoom = -1;
	bool selectedPlayerShip = false;
	Pad<3> _u0;
	Point availablePosition;
	gcc::vector<CrewBox*> crewBoxes;
	Point firstMouse, currentMouse, worldFirstMouse, worldCurrentMouse;
	bool mouseDown = false;
	bool bupdated = false;
	Pad<2> _u1;
	int activeTouch = 0;
	bool selectingCrew = false;
	bool selectedCrewOnPlayerShip = false;
	Pad<6> _u2;
	double selectingCrewStartTime = 0.0;
	bool doorControlMode = false, doorControlOpen = false, doorControlOpenSet = false;
	Pad<1> _u3;
	CombatControl* combatControl = nullptr;
	unsigned selectedCrewBox = -1;
	AnimationTracker crewMessage;
	gcc::string message;
	Button saveStations, returnStations;
	GL_Primitive* saveStationsBase = nullptr;
	GL_Primitive* returnStationsBase = nullptr;
	int stationsLastY = 0;
	Pad<4> _u4;
};

struct RandomAmount
{
	int min = 0, max = 0;
	float chanceNone = 0.f;
};

struct AsteroidGenerator
{
	gcc::queue<Projectile*> asteroidQueue;
	RandomAmount spawnRate[3];
	RandomAmount stateLength[3];
	int numberOfShips = 0;
	int iState = 0;
	int currentSpace = 0;
	int iNextDirection = 0;
	float fStateTimer = 0.f;
	float timer = 0.f;
	bool bRunning = false;
	Pad<3> _u0;
	int initShields = 0;
};

struct Scroller
{
	GL_Texture* imageId = nullptr;
	int size_x = 0, size_y = 0;
	int image_x = 0, image_y = 0;
	float fSpeed = 0.f;
	float current_x = 0.f;
	bool bInitialized = false;
	Pad<3> _u0;
};

struct NebulaCloud
{
	Point pos;
	float currAlpha = 0.f;
	float currScale = 0.f;
	float deltaAlpha = 0.f;
	float deltaScale = 0.f;
	float newTrigger = 0.f;
	bool newCloud = false;
	bool bLightning = false;
	Pad<2> _u0;
	AnimationTracker lightningFlash;
	float flashTimer = 0.f;
	float lightningRotation = 0.f;
};

struct SpaceManager
{
	struct FleetShip
	{
		GL_Texture* image = nullptr;
		Point location;
	};

	gcc::vector<Projectile*> projectiles;
	AsteroidGenerator asteroidGenerator;
	gcc::vector<ShipManager*> ships;
	gcc::vector<SpaceDrone*> drones;
	bool dangerZone = false;
	Pad<3> _u0;
	GL_Texture* currentBack = nullptr;
	ImageDesc currentPlanet;
	CachedImage planetImage;
	ImageDesc fleetShip;
	GL_Texture* shipIds[8]{
		nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr
	};
	FleetShip fleetShips[9];
	Scroller asteroidScroller[3];
	GL_Texture* sunImage = nullptr;
	GL_Texture* sunGlow = nullptr;
	AnimationTracker sunGlow1;
	AnimationTracker sunGlow2;
	AnimationTracker sunGlow3;
	bool sunLevel = false;
	bool pulsarLevel = false;
	Pad<2> _u1;
	GL_Texture* pulsarFront = nullptr;
	GL_Texture* pulsarBack = nullptr;
	GL_Texture* lowendPulsar = nullptr;
	bool bPDS = false;
	Pad<3> _u2;
	int envTarget = 0;
	Point shipPosition;
	float randomPDSTimer = 0.f;
	gcc::vector<Projectile*> pdsQueue;
	TimerHelper flashTimer;
	AnimationTracker flashTracker;
	ImageDesc currentBeacon;
	ImageDesc currentBeaconFlash;
	AnimationTracker beaconTracker;
	bool flashSound = false;
	bool bNebula = false;
	bool bStorm = false;
	Pad<1> _u3;
	gcc::vector<NebulaCloud> nebulaClouds;
	GL_Texture* lowendNebula = nullptr;
	GL_Texture* lowendStorm = nullptr;
	GL_Texture* lowendSun = nullptr;
	GL_Texture* lowendAsteroids = nullptr;
	float shipHealth = 0.f;
	bool gamePaused = false;
	Pad<3> _u4;
	TimerHelper pdsFireTimer;
	int pdsCountdown = 0;
	gcc::vector<Animation> pdsSmokeAnims;
	bool queueScreenShake = false;
	bool playerShipInFront = false;
	Pad<4> _u5;
};

struct HandAnimation
{
	GL_Texture* hand = nullptr;
	Point start;
	Point finish;
	Pointf location;
	bool bRunning = false;
	Pad<3> _u0;
	float pause = 0.f;
};

struct ArmamentBox
{
	vptr _vptr = nullptr;
	gcc::vector<GL_Primitive*> background;
	GL_Primitive* emptyBackground = nullptr;
	GL_Primitive* hoverHighlight = nullptr;
	GL_Primitive* outline = nullptr;
	GL_Primitive* emptyOutline = nullptr;
	GL_Primitive* powerBarGlow[4]{nullptr, nullptr, nullptr, nullptr};
	GL_Primitive* iconBackground = nullptr;
	GL_Primitive* iconInsetBackground = nullptr;
	GL_Primitive* icon = nullptr;
	GL_Primitive* iconDoubleSize = nullptr;
	gcc::string iconName;
	gcc::string iconBackgroundName;
	Point lastIconPos;
	Point location;
	int xOffset = 0;
	Point largeIconOffset;
	Point nameOffset;
	int nameWidth = 0;
	bool mouseHover = false;
	bool touchHover = false;
	bool touchHighlight = false;
	bool selected = false;
	int hotKey = 0;
	int activeTouch = 0;
	TouchTooltip* touchTooltip = nullptr;
	Animation hackAnimation;
	GL_Primitive* touchButtonBorder = nullptr;
	Rect touchButtonBorderRect;
	float touchButtonSlidePos = 0.f;
	gcc::vector<GenericButton*> touchButtons;
	Rect touchButtonHitbox;
	GL_Color iconColor;
	bool droneVariation = false;
	bool bIoned = false;
	Pad<2> _u0;
};

struct CommandGui;

struct ArmamentControl
{
	vptr _vptr = nullptr;
	int systemId = 0;
	CommandGui* gui = nullptr;
	ShipManager* shipManager = nullptr;
	gcc::vector<ArmamentBox*> boxes;
	Point location;
	Rect touchHitBox;
	GL_Texture* holderImage = nullptr;
	GL_Primitive* holder = nullptr;
	GL_Primitive* holderTab = nullptr;
	gcc::vector<GL_Primitive*> smallBoxHolder;
	gcc::vector<Animation> smallBoxHackAnim;
	int smallBoxHolderTop = 0;
	bool bOpen = false;
	Pad<3> _u0;
	Point lastMouse;
	Point currentMouse;
	int draggingBox = 0;
	int draggingTouch = 0;
	bool bDragging = false;
	Pad<3> _u1;
	int iLastSwapSlot = 1;
	bool bTutorialFlash = false;
	Pad<3> _u2;
	int iFlashSlot = 0;
	int activeTouch = 0;
};

struct DroneControl : ArmamentControl
{
	WarningMessage droneMessage;
	WarningMessage noTargetMessage;
	WarningMessage systemMessage;
};

struct WeaponControl : ArmamentControl
{
	Targetable* currentTarget = nullptr;
	ProjectileFactory* armedWeapon = nullptr;
	bool autoFiring = false;
	Pad<3> _u0;
	TextButton autoFireButton;
	GL_Primitive* autoFireBase = nullptr;
	GL_Primitive* targetIcon[4]{ nullptr, nullptr, nullptr, nullptr };
	GL_Primitive* targetIconYellow[4]{ nullptr, nullptr, nullptr, nullptr };
	Pointf autoFireFocus;
	WarningMessage missileMessage;
	WarningMessage systemMessage;
	int armedSlot = 0;
};

struct CrewAI
{
	ShipManager* ship = nullptr;
	bool bAIon = false;
	bool bAirlockRequested = false;
	bool bMedbayRequested = false;
	bool bHurtCrew = false;
	bool bCalmShip = false;
	Pad<3> _u0;
	gcc::vector<CrewMember*> crewList;
	gcc::vector<CrewMember*> intruderList;
	gcc::vector<Repairable*> hullBreaches;
	gcc::vector<CrewTask> desiredTaskList;
	gcc::vector<CrewTask> bonusTaskList;
	gcc::vector<bool> breachedRooms;
	int iTeleportRequest = 0;
	bool bUrgentTeleport = false;
	Pad<3> _u1;
	int startingCrewCount = 0;
	bool bMultiracialCrew = false;
	bool bOverrideRace = false;
	Pad<2> _u2;
};

struct CombatAI
{
	ShipManager* target = nullptr;
	gcc::vector<ProjectileFactory*> weapons;
	gcc::vector<SpaceDrone*> drones;
	int stance = 0;
	gcc::vector<int> system_targets;
	bool bFiringWhileCloaked = false;
	Pad<3> _u0;
	ShipManager* self = nullptr;
};

struct PowerProfile
{

};

struct ShipAI
{
	ShipManager* ship = nullptr;
	ShipManager* target = nullptr;
	CrewAI crewAI;
	CombatAI combatAI;
	bool playerShip = false;
	bool surrendered = false;
	bool escaping = false;
	bool destroyed = false;
	int surrenderThreshold = 0;
	int escapeThreshold = 0;
	float escapeTimer = 0.f;
	int lastMaxPower = 0;
	gcc::map<gcc::string, PowerProfile> powerProfiles;
	int boardingProfile = 0;
	int iTeleportRequest = 0;
	int iTeleportTarget = 0;
	int brokenSystems = 0;
	int boardingAi = 0;
	int iCrewNeeded = 0;
	bool bStalemateTrigger = false;
	Pad<3> _u0;
	float fStalemateTimer = 0.f;
	int lastHealth = 0;
	bool bBoss = false;
	Pad<3> _u1;
	int iTimesTeleported = 0;
};

struct CompleteShip
{
	vptr _vptr = nullptr;
	int iShipId = 0;
	ShipManager* shipManager = nullptr;
	SpaceManager* spaceManager = nullptr;
	CompleteShip* enemyShip = nullptr;
	bool bPlayerShip = false;
	Pad<3> _0;
	ShipAI shipAI;
	gcc::vector<CrewMember*> arrivingParty;
	gcc::vector<CrewMember*> leavingParty;
	int teleTargetRoom = 0;
};

struct CombatControl
{
	CommandGui* gui = nullptr;
	ShipManager* shipManager = nullptr;
	Point playerShipPosition;
	SpaceManager* space = nullptr;
	WeaponControl weapControl;
	DroneControl droneControl;
	gcc::vector<SystemBox*> sysBoxes;
	gcc::vector<CompleteShip*> enemyShips;
	CompleteShip* currentTarget;
	SpaceDrone* currentDrone = nullptr;
	Point position;
	int selectedRoom = 0;
	int selectedSelfRoom = 0;
	Point targetPosition;
	Point boxPosition;
	WindowFrame* hostileBoxFrame = nullptr;
	CachedImage healthMask;
	CachedImage shieldCircleCharged[5];
	CachedImage shieldCircleUncharged[5];
	CachedImage shieldCircleHacked[5];
	CachedImage shieldCircleHackedCharged[5];
	CachedImage shieldChargeBox;
	CachedImage superShieldBox5;
	CachedImage superShieldBox12;
	bool open = false;
	Pad<3> _u0;
	float shipIconSize = 0.f;
	Pointf potentialAiming;
	gcc::vector<Pointf> aimingPoints;
	Pointf lastMouse;
	bool mouseDown = false;
	bool isAimingTouch = false;
	bool movingBeam = false;
	Pad<1> _u1;
	Point beamMoveLast;
	bool invalidBeamTouch = false;
	Pad<3> _u2;
	Point screenReposition;
	std::pair<int, int> teleportCommand{ 0, 0 };
	int iTeleportArmed = 0;
	CachedImage teleportTarget_send;
	CachedImage teleportTarget_return;
	CachedImage hackTarget;
	CachedImage mindTarget;
	AnimationTracker ftl_timer;
	WarningMessage ftlWarning;
	AnimationTracker hacking_timer;
	gcc::vector<gcc::string> hackingMessages;
	bool boss_visual = false;
	bool bTeachingBeam = false;
	Pad<2> _u3;
	WindowFrame* tipBox = nullptr;
	HandAnimation hand;
};

struct SystemControl
{
	ShipManager* shipManger = nullptr;
	CombatControl* combatControl = nullptr;
	gcc::vector<SystemBox*> sysBoxes;
	Rect SystemPower;
	bool bSystemPowerHover = false;
	Pad<3> _u0;
	Point position, systemPowerPosition, subSystemPosition;
	GL_Primitive* wiresImage = nullptr;
	GL_Primitive* wiresMask = nullptr;
	GL_Primitive* noButton = nullptr;
	GL_Primitive* button = nullptr;
	GL_Primitive* noButton_cap = nullptr;
	GL_Primitive* button_cap = nullptr;
	GL_Primitive* drone = nullptr;
	GL_Primitive* drone3 = nullptr;
	GL_Primitive* drone2 = nullptr;
	GL_Primitive* sub_box = nullptr;
	int sub_spacing = 0;
	WarningMessage* notEnoughPower = nullptr;
	AnimationTracker flashBatteryPower;
	AnimationTracker flashTracker;
};

struct TextButton0 : GenericButton
{
	GL_Primitive* primitives[3]{ nullptr, nullptr, nullptr };
	GL_Texture* baseImage = nullptr;
	Point baseImageOffset;
	GL_Primitive* basePrimitive = nullptr;
	bool colorsSet = true;
	Pad<3> _u0;
	GL_Color colors[3];
	GL_Color textColor;
	Point buttonSize;
	int cornerInset = 0;
	bool autoWidth = false;
	Pad<3> _u1;
	int autoWidthMargin = 0, autoWidthMin = 0;
	bool autoRightAlign = false;
	Pad<3> _u2;
	TextString label;
	int font = 0;
	int lineHeight = 0;
	int textYOffset = 0;
};

struct FTLButton : TextButton0
{
	bool auotShrinkText = false;
	bool ready = false;
	Pad<2> _u0;
	float ftl_blink = 0.f, ftl_blink_dx = 0.f;
	float pullout = 0.f;
	ShipManager* ship = nullptr;
	GL_Primitive* baseImage = nullptr;
	GL_Primitive* baseImageRed = nullptr;
	GL_Primitive* pulloutBase = nullptr;
	GL_Primitive* pulloutBaseRed = nullptr;
	GL_Primitive* pilotOn = nullptr;
	GL_Primitive* pilotOff1 = nullptr;
	GL_Primitive* pilotOff2 = nullptr;
	GL_Primitive* enginesOn = nullptr;
	GL_Primitive* enginesOff1 = nullptr;
	GL_Primitive* enginesOff2 = nullptr;
	GL_Texture* FTL_loadingbars = nullptr;
	GL_Texture* FTL_loadingbars_off = nullptr;
	GL_Primitive* loadingBars = nullptr;
	GL_Primitive* loadingBarsOff = nullptr;
	int lastBarsWidth = 0;
	WarningMessage* enginesDown = nullptr;
	bool bOutOfFuel = false;
	bool bBossFight = false;
	bool bHoverRaw = false;
	bool bHoverPilot = false;
	bool bHoverEngine = false;
	Pad<3> _u1;
};

struct SpaceStatus
{
	GL_Primitive* warningImages[10]{
		nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr
	};
	WarningMessage* warningMessage = nullptr;
	WarningMessage* incomingFire = nullptr;
	Rect hitbox, hitbox2;
	int currentEffect = 0, currentEffect2 = 0;
	SpaceManager* space = nullptr;
	Point position;
	int touchedTooltip;
};

struct ChoiceReq
{
	gcc::string object;
	int min_level = 0;
	int max_level = 0;
	int max_group = 0;
	bool blue = false;
	Pad<3> _u0;
};

struct CrewDesc
{
	gcc::string type;
	float prop = 0.f;
	int amount = 0;
};

struct ShipEvent
{
	bool present = false;
	Pad<3> _u0;
	gcc::string name;
	gcc::string blueprint;
	gcc::string auto_blueprint;
	gcc::vector<gcc::string> blueprintList;
	ShipBlueprint actualBlueprint;
	bool hostile = false;
	Pad<3> _u1;
	gcc::string surrender;
	gcc::string escape;
	gcc::string destroyed;
	gcc::string deadCrew;
	gcc::string gotaway;
	int escapeTimer = 0;
	RandomAmount surrenderThreshold;
	RandomAmount escapeThreshold;
	gcc::vector<CrewDesc> crewOverride;
	gcc::vector<gcc::string> weaponOverride;
	int weaponOverCount = 0;
	gcc::vector<gcc::string> droneOverride;
	int droneOverCount = 0;
	int shipSeed = 0;
};

struct StoreBox
{
	vptr _vptr = nullptr;
	int itemId = 0;
	int itemBox = 0;
	gcc::string buttonImage;
	Button button;
	GL_Primitive* symbol = nullptr;
	Description desc;
	int count = 0;
	int cost_position = 0;
	ShipManager* shopper = nullptr;
	Equipment* equipScreen = nullptr;
	Blueprint* pBlueprint = nullptr;
	bool bEquipmentBox = false;
	Pad<3> _u0;
	float fIconScale = 0.f;
	Point pushIcon;
};

struct Store : FocusWindow
{
	GL_Texture* box = nullptr;
	TextString headingTitle[4];
	Button page1;
	Button page2;
	ConfirmWindow confirmDialog;
	Button* currentButton = nullptr;
	Description currentDescription;
	gcc::string unavailable;
	gcc::vector<StoreBox*> vStoreBoxes;
	gcc::vector<StoreBox*> vItemBoxes;
	ShipManager* shopper = nullptr;
	int selectedWeapon = 0;
	int selectedDrone = 0;
	InfoBox infoBox;
	Point infoBoxLoc;
	Button exitButton;
	int worldLevel = 0;
	int sectionCount = 0;
	int types[4]{0,0,0,0};
	bool bShowPage2 = false;
	Pad<3> _u0;
	StoreBox* confirmBuy;
	int forceSystemInfoWidth;
};

struct EventDamage
{
	int system = 0;
	int amount = 0;
	int effect = 0;
};

struct StatusEffect
{
	int type = 0;
	int system = 0;
	int amount = 0;
	int target = 0;
};

struct BoardingEvent
{
	gcc::string type;
	int min = 0;
	int max = 0;
	int amount = 0;
	bool breach = false;
	Pad<3> _u0;
};

struct LocationEvent
{
	struct Choice
	{
		LocationEvent* event = nullptr;
		TextString text;
		ChoiceReq requirement;
		bool hiddenReward = false;
		Pad<3> _u0;
	};

	TextString text;
	ShipEvent ship;
	ResourceEvent stuff;
	int environment = 0;
	int environmentTarget = 0;
	bool store = false;
	bool gap_ex_cleared = false;
	Pad<2> _u0;
	int fleetPosition = 0;
	bool beacon = false;
	bool reveal_map = false;
	bool distressBeacon = false;
	bool repair = false;
	int modifyPursuit = 0;
	Store* pStore = nullptr;
	gcc::vector<EventDamage> damage;
	gcc::string quest;
	gcc::vector<StatusEffect> statusEffects;
	gcc::vector<std::pair<gcc::string,gcc::string>> nameDefinitions;
	gcc::string spaceImage;
	gcc::string planetImage;
	gcc::string eventName;
	ResourceEvent reward;
	BoardingEvent boarders;
	gcc::vector<Choice> choices;
	int unlockShip;
	TextString unlockShipText;
	bool secretSector;
};

struct Location
{
	Pointf loc;
	gcc::vector<Location*> connectedLocations;
	bool beacon = false;
	bool known = false;
	Pad<2> _u0;
	int visited = 0;
	bool dangerZone = false;
	bool newSector = false;
	bool nebula = false;
	bool boss = false;
	LocationEvent* event = nullptr;
	ImageDesc planet;
	ImageDesc space;
	ImageDesc beaconImage;
	GL_Texture* imageId = nullptr;
	bool questLoc = false;
	Pad<3> _u1;
	AnimationTracker flashTracker;
	bool fleetChanging = false;
	Pad<3> _u2;
	gcc::string planetImage;
	gcc::string spaceImage;
};

struct DistressButton : TextButton
{
	TextString labels[2];
	bool state = false;
	Pad<3> _u0;
};

struct SectorDescription
{
	gcc::vector<std::pair<gcc::string,RandomAmount>> eventCounts;
	gcc::vector<std::pair<gcc::string,int>> rarities;
	bool unique = false;
	Pad<3> _u0;
	gcc::vector<TextString> names;
	gcc::vector<TextString> shortNames;
	gcc::vector<gcc::string> musicTracks;
	gcc::string type;
	TextString name;
	TextString shortName;
	int minSector = 0;
	bool used = false;
	Pad<3> _u1;
	gcc::string firstEvent;
};

struct Sector
{
	int type = 0;
	bool visited = false;
	bool reachable = false;
	Pad<2> _u0;
	gcc::vector<Sector*> neighbors;
	Point location;
	int level = 0;
	SectorDescription description;
};

struct StarMap : FocusWindow
{
	struct NebulaInfo
	{
		GL_Primitive* primitive = nullptr;
		int x = 0, y = 0,  w = 0, h = 0;
	};

	float visual_size = 0.f;
	gcc::vector<Location*> locations;
	gcc::map<Point, Location*> locations_grid;
	gcc::vector<Location*> temp_path;
	Location* currentLoc = nullptr;
	Location* potentialLoc = nullptr;
	Location* hoverLoc = nullptr;
	Point position;
	int iPopulatedTiles = 0;
	int iLocationCount = 0;
	int iEmptyTiles = 0;
	bool bInitializedDisplay = false;
	Pad<3> _u0;
	Pointf translation;
	bool readyToTravel = false;
	Pad<3> _u1;
	Point dangerZone;
	float dangerZoneRadius = 0.f;
	float shipRotation[2]{ 0.f, 0.f };
	TextButton endButton;
	TextButton waitButton;
	DistressButton distressButton;
	TextButton jumpButton;
	int worldLevel = 0;
	bool bMapRevealed = false;
	Pad<3> _u2;
	int pursuitDelay = 0;
	int sectorNameFont = 0;
	WindowFrame mapBorder;
	GL_Primitive* mapBorderTitle = nullptr;
	GL_Primitive* mapBorderTitleMask = nullptr;
	GL_Texture* mapBorderSector = nullptr;
	GL_Texture* mapInsetTextLeft = nullptr;
	GL_Texture* mapInsetTextMiddle = nullptr;
	GL_Texture* mapInsetTextRight = nullptr;
	GL_Texture* mapInsetTextJump = nullptr;
	GL_Texture* mapInsetWaitDistress = nullptr;
	GL_Primitive* redLight = nullptr;
	GL_Primitive* fuelMessage = nullptr;
	GL_Primitive* waitingMessage = nullptr;
	GL_Primitive* unexplored = nullptr;
	GL_Primitive* explored = nullptr;
	GL_Primitive* danger = nullptr;
	GL_Primitive* warning = nullptr;
	GL_Primitive* yellow_warning = nullptr;
	GL_Primitive* warning_circle = nullptr;
	GL_Primitive* nebula_circle = nullptr;
	GL_Texture* boxGreen[3]{nullptr, nullptr, nullptr};
	GL_Texture* boxPurple[3]{nullptr, nullptr, nullptr};
	GL_Texture* boxWhite[3]{nullptr, nullptr, nullptr};
	GL_Primitive* ship = nullptr;
	GL_Primitive* shipNoFuel = nullptr;
	GL_Primitive* bossShip = nullptr;
	GL_Primitive* dangerZoneEdge = nullptr;
	GL_Texture* dangerZoneTile = nullptr;
	GL_Primitive* dangerZoneAdvance = nullptr;
	GL_Primitive* targetBox = nullptr;
	GL_Primitive* sectorTargetBox_green = nullptr;
	GL_Primitive* sectorTargetBox_yellow = nullptr;
	AnimationTracker targetBoxTimer;
	TextButton closeButton;
	WindowFrame* descBox = nullptr;
	GL_Primitive* shadow = nullptr;
	GL_Primitive* warning_shadow = nullptr;
	GL_Primitive* fuelOverlay = nullptr;
	GL_Primitive* dangerFlash = nullptr;
	GL_Primitive* mapsBottom[3]{nullptr, nullptr, nullptr};
	GL_Texture* dottedLine = nullptr;
	GL_Texture* cross = nullptr;
	GL_Texture* boss_jumps_box = nullptr;
	gcc::vector<ImageDesc> smallNebula;
	gcc::vector<ImageDesc> largeNebula;
	gcc::vector<NebulaInfo> currentNebulas;
	ShipManager* shipManager = nullptr;
	bool outOfFuel = false;
	Pad<3> _u3;
	AnimationTracker waiting;
	int dangerWaitStart = 0;
	AnimationTracker distressAnim;
	bool bTutorialGenerated = false;
	Pad<3> _u4;
	gcc::vector<gcc::string> delayedQuests;
	gcc::vector<Sector*> sectors;
	Sector* currentSector = nullptr;
	Sector* secretSector = nullptr;
	bool bChoosingNewSector = false;
	bool bSecretSector = false;
	Pad<2> _u5;
	Location dummyNewSector;
	int mapsAnalyzed = 0;
	int locationsCreated = 0;
	int shipsCreated = 0;
	gcc::map<gcc::string, int> scrapCollected;
	gcc::map<gcc::string, int> dronesCollected;
	gcc::map<gcc::string, int> fuelCollected;
	gcc::map<gcc::string, int> weaponFound;
	gcc::map<gcc::string, int> droneFound;
	int bossLoc = 0;
	int arrivedAtBase = 0;
	bool reversedPath = false;
	bool bossJumping = false;
	Pad<2> _u6;
	gcc::vector<Location*> boss_path;
	bool bossLevel = false;
	bool boss_wait = false;
	Pad<2> _u7;
	Pointf bossPosition;
	gcc::string forceSectorChoice;
	bool bEnemyShip = false;
	bool bNebulaMap = false;
	bool bInfiniteMode = false;
	Pad<1> _u8;
	gcc::vector<Sector*> lastSectors;
	TextButton closeSectorButton;
	int sectorMapSeed = 0;
	int currentSectorSeed = 0;
	int fuelEventSeed = 0;
	gcc::string lastEscapeEvent;
	bool waitedLast;
	Pad<3> _u9;
	gcc::vector<Store*> storeTrash;
	gcc::vector<std::pair<gcc::string,int>> addedQuests;
	int bossStage = 0;
	TextString bossMessage;
	WarningMessage* bossJumpingWarning = nullptr;
	bool crystalAlienFound = false;
	Pad<3> _u10;
	gcc::map<Location*, bool> foundMap;
	Point sectorMapOffset;
	int potentialSectorChoice = 0;
	int finalSectorChoice = 0;
	gcc::vector<Rect> sectorHitBoxes;
};

struct MenuScreen : FocusWindow
{
	GL_Texture* mainImage = nullptr;
	GL_Primitive* menuPrimitive = nullptr;
	int menuWidth = 0;
	gcc::vector<TextButton*> buttons;
	int command = 0;
	gcc::vector<int> commands;
	Point position;
	ConfirmWindow confirmDialog;
	int tempCommand = 0;
	GenericButton* saveQuit = nullptr;
	bool bShowControls = false;
	Pad<3> _u0;
	Point statusPosition;
	GL_Texture* difficultyBox = nullptr;
	int difficultyWidth = 0;
	gcc::string difficultyLabel;
	gcc::string difficultyText;
	GL_Texture* dlcBox = nullptr;
	int dlcWidth = 0;
	gcc::string dlcLabel;
	gcc::string dlcText;
	GL_Texture* achBox = nullptr;
	GL_Primitive* achBoxPrimitive = nullptr;
	int achWidth = 0;
	gcc::string achLabel;
	gcc::vector<ShipAchievementInfo> shipAchievements;
	int selectedAch = 0;
	InfoBox info;
};

struct CreditScreen
{
	float scroll = 0.f;
	gcc::string shipName, crewString;
	float pausing = 0.f;
	GL_Texture* bg = nullptr;
	gcc::vector<gcc::string> creditNames;
	int lastValidCredit = 0;
	int touchesDown = 0;
	double touchDownTime = 0.0;
	float skipMessageTimer = 0.f;
	Pad<3> _u0;
	Pad<1> _u1;
};

constexpr size_t test = sizeof(gcc::string[2]);
constexpr size_t test2 = sizeof(CreditScreen);

struct GameOver : FocusWindow
{
	gcc::vector<TextButton*> buttons;
	GL_Primitive* box = nullptr;
	int boxWidth = 0;
	int command = 0;
	gcc::vector<int> commands;
	bool bShowStats = false;
	Pad<3> _u0;
	Point position;
	gcc::string gameoverText;
	bool bVictory = false;
	Pad<3> _u1;
	float openedTimer = 0.f;
	CreditScreen credits;
	bool bShowingCredits = false;
	Pad<3> _u2;
};

struct ReactorButton : Button
{
	int tempUpgrade = 0;
	ShipManager* ship = nullptr;
	bool selected = false;
	Pad<3> _u0;
};

struct UpgradeBox
{
	ShipSystem* system = nullptr;
	ShipManager* ship = nullptr;
	const SystemBlueprint* blueprint = nullptr;
	Point location;
	int tempUpgrade = 0;
	Button* currentButton = nullptr;
	gcc::string buttonBaseName;
	Button maxButton;
	Button boxButton;
	bool subsystem = false;
	bool isDummy = false;
	Pad<2> _u0;
	GL_Primitive* dummyBox = nullptr;
};

struct Upgrades : FocusWindow
{
	GL_Texture* box = nullptr;
	gcc::vector<UpgradeBox*> vUpgradeBoxes;
	ShipManager* shipManager = nullptr;
	TextButton undoButton;
	ReactorButton reactorButton;
	InfoBox infoBox;
	Point infoBoxLoc;
	int systemCount = 0;
	int forceSystemInfoWidth = 0;
};

struct CrewManifest : FocusWindow
{
	GL_Primitive* box = nullptr;
	DropBox overBox;
	ShipManager* shipManager = nullptr;
	gcc::vector<CrewEquipBox*> crewBoxes;
	InfoBox infoBox;
	int confirmingDelete = 0;
	ConfirmWindow deleteDialog;
};

struct LanguageChooser : FocusWindow
{
	gcc::vector<TextButton*> buttons;
	int iChoice = -1;
};

struct ControlButton
{
	Rect rect;
	gcc::string value, desc, key;
	int state = 0;
	int descLength = 0;
};

struct ControlsScreen
{
	gcc::vector<ControlButton> buttons[4];
	int selectedButton = -1;
	TextButton defaultButton;
	ConfirmWindow resetDialog;
	Button pageButtons[4];
	int currentPage = 0;
	WindowFrame* customBox = nullptr;
};

struct OptionsScreen : ChoiceBox
{
	Point position;
	Point wipeProfilePosition;
	SlideBar soundVolume, musicVolume;
	bool bCustomizeControls = false;
	Pad<3> _u0;
	ControlsScreen controls;
	TextButton closeButton;
	TextButton wipeProfileButton;
	int choiceFullScreen = 0;
	int choiceVSync = 0;
	int choiceFrameLimit = 0;
	int choiceLowend = 0;
	int choiceColorblind = 0;
	int choiceLanguage = 0;
	int choiceDialogKeys = 0;
	int choiceShowPaths = 0;
	int choiceAchievementPopups = 0;
	int choiceAutoPause = 0;
	int choiceTouchAutoPause = 0;
	int choiceControls = 0;
	int lastFullScreen = 0;
	bool isSoundTouch = false;
	bool isMusicTouch = false;
	Pad<2> _u1;
	LanguageChooser langChooser;
	bool showWipeButton = false;
	Pad<3> _u2;
	ConfirmWindow wipeProfileDialog;
	ChoiceBox restartRequiredDialog;
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
	gcc::vector<FocusWindow*> focusWindows;
	Point pauseTextLoc;
	GL_Primitive* pauseImage = nullptr;
	GL_Primitive* pauseImage2 = nullptr;
	GL_Primitive* pauseImageAuto = nullptr;
	GL_Primitive* pauseCrewImage = nullptr;
	GL_Primitive* pauseDoorsImage = nullptr;
	GL_Primitive* pauseHackingImage = nullptr;
	GL_Primitive* pauseMindImage = nullptr;
	GL_Primitive* pauseRoomImage = nullptr;
	GL_Primitive* pauseTargetImage = nullptr;
	GL_Primitive* pauseTargetBeamImage = nullptr;
	GL_Primitive* pauseTeleportLeaveImage = nullptr;
	GL_Primitive* pauseTeleportArriveImage = nullptr;
	GL_Primitive* flareImage = nullptr;
	Point shipPosition;
	gcc::string locationText;
	gcc::string loadEvent;
	int loadSector = -1;
	ChoiceBox choiceBox;
	bool gameover = false;
	bool alreadyWon = false;
	bool outOfFuel = false;
	Pad<1> _u0;
	MenuScreen menuBox;
	Pad<4> _u1;
	GameOver gameOverScreen;
	OptionsScreen optionsBox;
	bool bPaused = false, bAutoPaused = false, menu_pause = false, event_pause = false, touch_pause = false;
	Pad<3> _u2;
	int touchPauseReason = 0;
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
	float pause_anim_time = 0.f, pause_animation = 0.f;
	gcc::vector<Store*> storeTrash;
	TimerHelper flickerTimer;
	TimerHelper showtimer;
	bool bHideUI = false;
	Pad<3> _u3;
	CompleteShip* enemyShip = nullptr;
	bool waitLocation = false;
	bool lastLocationwait = false;
	bool dangerLocation = false;
	Pad<1> _u4;
	gcc::vector<int> commandKey;
	bool jumpComplete = false;
	Pad<3> _u5;
	int mapId = 0;
	ConfirmWindow leaveCrewDialog;
	bool secretSector = false;
	Pad<3> _u6;
	int activeTouch = 0;
	bool activeTouchIsButton = false;
	bool activeTouchIsCrewBox = false;
	bool activeTouchIsShip = false;
	bool activeTouchIsNull = false;
	gcc::vector<int> extraTouches;
	bool bTutorialWasRunning = false;
	bool focusAteMouse = false;
	bool choiceBoxOpen = false;
	Pad<1> _u7;
	int systemDetailsWidth = 0;
	ChoiceBox writeErrorDialog;
	bool suppressWriteError = false;
	Pad<3> _u8;
};

struct BossShip : CompleteShip
{
	int currentStage = 0;
	TimerHelper powerTimer;
	int powerCount = 0;
	gcc::vector<int> crewCounts;
	bool bDeathBegan = false;
	Pad<3> _u0;
	int nextStage = 0;
};

struct WorldManager
{
	CompleteShip* playerShip = nullptr;
	BossShip* bossShip = nullptr;
	SpaceManager space;
	int currentDifficulty = 0;
	gcc::vector<CompleteShip*> ships;
	StarMap starMap;
	CommandGui* commandGui = nullptr;
	LocationEvent* baseLocationEvent = nullptr;
	LocationEvent* lastLocationEvent = nullptr;
	ShipEvent currentShipEvent;
	gcc::vector<StatusEffect> currentEffects;
	gcc::string startingText;
	Location* newLocation = nullptr;
	bool bStartedGame = false;
	bool bLoadingGame = false;
	bool vAutoSaved = false;
	bool bExtraChoice = false;
	gcc::vector<int> choiceHistory;
	gcc::string generatedEvent;
	TextString lastMainText;
	int playerCrewCount = 0;
	int killedCrew = 0;
	int playerHull = 0;
	gcc::vector<int> blueRaceChoices;
	int lastSelectedCrewSeed = -1;
	bool testingBlueprints = false;
	Pad<3> _u0;
	gcc::vector<LocationEvent::Choice> originalChoiceList;
};

struct ShipButton : Button
{
	GL_Texture* iShipImage = nullptr;
	bool bShipLocked = false;
	bool bLayoutLocked = false;
	bool bNoExist = false;
	Pad<1> _u0;
	gcc::vector<CAchievement*> achievements;
	int iSelectedAch = 0;
	bool bSelected = false;
	Pad<3> _u1;
};

struct UnlockArrow
{
	int direction = 0, status = 0;
	Rect shape;
};

struct ShipSelect
{
	Point position, titlePos;
	gcc::vector<GL_Primitive*> shipListBase;
	gcc::vector<ShipButton*> shipButtons;
	gcc::vector<UnlockArrow> arrows;
	bool bOpen = false;
	Pad<3> _u0;
	int selectedShip = -1;
	InfoBox infoBox;
	int currentType = 0;
	TextButton typeA, typeB, typeC;
	TextButton confirm;
	bool bConfirmed = false;
	Pad<3> _u1;
	int activeTouch = 0;
	ChoiceBox tutorial;
	int tutorialPage = 0;
};

struct ShipBuilder
{
	ShipManager* currentShip = nullptr;
	GL_Primitive* nameBoxPrimitive = nullptr;
	GL_Primitive* enableAdvancedPrimitive = nullptr;
	Button resetButton, clearButton;
	TextButton startButton, backButton, renameButton;
	Button leftButton, rightButton;
	TextButton listButton, showButton;
	TextButton easyButton, normalButton, hardButton;
	TextButton typeA, typeB, typeC;
	Point typeALoc, typeBLoc, typeCLoc;
	TextButton randomButton;
	TextButton advancedOffButton, advancedOnButton;
	gcc::vector<GenericButton*> buttons;
	gcc::vector<Animation> animations;
	gcc::vector<CrewCustomizeBox*> vCrewBoxes;
	bool bOpen = false;
	Pad<3> _u0;
	GL_Primitive* baseImage = nullptr;
	GL_Primitive* shipSelectBox = nullptr;
	GL_Primitive* shipAchBox = nullptr;
	GL_Primitive* shipEquipBox = nullptr;
	GL_Primitive* startButtonBox = nullptr;
	GL_Primitive* advancedButtonBox = nullptr;
	int typeAOffset = 0, typeBOffset = 0, typeCOffset = 0;
	int shipAchPadding = 0, advancedTitleOffset = 0;
	gcc::vector<EquipmentBox*> vEquipmentBoxes;
	InfoBox infoBox;
	gcc::vector<SystemCustomBox*> sysBoxes;
	int shoppingId = -1;
	int currentSlot = -1;
	int currentBox = -1;
	bool bDone = false;
	Pad<3> _u1;
	ShipBlueprint* ships[30]{
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
		nullptr, nullptr, nullptr, nullptr, nullptr, nullptr
	};
	int currentShipId = 0;
	int storeIds[4]{ 0, 0, 0, 0 };
	bool bRenaming = false;
	Pad<3> _u2;
	gcc::string currentName;
	bool bShowRooms = false;
	bool bCustomizingCrew = false;
	Pad<2> _u3;
	Animation walkingMan;
	Pointf walkingManPos;
	ShipSelect shipSelect;
	ChoiceBox introScreen;
	bool bShowedIntro = false;
	Pad<3> _u4;
	int currentType = 0;
	TextInput nameInput;
	int activeTouch = 0;
	bool activeTouchIsShip = false;
	int shipDragActive = false;
	int shipDragVertical = false;
	Point shipDragOffset;
	gcc::vector<ShipAchievementInfo> shipAchievements;
	int selectedAch = -1;
	GL_Texture* arrow = nullptr;
	WindowFrame* descBox = nullptr;
	AnimationTracker tracker;
	bool encourageShipList = false;
	Pad<3> _u5;
};

struct MainMenu
{
	bool bOpen = false;
	Pad<3> _u0;
	int activeTouch = 0;
	GL_Texture* background = nullptr;
	GL_Texture* glowy = nullptr;
	AnimationTracker glowTracker;
	Button continueButton;
	Button startButton;
	Button helpButton;
	Button statButton;
	Button optionsButton;
	Button creditsButton;
	Button quitButton;
	bool itbButtonActive = false;
	Pad<3> _u1;
	Button itbButton;
	Animation* itbAnim = nullptr;
	gcc::vector<Button*> buttons;
	int finalChoice = -1;
	ShipBuilder shipBuilder;
	bool bScoreScreen = false;
	Pad<3> _u2;
	OptionsScreen optionScreen;
	bool bSelectSave = false;
	Pad<3> _u3;
	ConfirmWindow confirmNewGame;
	ChoiceBox changelog;
	bool bCreditScreen = false;
	Pad<7> _u4;
	CreditScreen credits;
	bool bChangedLogin = false;
	Pad<3> _u5;
	gcc::vector<CrewMember*> testCrew;
	bool bChangedScreen = false;
	Pad<3> _u6;
	gcc::string error;
	Pad<4> _u7;
};

struct CApp : CEvent
{
	bool Running = false;
	bool shift_held = false;
	Pad<2> _u0;
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
	Pad<3> _u1;
	float mouseModifier_x = 0.f;
	float mouseModifier_y = 0.f;
	GL_FrameBuffer* frameBuffer = nullptr;
	bool fboSupport = false;
	Pad<3> _u2;
	int x_bar = -1;
	int y_bar = -1;
	bool lCtrl = false;
	bool useFrameBuffer = false;
	bool manualResolutionError = false;
	Pad<1> _u3;
	int manualResErrorX = -1;
	int manualResErrorY = -1;
	bool nativeFullScreenError = false;
	bool fbStretchError = false;
	Pad<2> _u4;
	gcc::string lastLanguage;
	bool inputFocus = false;
	bool useDirect3D = false;
};

struct HotkeyDesc
{
	gcc::string name;
	int key = -1;
};

struct CrewMemberFactory
{
	int playerCrew = 0;
	int enemyCrew = 0;
	int enemyCloneCount = 0;
	gcc::vector<CrewMember*> crewMembers;
	gcc::vector<CrewMember*> lostMembers;
};

struct SettingValues
{
	int fullscreen = 0;
	int currentFullscreen = 0;
	int lastFullscreen = 0;
	int sound = 0;
	int music = 0;
	int difficulty = 0;
	bool commandConsole = false;
	bool altPause = false;
	bool touchAutoPause = false;
	bool lowend = false;
	bool fbError = false;
	Pad<3> _u0;
	gcc::string language;
	bool languageSet = false;
	Pad<3> _u1;
	Point screenResolution;
	int dialogKeys = 0;
	bool logging = false;
	bool bShowChangelog = false;
	Pad<2> _u2;
	int loadingSaveVersion = 0;
	bool achPopups = false;
	bool vsync = false;
	bool frameLimit = false;
	bool manualResolution = false;
	bool manualWindowed = false;
	bool manualStretched = false;
	bool showPaths = false;
	bool swapTextureType = false;
	bool colorblind = false;
	Pad<3> _u3;
	std::array<gcc::vector<HotkeyDesc>, 4> hotkeys;
	bool holdingModifier = false;
	bool bDlcEnabled = false;
	Pad<2> _u4;
	int openedList = 0;
	bool beamTutorial = false;
	Pad<3> _u5;
};

struct BlueprintManager
{
	int rarityTotal = 0;
	gcc::map<gcc::string, ShipBlueprint> shipBlueprints;
	gcc::map<gcc::string, WeaponBlueprint> weaponBlueprints;
	gcc::map<gcc::string, DroneBlueprint> droneBlueprints;
	gcc::map<gcc::string, AugmentBlueprint> augmentBlueprints;
	gcc::map<gcc::string, CrewBlueprint> crewBlueprints;
	gcc::map<gcc::string, bool> nameList;
	gcc::map<gcc::string, gcc::string> shortNames;
	gcc::map<gcc::string, gcc::map<gcc::string,bool>> languageNameLists;
	gcc::map<gcc::string, ItemBlueprint> itemBlueprints;
	gcc::map<gcc::string, SystemBlueprint> systemBlueprints;
	gcc::map<gcc::string, gcc::vector<gcc::string>> blueprintLists;
	gcc::vector<gcc::string> currentNames;
};

struct PowerManager
{
	std::pair<int, int> currentPower{ 0, 0 };
	int over_powered = 0;
	float fFuel = 0.f;
	bool failedPowerup = false;
	int iTempPowerCap = 0;
	int iTempPowerLoss = 0;
	int iTempDividePower = 0;
	int iHacked = 0;
	std::pair<int, int> batteryPower{ 0, 0 };
};

struct PowerManagerContainer
{
	gcc::vector<PowerManager> powerManagers;
};

struct State
{
	CApp* app = nullptr;
	CrewMemberFactory* crewMemberFactory = nullptr;
	SettingValues* settingValues = nullptr;
	BlueprintManager* blueprints = nullptr;
	PowerManagerContainer* powerManagerContainer = nullptr;
};

constexpr size_t __SIZE_TEST = sizeof(Button);
constexpr uintptr_t CAppPtr = 0x4C5020;
constexpr uintptr_t CrewMemberFactoryPtr = 0x4C6E40;
constexpr uintptr_t SettingValuesPtr = 0x4C8CA0;
constexpr uintptr_t BlueprintManagerPtr = 0x4CBD60;
constexpr uintptr_t PowerManagerContainerPtr = 0x4CCA40;

}
