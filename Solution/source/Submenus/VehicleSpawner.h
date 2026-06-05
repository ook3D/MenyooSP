#pragma once

#include "../Menu/Submenu.h"

namespace Menu {

class VehicleSpawnerSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_spawner"; }
	const char* Title() const override { return "Vehicles"; }
	void Draw() override;
};

class VehicleSpawnerOptionsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_spawner_options"; }
	const char* Title() const override { return "Spawn Settings"; }
	void Draw() override;
};

class VehicleSpawnerAllCatsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_spawner_all_cats"; }
	const char* Title() const override { return "Category"; }
	void Draw() override;
};

class VehicleSpawnerDlcSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_spawner_dlc"; }
	const char* Title() const override { return "DLC"; }
	void Draw() override;
};

class VehicleSpawnerDlcSelectionSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_spawner_dlc_selection"; }
	const char* Title() const override { return "DLC Vehicles"; }
	void Draw() override;
};

class VehicleSpawnerFavouritesSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_spawner_favourites"; }
	const char* Title() const override { return "Favourites"; }
	void Draw() override;
};

class VehicleSaverSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_saver"; }
	const char* Title() const override { return "Saved Vehicles"; }
	void Draw() override;
};

class VehicleSaverInItemSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_saver_in_item"; }
	const char* Title() const override { return "Saved Vehicle"; }
	void Draw() override;
};

}