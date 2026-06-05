#pragma once

#include "../Menu/Submenu.h"

namespace Menu {

class WeaponSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "weapon"; }
	const char* Title() const override { return "Weapon Options"; }
	void Draw() override;
};

class WeaponFavouritesSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "weapon_favourites"; }
	const char* Title() const override { return "Favourites"; }
	void Draw() override;
};

class WeaponCategoriesSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "weapon_categories"; }
	const char* Title() const override { return "Individual Weapons"; }
	void Draw() override;
};

class WeaponInCategorySubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "weapon_in_category"; }
	const char* Title() const override { return "Weapons"; }
	void Draw() override;
};

class WeaponInItemSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "weapon_in_item"; }
	const char* Title() const override { return "Weapon"; }
	void Draw() override;
};

class WeaponInItemModsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "weapon_in_item_mods"; }
	const char* Title() const override { return "Attachments & Tints"; }
	void Draw() override;
};

class WeaponLoadoutsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "weapon_loadouts"; }
	const char* Title() const override { return "Loadouts"; }
	void Draw() override;
};

class WeaponLoadoutItemSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "weapon_loadout_item"; }
	const char* Title() const override { return "Loadout File"; }
	void Draw() override;
};

class WeaponParachuteSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "weapon_parachute"; }
	const char* Title() const override { return "Parachute"; }
	void Draw() override;
};

class WeaponLaserSightSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "weapon_laser_sight"; }
	const char* Title() const override { return "Laser Sight"; }
	void Draw() override;
};

class ForgeGunSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "weapon_forge_gun"; }
	const char* Title() const override { return "Forge Gun"; }
	void Draw() override;
};

class GravityGunSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "weapon_gravity_gun"; }
	const char* Title() const override { return "Gravity Gun"; }
	void Draw() override;
};

class KaboomGunSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "weapon_kaboom_gun"; }
	const char* Title() const override { return "Kaboom Gun"; }
	void Draw() override;
};

class TriggerFxGunSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "weapon_trigger_fx_gun"; }
	const char* Title() const override { return "TriggerFX Gun"; }
	void Draw() override;
};

class BulletGunSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "weapon_bullet_gun"; }
	const char* Title() const override { return "Bullet Gun"; }
	void Draw() override;
};

class PedGunSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "weapon_ped_gun"; }
	const char* Title() const override { return "Ped Gun"; }
	void Draw() override;
};

class PedGunAllPedsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "weapon_ped_gun_all"; }
	const char* Title() const override { return "All Peds"; }
	void Draw() override;
};

class ObjectGunSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "weapon_object_gun"; }
	const char* Title() const override { return "Object & Vehicle Gun"; }
	void Draw() override;
};

class WeaponVehicleCatsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "weapon_vehicle_cats"; }
	const char* Title() const override { return "Vehicles"; }
	void Draw() override;
};

class WeaponObjectSpawnerSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "weapon_object_spawner"; }
	const char* Title() const override { return "All Objects"; }
	void Draw() override;
};

}