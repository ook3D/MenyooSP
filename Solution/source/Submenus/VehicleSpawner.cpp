#include "VehicleSpawner.h"

#include "../Menu/Engine.h"
#include "../Menu/SubmenuRegistry.h"
#include "../Menu/Menu.h"      // Checkbox, Menu::bitController, VirtualKey
#include "../Menu/Routine.h"   // shared globals: g_spawnVehicle*
#include "PlayerRuntime.h"
#include "VehicleModShopRuntime.h" // SetVehicleMaxUpgrades
#include "../Menu/FolderPreviewBmps.h"
#include "Misc.h"              // dict/dict2/dict3, GetRandomSpriteId

#include "../Natives/natives2.h"
#include "../Natives/types.h"  // RgbS

#include "../Scripting/Game.h"
#include "../Scripting/GTAped.h"
#include "../Scripting/GTAentity.h"
#include "../Scripting/GTAvehicle.h"
#include "../Scripting/Model.h"
#include "../Scripting/ModelNames.h"

#include "../Util/keyboard.h"
#include "../Util/StringManip.h"
#include "../Util/ExePath.h"

#include "VehicleSpawnerRuntime.h"
#include "VehicleModShopRuntime.h"     // SetVehicleMaxUpgrades

#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include <pugixml/src/pugixml.hpp>
#include <dirent/include/dirent.h>

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace {
inline void toUpperInPlace(std::string& s)
{
	std::transform(s.begin(), s.end(), s.begin(),
		[](unsigned char c) { return static_cast<char>(std::toupper(c)); });
}
inline std::string toLowerCopy(const std::string& s)
{
	std::string out(s);
	std::transform(out.begin(), out.end(), out.begin(),
		[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	return out;
}
inline std::string toUpperCopy(const std::string& s)
{
	std::string out(s); toUpperInPlace(out); return out;
}
} // namespace


namespace Menu {

namespace {

void SpawnAndConfigureVehicle(const GTAmodel::Model& vehModel, Ped ped)
{
	if (!vehModel.IsInCdImage() || !vehModel.IsVehicle())
	{
		Game::Print::PrintErrorInvalidModel(vehModel.VehicleModelName());
		return;
	}

	Vector3 toBeNodeCoordinates = GTAped(ped).GetPosition();
	Vehicle vehicle = sub::SpawnVehicle(vehModel, ped, g_spawnVehicleDeleteOld, g_spawnVehicleAutoSit);
	sub::SetVehicleMaxUpgrades(vehicle, g_spawnVehicleAutoUpgrade, g_spawnVehicleInvincible,
		g_spawnVehiclePlateType, g_spawnVehiclePlateTexterValue == 0 ? g_spawnVehiclePlateText : "",
		g_spawnVehicleNeonToggle,
		g_spawnVehicleNeonColor.R, g_spawnVehicleNeonColor.G, g_spawnVehicleNeonColor.B,
		g_spawnVehiclePrimaryColor, g_spawnVehicleSecondaryColor);

	if (g_addBlip)
	{
		Blip b = ADD_BLIP_FOR_ENTITY(vehicle);
		HUD::SET_BLIP_SPRITE(b, GetRandomSpriteId());
		Game::Print::PrintBottomLeft("Added a blip.");
	}
	if (g_warpNear)
	{
		Vector3_t outPos;
		for (int i = 1; i < 40; i++)
		{
			GET_NTH_CLOSEST_VEHICLE_NODE(toBeNodeCoordinates.x, toBeNodeCoordinates.y, toBeNodeCoordinates.z,
				i, &outPos, 1, 0x40400000, 0);
			if (!IS_POINT_OBSCURED_BY_A_MISSION_ENTITY(outPos.x, outPos.y, outPos.z, 5.0f, 5.0f, 5.0f, 0))
			{
				SET_ENTITY_COORDS(vehicle, outPos.x, outPos.y, outPos.z, 0, 0, 0, 0);
			}
		}
	}
	if (!NETWORK_IS_IN_SESSION() && !g_spawnVehiclePersistent)
	{
		SET_VEHICLE_AS_NO_LONGER_NEEDED(&vehicle);
	}
}

bool DrawFavouriteHotkey(const GTAmodel::Model& vehModel)
{
	Engine* engine = Engine::Current();
	if (!engine) return false;

	const bool bIsAFav = sub::SpawnVehicleIsVehicleModelAFavourite(vehModel);
	const std::string hintLabel = (!bIsAFav ? "Add to" : "Remove from") + std::string(" favourites");

	if (Menu::bitController)
	{
		engine->AddInstructionalButton(INPUT_SCRIPT_RLEFT, hintLabel, /*isKey=*/false);
		if (IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_SCRIPT_RLEFT))
		{
			if (!bIsAFav)
				sub::SpawnVehicleAddVehicleModelToFavourites(vehModel,
					Game::InputBox("", 28U, "Enter custom name:", vehModel.VehicleDisplayName(true)));
			else
				sub::SpawnVehicleRemoveVehicleModelFromFavourites(vehModel);
			return true;
		}
	}
	else
	{
		engine->AddInstructionalButton(static_cast<int>(VirtualKey::B), hintLabel, /*isKey=*/true);
		if (IsKeyJustUp(VirtualKey::B))
		{
			if (!bIsAFav)
				sub::SpawnVehicleAddVehicleModelToFavourites(vehModel,
					Game::InputBox("", 28U, "Enter custom name:", vehModel.VehicleDisplayName(true)));
			else
				sub::SpawnVehicleRemoveVehicleModelFromFavourites(vehModel);
			return true;
		}
	}
	return false;
}

const std::vector<GTAmodel::Model>* CategoryVec(int idx)
{
	using namespace sub::VehicleSpawner;
	switch (idx)
	{
		case OPENWHEEL:     return &g_vehHashes_OPENWHEEL;
		case SUPER:         return &g_vehHashes_SUPER;
		case SPORT:         return &g_vehHashes_SPORT;
		case SPORTSCLASSIC: return &g_vehHashes_SPORTSCLASSIC;
		case COUPE:         return &g_vehHashes_COUPE;
		case MUSCLE:        return &g_vehHashes_MUSCLE;
		case OFFROAD:       return &g_vehHashes_OFFROAD;
		case SUV:           return &g_vehHashes_SUV;
		case SEDAN:         return &g_vehHashes_SEDAN;
		case COMPACT:       return &g_vehHashes_COMPACT;
		case VAN:           return &g_vehHashes_VAN;
		case SERVICE:       return &g_vehHashes_SERVICE;
		case TRAIN:         return &g_vehHashes_TRAIN;
		case EMERGENCY:     return &g_vehHashes_EMERGENCY;
		case MOTORCYCLE:    return &g_vehHashes_MOTORCYCLE;
		case BICYCLE:       return &g_vehHashes_BICYCLE;
		case PLANE:         return &g_vehHashes_PLANE;
		case HELICOPTER:    return &g_vehHashes_HELICOPTER;
		case BOAT:          return &g_vehHashes_BOAT;
		case INDUSTRIAL:    return &g_vehHashes_INDUSTRIAL;
		case COMMERCIAL:    return &g_vehHashes_COMMERCIAL;
		case UTILITY:       return &g_vehHashes_UTILITY;
		case MILITARY:      return &g_vehHashes_MILITARY;
		case OTHER:         return &g_vehHashes_OTHER;
		case DRIFT:         return &g_vehHashes_DRIFT;
	}
	return nullptr;
}

void DrawCategoryRow(const std::string& text, int index)
{
	const std::vector<GTAmodel::Model>* vec = CategoryVec(index);
	if (!vec || vec->empty()) return;

	Engine* engine = Engine::Current();
	if (!engine) return;
	if (engine->AddOption(text, /*showArrow=*/true))
	{
		sub::VehicleSpawner::spawnVehicleIndex = static_cast<UINT8>(index);
		dict = text;
		engine->RequestNavigate("vehicle_spawner_all_cats");
	}
}

} // namespace

void VehicleSpawnerSubmenu::Draw()
{
	DrawTitle();

	dict2.clear();
	dict3.clear();

	if (DrawOption("Spawn Settings")) NavigateTo("vehicle_spawner_options");
	if (DrawOption("Saved Vehicles")) NavigateTo("vehicle_saver");
	if (DrawOption("Favourites"))     NavigateTo("vehicle_spawner_favourites");
	if (DrawOption("Funny Vehicles (Old)")) NavigateTo("funny_vehicles");

	DrawBreak("---Cars---");
	using namespace sub::VehicleSpawner;
	DrawCategoryRow("Open Wheel",      OPENWHEEL);
	DrawCategoryRow("Super",           SUPER);
	DrawCategoryRow("Sports",          SPORT);
	DrawCategoryRow("Sports Classics", SPORTSCLASSIC);
	DrawCategoryRow("Coupes",          COUPE);
	DrawCategoryRow("Muscle",          MUSCLE);
	DrawCategoryRow("Offroad",         OFFROAD);
	DrawCategoryRow("SUVs",            SUV);
	DrawCategoryRow("Sedans",          SEDAN);
	DrawCategoryRow("Compacts",        COMPACT);
	DrawCategoryRow("Drift",           DRIFT);

	DrawBreak("---Industrial---");
	DrawCategoryRow("Vans",       VAN);
	DrawCategoryRow("Services",   SERVICE);
	DrawCategoryRow("Industrial", INDUSTRIAL);
	DrawCategoryRow("Commercial", COMMERCIAL);
	DrawCategoryRow("Utility",    UTILITY);
	DrawCategoryRow("Trains",     TRAIN);

	DrawBreak("---Others---");
	DrawCategoryRow("Emergency",   EMERGENCY);
	DrawCategoryRow("Military",    MILITARY);
	DrawCategoryRow("Motorcycles", MOTORCYCLE);
	DrawCategoryRow("Bicycles",    BICYCLE);
	DrawCategoryRow("Planes",      PLANE);
	DrawCategoryRow("Helicopters", HELICOPTER);
	DrawCategoryRow("Boats",       BOAT);
	DrawCategoryRow("Others",      OTHER);

	if (DrawOption("Random Vehicle"))
	{
		if (g_vehHashes.empty()) return;
		Model model = g_vehHashes[GET_RANDOM_INT_IN_RANGE(0, (int)g_vehHashes.size())];
		SpawnAndConfigureVehicle(model, g_Ped1);
		return;
	}

	if (DrawOption("~b~Input~s~ Model"))
	{
		std::string inputStr = Game::InputBox("", 64U, "Enter vehicle model name (e.g. adder):");
		if (inputStr.length() > 0)
		{
			Model model = GET_HASH_KEY(inputStr);
			SpawnAndConfigureVehicle(model, g_Ped1);
		}
		return;
	}
}

void VehicleSpawnerOptionsSubmenu::Draw()
{
	DrawTitle();

	Engine* engine = Engine::Current();

	DrawToggle("Delete Old Vehicle",          g_spawnVehicleDeleteOld);
	DrawToggle("Auto-Sit In Vehicle",         g_spawnVehicleAutoSit);
	DrawToggle("Add Blip For Spawned Vehicles", g_addBlip);
	DrawToggle("Spawn At Nearest Node",       g_warpNear);
	DrawToggle("Spawn Pre-Upgraded",          g_spawnVehicleAutoUpgrade);
	DrawToggle("Spawn Invincible",            g_spawnVehicleInvincible);
	DrawToggle("Spawn Persistent",            g_spawnVehiclePersistent);

	if (DrawOption("Primary Paint"))
	{
		msCurrentPaintIndex = 10;
		NavigateTo("vehicle_modshop_paints");
	}
	if (DrawOption("Secondary Paint"))
	{
		msCurrentPaintIndex = 11;
		NavigateTo("vehicle_modshop_paints");
	}

	DrawBreak("---Neons---");
	DrawToggle("Toggle", g_spawnVehicleNeonToggle);
	if (DrawOption("RGB Colour"))
	{
		bitMSPaintsRGBMode = 9;
		NavigateTo("vehicle_modshop_paints_rgb");
	}
	if (engine && IsCurrentRowSelected())
	{
		engine->AddPresetColourOptionsPreview(g_spawnVehicleNeonColor);
	}

	DrawBreak("---Plate---");

	// Plate Type: 13-entry text list, non-wrapping clamp.
	{
		static const std::vector<std::string> plateTypeNames{
			"CMOD_PLA_0", "CMOD_PLA_4", "CMOD_PLA_3", "CMOD_PLA_1", "CMOD_PLA_2",
			"Yankton", "CMOD_PLA_6", "CMOD_PLA_7", "CMOD_PLA_8", "CMOD_PLA_9",
			"CMOD_PLA_10", "CMOD_PLA_11", "CMOD_PLA_12"
		};
		int idx = g_spawnVehiclePlateType;
		const ::Menu::InputResult res = engine
			? engine->AddTextList("Type", idx, plateTypeNames)
			: ::Menu::InputResult{};
		if (res.rightPressed && g_spawnVehiclePlateType < (INT8)(plateTypeNames.size() - 1))
		{
			++g_spawnVehiclePlateType;
			return;
		}
		if (res.leftPressed && g_spawnVehiclePlateType > 0)
		{
			--g_spawnVehiclePlateType;
			return;
		}
	}

	{
		std::vector<std::string> plateTexter{ g_spawnVehiclePlateText, "Random" };
		int idx = g_spawnVehiclePlateTexterValue;
		const ::Menu::InputResult res = engine
			? engine->AddTextList("Text", idx, plateTexter)
			: ::Menu::InputResult{};
		if (res.rightPressed && g_spawnVehiclePlateTexterValue < 1)
		{
			++g_spawnVehiclePlateTexterValue;
			return;
		}
		if (res.leftPressed && g_spawnVehiclePlateTexterValue > 0)
		{
			--g_spawnVehiclePlateTexterValue;
			return;
		}
		if (res.accepted && g_spawnVehiclePlateTexterValue == 0)
		{
			std::string inputStr = Game::InputBox("", 9U, "CMOD_MOD_18_D", g_spawnVehiclePlateText);
			if (inputStr.length() > 0 && inputStr.length() <= 8)
			{
				g_spawnVehiclePlateText = inputStr;
			}
			else
			{
				Game::Print::PrintErrorInvalidInput(inputStr);
			}
			return;
		}
	}

	DrawBreak("---Previews---");
	DrawToggle("Enable Previews", g_spawnVehicleDrawBMPs);
	if (DrawOption("Reload Previews"))
	{
		sub::VehicleSpawner::PopulateVehicleBmps();
		sub::FolderPreviewBmps_catind::PopulateFolderBmps();
	}
}

void VehicleSpawnerAllCatsSubmenu::Draw()
{
	Engine* engine = Engine::Current();
	if (engine) engine->AddTitle(dict.empty() ? "Category" : dict);

	const std::vector<GTAmodel::Model>* tempVehicleCopy = CategoryVec(sub::VehicleSpawner::spawnVehicleIndex);
	if (!tempVehicleCopy || tempVehicleCopy->empty())
	{
		if (engine) engine->GoBack();
		return;
	}

	for (const GTAmodel::Model& vehModel : *tempVehicleCopy)
	{
		if (!vehModel.IsInCdImage()) continue;

		if (DrawOption(vehModel.VehicleDisplayName(true)))
		{
			SpawnAndConfigureVehicle(vehModel, g_Ped1);
		}

		if (engine && IsCurrentRowSelected())
		{
			if (g_spawnVehicleDrawBMPs)
				sub::VehicleSpawner::DrawVehicleBmp(vehModel);
			sub::VehicleSpawner::DrawVehicleModelName(vehModel);

			if (DrawFavouriteHotkey(vehModel)) return;
		}
	}
}

void VehicleSpawnerFavouritesSubmenu::Draw()
{
	DrawTitle();

	std::string& searchStr = dict2;

	GTAped myPed = g_Ped1;
	GTAvehicle myVehicle = myPed.CurrentVehicle();
	const bool isInVehicle = myVehicle.Exists();

	const std::string xmlAddedVehicleModels = "AddedVehicleModels.xml";
	const std::string xmlPath = (std::string)GetPathffA(Pathff::Main, true) + xmlAddedVehicleModels;

	pugi::xml_document doc;
	if (doc.load_file(xmlPath.c_str()).status != pugi::status_ok)
	{
		doc.reset();
		auto nodeDecleration = doc.append_child(pugi::node_declaration);
		nodeDecleration.append_attribute("version") = "1.0";
		nodeDecleration.append_attribute("encoding") = "ISO-8859-1";
		doc.append_child("AddedVehicleModels");
		doc.save_file(xmlPath.c_str());
		return;
	}
	pugi::xml_node nodeRoot = doc.child("AddedVehicleModels");

	if (DrawOption("Add New Vehicle Model"))
	{
		std::string inputStrModel = Game::InputBox("", 28U, "Enter model name (e.g. adder):");
		if (inputStrModel.length() > 0)
		{
			WAIT(500);
			Model ism = GET_HASH_KEY(inputStrModel);
			if (ism.IsInCdImage())
			{
				std::string inputStrName = Game::InputBox("", 28U, "Enter custom name:", ism.VehicleDisplayName(true));
				if (sub::SpawnVehicleAddVehicleModelToFavourites(ism, inputStrName))
					Game::Print::PrintBottomLeft("Model ~b~added~s~.");
				else
					Game::Print::PrintBottomLeft("~r~Error:~s~ Unable to add model.");
			}
		}
		else
		{
			Game::Print::PrintErrorInvalidModel(inputStrModel);
		}
	}

	if (isInVehicle)
	{
		const Model& myVehicleModel = myVehicle.Model();
		const bool bIsCurrentModelAFav = sub::SpawnVehicleIsVehicleModelAFavourite(myVehicleModel);
		if (DrawSelectionItem("Current Vehicle's Model", bIsCurrentModelAFav,
			Checkbox::BOXTICK, Checkbox::BOXBLANK))
		{
			if (!bIsCurrentModelAFav)
			{
				std::string inputStrName = Game::InputBox("", 28U, "Enter custom name:",
					myVehicleModel.VehicleDisplayName(true));
				if (inputStrName.length() > 0)
				{
					if (sub::SpawnVehicleAddVehicleModelToFavourites(myVehicleModel, inputStrName))
						Game::Print::PrintBottomLeft("Model ~b~added~s~.");
					else
						Game::Print::PrintBottomLeft("~r~Error:~s~ Unable to add model.");
				}
				else
				{
					Game::Print::PrintErrorInvalidInput(inputStrName);
				}
			}
			else
			{
				if (sub::SpawnVehicleRemoveVehicleModelFromFavourites(myVehicleModel))
					Game::Print::PrintBottomLeft("Model ~b~removed~s~.");
				else
					Game::Print::PrintBottomLeft("~r~Error:~s~ Unable to remove model.");
			}
		}
	}

	if (nodeRoot.first_child())
	{
		DrawBreak("---Added Models---");

		if (DrawOption(searchStr.empty() ? "SEARCH" : searchStr))
		{
			searchStr = Game::InputBox(searchStr, 126U, "SEARCH", toLowerCopy(searchStr));
			toUpperInPlace(searchStr);
		}

		Engine* engine = Engine::Current();
		for (auto nodeLocToLoad = nodeRoot.first_child(); nodeLocToLoad;
			nodeLocToLoad = nodeLocToLoad.next_sibling())
		{
			std::string vehModelName = nodeLocToLoad.attribute("modelName").as_string();
			Model vehModel = nodeLocToLoad.attribute("modelHash").as_uint(0);
			const std::string vehCustomName = nodeLocToLoad.attribute("customName").as_string();

			if (!searchStr.empty()
				&& toUpperCopy(vehModelName).find(searchStr) == std::string::npos)
			{
				continue;
			}

			if (vehModel.hash == 0)
				vehModel = GET_HASH_KEY(vehModelName);

			const std::string vehDisplayName = vehCustomName.length()
				? vehCustomName : vehModel.VehicleDisplayName(true);

			if (!vehModel.IsInCdImage())
			{
				if (DrawOption(vehDisplayName + " (Invalid)"))
				{
					// no-op: row exists only so the user sees the entry.
				}
				continue;
			}

			if (DrawOption(vehDisplayName))
			{
				SpawnAndConfigureVehicle(vehModel, g_Ped1);
			}

			if (engine && IsCurrentRowSelected())
			{
				if (g_spawnVehicleDrawBMPs)
					sub::VehicleSpawner::DrawVehicleBmp(vehModel);
				sub::VehicleSpawner::DrawVehicleModelName(vehModel);

				// "Remove from favourites" contextual hotkey.
				if (Menu::bitController)
				{
					engine->AddInstructionalButton(INPUT_SCRIPT_RLEFT, "Remove", /*isKey=*/false);
					if (IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_SCRIPT_RLEFT))
					{
						nodeLocToLoad.parent().remove_child(nodeLocToLoad);
						doc.save_file(xmlPath.c_str());
						return;
					}
				}
				else
				{
					engine->AddInstructionalButton(static_cast<int>(VirtualKey::B), "Remove", /*isKey=*/true);
					if (IsKeyJustUp(VirtualKey::B))
					{
						nodeLocToLoad.parent().remove_child(nodeLocToLoad);
						doc.save_file(xmlPath.c_str());
						return;
					}
				}
			}
		}
	}
}

void VehicleSpawnerDlcSelectionSubmenu::Draw()
{
	DrawTitle();

	Engine* engine = Engine::Current();
	for (size_t i = 0; i < sub::VehicleDlcCategories.size(); ++i)
	{
		if (engine && engine->AddOption(sub::VehicleDlcCategories[i].name, /*showArrow=*/true))
		{
			sub::vehDLCCategoryID = static_cast<int>(i);
			NavigateTo("vehicle_spawner_dlc");
		}
	}
}

void VehicleSpawnerDlcSubmenu::Draw()
{
	Engine* engine = Engine::Current();

	if (sub::vehDLCCategoryID < 0
		|| sub::vehDLCCategoryID >= static_cast<int>(sub::VehicleDlcCategories.size()))
	{
		if (engine) engine->AddTitle("DLC");
		Game::Print::PrintBottomLeft("Invalid Category");
		return;
	}

	const auto& selectedCategory = sub::VehicleDlcCategories[sub::vehDLCCategoryID];
	if (engine) engine->AddTitle(selectedCategory.name);

	for (size_t i = 0; i < selectedCategory.captions.size(); ++i)
	{
		const std::string& caption = selectedCategory.captions[i];
		const std::string& valueStr = (i < selectedCategory.values.size())
			? selectedCategory.values[i] : std::string{};

		if (DrawOption(caption))
		{
			if (!valueStr.empty())
			{
				Model model(valueStr);
				SpawnAndConfigureVehicle(model, g_Ped1);
			}
		}

		if (engine && IsCurrentRowSelected() && !valueStr.empty())
		{
			sub::vehDlcIdToSpawn = static_cast<int>(i);

			Model vehModel(valueStr);
			if (g_spawnVehicleDrawBMPs)
				sub::VehicleSpawner::DrawVehicleBmp(vehModel);

			if (DrawFavouriteHotkey(vehModel)) return;
		}
	}
}

void VehicleSaverSubmenu::Draw()
{
	DrawTitle();

	std::string& searchStr = dict2;
	std::string& name = dict;
	std::string& dirStr = dict3;

	auto& ped = g_Ped1;
	auto vehicle = GET_VEHICLE_PED_IS_USING(ped);
	const bool isPedInVeh = IS_PED_IN_ANY_VEHICLE(ped, 0) || IS_PED_SITTING_IN_ANY_VEHICLE(ped);

	Engine* engine = Engine::Current();

	{
		static const std::vector<std::string> persistOpts{ "FileDecides", "ForceOff", "ForceOn" };
		int idx = sub::VehicleSaver::_persistentAttachmentsTexterIndex;
		const ::Menu::InputResult res = engine
			? engine->AddTextList("AddAttachmentsToSpoonerDB", idx, persistOpts)
			: ::Menu::InputResult{};
		if (res.rightPressed && sub::VehicleSaver::_persistentAttachmentsTexterIndex < 2)
			++sub::VehicleSaver::_persistentAttachmentsTexterIndex;
		else if (res.leftPressed && sub::VehicleSaver::_persistentAttachmentsTexterIndex > 0)
			--sub::VehicleSaver::_persistentAttachmentsTexterIndex;
	}

	{
		static const std::vector<std::string> driverVisiOpts{ "FileDecides", "Retain" "ForceOff", "ForceOn" };
		int idx = sub::VehicleSaver::_driverVisibilityTexterIndex;
		const ::Menu::InputResult res = engine
			? engine->AddTextList("Driver Visibility", idx, driverVisiOpts)
			: ::Menu::InputResult{};
		if (res.rightPressed && sub::VehicleSaver::_driverVisibilityTexterIndex < 3)
			++sub::VehicleSaver::_driverVisibilityTexterIndex;
		else if (res.leftPressed && sub::VehicleSaver::_driverVisibilityTexterIndex > 0)
			--sub::VehicleSaver::_driverVisibilityTexterIndex;
	}

	bool save2Pressed = DrawOption("Save Current Vehicle");
	bool savecarvarPressed = DrawOption("Store CarVariations");
	bool msPaintsSaveRgbPressed = DrawOption("Save Colour Profile");
	bool createFolderPressed = DrawOption("Create New Folder");

	if (dirStr.empty())
		dirStr = GetPathffA(Pathff::Vehicle, false);

	std::vector<std::string> vfilnames;
	DIR* dir_point = opendir(dirStr.c_str());
	if (dir_point)
	{
		dirent* entry = readdir(dir_point);
		while (entry)
		{
			vfilnames.push_back(entry->d_name);
			entry = readdir(dir_point);
		}
		closedir(dir_point);
	}

	DrawBreak("---Found Files---");

	if (DrawOption(".."))
	{
		dirStr = dirStr.substr(0, dirStr.rfind("\\"));
	}

	if (!vfilnames.empty())
	{
		if (DrawOption(searchStr.empty() ? "SEARCH" : searchStr))
		{
			searchStr = Game::InputBox(searchStr, 126U, "SEARCH", toLowerCopy(searchStr));
			toUpperInPlace(searchStr);
		}

		for (auto& filname : vfilnames)
		{
			if (filname.front() == '.' || filname.front() == ',')
				continue;
			if (!searchStr.empty()
				&& toUpperCopy(filname).find(searchStr) == std::string::npos)
			{
				continue;
			}

			const bool isFolder = PathIsDirectoryA((dirStr + "\\" + filname).c_str()) != 0;
			const bool isXml = filname.length() > 4 && filname.rfind(".xml") == filname.length() - 4;
			Checkbox icon = Checkbox::NONE;
			if (isFolder) icon = Checkbox::ARROWRIGHT;
			else if (isXml) icon = Checkbox::TICK2;

			if (isFolder)
			{
				if (DrawSelectionItem(filname + " >>>", true, icon, Checkbox::NONE))
				{
					dirStr = dirStr + "\\" + filname;
				}
				if (engine && IsCurrentRowSelected())
				{
					if (sub::FolderPreviewBmps_catind::bFolderBmpsEnabled)
						sub::FolderPreviewBmps_catind::DrawBmp(dirStr + "\\" + filname);
				}
			}
			else if (isXml)
			{
				if (DrawSelectionItem(filname, true, icon, Checkbox::NONE))
				{
					name = filname.substr(0, filname.rfind('.'));
					NavigateTo("vehicle_saver_in_item");
					return;
				}
			}
		}
	}

	if (save2Pressed)
	{
		if (isPedInVeh)
		{
			std::string inputStr = Game::InputBox("", 28U, "Enter file name:");
			if (inputStr.length() > 0)
			{
				if (!IsSafePath(inputStr))
					Game::Print::PrintBottomCentre("~r~Error:~s~ Invalid characters in name.");
				else
					sub::VehicleSaver::VehicleSaveToFile(dirStr + "\\" + inputStr + ".xml", vehicle);
			}
			else
			{
				Game::Print::PrintErrorInvalidInput(inputStr);
			}
		}
	}

	if (msPaintsSaveRgbPressed)
	{
		sub::VehicleSaver::saveColourVals();
	}

	if (savecarvarPressed)
	{
		if (isPedInVeh)
		{
			sub::VehicleSaver::saveCarVars(vehicle);
		}
	}

	if (createFolderPressed)
	{
		std::string inputStr = Game::InputBox("", 28U, "Enter folder name:");
		if (inputStr.length() > 0)
		{
			if (!IsSafePath(inputStr))
			{
				Game::Print::PrintBottomCentre("~r~Error:~s~ Invalid characters in name.");
			}
			else if (CreateDirectoryA((dirStr + "\\" + inputStr).c_str(), NULL)
				|| GetLastError() == ERROR_ALREADY_EXISTS)
			{
				dirStr = dirStr + "\\" + inputStr;
				Game::Print::PrintBottomLeft("Folder ~b~created~s~.");
			}
			else
			{
				Game::Print::PrintBottomCentre("~r~Failed~s~ to create folder.");
			}
		}
		else
		{
			Game::Print::PrintErrorInvalidInput(inputStr);
		}
		return;
	}
}

void VehicleSaverInItemSubmenu::Draw()
{
	Engine* engine = Engine::Current();
	if (engine) engine->AddTitle(dict.empty() ? "Saved Vehicle" : dict);

	std::string& name = dict;
	std::string& dirStr = dict3;
	const std::string filePath = dirStr + "\\" + name + ".xml";

	auto& ped = g_Ped1;
	auto vehicle = GET_VEHICLE_PED_IS_USING(ped);
	const bool isPedInVeh = IS_PED_IN_ANY_VEHICLE(ped, 0) || IS_PED_SITTING_IN_ANY_VEHICLE(ped);

	if (DrawOption("Spawn"))
	{
		sub::VehicleSaver::VehicleReadFromFile(filePath, ped);
	}

	if (DrawOption("Rename File"))
	{
		std::string inputStr = Game::InputBox("", 28U, "Enter new name:", name);
		if (inputStr.length() > 0)
		{
			if (!IsSafePath(inputStr))
			{
				Game::Print::PrintBottomCentre("~r~Error:~s~ Invalid characters in name.");
			}
			else
			{
				const std::string oldPath = dirStr + "\\" + name + ".xml";
				const std::string newPath = dirStr + "\\" + inputStr + ".xml";
				if (rename(oldPath.c_str(), newPath.c_str()) == 0)
				{
					Game::Print::PrintBottomLeft("File ~b~renamed~s~.");
					name = inputStr;
				}
				else
				{
					Game::Print::PrintBottomCentre("~r~Error~s~ renaming file.");
				}
			}
		}
		else
		{
			Game::Print::PrintErrorInvalidInput(inputStr);
		}
	}

	if (isPedInVeh)
	{
		if (DrawOption("Overwrite File"))
		{
			sub::VehicleSaver::VehicleSaveToFile(filePath, vehicle);
		}
	}

	if (DrawOption("Delete File"))
	{
		if (remove(filePath.c_str()) == 0)
			Game::Print::PrintBottomLeft("File ~b~deleted~s~.");
		else
			Game::Print::PrintBottomCentre("~r~Error~s~ deleting file.");

		if (engine) engine->GoBack();
		return;
	}

	pugi::xml_document doc;
	if (doc.load_file(filePath.c_str()).status != pugi::status_ok)
		return;

	DrawBreak("---Attributes---");

	auto nodeVehicle = doc.child("Vehicle");

	auto nodeDriverVisible = nodeVehicle.child("IsDriverVisible");
	if (nodeDriverVisible)
	{
		const bool driverVis = nodeDriverVisible.text().as_bool();
		if (DrawSelectionItem("Driver Visibility", driverVis, Checkbox::BOXTICK, Checkbox::BOXBLANK))
		{
			nodeDriverVisible.text() = !driverVis;
			doc.save_file(filePath.c_str());
		}
	}

	auto nodeAddAttachmentsToSpoonerDb = nodeVehicle.child("SpoonerAttachments")
		.attribute("SetAttachmentsPersistentAndAddToSpoonerDatabase");
	bool bAddAttachemntsToSpoonerDb = nodeAddAttachmentsToSpoonerDb.as_bool();
	if (DrawSelectionItem("Persistent Attachments (AddToSpoonerDb)",
		bAddAttachemntsToSpoonerDb, Checkbox::BOXTICK, Checkbox::BOXBLANK))
	{
		nodeAddAttachmentsToSpoonerDb = !bAddAttachemntsToSpoonerDb;
		bAddAttachemntsToSpoonerDb = !bAddAttachemntsToSpoonerDb;
		doc.save_file(filePath.c_str());
	}

	if (bAddAttachemntsToSpoonerDb)
	{
		auto nodeStartTaskSeqOnLoad = nodeVehicle.child("SpoonerAttachments")
			.attribute("StartTaskSequencesOnLoad");
		if (nodeStartTaskSeqOnLoad)
		{
			const bool startTaskSeq = nodeStartTaskSeqOnLoad.as_bool();
			if (DrawSelectionItem("Start Task Sequences Immediately",
				startTaskSeq, Checkbox::BOXTICK, Checkbox::BOXBLANK))
			{
				nodeStartTaskSeqOnLoad = !startTaskSeq;
				doc.save_file(filePath.c_str());
			}
		}
	}
}

}
REGISTER_SUBMENU(::Menu::VehicleSpawnerSubmenu)
REGISTER_SUBMENU(::Menu::VehicleSpawnerOptionsSubmenu)
REGISTER_SUBMENU(::Menu::VehicleSpawnerAllCatsSubmenu)
REGISTER_SUBMENU(::Menu::VehicleSpawnerDlcSubmenu)
REGISTER_SUBMENU(::Menu::VehicleSpawnerDlcSelectionSubmenu)
REGISTER_SUBMENU(::Menu::VehicleSpawnerFavouritesSubmenu)
REGISTER_SUBMENU(::Menu::VehicleSaverSubmenu)
REGISTER_SUBMENU(::Menu::VehicleSaverInItemSubmenu)
