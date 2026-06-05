#include "Teleport.h"

#include "../Menu/Engine.h"
#include "../Menu/SubmenuRegistry.h"
#include "../Menu/Menu.h"           // SUB::*, MenuPressTimer
#include "../Menu/Routine.h"        // g_Ped1
#include "PlayerRuntime.h"          // g_Ped1
#include "../Menu/submenu_enum.h"

#include "../Natives/natives2.h"
#include "../Util/GTAmath.h"
#include "../Util/ExePath.h"
#include "../Util/keyboard.h"
#include "../Scripting/Game.h"
#include "../Scripting/GTAentity.h"
#include "../Scripting/GTAblip.h"
#include "../Scripting/BlipEnums.h"
#include "../Scripting/World.h"
#include "../Memory/GTAmemory.h"

#include "Teleport/TeleLocation.h"
#include "Teleport/Locations.h"
#include "Teleport/TeleMethods.h"

#include <pugixml\src\pugixml.hpp>

#include <string>
#include <string_view>

using ::sub::TeleportLocations_catind::NamedTeleLocationList;
using ::sub::TeleportLocations_catind::TeleLocation;

namespace {

const NamedTeleLocationList* g_selectedCategory = nullptr;

const char* SubEnumToMenuId(SUB::SUB sub)
{
	switch (sub)
	{
	case SUB::TELEPORTOPS_OFFICEGARAGES:        return "teleport_office_garages";
	case SUB::TELEPORTOPS_IEVEHICLEWAREHOUSES:  return "teleport_ie_vehicle_warehouses";
	case SUB::TELEPORTOPS_BIKERCLUBHOUSES:      return "teleport_biker_clubhouses";
	case SUB::TELEPORTOPS_BUSINESSES:           return "teleport_biker_businesses";
	case SUB::TELEPORTOPS_BUNKERS:              return "teleport_bunkers";
	case SUB::TELEPORTOPS_MOC:                  return "teleport_moc";
	case SUB::TELEPORTOPS_HANGARS:              return "teleport_hangars";
	case SUB::TELEPORTOPS_FACILITIES:           return "teleport_facilities";
	case SUB::TELEPORTOPS_NIGHTCLUBS:           return "teleport_nightclubs";
	case SUB::TELEPORTOPS_ARENAWAR:             return "teleport_arena_war";
	case SUB::TELEPORTOPS_YACHTS:               return "teleport_yachts";
	default:                                    return nullptr;
	}
}

}

namespace Menu {

using ::sub::TeleportLocations_catind::TeleMethods::ToForward241;
using ::sub::TeleportLocations_catind::TeleMethods::ToWaypoint241;
using ::sub::TeleportLocations_catind::TeleMethods::ToMissionBlip241;
using ::sub::TeleportLocations_catind::TeleMethods::ToCoordinates241;
using ::sub::TeleportLocations_catind::TeleMethods::ToTeleLocation241;

void TeleportSubmenu::Draw()
{
	DrawTitle();

	if (DrawOption("Forward"))         ToForward241();
	if (DrawOption("Waypoint"))        ToWaypoint241();
	if (DrawOption("Mission Objective")) ToMissionBlip241();
	if (DrawOption("Map Blips"))       NavigateTo("teleport_blip_list");

	DrawBreak("---Ready To Go---");

	for (auto& cat : ::sub::TeleportLocations_catind::Locations::vAllCategories)
	{
		if (DrawOption(cat.categoryName))
		{
			g_selectedCategory = &cat;

			const auto rawPtr = reinterpret_cast<unsigned long long>(cat.nextNamedLocListList);
			if (cat.nextNamedLocListList != nullptr && rawPtr < static_cast<unsigned long long>(SUB::MAX_SUBS))
			{
				const SUB::SUB subEnum = static_cast<SUB::SUB>(rawPtr);
				if (const char* menuId = SubEnumToMenuId(subEnum))
				{
					NavigateTo(menuId);
					continue;
				}
			}
			NavigateTo("teleport_selected_category");
		}
	}

	DrawBreak("---Custom---");
	if (DrawOption("Custom Coordinates")) NavigateTo("teleport_custom_coords");
	if (DrawOption("Favourites"))         NavigateTo("teleport_saved_locations");
}

void TeleportCustomCoordsSubmenu::Draw()
{
	GTAentity thisEntity = g_Ped1;

	if (!grabbedCoords)
	{
		customTeleLoc = GET_ENTITY_COORDS(PLAYER_PED_ID(), 0);
		grabbedCoords = true;
	}

	DrawTitle();

	if (DrawOption("Update to current"))
	{
		grabbedCoords = false;
	}

	bool xChanged = DrawNumber("  X", customTeleLoc.x, 0.1f, 4);
	if (IsCurrentRowSelected() && MenuPressTimer::IsButtonTapped(MenuPressTimer::Button::Accept))
	{
		try
		{
			customTeleLoc.x = std::stof(Game::InputBox(std::to_string(customTeleLoc.x), 11U, std::string(),
				std::to_string(customTeleLoc.x)));
		}
		catch (...) {}
	}

	bool yChanged = DrawNumber("  Y", customTeleLoc.y, 0.1f, 4);
	if (IsCurrentRowSelected() && MenuPressTimer::IsButtonTapped(MenuPressTimer::Button::Accept))
	{
		try
		{
			customTeleLoc.y = std::stof(Game::InputBox(std::to_string(customTeleLoc.y), 11U, std::string(),
				std::to_string(customTeleLoc.y)));
		}
		catch (...) {}
	}

	bool zChanged = DrawNumber("  Z", customTeleLoc.z, 0.1f, 4);
	if (IsCurrentRowSelected() && MenuPressTimer::IsButtonTapped(MenuPressTimer::Button::Accept))
	{
		try
		{
			customTeleLoc.z = std::stof(Game::InputBox(std::to_string(customTeleLoc.z), 11U, std::string(),
				std::to_string(customTeleLoc.z)));
		}
		catch (...) {}
	}

	(void)xChanged; (void)yChanged; (void)zChanged;

	if (DrawOption("Apply"))
	{
		grabbedCoords = false;
		TeleportNetPed(thisEntity, customTeleLoc.x, customTeleLoc.y, customTeleLoc.z);
	}
}

void TeleportSelectedCategorySubmenu::Draw()
{
	if (g_selectedCategory == nullptr)
	{
		Engine* engine = Engine::Current();
		if (engine) engine->GoBack();
		return;
	}

	Engine* engine = Engine::Current();
	if (engine) engine->AddTitle(g_selectedCategory->categoryName);

	if (g_selectedCategory->locList_ptr != nullptr)
	{
		for (auto& loc : *g_selectedCategory->locList_ptr)
		{
			if (DrawOption(loc.name)) ToTeleLocation241(loc);
		}
	}
	if (g_selectedCategory->nextNamedLocListList != nullptr)
	{
		const auto rawPtr = reinterpret_cast<unsigned long long>(g_selectedCategory->nextNamedLocListList);
		if (rawPtr >= static_cast<unsigned long long>(SUB::MAX_SUBS))
		{
			for (auto& locList : *g_selectedCategory->nextNamedLocListList)
			{
				DrawBreak(locList.categoryName);
				if (locList.locList_ptr != nullptr)
				{
					for (auto& loc : *locList.locList_ptr)
					{
						if (DrawOption(loc.name)) ToTeleLocation241(loc);
					}
				}
			}
		}
	}
}

void TeleportBlipListSubmenu::Draw()
{
	DrawTitle();

	BlipList* blipList = GTAmemory::GetBlipList();
	for (UINT16 i = 0; i <= 1000; i++)
	{
		Blipx* blip = blipList->m_Blips[i];
		if (!blip) continue;
		if (blip->iIcon > 521) continue;

		const Vector3& blipPosition = Vector3(blip->x, blip->y, blip->z);
		auto bnit = BlipIcon::vNames.find(blip->iIcon);
		const std::string& blipName = bnit == BlipIcon::vNames.end() ? "Unknown" : bnit->second;
		if (DrawOption(blipName + " (" + World::GetZoneName(blipPosition, true) + ")"))
		{
			ToCoordinates241(blipPosition);
		}
	}
}

void TeleportSavedLocationsSubmenu::Draw()
{
	DrawTitle();

	const std::string xmlSavedMapLocations = "SavedMapLocations.xml";
	pugi::xml_document doc;
	if (doc.load_file((const char*)(GetPathffA(Pathff::Main, true) + xmlSavedMapLocations).c_str()).status != pugi::status_ok)
	{
		doc.reset();
		auto nodeDecleration = doc.append_child(pugi::node_declaration);
		nodeDecleration.append_attribute("version") = "1.0";
		nodeDecleration.append_attribute("encoding") = "ISO-8859-1";
		doc.append_child("SavedMapLocations");
		doc.save_file((const char*)(GetPathffA(Pathff::Main, true) + xmlSavedMapLocations).c_str());
		return;
	}
	pugi::xml_node nodeRoot = doc.child("SavedMapLocations");

	if (DrawOption("Save Current Location"))
	{
		std::string inputStr = Game::InputBox("", 28U, "Enter name:");
		if (inputStr.length() > 0)
		{
			GTAentity ent = g_Ped1;
			const Vector3& myPos = ent.GetPosition();
			const Vector3& myRot = ent.Rotation_get();
			auto nodeOldLoc = nodeRoot.find_child_by_attribute("name", inputStr.c_str());
			if (nodeOldLoc)
			{
				nodeRoot.remove_child(nodeOldLoc);
			}
			auto nodeNewLoc = nodeRoot.append_child("Loc");
			nodeNewLoc.append_attribute("name") = inputStr.c_str();
			nodeNewLoc.append_child("X").text() = myPos.x;
			nodeNewLoc.append_child("Y").text() = myPos.y;
			nodeNewLoc.append_child("Z").text() = myPos.z;
			nodeNewLoc.append_child("Yaw").text() = myRot.z;
			if (doc.save_file((const char*)(GetPathffA(Pathff::Main, true) + xmlSavedMapLocations).c_str()))
			{
				Game::Print::PrintBottomLeft("Location ~b~saved~s~.");
			}
		}
		else
		{
			Game::Print::PrintErrorInvalidInput(inputStr);
		}
	}

	if (nodeRoot.first_child())
	{
		DrawBreak("---Locations---");
		for (auto nodeLocToLoad = nodeRoot.first_child(); nodeLocToLoad; nodeLocToLoad = nodeLocToLoad.next_sibling())
		{
			Vector3 locPos;
			locPos.x = nodeLocToLoad.child("X").text().as_float();
			locPos.y = nodeLocToLoad.child("Y").text().as_float();
			locPos.z = nodeLocToLoad.child("Z").text().as_float();

			const std::string label = (std::string)nodeLocToLoad.attribute("name").as_string()
				+ " - " + World::GetZoneName(locPos, true);
			if (DrawOption(label))
			{
				ToCoordinates241(locPos);
			}

			// Per-row "Remove" affordance on the selected row.
			if (IsCurrentRowSelected())
			{
				Engine* engine = Engine::Current();
				if (Menu::bitController)
				{
					if (engine) engine->AddInstructionalButton(INPUT_SCRIPT_RLEFT, "Remove");
					if (IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_SCRIPT_RLEFT))
					{
						nodeLocToLoad.parent().remove_child(nodeLocToLoad);
						doc.save_file((const char*)(GetPathffA(Pathff::Main, true) + xmlSavedMapLocations).c_str());
						return;
					}
				}
				else
				{
					if (engine) engine->AddInstructionalButton(static_cast<int>(VirtualKey::B), "Remove", true);
					if (IsKeyJustUp(VirtualKey::B))
					{
						nodeLocToLoad.parent().remove_child(nodeLocToLoad);
						doc.save_file((const char*)(GetPathffA(Pathff::Main, true) + xmlSavedMapLocations).c_str());
						return;
					}
				}
			}
		}
	}
}

}
REGISTER_SUBMENU(::Menu::TeleportSubmenu)
REGISTER_SUBMENU(::Menu::TeleportCustomCoordsSubmenu)
REGISTER_SUBMENU(::Menu::TeleportSelectedCategorySubmenu)
REGISTER_SUBMENU(::Menu::TeleportBlipListSubmenu)
REGISTER_SUBMENU(::Menu::TeleportSavedLocationsSubmenu)
