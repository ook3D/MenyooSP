#include "TeleportHangars.h"

#include "../Menu/Engine.h"
#include "../Menu/SubmenuRegistry.h"
#include "../Menu/Menu.h"
#include "../Menu/Routine.h"
#include "PlayerRuntime.h"

#include "../Natives/natives2.h"
#include "../Scripting/GTAped.h"

#include "Teleport/TeleMethods.h"

#include <Windows.h>   // GetTickCount

namespace Menu {

const std::vector<TeleportHangarsSubmenu::HangarLocation>
TeleportHangarsSubmenu::vLocations
{
	{ "Regular", { -1253.6600f, -2998.8000f, -48.4900f },
		{ "sm_smugdlc_interior_placement",
		  "sm_smugdlc_interior_placement_interior_0_smugdlc_int_01_milo_" },
		"sm_smugdlc_int_01" },
};

const std::vector<TeleportHangarsSubmenu::HangarInteriorOption>
TeleportHangarsSubmenu::vDefaultOptions
{
	{ std::string(), "set_lighting_tint_props", 10 },
};
const std::vector<TeleportHangarsSubmenu::HangarInteriorOption>
TeleportHangarsSubmenu::vMainShellOptions
{
	{ "Normal", "set_tint_shell", 10 },
};
const std::vector<TeleportHangarsSubmenu::HangarInteriorOption>
TeleportHangarsSubmenu::vBedroomOptions
{
	{ "Disabled", "", 0 },
	{ "Enabled", "set_bedroom_tint", 10 },
};
const std::vector<TeleportHangarsSubmenu::HangarInteriorOption>
TeleportHangarsSubmenu::vBedroomStyleOptions
{
	{ "Empty", "", 0 },
	{ "Traditional", "set_bedroom_traditional", 0 },
	{ "Modern", "set_bedroom_modern", 0 },
};
const std::vector<TeleportHangarsSubmenu::HangarInteriorOption>
TeleportHangarsSubmenu::vBedroomClutterOptions
{
	{ "Disabled", "", 0 },
	{ "Enabled", "set_bedroom_clutter", 0 },
};
const std::vector<TeleportHangarsSubmenu::HangarInteriorOption>
TeleportHangarsSubmenu::vBedroomBlindsOptions
{
	{ "None", "", 0 },
	{ "Closed", "set_bedroom_blinds_closed", 0 },
	{ "Open", "set_bedroom_blinds_open", 0 },
};
const std::vector<TeleportHangarsSubmenu::HangarInteriorOption>
TeleportHangarsSubmenu::vModAreaOptions
{
	{ "Disabled", "", 0 },
	{ "Enabled", "set_modarea", 10 },
};
const std::vector<TeleportHangarsSubmenu::HangarInteriorOption>
TeleportHangarsSubmenu::vCraneOptions
{
	{ "Disabled", "", 0 },
	{ "Enabled", "set_crane_tint", 10 },
};
const std::vector<TeleportHangarsSubmenu::HangarInteriorOption>
TeleportHangarsSubmenu::vOfficeOptions
{
	{ "Basic", "set_office_basic", 0 },
	{ "Traditional", "set_office_traditional", 0 },
	{ "Modern", "set_office_modern", 0 },
};
const std::vector<TeleportHangarsSubmenu::HangarInteriorOption>
TeleportHangarsSubmenu::vFloorOptions
{
	{ "1", "set_floor_1", 0 },
	{ "2", "set_floor_2", 10 },
};
const std::vector<TeleportHangarsSubmenu::HangarInteriorOption>
TeleportHangarsSubmenu::vFloorDecalOptions
{
	{ "1", "set_floor_decal_1", 10 },
	{ "2", "set_floor_decal_2", 10 },
	{ "3", "set_floor_decal_3", 10 },
	{ "4", "set_floor_decal_4", 10 },
	{ "5", "set_floor_decal_5", 10 },
	{ "6", "set_floor_decal_6", 10 },
	{ "7", "set_floor_decal_7", 10 },
	{ "8", "set_floor_decal_8", 10 },
	{ "9", "set_floor_decal_9", 10 },
};
const std::vector<TeleportHangarsSubmenu::HangarInteriorOption>
TeleportHangarsSubmenu::vHangarLightingOptions
{
	{ "1", "set_lighting_hangar_a", 0 },
	{ "2", "set_lighting_hangar_b", 0 },
	{ "3", "set_lighting_hangar_c", 0 },
};
const std::vector<TeleportHangarsSubmenu::HangarInteriorOption>
TeleportHangarsSubmenu::vWallLightingOptions
{
	{ "Neutral", "set_lighting_wall_neutral", 0 },
	{ "Tint 1", "set_lighting_wall_tint01", 0 },
	{ "Tint 2", "set_lighting_wall_tint02", 0 },
	{ "Tint 3", "set_lighting_wall_tint03", 0 },
	{ "Tint 4", "set_lighting_wall_tint04", 0 },
	{ "Tint 5", "set_lighting_wall_tint05", 0 },
	{ "Tint 6", "set_lighting_wall_tint06", 0 },
	{ "Tint 7", "set_lighting_wall_tint07", 0 },
	{ "Tint 8", "set_lighting_wall_tint08", 0 },
	{ "Tint 9", "set_lighting_wall_tint09", 0 },
};

const std::vector<TeleportHangarsSubmenu::HangarInteriorOptionArray>
TeleportHangarsSubmenu::vOptionArrays
{
	{ std::string(),       nullptr,                                                 &TeleportHangarsSubmenu::vDefaultOptions        },
	{ "Main Shell",        &TeleportHangarsSubmenu::HangarInfoStructure::mainShellOption,        &TeleportHangarsSubmenu::vMainShellOptions      },
	{ "Bedroom",           &TeleportHangarsSubmenu::HangarInfoStructure::bedroomOption,          &TeleportHangarsSubmenu::vBedroomOptions        },
	{ "Bedroom Style",     &TeleportHangarsSubmenu::HangarInfoStructure::bedroomStyleOption,     &TeleportHangarsSubmenu::vBedroomStyleOptions   },
	{ "Bedroom Clutter",   &TeleportHangarsSubmenu::HangarInfoStructure::bedroomClutterOption,   &TeleportHangarsSubmenu::vBedroomClutterOptions },
	{ "Bedroom Blinds",    &TeleportHangarsSubmenu::HangarInfoStructure::bedroomBlindsOption,    &TeleportHangarsSubmenu::vBedroomBlindsOptions  },
	{ "Auto Shop",         &TeleportHangarsSubmenu::HangarInfoStructure::modAreaOption,          &TeleportHangarsSubmenu::vModAreaOptions        },
	{ "Crane",             &TeleportHangarsSubmenu::HangarInfoStructure::craneOption,            &TeleportHangarsSubmenu::vCraneOptions          },
	{ "Office",            &TeleportHangarsSubmenu::HangarInfoStructure::officeOption,           &TeleportHangarsSubmenu::vOfficeOptions         },
	{ "Floor",             &TeleportHangarsSubmenu::HangarInfoStructure::floorOption,            &TeleportHangarsSubmenu::vFloorOptions          },
	{ "Floor Decoration",  &TeleportHangarsSubmenu::HangarInfoStructure::floorDecalOption,       &TeleportHangarsSubmenu::vFloorDecalOptions     },
	{ "Hangar Lighting",   &TeleportHangarsSubmenu::HangarInfoStructure::hangarLightingOption,   &TeleportHangarsSubmenu::vHangarLightingOptions },
	{ "Wall Lighting",     &TeleportHangarsSubmenu::HangarInfoStructure::wallLightingOption,     &TeleportHangarsSubmenu::vWallLightingOptions   },
};

TeleportHangarsSubmenu::HangarInfoStructure TeleportHangarsSubmenu::currentHangarInfo{};
const TeleportHangarsSubmenu::HangarInteriorOptionArray* TeleportHangarsSubmenu::selectedOptionArray = nullptr;

void TeleportHangarsSubmenu::CreateHangar(HangarInfoStructure& info)
{
	if (info.location == nullptr)
		return;

	auto& loc = *info.location;
	auto& pos = loc.pos;

	SET_INSTANCE_PRIORITY_MODE(true);
	ON_ENTER_MP();
	for (auto& ipl : loc.ipls)
		REQUEST_IPL(ipl.c_str());
	int interior = GET_INTERIOR_AT_COORDS_WITH_TYPE(pos.x, pos.y, pos.z, loc.interior.c_str());
	DISABLE_INTERIOR(interior, true);
	PIN_INTERIOR_IN_MEMORY(interior);
	DISABLE_INTERIOR(interior, false);
	SET_INSTANCE_PRIORITY_MODE(false);
	WAIT(200);

	for (auto& oa : vOptionArrays)
	{
		for (auto& o : *oa.arr)
			DEACTIVATE_INTERIOR_ENTITY_SET(interior, o.value.c_str());
	}
	for (auto& oa : vOptionArrays)
	{
		if (oa.memberPtr == nullptr)
		{
			for (auto& o : *oa.arr)
				ACTIVATE_INTERIOR_ENTITY_SET(interior, o.value.c_str());
		}
		else
		{
			const HangarInteriorOptionIndex& ptr = info.*(oa.memberPtr);
			auto& o = oa.arr->at(ptr.index);
			ACTIVATE_INTERIOR_ENTITY_SET(interior, o.value.c_str());
			if (o.maxTints > 0)
			{
				for (DWORD timeOut = GetTickCount() + 250; GetTickCount() < timeOut;)
				{
					if (IS_INTERIOR_ENTITY_SET_ACTIVE(interior, o.value.c_str()))
						break;
					WAIT(0);
				}
				SET_INTERIOR_ENTITY_SET_TINT_INDEX(interior, o.value.c_str(), ptr.currTint);
			}
		}
	}
	REFRESH_INTERIOR(interior);
}

void TeleportHangarsSubmenu::UpdateHangarProp(HangarInfoStructure& info, const HangarInteriorOptionArray& arr)
{
	if (info.location == nullptr || arr.memberPtr == nullptr)
		return;

	auto& loc = *info.location;
	auto& pos = loc.pos;

	int interior = GET_INTERIOR_AT_COORDS_WITH_TYPE(pos.x, pos.y, pos.z, loc.interior.c_str());
	if (IS_INTERIOR_DISABLED(interior))
		return;

	const HangarInteriorOptionIndex& ptr = info.*(arr.memberPtr);
	for (auto& p : *arr.arr)
		DEACTIVATE_INTERIOR_ENTITY_SET(interior, p.value.c_str());
	auto& p = arr.arr->at(ptr.index);
	const std::string& propName = p.value;
	ACTIVATE_INTERIOR_ENTITY_SET(interior, propName.c_str());
	if (p.maxTints > 0)
	{
		for (DWORD timeOut = GetTickCount() + 250; GetTickCount() < timeOut;)
		{
			if (IS_INTERIOR_ENTITY_SET_ACTIVE(interior, propName.c_str()))
				break;
			WAIT(0);
		}
		SET_INTERIOR_ENTITY_SET_TINT_INDEX(interior, propName.c_str(), ptr.currTint);
	}
	REFRESH_INTERIOR(interior);
}

void TeleportHangarsSubmenu::Draw()
{
	DrawTitle();

	for (auto& loc : vLocations)
	{
		if (DrawOption(loc.name))
		{
			currentHangarInfo.location = &loc;
			NavigateTo("teleport_hangars_in_loc");
		}
	}
}

void TeleportHangarsInLocSubmenu::Draw()
{
	auto& info = TeleportHangarsSubmenu::currentHangarInfo;

	if (info.location == nullptr)
	{
		Engine* engine = Engine::Current();
		if (engine) engine->GoBack();
		return;
	}

	DrawTitle();

	GTAped ped = g_Ped1;
	Engine* engine = Engine::Current();

	for (auto& o : TeleportHangarsSubmenu::vOptionArrays)
	{
		if (o.name.empty() || o.memberPtr == nullptr)
			continue;

		TeleportHangarsSubmenu::HangarInteriorOptionIndex& ptr = info.*(o.memberPtr);
		const std::vector<std::string> singleEntry{ o.arr->at(ptr.index).name };

		int displayIdx = 0;
		const ::Menu::InputResult res = engine
			? engine->AddTextList(o.name, displayIdx, singleEntry)
			: ::Menu::InputResult{};

		if (res.rightPressed)
		{
			if (ptr.index < o.arr->size() - 1)
			{
				++ptr.index;
				TeleportHangarsSubmenu::UpdateHangarProp(info, o);
			}
		}
		else if (res.leftPressed)
		{
			if (ptr.index > 0)
			{
				--ptr.index;
				TeleportHangarsSubmenu::UpdateHangarProp(info, o);
			}
		}
		if (res.accepted)
		{
			TeleportHangarsSubmenu::selectedOptionArray = &o;
			NavigateTo("teleport_hangars_in_option");
		}
	}

	if (DrawOption("Build Hangar"))
	{
		DO_SCREEN_FADE_OUT(50);
		TeleportHangarsSubmenu::CreateHangar(info);
		if (info.location != nullptr)
			TeleportNetPed(ped, info.location->pos);
		DO_SCREEN_FADE_IN(200);
		return;
	}
}

void TeleportHangarsInOptionSubmenu::Draw()
{
	auto& info = TeleportHangarsSubmenu::currentHangarInfo;
	const auto* arr = TeleportHangarsSubmenu::selectedOptionArray;

	if (info.location == nullptr || arr == nullptr || arr->memberPtr == nullptr)
	{
		Engine* engine = Engine::Current();
		if (engine) engine->GoBack();
		return;
	}

	Engine* engine = Engine::Current();
	if (engine)
		engine->AddTitle(arr->name.empty() ? std::string("Option") : arr->name);

	TeleportHangarsSubmenu::HangarInteriorOptionIndex& ptr = info.*(arr->memberPtr);

	for (unsigned i = 0; i < arr->arr->size(); ++i)
	{
		auto& o = arr->arr->at(i);
		const bool isSelected = (ptr.index == i);
		if (isSelected && o.maxTints > 0)
		{
			int tintValue = ptr.currTint;
			if (DrawNumber(o.name, tintValue, 1, 1, o.maxTints))
			{
				ptr.currTint = static_cast<unsigned __int8>(tintValue);
				TeleportHangarsSubmenu::UpdateHangarProp(info, *arr);
			}
		}
		else
		{
			if (DrawSelectionItem(o.name, isSelected))
			{
				ptr.index = static_cast<unsigned __int8>(i);
				if (ptr.currTint > o.maxTints) ptr.currTint = o.maxTints;
				TeleportHangarsSubmenu::UpdateHangarProp(info, *arr);
			}
		}
	}
}

}
REGISTER_SUBMENU(::Menu::TeleportHangarsSubmenu)
REGISTER_SUBMENU(::Menu::TeleportHangarsInLocSubmenu)
REGISTER_SUBMENU(::Menu::TeleportHangarsInOptionSubmenu)
