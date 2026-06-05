#include "TeleportGunRunningInteriors.h"

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

const std::vector<BunkerLocation>& TeleportBunkersSubmenu::Locations()
{
	static const std::vector<BunkerLocation> kLocations
	{
		{ "Regular", { 938.3077f, -3196.1120f, -98.0000f }, "gr_grdlc_interior_placement_interior_1_grdlc_int_02_milo_" },
	};
	return kLocations;
}

const std::vector<BunkerInteriorOption>& TeleportBunkersSubmenu::StyleOptions()
{
	static const std::vector<BunkerInteriorOption> kStyle
	{
		{ "A", "bunker_style_a" },
		{ "B", "bunker_style_b" },
		{ "C", "bunker_style_c" },
	};
	return kStyle;
}

const std::vector<BunkerInteriorOption>& TeleportBunkersSubmenu::SetOptions()
{
	static const std::vector<BunkerInteriorOption> kSet
	{
		{ "Standard", "standard_bunker_set" },
		{ "Upgraded", "upgrade_bunker_set" },
	};
	return kSet;
}

const std::vector<BunkerInteriorOption>& TeleportBunkersSubmenu::SecurityOptions()
{
	static const std::vector<BunkerInteriorOption> kSecurity
	{
		{ "Standard", "standard_security_set" },
		{ "Upgraded", "security_upgrade" },
	};
	return kSecurity;
}

const std::vector<BunkerInteriorOption>& TeleportBunkersSubmenu::OfficeOptions()
{
	static const std::vector<BunkerInteriorOption> kOffice
	{
		{ "Blocked", "Office_blocker_set" },
		{ "Upgraded", "Office_Upgrade_set" },
	};
	return kOffice;
}

const std::vector<BunkerInteriorOption>& TeleportBunkersSubmenu::GunRangeOptions()
{
	static const std::vector<BunkerInteriorOption> kGunRange
	{
		{ "Blcoked Section", "gun_range_blocker_set" }, // (sic; preserved verbatim)
		{ "Blocked Gun Range", "gun_wall_blocker" },
		{ "Present", "gun_range_lights" },
	};
	return kGunRange;
}

const std::vector<BunkerInteriorOption>& TeleportBunkersSubmenu::GunLockerOptions()
{
	static const std::vector<BunkerInteriorOption> kGunLocker
	{
		{ "None", "" },
		{ "Present", "gun_locker_upgrade" },
	};
	return kGunLocker;
}

const std::vector<BunkerInteriorOption>& TeleportBunkersSubmenu::GunSchematicOptions()
{
	static const std::vector<BunkerInteriorOption> kGunSchematic
	{
		{ "None", "" },
		{ "Present", "Gun_schematic_set" },
	};
	return kGunSchematic;
}

BunkerInfoStructure& TeleportBunkersSubmenu::CurrentInfo()
{
	static BunkerInfoStructure info{ nullptr, 0, 0, 0, 0, 0, 0, 0 };
	return info;
}

namespace {

struct BunkerOptionArrayEntry
{
	const char* name;
	unsigned char* ptr;
	const std::vector<BunkerInteriorOption>* arr;
};

std::vector<BunkerOptionArrayEntry> BuildBunkerOptionArrays()
{
	auto& info = TeleportBunkersSubmenu::CurrentInfo();
	return {
		{ "Style",          &info.styleOption,         &TeleportBunkersSubmenu::StyleOptions() },
		{ "Set",            &info.setOption,           &TeleportBunkersSubmenu::SetOptions() },
		{ "Security",       &info.securityOption,      &TeleportBunkersSubmenu::SecurityOptions() },
		{ "Office",         &info.officeOption,        &TeleportBunkersSubmenu::OfficeOptions() },
		{ "Gun Range",      &info.gunRangeOption,      &TeleportBunkersSubmenu::GunRangeOptions() },
		{ "Gun Locker",     &info.gunLockerOption,     &TeleportBunkersSubmenu::GunLockerOptions() },
		{ "Gun Schematics", &info.gunSchematicOption,  &TeleportBunkersSubmenu::GunSchematicOptions() },
	};
}

void CreateBunker(BunkerInfoStructure& info)
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

	const auto optionArrays = BuildBunkerOptionArrays();
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

void TeleportPedToBunker(GTAentity ped, BunkerInfoStructure& info)
{
	if (info.location != nullptr)
	{
		TeleportNetPed(ped, info.location->pos);
	}
}

}

void TeleportBunkersSubmenu::Draw()
{
	DrawTitle();

	auto& info = CurrentInfo();
	for (auto& loc : Locations())
	{
		if (DrawOption(loc.name))
		{
			info.location = &loc;
			NavigateTo("teleport_bunkers_in_loc");
		}
	}
}

void TeleportBunkersInLocSubmenu::Draw()
{
	auto& info = TeleportBunkersSubmenu::CurrentInfo();
	if (info.location == nullptr)
	{
		Engine* engine = Engine::Current();
		if (engine) engine->GoBack();
		return;
	}

	GTAped ped = g_Ped1;
	Engine* engine = Engine::Current();
	if (engine) engine->AddTitle(info.location->name);

	const auto optionArrays = BuildBunkerOptionArrays();
	for (auto& o : optionArrays)
	{
		const std::vector<std::string> singleItem{ o.arr->at(*o.ptr).name };
		InputResult input = engine
			? engine->AddTextList(o.name, 0, singleItem)
			: InputResult{};
		if (input.rightPressed && *o.ptr < o.arr->size() - 1) (*o.ptr)++;
		if (input.leftPressed && *o.ptr > 0) (*o.ptr)--;
	}

	if (DrawOption("Build Bunker"))
	{
		DO_SCREEN_FADE_OUT(50);
		CreateBunker(info);
		TeleportPedToBunker(ped, info);
		DO_SCREEN_FADE_IN(200);
		return;
	}
}

const std::vector<MocLocation>& TeleportMocSubmenu::Locations()
{
	static const std::vector<MocLocation> kLocations
	{
		{ "Regular", { 1103.5620f, -3000.00000f, -38.0000f }, "gr_grdlc_interior_placement_interior_0_grdlc_int_01_milo_" },
	};
	return kLocations;
}

const std::vector<MocInteriorOption>& TeleportMocSubmenu::StyleOptions()
{
	static const std::vector<MocInteriorOption> kStyle
	{
		{ "", "" },
	};
	return kStyle;
}

MocInfoStructure& TeleportMocSubmenu::CurrentInfo()
{
	static MocInfoStructure info{ nullptr, 0 };
	return info;
}

namespace {

struct MocOptionArrayEntry
{
	const char* name;
	unsigned char* ptr;
	const std::vector<MocInteriorOption>* arr;
};

std::vector<MocOptionArrayEntry> BuildMocOptionArrays()
{
	auto& info = TeleportMocSubmenu::CurrentInfo();
	return {
		{ "~italics~no options available yet", &info.styleOption, &TeleportMocSubmenu::StyleOptions() },
	};
}

void CreateMoc(MocInfoStructure& info)
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

	const auto optionArrays = BuildMocOptionArrays();
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

void TeleportPedToMoc(GTAentity ped, MocInfoStructure& info)
{
	if (info.location != nullptr)
	{
		TeleportNetPed(ped, info.location->pos);
	}
}

}

void TeleportMocSubmenu::Draw()
{
	DrawTitle();

	auto& info = CurrentInfo();
	for (auto& loc : Locations())
	{
		if (DrawOption(loc.name))
		{
			info.location = &loc;
			NavigateTo("teleport_moc_in_loc");
		}
	}
}

void TeleportMocInLocSubmenu::Draw()
{
	auto& info = TeleportMocSubmenu::CurrentInfo();
	if (info.location == nullptr)
	{
		Engine* engine = Engine::Current();
		if (engine) engine->GoBack();
		return;
	}

	GTAped ped = g_Ped1;
	Engine* engine = Engine::Current();
	if (engine) engine->AddTitle(info.location->name);

	const auto optionArrays = BuildMocOptionArrays();
	for (auto& o : optionArrays)
	{
		const std::vector<std::string> singleItem{ o.arr->at(*o.ptr).name };
		InputResult input = engine
			? engine->AddTextList(o.name, 0, singleItem)
			: InputResult{};
		if (input.rightPressed && *o.ptr < o.arr->size() - 1) (*o.ptr)++;
		if (input.leftPressed && *o.ptr > 0) (*o.ptr)--;
	}

	if (DrawOption("Build MOC"))
	{
		DO_SCREEN_FADE_OUT(50);
		CreateMoc(info);
		TeleportPedToMoc(ped, info);
		DO_SCREEN_FADE_IN(200);
		return;
	}
}

}
REGISTER_SUBMENU(::Menu::TeleportBunkersSubmenu)
REGISTER_SUBMENU(::Menu::TeleportBunkersInLocSubmenu)
REGISTER_SUBMENU(::Menu::TeleportMocSubmenu)
REGISTER_SUBMENU(::Menu::TeleportMocInLocSubmenu)
