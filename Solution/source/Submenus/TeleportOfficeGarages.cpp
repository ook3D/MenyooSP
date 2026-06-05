#include "TeleportOfficeGarages.h"

#include "../Menu/Engine.h"
#include "../Menu/SubmenuRegistry.h"
#include "../Menu/Routine.h"        // g_Ped1
#include "PlayerRuntime.h"          // g_Ped1

#include "../Natives/natives2.h"
#include "../Scripting/GTAentity.h"
#include "../Scripting/GTAped.h"
#include "../Util/GTAmath.h"

#include "Teleport/Locations.h"     // IplNames::vAllOfficeGarages1
#include "Teleport/TeleMethods.h"   // TeleportNetPed

#include <string>
#include <utility>
#include <vector>

namespace Menu {

const std::vector<std::string>& TeleportOfficeGaragesSubmenu::GarageIds()
{
	static const std::vector<std::string> kIds{ "1", "2", "3", "Autoshop" };
	return kIds;
}

namespace {

const std::vector<OfficeGarageLocation>& OfficesArcadius()
{
	static const std::vector<OfficeGarageLocation> kData
	{
		{ { -197.5016f, -579.3605f, 136.0005f }, "imp_dt1_02_cargarage_a" },
		{ { -123.7161f, -569.9810f, 136.0005f }, "imp_dt1_02_cargarage_b" },
		{ { -136.6278f, -623.5266f, 136.0005f }, "imp_dt1_02_cargarage_c" },
		{ { -144.2712f, -593.0843f, 167.0001f }, "imp_dt1_02_modgarage" },
	};
	return kData;
}

const std::vector<OfficeGarageLocation>& OfficesMazeBank()
{
	static const std::vector<OfficeGarageLocation> kData
	{
		{ { -90.7866f, -821.2824f, 222.0005f }, "imp_dt1_11_cargarage_a" },
		{ { -71.7458f, -831.5193f, 222.0005f }, "imp_dt1_11_cargarage_b" },
		{ { -79.7060f, -806.6108f, 222.0005f }, "imp_dt1_11_cargarage_c" },
		{ { -73.9068f, -815.3940f, 285.0001f }, "imp_dt1_11_modgarage" },
	};
	return kData;
}

const std::vector<OfficeGarageLocation>& OfficesLombank()
{
	static const std::vector<OfficeGarageLocation> kData
	{
		{ { -1585.6804f, -561.9070f, 86.5005f }, "imp_sm_13_cargarage_a" },
		{ { -1563.4013f, -557.4779f, 86.5005f }, "imp_sm_13_cargarage_b" },
		{ { -1560.1299f, -579.9451f, 86.5005f }, "imp_sm_13_cargarage_c" },
		{ { -1574.5394f, -571.2640f, 105.2001f }, "imp_sm_13_modgarage" },
	};
	return kData;
}

const std::vector<OfficeGarageLocation>& OfficesMazeBankWest()
{
	static const std::vector<OfficeGarageLocation> kData
	{
		{ { -1395.2725f, -480.5121f, 57.1005f }, "imp_sm_15_cargarage_a" },
		{ { -1394.6155f, -481.2531f, 49.1005f }, "imp_sm_15_cargarage_b" },
		{ { -1368.8768f, -471.6262f, 57.1005f }, "imp_sm_15_cargarage_c" },
		{ { -1389.9446f, -480.1762f, 78.2001f }, "imp_sm_15_modgarage" },
	};
	return kData;
}

}

const std::vector<TeleportOfficeGaragesSubmenu::LocationPair>& TeleportOfficeGaragesSubmenu::Locations()
{
	static const std::vector<LocationPair> kLocations
	{
		{ "Arcadius Business Centre", &OfficesArcadius() },
		{ "Maze Bank Building",       &OfficesMazeBank() },
		{ "Lombank West",             &OfficesLombank() },
		{ "Maze Bank West",           &OfficesMazeBankWest() },
	};
	return kLocations;
}

const std::vector<OfficeGarageInteriorOption>& TeleportOfficeGaragesSubmenu::FloorOptions()
{
	static const std::vector<OfficeGarageInteriorOption> kFloor
	{
		{ "None", "Floor_vinyl_00" },
		{ "1",  "Floor_vinyl_01" }, { "2",  "Floor_vinyl_02" }, { "3",  "Floor_vinyl_03" },
		{ "4",  "Floor_vinyl_04" }, { "5",  "Floor_vinyl_05" }, { "6",  "Floor_vinyl_06" },
		{ "7",  "Floor_vinyl_07" }, { "8",  "Floor_vinyl_08" }, { "9",  "Floor_vinyl_09" },
		{ "10", "Floor_vinyl_10" }, { "11", "Floor_vinyl_11" }, { "12", "Floor_vinyl_12" },
		{ "13", "Floor_vinyl_13" }, { "14", "Floor_vinyl_14" }, { "15", "Floor_vinyl_15" },
		{ "16", "Floor_vinyl_16" }, { "17", "Floor_vinyl_17" }, { "18", "Floor_vinyl_18" },
		{ "19", "Floor_vinyl_19" }, { "20", "Floor_vinyl_20" },
	};
	return kFloor;
}

const std::vector<OfficeGarageInteriorOption>& TeleportOfficeGaragesSubmenu::DecorOptions()
{
	static const std::vector<OfficeGarageInteriorOption> kDecor
	{
		{ "None", "Garage_Decor_00" },
		{ "1",  "Garage_Decor_01" }, { "2",  "Garage_Decor_02" }, { "3",  "Garage_Decor_03" },
		{ "4",  "Garage_Decor_04" }, { "5",  "Garage_Decor_05" }, { "6",  "Garage_Decor_06" },
		{ "7",  "Garage_Decor_07" }, { "8",  "Garage_Decor_08" }, { "9",  "Garage_Decor_09" },
		{ "10", "Garage_Decor_10" }, { "11", "Garage_Decor_11" }, { "12", "Garage_Decor_12" },
		{ "13", "Garage_Decor_13" }, { "14", "Garage_Decor_14" }, { "15", "Garage_Decor_15" },
		{ "16", "Garage_Decor_16" }, { "17", "Garage_Decor_17" }, { "18", "Garage_Decor_18" },
		{ "19", "Garage_Decor_19" }, { "20", "Garage_Decor_20" },
	};
	return kDecor;
}

const std::vector<OfficeGarageInteriorOption>& TeleportOfficeGaragesSubmenu::LightingOptions()
{
	static const std::vector<OfficeGarageInteriorOption> kLighting
	{
		{ "None", "Lighting_Option00" },
		{ "1",  "Lighting_Option01" }, { "2",  "Lighting_Option02" }, { "3",  "Lighting_Option03" },
		{ "4",  "Lighting_Option04" }, { "5",  "Lighting_Option05" }, { "6",  "Lighting_Option06" },
		{ "7",  "Lighting_Option07" }, { "8",  "Lighting_Option08" }, { "9",  "Lighting_Option09" },
		{ "10", "Lighting_Option10" }, { "11", "Lighting_Option11" }, { "12", "Lighting_Option12" },
		{ "13", "Lighting_Option13" }, { "14", "Lighting_Option14" }, { "15", "Lighting_Option15" },
		{ "16", "Lighting_Option16" }, { "17", "Lighting_Option17" }, { "18", "Lighting_Option18" },
		{ "19", "Lighting_Option19" }, { "20", "Lighting_Option20" },
	};
	return kLighting;
}

const std::vector<OfficeGarageInteriorOption>& TeleportOfficeGaragesSubmenu::NumStyleOptions()
{
	static const std::vector<OfficeGarageInteriorOption> kNumStyle
	{
		{ "None", "Numbering_Style00" },
		{ "1",  "Numbering_Style01" }, { "2",  "Numbering_Style02" }, { "3",  "Numbering_Style03" },
		{ "4",  "Numbering_Style04" }, { "5",  "Numbering_Style05" }, { "6",  "Numbering_Style06" },
		{ "7",  "Numbering_Style07" }, { "8",  "Numbering_Style08" }, { "9",  "Numbering_Style09" },
		{ "10", "Numbering_Style10" }, { "11", "Numbering_Style11" }, { "12", "Numbering_Style12" },
		{ "13", "Numbering_Style13" }, { "14", "Numbering_Style14" }, { "15", "Numbering_Style15" },
		{ "16", "Numbering_Style16" }, { "17", "Numbering_Style17" }, { "18", "Numbering_Style18" },
		{ "19", "Numbering_Style19" }, { "20", "Numbering_Style20" },
	};
	return kNumStyle;
}

OfficeGarageInfoStructure& TeleportOfficeGaragesSubmenu::CurrentInfo()
{
	static OfficeGarageInfoStructure info{ nullptr, 0, 0, 0, 0, 0 };
	return info;
}

namespace {

void CreateOfficeGarage(OfficeGarageInfoStructure& garageInfo)
{
	if (garageInfo.location == nullptr) return;

	auto& loc = garageInfo.location->second->at(garageInfo.garageId);
	auto& pos = loc.pos;

	SET_INSTANCE_PRIORITY_MODE(true);
	ON_ENTER_MP();
	for (auto& ipl : ::sub::TeleportLocations_catind::IplNames::vAllOfficeGarages1)
	{
		REMOVE_IPL(ipl.data());
	}

	REQUEST_IPL(loc.ipl.c_str());
	int interior = GET_INTERIOR_AT_COORDS(pos.x, pos.y, pos.z);
	DISABLE_INTERIOR(interior, true);
	PIN_INTERIOR_IN_MEMORY(interior);
	DISABLE_INTERIOR(interior, false);
	SET_INSTANCE_PRIORITY_MODE(false);
	WAIT(200);

	for (auto& ip : TeleportOfficeGaragesSubmenu::FloorOptions())
		DEACTIVATE_INTERIOR_ENTITY_SET(interior, ip.value.c_str());
	for (auto& ip : TeleportOfficeGaragesSubmenu::DecorOptions())
		DEACTIVATE_INTERIOR_ENTITY_SET(interior, ip.value.c_str());
	for (auto& ip : TeleportOfficeGaragesSubmenu::LightingOptions())
		DEACTIVATE_INTERIOR_ENTITY_SET(interior, ip.value.c_str());
	for (auto& ip : TeleportOfficeGaragesSubmenu::NumStyleOptions())
	{
		for (unsigned char i = 1; i <= 3; i++)
			DEACTIVATE_INTERIOR_ENTITY_SET(interior, (ip.value + "_N" + std::to_string(i)).c_str());
	}

	ACTIVATE_INTERIOR_ENTITY_SET(interior, TeleportOfficeGaragesSubmenu::FloorOptions()[garageInfo.floorOption].value.c_str());
	ACTIVATE_INTERIOR_ENTITY_SET(interior, TeleportOfficeGaragesSubmenu::DecorOptions()[garageInfo.decorOption].value.c_str());
	ACTIVATE_INTERIOR_ENTITY_SET(interior, TeleportOfficeGaragesSubmenu::LightingOptions()[garageInfo.lightingOption].value.c_str());
	ACTIVATE_INTERIOR_ENTITY_SET(interior,
		(TeleportOfficeGaragesSubmenu::NumStyleOptions()[garageInfo.numStyleOption].value
			+ "_N" + std::to_string(garageInfo.garageId + 1)).c_str());

	REFRESH_INTERIOR(interior);
}

void TeleportPedToOfficeGarage(GTAentity ped, OfficeGarageInfoStructure& garageInfo)
{
	if (garageInfo.location != nullptr)
	{
		TeleportNetPed(ped, garageInfo.location->second->at(garageInfo.garageId).pos);
	}
}

// Single-item Texter helper — mirrors the legacy AddTexter step pattern.
void StepTexter(const std::string& name, unsigned char& value, const std::vector<OfficeGarageInteriorOption>& options)
{
	Engine* engine = Engine::Current();
	const std::vector<std::string> singleItem{ options[value].name };
	InputResult input = engine
		? engine->AddTextList(name, 0, singleItem)
		: InputResult{};
	if (input.rightPressed && value < options.size() - 1) value++;
	if (input.leftPressed && value > 0) value--;
}

}

void TeleportOfficeGaragesSubmenu::Draw()
{
	DrawTitle();

	auto& info = CurrentInfo();
	for (auto& loc : Locations())
	{
		if (DrawOption(loc.first))
		{
			info.location = &loc;
			NavigateTo("teleport_office_garages_in_loc");
		}
	}
}

void TeleportOfficeGaragesInLocSubmenu::Draw()
{
	auto& info = TeleportOfficeGaragesSubmenu::CurrentInfo();
	if (info.location == nullptr)
	{
		Engine* engine = Engine::Current();
		if (engine) engine->GoBack();
		return;
	}

	GTAped ped = g_Ped1;
	Engine* engine = Engine::Current();
	if (engine) engine->AddTitle(info.location->first);

	// Garage selector (uses plain string list, not the OfficeGarageInteriorOption struct).
	{
		const auto& ids = TeleportOfficeGaragesSubmenu::GarageIds();
		const std::vector<std::string> singleItem{ ids[info.garageId] };
		InputResult input = engine
			? engine->AddTextList("Garage", 0, singleItem)
			: InputResult{};
		if (input.rightPressed && info.garageId < ids.size() - 1) info.garageId++;
		if (input.leftPressed && info.garageId > 0) info.garageId--;
	}

	if (info.garageId == 3) // Autoshop
	{
		StepTexter("Floor", info.floorOption, TeleportOfficeGaragesSubmenu::FloorOptions());
	}
	else
	{
		StepTexter("Theme", info.decorOption, TeleportOfficeGaragesSubmenu::DecorOptions());
		StepTexter("Lighting", info.lightingOption, TeleportOfficeGaragesSubmenu::LightingOptions());
		StepTexter("Numbering Style", info.numStyleOption, TeleportOfficeGaragesSubmenu::NumStyleOptions());
	}

	if (DrawOption("Build Office Garage"))
	{
		DO_SCREEN_FADE_OUT(50);
		CreateOfficeGarage(info);
		TeleportPedToOfficeGarage(ped, info);
		DO_SCREEN_FADE_IN(200);
		return;
	}
}

}
REGISTER_SUBMENU(::Menu::TeleportOfficeGaragesSubmenu)
REGISTER_SUBMENU(::Menu::TeleportOfficeGaragesInLocSubmenu)
