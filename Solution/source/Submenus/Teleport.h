#pragma once

#include "../Menu/Submenu.h"

#include "../Util/GTAmath.h"

namespace Menu {

class TeleportSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "teleport"; }
	const char* Title() const override { return "Locations"; }
	void Draw() override;
};

class TeleportCustomCoordsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "teleport_custom_coords"; }
	const char* Title() const override { return "Custom Coordinates"; }
	void Draw() override;

private:
	bool grabbedCoords = false;
	Vector3 customTeleLoc;
};

class TeleportSelectedCategorySubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "teleport_selected_category"; }
	const char* Title() const override { return "Category"; }
	void Draw() override;
};

class TeleportBlipListSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "teleport_blip_list"; }
	const char* Title() const override { return "Map Blips"; }
	void Draw() override;
};

class TeleportSavedLocationsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "teleport_saved_locations"; }
	const char* Title() const override { return "Favourites"; }
	void Draw() override;
};

}