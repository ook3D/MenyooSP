#include "TeleportIeVehicleWarehouses.h"

#include "../Menu/Engine.h"
#include "../Menu/SubmenuRegistry.h"
#include "../Menu/Routine.h"        // g_Ped1
#include "PlayerRuntime.h"          // g_Ped1

#include "../Natives/natives2.h"
#include "../Scripting/GTAentity.h"
#include "../Scripting/GTAped.h"
#include "../Util/GTAmath.h"

#include "Teleport/TeleMethods.h"   // TeleportNetPed

#include <string>
#include <vector>

namespace Menu {

const std::vector<IeWarehouseLocation>& TeleportIeVehicleWarehousesSubmenu::Locations()
{
	static const std::vector<IeWarehouseLocation> kLocations
	{
		{ "Warehouse",      { 973.5615f,  -2999.5610f, -39.6470f }, "imp_impexp_interior_placement_interior_1_impexp_intwaremed_milo_" },
		{ "Vehicle Bunker", { 1001.2706f, -2997.8494f, -47.6470f }, "imp_impexp_interior_placement_interior_3_impexp_int_02_milo_" },
	};
	return kLocations;
}

const std::vector<IeWarehouseInteriorOption>& TeleportIeVehicleWarehousesSubmenu::StyleSetOptions()
{
	static const std::vector<IeWarehouseInteriorOption> kStyle
	{
		{ "None", "" },
		{ "Basic",   "Basic_style_set" },
		{ "Urban",   "Urban_style_set" },
		{ "Branded", "Branded_style_set" },
	};
	return kStyle;
}

const std::vector<IeWarehouseInteriorOption>& TeleportIeVehicleWarehousesSubmenu::PumpOptions()
{
	static const std::vector<IeWarehouseInteriorOption> kPump
	{
		{ "None", "Pump_00" },
		{ "1", "Pump_01" }, { "2", "Pump_02" }, { "3", "Pump_03" },
		{ "4", "Pump_04" }, { "5", "Pump_05" }, { "6", "Pump_06" },
		{ "7", "Pump_07" }, { "8", "Pump_08" },
	};
	return kPump;
}

IeWarehouseInfoStructure& TeleportIeVehicleWarehousesSubmenu::CurrentInfo()
{
	static IeWarehouseInfoStructure info{ nullptr, 0, 0 };
	return info;
}

namespace {

struct IeOptionArrayEntry
{
	const char* name;
	unsigned char* ptr;
	const std::vector<IeWarehouseInteriorOption>* arr;
};

std::vector<IeOptionArrayEntry> BuildOptionArrays()
{
	auto& info = TeleportIeVehicleWarehousesSubmenu::CurrentInfo();
	return {
		{ "Style",         &info.styleSetOption, &TeleportIeVehicleWarehousesSubmenu::StyleSetOptions() },
		{ "Basement Pump", &info.pumpOption,     &TeleportIeVehicleWarehousesSubmenu::PumpOptions() },
	};
}

void CreateWarehouse(IeWarehouseInfoStructure& info)
{
	if (info.location == nullptr) return;
	auto& loc = *info.location;
	auto& pos = loc.pos;

	SET_INSTANCE_PRIORITY_MODE(true);
	ON_ENTER_MP();
	REQUEST_IPL(loc.ipl.c_str());
	int interior = GET_INTERIOR_AT_COORDS(pos.x, pos.y, pos.z);
	DISABLE_INTERIOR(interior, true);
	PIN_INTERIOR_IN_MEMORY(interior);
	DISABLE_INTERIOR(interior, false);
	SET_INSTANCE_PRIORITY_MODE(false);
	WAIT(200);

	const auto optionArrays = BuildOptionArrays();
	for (auto& oa : optionArrays)
	{
		for (auto& o : *oa.arr)
			DEACTIVATE_INTERIOR_ENTITY_SET(interior, o.value.c_str());
	}
	for (auto& oa : optionArrays)
	{
		ACTIVATE_INTERIOR_ENTITY_SET(interior, oa.arr->at(*oa.ptr).value.c_str());
	}
	REFRESH_INTERIOR(interior);
}

void TeleportPedToWarehouse(GTAentity ped, IeWarehouseInfoStructure& info)
{
	if (info.location != nullptr)
	{
		TeleportNetPed(ped, info.location->pos);
	}
}

}

void TeleportIeVehicleWarehousesSubmenu::Draw()
{
	DrawTitle();

	auto& info = CurrentInfo();
	for (auto& loc : Locations())
	{
		if (DrawOption(loc.name))
		{
			info.location = &loc;
			NavigateTo("teleport_ie_vehicle_warehouses_in_loc");
		}
	}
}

void TeleportIeVehicleWarehousesInLocSubmenu::Draw()
{
	auto& info = TeleportIeVehicleWarehousesSubmenu::CurrentInfo();
	if (info.location == nullptr)
	{
		Engine* engine = Engine::Current();
		if (engine) engine->GoBack();
		return;
	}

	GTAped ped = g_Ped1;
	Engine* engine = Engine::Current();
	if (engine) engine->AddTitle(info.location->name);

	const auto optionArrays = BuildOptionArrays();
	for (auto& o : optionArrays)
	{
		const std::vector<std::string> singleItem{ o.arr->at(*o.ptr).name };
		InputResult input = engine
			? engine->AddTextList(o.name, 0, singleItem)
			: InputResult{};
		if (input.rightPressed && *o.ptr < o.arr->size() - 1) (*o.ptr)++;
		if (input.leftPressed && *o.ptr > 0) (*o.ptr)--;
	}

	if (DrawOption("Build Warehouse"))
	{
		DO_SCREEN_FADE_OUT(50);
		CreateWarehouse(info);
		TeleportPedToWarehouse(ped, info);
		DO_SCREEN_FADE_IN(200);
		return;
	}
}

}
REGISTER_SUBMENU(::Menu::TeleportIeVehicleWarehousesSubmenu)
REGISTER_SUBMENU(::Menu::TeleportIeVehicleWarehousesInLocSubmenu)
