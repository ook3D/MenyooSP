#pragma once

#include "../Menu/Submenu.h"

#include "../Util/GTAmath.h"

#include <string>
#include <vector>

namespace Menu {

struct BikerClubhouseLocation
{
	std::string name;
	Vector3 pos;
	std::string ipl;
};

struct BikerClubhouseInteriorOption
{
	std::string name;
	std::string value;
};

struct BikerClubhouseInfoStructure
{
	const BikerClubhouseLocation* location;
	unsigned char muralOption;
	unsigned char wallsOption;
	unsigned char decorativeOption;
	unsigned char furnishingsOption;
	unsigned char modBoothOption;
	unsigned char gunLockerOption;
};

class TeleportBikerClubhousesSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "teleport_biker_clubhouses"; }
	const char* Title() const override { return "Clubhouses"; }
	void Draw() override;

	static const std::vector<BikerClubhouseLocation>& Locations();
	static const std::vector<BikerClubhouseInteriorOption>& MuralOptions();
	static const std::vector<BikerClubhouseInteriorOption>& WallsOptions();
	static const std::vector<BikerClubhouseInteriorOption>& DecorativeOptions();
	static const std::vector<BikerClubhouseInteriorOption>& FurnishingsOptions();
	static const std::vector<BikerClubhouseInteriorOption>& ModBoothOptions();
	static const std::vector<BikerClubhouseInteriorOption>& GunLockerOptions();
	static BikerClubhouseInfoStructure& CurrentInfo();
};

class TeleportBikerClubhousesInLocSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "teleport_biker_clubhouses_in_loc"; }
	const char* Title() const override { return "Clubhouse"; }
	void Draw() override;
};

struct BikerBusinessLocation
{
	std::string name;
	Vector3 pos;
	std::string ipl;
	std::vector<std::string> options;
};

struct BikerBusinessInfoStructure
{
	const BikerBusinessLocation* location;
	unsigned char option;
};

class TeleportBikerBusinessesSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "teleport_biker_businesses"; }
	const char* Title() const override { return "Businesses"; }
	void Draw() override;

	static const std::vector<BikerBusinessLocation>& Locations();
	static BikerBusinessInfoStructure& CurrentInfo();
};

class TeleportBikerBusinessesInLocSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "teleport_biker_businesses_in_loc"; }
	const char* Title() const override { return "Business"; }
	void Draw() override;
};

}