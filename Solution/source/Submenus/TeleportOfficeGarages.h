#pragma once

#include "../Menu/Submenu.h"

#include "../Util/GTAmath.h"

#include <string>
#include <utility>
#include <vector>

namespace Menu {

struct OfficeGarageLocation
{
	Vector3 pos;
	std::string ipl;
};

struct OfficeGarageInteriorOption
{
	std::string name;
	std::string value;
};

struct OfficeGarageInfoStructure
{
	const std::pair<std::string, const std::vector<OfficeGarageLocation>*>* location;
	unsigned char garageId;
	unsigned char floorOption;
	unsigned char decorOption;
	unsigned char lightingOption;
	unsigned char numStyleOption;
};

class TeleportOfficeGaragesSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "teleport_office_garages"; }
	const char* Title() const override { return "Office Garages"; }
	void Draw() override;

	using LocationPair = std::pair<std::string, const std::vector<OfficeGarageLocation>*>;

	static const std::vector<std::string>& GarageIds();
	static const std::vector<OfficeGarageInteriorOption>& FloorOptions();
	static const std::vector<OfficeGarageInteriorOption>& DecorOptions();
	static const std::vector<OfficeGarageInteriorOption>& LightingOptions();
	static const std::vector<OfficeGarageInteriorOption>& NumStyleOptions();
	static const std::vector<LocationPair>& Locations();
	static OfficeGarageInfoStructure& CurrentInfo();
};

class TeleportOfficeGaragesInLocSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "teleport_office_garages_in_loc"; }
	const char* Title() const override { return "Office Garage"; }
	void Draw() override;
};

}