#include "TeleportBikerInteriors.h"

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

const std::vector<BikerClubhouseLocation>& TeleportBikerClubhousesSubmenu::Locations()
{
	static const std::vector<BikerClubhouseLocation> kLocations
	{
		{ "1 floor",  { 1109.1124f, -3164.1536f, -37.5186f }, "bkr_biker_interior_placement_interior_0_biker_dlc_int_01_milo_" },
		{ "2 floors", { 998.3676f,  -3164.6531f, -38.9073f }, "bkr_biker_interior_placement_interior_1_biker_dlc_int_02_milo_" },
	};
	return kLocations;
}

const std::vector<BikerClubhouseInteriorOption>& TeleportBikerClubhousesSubmenu::MuralOptions()
{
	static const std::vector<BikerClubhouseInteriorOption> kMural
	{
		{ "None", "Mural_00" },
		{ "1", "Mural_01" }, { "2", "Mural_02" }, { "3", "Mural_03" },
		{ "4", "Mural_04" }, { "5", "Mural_05" }, { "6", "Mural_06" },
		{ "7", "Mural_07" }, { "8", "Mural_08" }, { "9", "Mural_09" },
	};
	return kMural;
}

const std::vector<BikerClubhouseInteriorOption>& TeleportBikerClubhousesSubmenu::WallsOptions()
{
	static const std::vector<BikerClubhouseInteriorOption> kWalls
	{
		{ "None", "Walls_00" }, { "1", "Walls_01" }, { "2", "Walls_02" },
	};
	return kWalls;
}

const std::vector<BikerClubhouseInteriorOption>& TeleportBikerClubhousesSubmenu::DecorativeOptions()
{
	static const std::vector<BikerClubhouseInteriorOption> kDecor
	{
		{ "None", "Decorative_00" }, { "1", "Decorative_01" }, { "2", "Decorative_02" },
	};
	return kDecor;
}

const std::vector<BikerClubhouseInteriorOption>& TeleportBikerClubhousesSubmenu::FurnishingsOptions()
{
	static const std::vector<BikerClubhouseInteriorOption> kFurn
	{
		{ "None", "Furnishings_00" }, { "1", "Furnishings_01" }, { "2", "Furnishings_02" },
	};
	return kFurn;
}

const std::vector<BikerClubhouseInteriorOption>& TeleportBikerClubhousesSubmenu::ModBoothOptions()
{
	static const std::vector<BikerClubhouseInteriorOption> kModBooth
	{
		{ "None", "NO_MOD_BOOTH" }, { "Present", "Mod_Booth" },
	};
	return kModBooth;
}

const std::vector<BikerClubhouseInteriorOption>& TeleportBikerClubhousesSubmenu::GunLockerOptions()
{
	static const std::vector<BikerClubhouseInteriorOption> kGunLocker
	{
		{ "None", "NO_Gun_Locker" }, { "Present", "Gun_Locker" },
	};
	return kGunLocker;
}

BikerClubhouseInfoStructure& TeleportBikerClubhousesSubmenu::CurrentInfo()
{
	static BikerClubhouseInfoStructure info{ nullptr, 0, 0, 0, 0, 0, 0 };
	return info;
}

namespace {

struct ClubhouseOptionArrayEntry
{
	const char* name;
	unsigned char* ptr;
	const std::vector<BikerClubhouseInteriorOption>* arr;
};

std::vector<ClubhouseOptionArrayEntry> BuildClubhouseOptionArrays()
{
	auto& info = TeleportBikerClubhousesSubmenu::CurrentInfo();
	return {
		{ "Murals",     &info.muralOption,       &TeleportBikerClubhousesSubmenu::MuralOptions() },
		{ "Walls",      &info.wallsOption,       &TeleportBikerClubhousesSubmenu::WallsOptions() },
		{ "Decoration", &info.decorativeOption,  &TeleportBikerClubhousesSubmenu::DecorativeOptions() },
		{ "Furnishing", &info.furnishingsOption, &TeleportBikerClubhousesSubmenu::FurnishingsOptions() },
		{ "Mod Booth",  &info.modBoothOption,    &TeleportBikerClubhousesSubmenu::ModBoothOptions() },
		{ "Gun Locker", &info.gunLockerOption,   &TeleportBikerClubhousesSubmenu::GunLockerOptions() },
	};
}

void CreateClubhouse(BikerClubhouseInfoStructure& info)
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

	const auto optionArrays = BuildClubhouseOptionArrays();
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

void TeleportPedToClubhouse(GTAentity ped, BikerClubhouseInfoStructure& info)
{
	if (info.location != nullptr)
	{
		TeleportNetPed(ped, info.location->pos);
	}
}

}

void TeleportBikerClubhousesSubmenu::Draw()
{
	DrawTitle();

	auto& info = CurrentInfo();
	for (auto& loc : Locations())
	{
		if (DrawOption(loc.name))
		{
			info.location = &loc;
			NavigateTo("teleport_biker_clubhouses_in_loc");
		}
	}
}

void TeleportBikerClubhousesInLocSubmenu::Draw()
{
	auto& info = TeleportBikerClubhousesSubmenu::CurrentInfo();
	if (info.location == nullptr)
	{
		Engine* engine = Engine::Current();
		if (engine) engine->GoBack();
		return;
	}

	GTAped ped = g_Ped1;
	Engine* engine = Engine::Current();
	if (engine) engine->AddTitle(info.location->name);

	const auto optionArrays = BuildClubhouseOptionArrays();
	for (auto& o : optionArrays)
	{
		const std::vector<std::string> singleItem{ o.arr->at(*o.ptr).name };
		InputResult input = engine
			? engine->AddTextList(o.name, 0, singleItem)
			: InputResult{};
		if (input.rightPressed && *o.ptr < o.arr->size() - 1) (*o.ptr)++;
		if (input.leftPressed && *o.ptr > 0) (*o.ptr)--;
	}

	if (DrawOption("Build Clubhouse"))
	{
		DO_SCREEN_FADE_OUT(50);
		CreateClubhouse(info);
		TeleportPedToClubhouse(ped, info);
		DO_SCREEN_FADE_IN(200);
		return;
	}
}

const std::vector<BikerBusinessLocation>& TeleportBikerBusinessesSubmenu::Locations()
{
	static const std::vector<BikerBusinessLocation> kLocations
	{
		{ "Meth Lab",                  { 1009.5000f, -3196.6000f, -38.5000f }, "bkr_biker_interior_placement_interior_2_biker_dlc_int_ware01_milo_", {
			"meth_lab_basic", "meth_lab_upgrade", "meth_lab_security_high" } },

		{ "Weed Farm",                 { 1049.6000f, -3196.6000f, -38.5000f }, "bkr_biker_interior_placement_interior_3_biker_dlc_int_ware02_milo_", {
			"weed_standard_equip", "weed_upgrade_equip",
			"light_growtha_stage23_upgrade", "light_growthb_stage23_upgrade",
			"light_growthc_stage23_upgrade", "light_growthd_stage23_upgrade",
			"light_growthe_stage23_upgrade", "light_growthf_stage23_upgrade",
			"light_growthg_stage23_upgrade", "light_growthh_stage23_upgrade",
			"light_growthi_stage23_upgrade",
			"weed_low_security", "weed_security_upgrade" } },

		{ "Cocaine Warehouse",         { 1093.6000f, -3196.6000f, -38.5000f }, "bkr_biker_interior_placement_interior_4_biker_dlc_int_ware03_milo_", {
			"equipment_basic", "production_basic", "equipment_upgrade",
			"table_equipment_upgrade", "coke_press_basic", "coke_press_upgrade",
			"coke_cut_04", "coke_cut_05", "security_low", "security_high" } },

		{ "Counterfeit Cash Factory",  { 1124.6000f, -3196.6000f, -38.5000f }, "bkr_biker_interior_placement_interior_5_biker_dlc_int_ware04_milo_", {
			"counterfeit_standard_equip", "dryerc_on", "dryerd_on",
			"counterfeit_upgrade_equip", "counterfeit_low_security", "counterfeit_security" } },

		{ "Document Forgery Office",   { 1165.0000f, -3196.6000f, -38.2000f }, "bkr_biker_interior_placement_interior_6_biker_dlc_int_ware05_milo_", {
			"interior_basic", "equipment_basic",
			"interior_upgrade" "equipment_upgrade", // (sic; preserved verbatim from legacy)
			"clutter", "Chair05", "Chair04", "Chair07", "security_low", "security_high" } },
	};
	return kLocations;
}

BikerBusinessInfoStructure& TeleportBikerBusinessesSubmenu::CurrentInfo()
{
	static BikerBusinessInfoStructure info{ nullptr, 0 };
	return info;
}

namespace {

void CreateBusiness(BikerBusinessInfoStructure& info)
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

	for (auto& ip : loc.options)
		DEACTIVATE_INTERIOR_ENTITY_SET(interior, ip.c_str());
	if (info.option)
	{
		for (unsigned char o = 0; o < info.option; o++)
			ACTIVATE_INTERIOR_ENTITY_SET(interior, loc.options[o].c_str());
	}
	REFRESH_INTERIOR(interior);
}

void TeleportPedToBusiness(GTAentity ped, BikerBusinessInfoStructure& info)
{
	if (info.location != nullptr)
	{
		TeleportNetPed(ped, info.location->pos);
	}
}

}

void TeleportBikerBusinessesSubmenu::Draw()
{
	DrawTitle();

	auto& info = CurrentInfo();
	for (auto& loc : Locations())
	{
		if (DrawOption(loc.name))
		{
			info.location = &loc;
			info.option = 0;
			NavigateTo("teleport_biker_businesses_in_loc");
		}
	}
}

void TeleportBikerBusinessesInLocSubmenu::Draw()
{
	auto& info = TeleportBikerBusinessesSubmenu::CurrentInfo();
	if (info.location == nullptr)
	{
		Engine* engine = Engine::Current();
		if (engine) engine->GoBack();
		return;
	}

	GTAped ped = g_Ped1;
	Engine* engine = Engine::Current();
	if (engine) engine->AddTitle(info.location->name);

	const std::string currentValue = info.option == 0
		? std::string("Empty")
		: info.location->options[info.option - 1];

	const std::vector<std::string> singleItem{ currentValue };
	InputResult input = engine
		? engine->AddTextList("Option", 0, singleItem)
		: InputResult{};
	if (input.rightPressed && info.option < info.location->options.size()) info.option++;
	if (input.leftPressed && info.option > 0) info.option--;

	if (DrawOption("Build Business"))
	{
		DO_SCREEN_FADE_OUT(50);
		CreateBusiness(info);
		TeleportPedToBusiness(ped, info);
		DO_SCREEN_FADE_IN(200);
		return;
	}
}

}
REGISTER_SUBMENU(::Menu::TeleportBikerClubhousesSubmenu)
REGISTER_SUBMENU(::Menu::TeleportBikerClubhousesInLocSubmenu)
REGISTER_SUBMENU(::Menu::TeleportBikerBusinessesSubmenu)
REGISTER_SUBMENU(::Menu::TeleportBikerBusinessesInLocSubmenu)
