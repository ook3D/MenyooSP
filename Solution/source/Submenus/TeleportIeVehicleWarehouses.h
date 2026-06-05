#pragma once

#include "../Menu/Submenu.h"

#include "../Util/GTAmath.h"

#include <string>
#include <vector>

namespace Menu {

struct IeWarehouseLocation
{
	std::string name;
	Vector3 pos;
	std::string ipl;
};

struct IeWarehouseInteriorOption
{
	std::string name;
	std::string value;
};

struct IeWarehouseInfoStructure
{
	const IeWarehouseLocation* location;
	unsigned char styleSetOption;
	unsigned char pumpOption;
};

class TeleportIeVehicleWarehousesSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "teleport_ie_vehicle_warehouses"; }
	const char* Title() const override { return "Vehicle Warehouses"; }
	void Draw() override;

	static const std::vector<IeWarehouseLocation>& Locations();
	static const std::vector<IeWarehouseInteriorOption>& StyleSetOptions();
	static const std::vector<IeWarehouseInteriorOption>& PumpOptions();
	static IeWarehouseInfoStructure& CurrentInfo();
};

class TeleportIeVehicleWarehousesInLocSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "teleport_ie_vehicle_warehouses_in_loc"; }
	const char* Title() const override { return "Vehicle Warehouse"; }
	void Draw() override;
};

}