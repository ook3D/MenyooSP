#include "TeleportFacilities.h"

#include "../Menu/Engine.h"
#include "../Menu/SubmenuRegistry.h"
#include "../Menu/Menu.h"
#include "../Menu/Routine.h"
#include "PlayerRuntime.h"

#include "../Natives/natives2.h"
#include "../Scripting/GTAped.h"

#include "Teleport/TeleMethods.h"

#include <Windows.h>

namespace Menu {

using sub::TeleportLocations_catind::TeleLocation;
using sub::TeleportLocations_catind::TeleMethods::ToTeleLocation241;

const std::vector<TeleLocation>
TeleportFacilitiesSubmenu::vOtherFacilityRelatedTeleports
{
	TeleLocation("Facility 1 (IAA)", 2047.0000f, 2942.0000f, -62.9025f,
		{ "xm_x17dlc_int_placement_interior_4_x17dlc_int_facility_milo"_sv }, {}, true),
	TeleLocation("Facility 2 (Datacentre)", 2168.0000f, 2920.8900f, -85.8000f,
		{ "xm_x17dlc_int_placement_interior_5_x17dlc_int_facility2_milo"_sv }, {}, true),
	TeleLocation("Submarine", 513.0700f, 4839.6900f, -62.5900f,
		{ "xm_x17dlc_int_placement_interior_8_x17dlc_int_sub_milo_"_sv }, {}, true),
	TeleLocation("Base", 567.1900f, 5954.8800f, -158.5500f,
		{ "xm_x17dlc_int_placement_interior_34_x17dlc_int_lab_milo_"_sv }, {}, true, false, true),
	TeleLocation("Lab", 244.5700f, 6163.3900f, -159.4200f,
		{ "xm_x17dlc_int_placement_interior_34_x17dlc_int_lab_milo_"_sv }, {}, true, false, true),
	TeleLocation("Silo", 368.4300f, 6307.8600f, -160.2500f,
		{ "xm_x17dlc_int_placement_interior_34_x17dlc_int_lab_milo_"_sv }, {}, true, false, true),
	TeleLocation("Avenger", 520.0000f, 4750.0000f, -70.0000f,
		{ "xm_x17dlc_int_placement_interior_9_x17dlc_int_01_milo_"_sv }, {},
		{ "shell_tint"_sv, "CONTROL_1"_sv, "CONTROL_2"_sv, "CONTROL_3"_sv,
		  "WEAPONS_MOD"_sv, "VEHICLE_MOD"_sv, "GOLD_BLING"_sv }, true, false, true)
};

const std::vector<TeleportFacilitiesSubmenu::FacilityLocation>
TeleportFacilitiesSubmenu::vLocations
{
	{ "Regular", { 462.0900f, 4820.4200f, -59.0000f },
		{ "xm_x17dlc_int_placement_interior_33_x17dlc_int_02_milo_" } },
};

const std::array<std::string, 10> TeleportFacilitiesSubmenu::vTintNames
{ {
	"",
	"Utility",
	"Expertise",
	"Altitude",
	"Power",
	"Authority",
	"Influence",
	"Order",
	"Empire",
	"Supremacy"
} };

const std::vector<TeleportFacilitiesSubmenu::FacilityInteriorOption>
TeleportFacilitiesSubmenu::vMainShellOptions
{
	{ "Normal", "set_int_02_shell", 10 },
};
const std::vector<TeleportFacilitiesSubmenu::FacilityInteriorOption>
TeleportFacilitiesSubmenu::vGraphicsOptions
{
	{ "1","set_int_02_decal_01", 1 },
	{ "2","set_int_02_decal_02", 1 },
	{ "3","set_int_02_decal_03", 1 },
	{ "4","set_int_02_decal_04", 1 },
	{ "5","set_int_02_decal_05", 1 },
	{ "6","set_int_02_decal_06", 1 },
	{ "7","set_int_02_decal_07", 1 },
	{ "8","set_int_02_decal_08", 1 },
	{ "9","set_int_02_decal_09", 1 },
};
const std::vector<TeleportFacilitiesSubmenu::FacilityInteriorOption>
TeleportFacilitiesSubmenu::vTrophyOptions
{
	{ "None", "", 0 },
	{ "1", "set_int_02_trophy1", 10 },
	{ "IAA", "set_int_02_trophy_iaa", 10 },
	{ "SUB", "set_int_02_trophy_sub", 10 },
};
const std::vector<TeleportFacilitiesSubmenu::FacilityInteriorOption>
TeleportFacilitiesSubmenu::vOrbitalCannonOptions
{
	{ "Disabled","set_int_02_no_cannon", 0 },
	{ "Enabled","set_int_02_cannon", 1 },
};
const std::vector<TeleportFacilitiesSubmenu::FacilityInteriorOption>
TeleportFacilitiesSubmenu::vSecurityRoomOptions
{
	{ "Disabled","set_int_02_no_security", 0 },
	{ "Enabled","set_int_02_security", 1 },
};
const std::vector<TeleportFacilitiesSubmenu::FacilityInteriorOption>
TeleportFacilitiesSubmenu::vLoungeOptions
{
	{ "None","", 0 },
	{ "Utilty","set_int_02_lounge1", 1 },
	{ "Prestige","set_int_02_lounge2", 1 },
	{ "Premier","set_int_02_lounge3", 1 },
};
const std::vector<TeleportFacilitiesSubmenu::FacilityInteriorOption>
TeleportFacilitiesSubmenu::vSleepingQuartersOptions
{
	{ "None","set_int_02_no_sleep", 0 },
	{ "Utility","set_int_02_sleep", 1 },
	{ "Prestige","set_int_02_sleep2", 1 },
	{ "Premier","set_int_02_sleep3", 1 },
};
const std::vector<TeleportFacilitiesSubmenu::FacilityInteriorOption>
TeleportFacilitiesSubmenu::vClutterOptions
{
	{ "None","", 0 },
	{ "1","set_int_02_clutter1", 1 },
	{ "2","set_int_02_clutter2", 1 },
	{ "3","set_int_02_clutter3", 1 },
	{ "4","set_int_02_clutter4", 1 },
	{ "5","set_int_02_clutter5", 1 },
};
const std::vector<TeleportFacilitiesSubmenu::FacilityInteriorOption>
TeleportFacilitiesSubmenu::vCrewEmblemOptions
{
	{ "None","", 0 },
	{ "Player","set_int_02_crewemblem", 0 },
};

const std::vector<TeleportFacilitiesSubmenu::FacilityInteriorOptionArray>
TeleportFacilitiesSubmenu::vOptionArrays
{
	{ "Main Shell",        &TeleportFacilitiesSubmenu::FacilityInfoStructure::mainShellOption,        &TeleportFacilitiesSubmenu::vMainShellOptions },
	{ "Graphics",          &TeleportFacilitiesSubmenu::FacilityInfoStructure::graphicsOption,         &TeleportFacilitiesSubmenu::vGraphicsOptions },
	{ "Trophy",            &TeleportFacilitiesSubmenu::FacilityInfoStructure::trophyOption,           &TeleportFacilitiesSubmenu::vTrophyOptions },
	{ "Orbital Cannon",    &TeleportFacilitiesSubmenu::FacilityInfoStructure::orbitalCannonOption,    &TeleportFacilitiesSubmenu::vOrbitalCannonOptions },
	{ "Security Room",     &TeleportFacilitiesSubmenu::FacilityInfoStructure::securityRoomOption,     &TeleportFacilitiesSubmenu::vSecurityRoomOptions },
	{ "Lounge",            &TeleportFacilitiesSubmenu::FacilityInfoStructure::loungeOption,           &TeleportFacilitiesSubmenu::vLoungeOptions },
	{ "Sleeping Quarters", &TeleportFacilitiesSubmenu::FacilityInfoStructure::sleepingQuartersOption, &TeleportFacilitiesSubmenu::vSleepingQuartersOptions },
	{ "Clutter",           &TeleportFacilitiesSubmenu::FacilityInfoStructure::clutterOption,          &TeleportFacilitiesSubmenu::vClutterOptions },
	{ "Crew Emblem",       &TeleportFacilitiesSubmenu::FacilityInfoStructure::crewEmblemOption,       &TeleportFacilitiesSubmenu::vCrewEmblemOptions },
};

TeleportFacilitiesSubmenu::FacilityInfoStructure TeleportFacilitiesSubmenu::currentFacilityInfo{};
const TeleportFacilitiesSubmenu::FacilityInteriorOptionArray* TeleportFacilitiesSubmenu::selectedOptionArray = nullptr;

void TeleportFacilitiesSubmenu::CreateFacility(FacilityInfoStructure& info)
{
	if (info.location == nullptr)
		return;

	auto& loc = *info.location;
	auto& pos = loc.pos;

	SET_INSTANCE_PRIORITY_MODE(true);
	ON_ENTER_MP();
	for (auto& ipl : loc.ipls)
		REQUEST_IPL(ipl.c_str());
	int interior = GET_INTERIOR_AT_COORDS(pos.x, pos.y, pos.z);
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
			const FacilityInteriorOptionIndex& ptr = info.*(oa.memberPtr);
			auto& o = oa.arr->at(ptr.index);
			ACTIVATE_INTERIOR_ENTITY_SET(interior, o.value.c_str());
			if (o.maxTints > 0)
			{
				for (DWORD timeOut = GetTickCount() + 250; GetTickCount() < timeOut;)
				{
					if (IS_INTERIOR_ENTITY_SET_ACTIVE(interior, o.value.c_str())) break;
					WAIT(0);
				}
				SET_INTERIOR_ENTITY_SET_TINT_INDEX(interior, o.value.c_str(), ptr.currTint);
			}
		}
	}
	REFRESH_INTERIOR(interior);
}

void TeleportFacilitiesSubmenu::UpdateFacilityProp(FacilityInfoStructure& info, const FacilityInteriorOptionArray& arr)
{
	if (info.location == nullptr || arr.memberPtr == nullptr)
		return;

	auto& loc = *info.location;
	auto& pos = loc.pos;

	int interior = GET_INTERIOR_AT_COORDS(pos.x, pos.y, pos.z);
	if (IS_INTERIOR_DISABLED(interior))
		return;

	const FacilityInteriorOptionIndex& ptr = info.*(arr.memberPtr);
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

void TeleportFacilitiesSubmenu::Draw()
{
	DrawTitle();

	for (auto& otherTele : vOtherFacilityRelatedTeleports)
	{
		if (DrawOption(otherTele.name))
		{
			ToTeleLocation241(otherTele);
		}
	}

	DrawBreak("---Build A Facility---");

	for (auto& loc : vLocations)
	{
		if (DrawOption(loc.name))
		{
			currentFacilityInfo.location = &loc;
			NavigateTo("teleport_facilities_in_loc");
		}
	}
}

void TeleportFacilitiesInLocSubmenu::Draw()
{
	auto& info = TeleportFacilitiesSubmenu::currentFacilityInfo;

	if (info.location == nullptr)
	{
		Engine* engine = Engine::Current();
		if (engine) engine->GoBack();
		return;
	}

	DrawTitle();

	GTAped ped = g_Ped1;
	Engine* engine = Engine::Current();

	for (auto& o : TeleportFacilitiesSubmenu::vOptionArrays)
	{
		if (o.name.empty() || o.memberPtr == nullptr)
			continue;

		TeleportFacilitiesSubmenu::FacilityInteriorOptionIndex& ptr = info.*(o.memberPtr);
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
				TeleportFacilitiesSubmenu::UpdateFacilityProp(info, o);
			}
		}
		else if (res.leftPressed)
		{
			if (ptr.index > 0)
			{
				--ptr.index;
				TeleportFacilitiesSubmenu::UpdateFacilityProp(info, o);
			}
		}
		if (res.accepted)
		{
			TeleportFacilitiesSubmenu::selectedOptionArray = &o;
			NavigateTo("teleport_facilities_in_option");
		}
	}

	if (DrawOption("Build Facility"))
	{
		DO_SCREEN_FADE_OUT(50);
		TeleportFacilitiesSubmenu::CreateFacility(info);
		if (info.location != nullptr)
			TeleportNetPed(ped, info.location->pos);
		DO_SCREEN_FADE_IN(200);
		return;
	}
}

void TeleportFacilitiesInOptionSubmenu::Draw()
{
	auto& info = TeleportFacilitiesSubmenu::currentFacilityInfo;
	const auto* arr = TeleportFacilitiesSubmenu::selectedOptionArray;

	if (info.location == nullptr || arr == nullptr || arr->memberPtr == nullptr)
	{
		Engine* engine = Engine::Current();
		if (engine) engine->GoBack();
		return;
	}

	Engine* engine = Engine::Current();
	if (engine)
		engine->AddTitle(arr->name.empty() ? std::string("Option") : arr->name);

	TeleportFacilitiesSubmenu::FacilityInteriorOptionIndex& ptr = info.*(arr->memberPtr);
	const std::vector<std::string> tintNamesVec(
		TeleportFacilitiesSubmenu::vTintNames.begin(),
		TeleportFacilitiesSubmenu::vTintNames.end());

	for (unsigned i = 0; i < arr->arr->size(); ++i)
	{
		auto& o = arr->arr->at(i);
		const bool isSelected = (ptr.index == i);
		if (isSelected && o.maxTints > 1 && engine != nullptr)
		{
			int idx = ptr.currTint;
			const ::Menu::InputResult res = engine->AddTextList(o.name, idx, tintNamesVec);
			if (res.rightPressed)
			{
				if (ptr.currTint < o.maxTints)
				{
					++ptr.currTint;
					TeleportFacilitiesSubmenu::UpdateFacilityProp(info, *arr);
				}
			}
			else if (res.leftPressed)
			{
				if (ptr.currTint > 1)
				{
					--ptr.currTint;
					TeleportFacilitiesSubmenu::UpdateFacilityProp(info, *arr);
				}
			}
		}
		else
		{
			if (DrawSelectionItem(o.name, isSelected))
			{
				ptr.index = static_cast<unsigned __int8>(i);
				if (ptr.currTint > o.maxTints) ptr.currTint = o.maxTints;
				TeleportFacilitiesSubmenu::UpdateFacilityProp(info, *arr);
			}
		}
	}
}

}
REGISTER_SUBMENU(::Menu::TeleportFacilitiesSubmenu)
REGISTER_SUBMENU(::Menu::TeleportFacilitiesInLocSubmenu)
REGISTER_SUBMENU(::Menu::TeleportFacilitiesInOptionSubmenu)
