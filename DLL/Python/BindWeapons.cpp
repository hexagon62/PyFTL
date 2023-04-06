#include "Bind.hpp"
#include "../State/State.hpp"

namespace python_bindings
{

void bindWeapons(py::module_& module)
{
	py::enum_<WeaponType>(module, "WeaponType", "The type of weapon")
		.value("Invalid", WeaponType::Invalid)
		.value("Laser", WeaponType::Laser)
		.value("Beam", WeaponType::Beam)
		.value("Burst", WeaponType::Burst)
		.value("Missiles", WeaponType::Missiles)
		.value("Bomb", WeaponType::Bomb)
		;

	py::class_<BoostPower>(module, "BoostPower", "The definition of a weapon's boost")
		.def_readonly("type", &BoostPower::type, "The type of the boost power")
		.def_readonly("amount", &BoostPower::amount, "The value used for the boost")
		.def_readonly("count", &BoostPower::count, "The amount of times boosted")
		;

	py::class_<Damage>(module, "Damage", "Damage that a weapon's projectile or other source will do")
		.def_readonly_static("HARDCODED_CREW_DAMAGE_FACTOR", &Damage::HARDCODED_CREW_DAMAGE_FACTOR, "What the damage to a crewmember must be a multiple of")
		.def_readonly("normal", &Damage::normal, "Normal damage - 1 point is also 1 system damage and 15 crew damage")
		.def_readonly("ion", &Damage::ion, "Ion damage")
		.def_readonly("system", &Damage::system, "System damage")
		.def_readonly("crew", &Damage::crew, "Crew damage")
		.def_readonly("fire_chance", &Damage::fireChance, "Chance of a fire")
		.def_readonly("breach_chance", &Damage::breachChance, "Chance of a breach")
		.def_readonly("stun_chance", &Damage::stunChance, "Chance of stunning a crew member")
		.def_readonly("pierce", &Damage::pierce, "Level of shields pierced")
		.def_readonly("stun_time", &Damage::stunTime, "Time it will stun a crew member (guaranteed, doesn't involve stun_chance)")
		.def_readonly("hull_bonus", &Damage::hullBonus, "If the damage is doubled for empty rooms")
		.def_readonly("friendly_fire", &Damage::friendlyFire, "If the weapon can damage your own crew")
		;

	py::class_<Weapon>(module, "Weapon", "A weapon")
		.def_readonly("cooldown", &Weapon::cooldown, "The time the weapon must wait to fire again, factoring in augments and manning")
		.def_readonly("blueprint", &Weapon::blueprint, "The blueprint of the weapon")
		.def_readonly("auto_fire", &Weapon::autoFire, "Whether or not the weapon is set to auto-fire")
		.def_readonly("fire_when_ready", &Weapon::fireWhenReady, "Whether or not the weapon will fire once off cooldown")
		.def_readonly("powered", &Weapon::powered, "If the weapon is powered")
		.def_readonly("artillery", &Weapon::artillery, "If the weapon is artillery")
		.def_readonly("targeting_player", &Weapon::targetingPlayer, "If the weapon is targeting the player")
		.def_readonly("firing_angle", &Weapon::firingAngle, "The angle projectiles will leave the weapon?")
		.def_readonly("entry_angle", &Weapon::entryAngle, "The angle projectiles will enter the enemy's space at")
		.def_readonly("mount", &Weapon::mount, "The weapon mount")
		.def_readonly("zoltan_power", &Weapon::zoltanPower, "The amount of power from Zoltan crew")
		.def_readonly("hack_level", &Weapon::hackLevel, "The hacking status")
		.def_readonly("boost", &Weapon::boost, "The boost status")
		.def_readonly("charge", &Weapon::charge, "How many charges the weapon has")
		.def_readonly("shot_timer", &Weapon::shotTimer, "Timer for each firing animation")
		.def_readonly("target_points", &Weapon::targetPoints, "List of points the projectiles are aimed at")
		;

	py::enum_<ProjectileType>(module, "ProjectileType", "The type of a projectile")
		.value("Invalid", ProjectileType::Invalid)
		.value("Miscellaneous", ProjectileType::Miscellaneous)
		.value("Laser", ProjectileType::Laser)
		.value("Asteroid", ProjectileType::Asteroid)
		.value("Missile", ProjectileType::Missile)
		.value("Bomb", ProjectileType::Bomb)
		.value("Beam", ProjectileType::Beam)
		.value("ABS", ProjectileType::ABS)
		;

	py::class_<Beam>(module, "Beam", "Info for a fired beam")
		.def_readonly("begin", &Beam::begin, "The start point of the beam attack")
		.def_readonly("end", &Beam::end, "The end point of the beam attack")
		.def_readonly("pierced", &Beam::pierced, "If the beam pierced the shields")
		.def_readonly("damaged_super_shield", &Beam::damagedSuperShield, "If the beam damaged a super shield")
		;

	py::class_<Bomb>(module, "Bomb", "Info for a fired bomb")
		.def_readonly("explosion_timer", &Bomb::explosionTimer, "How long until the bomb explodes")
		.def_readonly("damaged_super_shield", &Bomb::damagedSuperShield, "If the bomb damaged a super shield")
		.def_readonly("bypassed_super_shield", &Bomb::bypassedSuperShield, "If the bomb bypassed a super shield")
		;

	py::class_<Projectile>(module, "Projectile", "A projectile")
		.def_readonly("type", &Projectile::type, "The projectile's type")
		.def_readonly("position", &Projectile::position, "The projectile's position")
		.def_readonly("position_last", &Projectile::positionLast, "The projectile's last position")
		.def_readonly("target", &Projectile::target, "The projectile's target point")
		.def_readonly("speed", &Projectile::speed, "The projectile's speed")
		.def_readonly("lifespan", &Projectile::lifespan, "The projectile's lifespan")
		.def_readonly("heading", &Projectile::heading, "The projectile's heading")
		.def_readonly("entry_angle", &Projectile::entryAngle, "The projectile's entry angle")
		.def_readonly("angle", &Projectile::angle, "The projectile's angle")
		.def_readonly("spin_speed", &Projectile::spinSpeed, "The projectile's spin speed")
		.def_readonly("player", &Projectile::player, "If the projectile is player owned")
		.def_readonly("player_space", &Projectile::playerSpace, "If the projectile is in the player space")
		.def_readonly("player_space_is_destination", &Projectile::playerSpaceIsDestination, "If the projectile is headed to the player space")
		.def_readonly("dead", &Projectile::dead, "If the projectile is dead")
		.def_readonly("missed", &Projectile::missed, "If the projectile missed")
		.def_readonly("hit", &Projectile::hit, "If the projectile hit")
		.def_readonly("passed", &Projectile::passed, "If the projectile passed its target")
		.def_readonly("beam", &Projectile::beam, "Beam-specific info")
		.def_readonly("bomb", &Projectile::bomb, "Bomb-specific info")
		;
}

}
