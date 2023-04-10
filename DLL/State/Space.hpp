#pragma once

#include "Point.hpp"
#include "RandomAmount.hpp"
#include "EnvironmentType.hpp"

#include <optional>
#include <vector>
#include <array>

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
	bool dying = false;
	bool missed = false;
	bool hit = false;
	bool passed = false;
	std::optional<Beam> beam;
	std::optional<Bomb> bomb;
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

struct Space
{
	std::vector<Projectile> projectiles;
	EnvironmentType environment = EnvironmentType::Normal;
	std::optional<AsteroidInfo> asteroids;
	bool environmentTargetingEnemy = false;
	std::pair<float, float> hazardTimer;
};
