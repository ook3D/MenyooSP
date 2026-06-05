#include "SpoonerMenu.h"

#include "../Menu/Engine.h"
#include "../Menu/SubmenuRegistry.h"
#include "../Menu/Menu.h"      // Checkbox, Menu::bitController, VirtualKey, INPUT_*
#include "../Menu/Routine.h"   // g_Ped1/2/4, msCurrentPaintIndex
#include "PlayerRuntime.h"     // g_Ped1/2/4
#include "PedComponentRuntime.h"
#include "Neons.h"             // g_fadedRGB
#include "Misc.h"              // dict / dict2 / dict3

#include "../Natives/natives2.h"
#include "../Natives/types.h"

#include "../Scripting/Game.h"
#include "../Scripting/GTAentity.h"
#include "../Scripting/GTAprop.h"
#include "../Scripting/GTAvehicle.h"
#include "../Scripting/GTAped.h"
#include "../Scripting/GTAplayer.h"
#include "../Scripting/GTAblip.h"
#include "../Scripting/Camera.h"
#include "../Scripting/Model.h"
#include "../Scripting/ModelNames.h"
#include "../Scripting/WeaponIndivs.h"
#include "../Scripting/World.h"
#include "../Util/GTAmath.h"
#include "../Util/keyboard.h"
#include "../Util/StringManip.h"
#include "../Util/ExePath.h"
#include "../Memory/GTAmemory.h"

#include "Spooner/SpoonerEntity.h"
#include "Spooner/SpoonerMarker.h"
#include "Spooner/SpoonerMode.h"
#include "Spooner/SpoonerSettings.h"
#include "Spooner/Databases.h"
#include "Spooner/EntityManagement.h"
#include "Spooner/MarkerManagement.h"
#include "Spooner/FavouritesManagement.h"
#include "Spooner/RelationshipManagement.h"
#include "Spooner/TransformGizmo.h"
#include "Spooner/SpoonerShared.h"  // _searchStr / SpoonerVector3ManualPlacementPtrs etc. existing externs

#include "PedComponentRuntime.h"  // PedFavourites::ShowInstructionalButton, g_cam_componentChanger
#include "PedModelRuntime.h"
#include "VehicleRuntime.h"        // g_myVeh
#include "VehicleSpawnerRuntime.h"

#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include <pugixml/src/pugixml.hpp>
#include <dirent/include/dirent.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <string>
#include <tuple>
#include <vector>

namespace {
inline void toLowerInPlace(std::string& s)
{
	std::transform(s.begin(), s.end(), s.begin(),
		[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
}
inline std::string toUpperCopy(const std::string& s)
{
	std::string out(s);
	std::transform(out.begin(), out.end(), out.begin(),
		[](unsigned char c) { return static_cast<char>(std::toupper(c)); });
	return out;
}
} // namespace

using sub::Spooner::selectedEntity;
using sub::Spooner::selectedSpoonGroup;
using sub::Spooner::SelectedMarker;
using sub::Spooner::SpoonerEntity;
using sub::Spooner::SpoonerMarker;
using sub::Spooner::SpoonerMarkerPosition;
using sub::Spooner::eSpoonerModeMode;
using sub::Spooner::spoonerModeModeNames;
using sub::Spooner::EntityManagement::GetEntityIndexInDb;

namespace Menu {

namespace {

std::string& SearchStr() { return dict2; }

unsigned char g_copyEntTexterValue = 0;
unsigned char g_entTypeToShowTexterValue = 0;

float g_manualPlacementPrecision = 0.01f;
float g_fSaveRangeRadius = 5.0f;

std::tuple<GTAentity, Vector3*, Vector3*> g_v3ManualPlacementPtrs{ 0, nullptr, nullptr };

void SetEnt241() { g_Ped1 = selectedEntity.handle.Handle(); }
void SetEnt12()  { g_Ped4 = selectedEntity.handle.Handle(); }

void HandleKeyboardManipulation(Vector3& position, Vector3& rotation)
{
	constexpr float HUD_LINE_HEIGHT = 0.025f;
	const Vector2 HUD_FONT_SIZE(0.35f, 0.35f);
	const float hudX = 0.02f;
	float hudY = 0.8f;

	auto drawText = [&](const std::string& text, RGBA colour = {255, 255, 255, 255})
	{
		Game::Print::SetupDraw(GTAfont::Arial, HUD_FONT_SIZE, false, false, true, colour);
		Game::Print::drawstring(text, hudX, hudY);
		hudY += HUD_LINE_HEIGHT;
	};

	using namespace sub::Spooner::SpoonerMode;

	if (bEntityEditRotationMode)
	{
		drawText("~y~Rotation Mode:");
		drawText("~b~W/S: ~w~Pitch+ / Pitch-");
		drawText("~b~A/D: ~w~Yaw+ / Yaw-");
		drawText("~b~E/Q: ~w~Roll+ / Roll-");
		drawText("~b~=/-: ~w~+/- Sensitivity");
		drawText("~b~R: ~w~Toggle position");
	}
	else
	{
		drawText("~y~Position Mode:");
		drawText("~b~W/S: ~w~X+ / X-");
		drawText("~b~A/D: ~w~Y+ / Y-");
		drawText("~b~E/Q: ~w~Z+ / Z-");
		drawText("~b~=/-: ~w~+/- Sensitivity");
		drawText("~b~R: ~w~Toggle rotation");
	}
	drawText("~b~B: ~w~Switch to gizmo / disable controls.");

	static DWORD lastSensitivityChange = 0;
	if (IsKeyJustUp(VirtualKey::OEMPlus) && GetTickCount() - lastSensitivityChange > 200)
	{
		if (g_manualPlacementPrecision < 10.0f) g_manualPlacementPrecision *= 10;
		lastSensitivityChange = GetTickCount();
	}
	if (IsKeyJustUp(VirtualKey::OEMMinus) && GetTickCount() - lastSensitivityChange > 200)
	{
		if (g_manualPlacementPrecision > 0.0001f) g_manualPlacementPrecision /= 10;
		lastSensitivityChange = GetTickCount();
	}

	auto& target = bEntityEditRotationMode ? rotation : position;
	if (IsKeyDown(VirtualKey::W)) target.x += g_manualPlacementPrecision;
	if (IsKeyDown(VirtualKey::S)) target.x -= g_manualPlacementPrecision;
	if (IsKeyDown(VirtualKey::A)) target.y += g_manualPlacementPrecision;
	if (IsKeyDown(VirtualKey::D)) target.y -= g_manualPlacementPrecision;
	if (IsKeyDown(VirtualKey::E)) target.z += g_manualPlacementPrecision;
	if (IsKeyDown(VirtualKey::Q)) target.z -= g_manualPlacementPrecision;
}

void HandleGizmoManipulation()
{
	constexpr float HUD_LINE_HEIGHT = 0.025f;
	const Vector2 HUD_FONT_SIZE(0.35f, 0.35f);
	const float hudX = 0.02f;
	float hudY = 0.8f;

	auto drawText = [&](const std::string& text, RGBA colour = {255, 255, 255, 255})
	{
		Game::Print::SetupDraw(GTAfont::Arial, HUD_FONT_SIZE, false, false, true, colour);
		Game::Print::drawstring(text, hudX, hudY);
		hudY += HUD_LINE_HEIGHT;
	};

	using namespace sub::Spooner::SpoonerMode;

	if (bEntityEditRotationMode)
		drawText("~y~Gizmo Mode ~s~(Rotation Mode):");
	else
		drawText("~y~Gizmo Mode ~s~(Position Mode):");
	drawText("~b~Left Click:~w~ Grab axis handle");
	drawText("~b~R:~w~ Toggle position/rotation");
	drawText("~b~B:~w~ Disable controls");
	drawText(bGizmoCameraLocked ? "~r~Camera LOCKED ~s~- Mouse drag freely"
		: "~g~Camera UNLOCKED ~s~- Mouse rotates camera");
	drawText("~b~C:~w~ Toggle camera lock");
	drawText(bGizmoLocalSpace ? "~y~Gizmo Axes: LOCAL" : "~y~Gizmo Axes: WORLD");
	drawText("~b~L:~w~ Toggle world/local axes");
}

void HandleGizmoAttachmentManipulation(GTAentity& parentEntity, Vector3& position, Vector3& rotation)
{
	constexpr float HUD_LINE_HEIGHT = 0.025f;
	const Vector2 HUD_FONT_SIZE(0.35f, 0.35f);
	const float hudX = 0.02f;
	float hudY = 0.8f;

	auto drawText = [&](const std::string& text, RGBA colour = {255, 255, 255, 255})
	{
		Game::Print::SetupDraw(GTAfont::Arial, HUD_FONT_SIZE, false, false, true, colour);
		Game::Print::drawstring(text, hudX, hudY);
		hudY += HUD_LINE_HEIGHT;
	};

	using namespace sub::Spooner;

	static TransformGizmo gizmo;
	gizmo.SetMode(SpoonerMode::bEntityEditRotationMode ? GizmoMode::Rotate : GizmoMode::Translate);
	gizmo.ApplyAttachmentMovement(parentEntity, selectedEntity.handle, position, rotation,
		selectedEntity.attachmentArgs.boneIndex);

	if (SpoonerMode::bEntityEditRotationMode)
		drawText("~y~Gizmo Mode ~s~(Rotation Mode):");
	else
		drawText("~y~Gizmo Mode ~s~(Position Mode):");
	drawText("~b~Left Click:~w~ Grab axis handle");
	drawText("~b~R:~w~ Toggle position/rotation");
	drawText("~b~B:~w~ Disable controls");
	drawText(SpoonerMode::bGizmoCameraLocked ? "~r~Camera LOCKED ~s~- Mouse drag freely"
		: "~g~Camera UNLOCKED ~s~- Mouse rotates camera");
	drawText("~b~C:~w~ Toggle camera lock");
	drawText(SpoonerMode::bGizmoLocalSpace ? "~y~Gizmo Axes: LOCAL" : "~y~Gizmo Axes: WORLD");
	drawText("~b~L:~w~ Toggle world/local axes");
}

void HandleEntityEditingLogic(Vector3& position, Vector3& rotation, GTAentity* parentEntity)
{
	using namespace sub::Spooner;

	static bool lastBToggle = false;
	bool currentBToggle = IsKeyJustUp(VirtualKey::B);
	if (currentBToggle && !lastBToggle)
	{
		switch (SpoonerMode::entityEditMode)
		{
		case SpoonerMode::eEntityEditMode::Disabled:
			SpoonerMode::entityEditMode = SpoonerMode::eEntityEditMode::Keyboard;
			SpoonerMode::bGizmoCameraLocked = false;
			break;
		case SpoonerMode::eEntityEditMode::Keyboard:
			SpoonerMode::entityEditMode = SpoonerMode::eEntityEditMode::Gizmo;
			SpoonerMode::bGizmoCameraLocked = false;
			break;
		case SpoonerMode::eEntityEditMode::Gizmo:
			SpoonerMode::entityEditMode = SpoonerMode::eEntityEditMode::Disabled;
			SpoonerMode::bGizmoCameraLocked = false;
			break;
		}
	}
	lastBToggle = currentBToggle;

	static bool lastRToggle = false;
	bool currentRToggle = IsKeyJustUp(VirtualKey::R);
	if (currentRToggle && !lastRToggle)
	{
		SpoonerMode::bEntityEditRotationMode = !SpoonerMode::bEntityEditRotationMode;
	}
	lastRToggle = currentRToggle;

	if (SpoonerMode::entityEditMode == SpoonerMode::eEntityEditMode::Gizmo && IsKeyJustUp(VirtualKey::C))
		SpoonerMode::bGizmoCameraLocked = !SpoonerMode::bGizmoCameraLocked;
	if (SpoonerMode::entityEditMode == SpoonerMode::eEntityEditMode::Gizmo && IsKeyJustUp(VirtualKey::L))
		SpoonerMode::bGizmoLocalSpace = !SpoonerMode::bGizmoLocalSpace;

	constexpr float HUD_LINE_HEIGHT = 0.025f;
	const Vector2 HUD_FONT_SIZE(0.35f, 0.35f);
	const float hudX = 0.02f;
	float hudY = 0.8f;

	auto drawText = [&](const std::string& text, RGBA colour = {255, 255, 255, 255})
	{
		Game::Print::SetupDraw(GTAfont::Arial, HUD_FONT_SIZE, false, false, true, colour);
		Game::Print::drawstring(text, hudX, hudY);
		hudY += HUD_LINE_HEIGHT;
	};

	if (SpoonerMode::entityEditMode == SpoonerMode::eEntityEditMode::Disabled)
	{
		drawText("~r~Entity manipulation DISABLED.");
		drawText("~b~Press B:~w~ Enable keyboard controls or gizmo editing mode.");
		return;
	}

	if (SpoonerMode::entityEditMode == SpoonerMode::eEntityEditMode::Keyboard)
	{
		HandleKeyboardManipulation(position, rotation);
	}
	else if (SpoonerMode::entityEditMode == SpoonerMode::eEntityEditMode::Gizmo)
	{
		if (parentEntity != nullptr && parentEntity->Exists())
			HandleGizmoAttachmentManipulation(*parentEntity, position, rotation);
		else
			HandleGizmoManipulation();
	}
}

bool DrawDeleteEntityShortcut(const std::string& label)
{
	Engine* engine = Engine::Current();
	if (!engine) return false;
	if (Menu::bitController)
	{
		engine->AddInstructionalButton(INPUT_SCRIPT_RLEFT, label, /*isKey=*/false);
		return IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_SCRIPT_RLEFT) != 0;
	}
	engine->AddInstructionalButton(static_cast<int>(VirtualKey::B), label, /*isKey=*/true);
	return IsKeyJustUp(VirtualKey::B);
}

void DrawSpawnPedRow(const std::string& label, const Model& model)
{
	using namespace sub::Spooner;
	(void)label;
	(void)model;
}

bool DrawAddProp(const std::string& label, const Model& model)
{
	using namespace sub::Spooner;
	Engine* engine = Engine::Current();
	const bool pressed = engine ? engine->AddOption(label) : false;
	const bool selected = pressed || (engine && engine->ActiveSelection() == engine->printingOption);
	if (selected)
	{
		SpoonerMode::modelPreviewInfo.entityType = EntityType::PROP;
		SpoonerMode::modelPreviewInfo.model = model;
	}
	if (pressed)
		EntityManagement::AddProp(model, label);
	return pressed;
}
bool DrawAddPed(const std::string& label, const Model& model)
{
	using namespace sub::Spooner;
	Engine* engine = Engine::Current();
	const bool pressed = engine ? engine->AddOption(label) : false;
	const bool selected = pressed || (engine && engine->ActiveSelection() == engine->printingOption);
	if (selected)
	{
		SpoonerMode::modelPreviewInfo.entityType = EntityType::PED;
		SpoonerMode::modelPreviewInfo.model = model;
	}
	if (pressed)
		EntityManagement::AddPed(model, label);
	return pressed;
}

void DrawRelationshipTextScroller()
{
	using namespace sub::Spooner;
	std::vector<std::string> relationshipStringVec{ "NONE" };
	for (UINT8 i = 0; i < RelationshipManagement::relationshipGroups.size(); i++)
	{
		if (GTAped(selectedEntity.handle).GetRelationshipGroup() ==
			GET_HASH_KEY(RelationshipManagement::relationshipGroups[i]))
		{
			relationshipStringVec[0] = RelationshipManagement::relationshipGroups[i];
		}
	}

	Engine* engine = Engine::Current();
	const ::Menu::InputResult res = engine
		? engine->AddTextList("Relationship", 0, relationshipStringVec)
		: ::Menu::InputResult{};

	if (res.rightPressed)
	{
		Hash currHash;
		if (RelationshipManagement::GetPedRelationshipGroup(selectedEntity.handle, currHash))
		{
			for (INT8 i = 0; i < RelationshipManagement::relationshipGroups.size(); i++)
			{
				if (GTAped(selectedEntity.handle).GetRelationshipGroup() ==
					GET_HASH_KEY(RelationshipManagement::relationshipGroups[i]))
				{
					i++;
					if (i >= RelationshipManagement::relationshipGroups.size()) break;
					RelationshipManagement::SetPedRelationshipGroup(selectedEntity.handle,
						GET_HASH_KEY(RelationshipManagement::relationshipGroups[i]));
					break;
				}
			}
		}
		else
		{
			RelationshipManagement::SetPedRelationshipGroup(selectedEntity.handle,
				GET_HASH_KEY(RelationshipManagement::relationshipGroups[0]));
		}
	}
	else if (res.leftPressed)
	{
		for (INT8 i = 0; i < RelationshipManagement::relationshipGroups.size(); i++)
		{
			if (GTAped(selectedEntity.handle).GetRelationshipGroup() ==
				GET_HASH_KEY(RelationshipManagement::relationshipGroups[i]))
			{
				i--;
				if (i < 0) break;
				RelationshipManagement::SetPedRelationshipGroup(selectedEntity.handle,
					GET_HASH_KEY(RelationshipManagement::relationshipGroups[i]));
				break;
			}
		}
	}
}

} // namespace

void SpoonerSubmenu::Draw()
{
	using namespace sub::Spooner;

	SpoonerMode::entityEditMode = SpoonerMode::eEntityEditMode::Disabled;
	selectedEntity.handle = 0;
	SearchStr().clear();
	dict3.clear();

	DrawTitle();

	if (DrawToggleExternal("Spooner Mode", SpoonerMode::bEnabled))
		SpoonerMode::Toggle();
	if (DrawOption("Spawn Entity Into World"))    NavigateTo("spooner_spawn_categories");
	if (DrawOption("Manage Entity Database"))     NavigateTo("spooner_manage_db");
	if (DrawOption("Manage Markers"))             NavigateTo("spooner_manage_markers");
	if (DrawOption("Manage Saved Files"))         NavigateTo("spooner_save_files");
	if (DrawOption("Quick Manual Placement (Legacy)")) NavigateTo("spooner_quick_manual_placement");
	if (DrawOption("Edit Multiple Entities Simultaneously")) NavigateTo("spooner_group_spoon");
	if (DrawOption("Settings"))                   NavigateTo("spooner_settings");
}

void SpoonerSettingsSubmenu::Draw()
{
	using namespace sub::Spooner;

	DrawTitle();

	DrawToggle("Display Model Previews (Spooner Mode)", Settings::bShowModelPreviews);
	DrawToggle("Display Spooner Info", Settings::bDisplaySpoonerInfo);
	DrawToggle("Display Entity Surrounding Box", Settings::bShowBoxAroundSelectedEntity);
	DrawToggle("Spawn Dynamic Objects", Settings::bSpawnDynamicProps);
	DrawToggle("Spawn Dynamic Peds", Settings::bSpawnDynamicPeds);
	DrawToggle("Spawn Dynamic Vehicles", Settings::bSpawnDynamicVehicles);
	DrawToggle("Freeze Entity When Moving It (Spooner Mode)", Settings::bFreezeEntityWhenMovingIt);
	DrawToggle("Spawn Invincible Entities", Settings::bSpawnInvincibleEntities);
	DrawToggle("Spawn Still Peds (Block Fleeing)", Settings::bSpawnStillPeds);
	DrawToggle("Make Added (To DB) Entities Persistent", Settings::bAddToDbAsMissionEntities);
	DrawToggle("Teleport To Reference When Loading File", Settings::bTeleportToReferenceWhenLoadingFile);

	{
		int idx = static_cast<int>(Settings::spoonerModeMode);
		if (DrawTextList("Spooner Mode Method", idx, spoonerModeModeNames))
			Settings::spoonerModeMode = static_cast<eSpoonerModeMode>(idx);
	}

	auto numberField = [&](const std::string& label, float& val)
	{
		if (DrawNumber(label, val, 0.0005f, 4, 0.0f, FLT_MAX))
			return;
		if (IsCurrentRowSelected() && MenuPressTimer::IsButtonTapped(MenuPressTimer::Button::Accept))
		{
			std::string inputStr = Game::InputBox("", 11U, "", std::to_string(val).substr(0, 10));
			if (inputStr.length() > 0)
			{
				try { val = std::stof(inputStr); }
				catch (...) { Game::Print::PrintErrorInvalidInput(inputStr); }
			}
		}
	};

	if (Menu::bitController)
	{
		numberField("Movement Sensitivity (Gamepad)", Settings::cameraMovementSensitivityGamepad);
		numberField("Rotation Sensitivity (Gamepad)", Settings::cameraRotationSensitivityGamepad);
	}
	else
	{
		numberField("Movement Sensitivity (Keyboard)", Settings::cameraMovementSensitivityKeyboard);
		numberField("Rotation Sensitivity (Mouse)", Settings::cameraRotationSensitivityMouse);
	}

	if (DrawOption("Reload Model List Files"))
		PopulateGlobalEntityModelsArrays();
}

void SpoonerManageDbSubmenu::Draw()
{
	using namespace sub::Spooner;
	using namespace sub::Spooner::EntityManagement;

	selectedEntity.handle = 0;

	SpoonerEntity* p_entityToDelete = nullptr;
	GTAentity myPed = PLAYER_PED_ID();
	auto myIndexInDb = GetEntityIndexInDb(myPed);
	bool bIsSelfInDb = myIndexInDb >= 0;

	DrawTitle();

	if (DrawOption("Removal")) NavigateTo("spooner_manage_db_removal");

	if (DrawOption(std::string("Self (") + (bIsSelfInDb ? "Is In Database)" : "Is Not In Database)")))
	{
		if (bIsSelfInDb)
		{
			selectedEntity = Databases::EntityDb[myIndexInDb];
		}
		else
		{
			selectedEntity = SpoonerEntity();
			selectedEntity.dynamic = true;
			selectedEntity.handle = myPed;
			selectedEntity.hashName = GetPedModelLabel(myPed.Model(), true);
			if (selectedEntity.hashName.length() == 0)
				selectedEntity.hashName = "Player (Unk Model)";
			selectedEntity.isStill = true;
			selectedEntity.type = EntityType::PED;
		}
		NavigateTo("spooner_selected_entity_ops");
		return;
	}

	if (!Databases::EntityDb.empty())
	{
		DrawBreak("---Database---");

		{
			int idx = g_entTypeToShowTexterValue;
			if (DrawTextList("Show", idx,
				std::vector<std::string>{"All", "Peds", "Vehicles", "Objects"}))
			{
				g_entTypeToShowTexterValue = static_cast<unsigned char>(idx);
			}
		}

		for (UINT i = 0; i < Databases::EntityDb.size(); i++)
		{
			auto& e = Databases::EntityDb[i];
			if (g_entTypeToShowTexterValue != 0 &&
				static_cast<UINT8>(e.type) != g_entTypeToShowTexterValue)
				continue;

			const bool bEntityExists = e.handle.Exists();
			const bool isHighlighted = IsCurrentRowSelected();
			const std::string strEntTypeConcat = isHighlighted
				? "  ~bold~[" + e.TypeName() + "]~bold~"
				: std::string();

			if (DrawOption(e.hashName + (bEntityExists ? "" : " (Invalid)") + strEntTypeConcat))
			{
				if (bEntityExists)
				{
					selectedEntity = e;
					NavigateTo("spooner_selected_entity_ops");
					return;
				}
			}

			if (IsCurrentRowSelected())
			{
				ShowArrowAboveEntity(e.handle);
				if (DrawDeleteEntityShortcut(bEntityExists
					? "Delete Entity"
					: "Remove Invalid Entity From DB"))
				{
					p_entityToDelete = &e;
				}
			}
		}
	}

	if (p_entityToDelete != nullptr)
	{
		auto e = *p_entityToDelete;
		if (e.handle.Exists())
		{
			e.handle.RequestControl(600);
			DeleteEntity(e);
		}
		else
		{
			RemoveEntityFromDb(e);
		}
	}
}

void SpoonerManageDbRemovalSubmenu::Draw()
{
	using namespace sub::Spooner;
	using namespace sub::Spooner::EntityManagement;
	using sub::Spooner::MarkerManagement::RemoveAllMarkers;

	DrawTitle();

	DrawBreak("---Database---");
	if (DrawOption("Delete All Markers (" + std::to_string(Databases::MarkerDb.size()) + ")"))
		RemoveAllMarkers();
	if (DrawOption("Delete All Entities In Database (" + std::to_string(Databases::EntityDb.size()) + ")"))
		DeleteAllEntitiesInDb();
	if (DrawOption("Delete All Objects In Database"))     DeleteAllPropsInDb();
	if (DrawOption("Delete All Peds In Database"))        DeleteAllPedsInDb();
	if (DrawOption("Delete All Vehicles In Database"))    DeleteAllVehiclesInDb();
	if (DrawOption("Delete All Invalid Entities In Database")) DeleteInvalidEntitiesInDb();

	DrawBreak("---World---");
	if (DrawOption("Delete All Entities In World (" + std::to_string(worldEntities.size()) + ")"))
		DeleteAllEntitiesInWorld();
	if (DrawOption("Delete All Objects In World (" + std::to_string(worldObjects.size()) + ")"))
		DeleteAllPropsInWorld();
	if (DrawOption("Delete All Peds In World (" + std::to_string(worldPeds.size()) + ")"))
		DeleteAllPedsInWorld();
	if (DrawOption("Delete All Vehicles In World (" + std::to_string(worldVehicles.size()) + ")"))
		DeleteAllVehiclesInWorld();

	if (DrawOption("Clear Entity Database (And Keep Entities)"))
		ClearDb();
}

void SpoonerSelectedEntityOpsSubmenu::Draw()
{
	using namespace sub::Spooner;
	using namespace sub::Spooner::EntityManagement;

	SpoonerMode::entityEditMode = SpoonerMode::eEntityEditMode::Disabled;
	if (!selectedEntity.handle.Exists())
	{
		Engine* e = Engine::Current();
		if (e) e->GoBack();
		return;
	}
	selectedEntity.handle.RequestControlOnce();
	auto thisEntityIndexInDb = GetEntityIndexInDb(selectedEntity);
	bool isThisEntityInDb = thisEntityIndexInDb >= 0;
	if (isThisEntityInDb)
	{
		selectedEntity.taskSequence = Databases::EntityDb[thisEntityIndexInDb].taskSequence;
		Databases::EntityDb[thisEntityIndexInDb] = selectedEntity;
	}
	Model selectedEntityModel = selectedEntity.handle.Model();

	{
		Engine* e = Engine::Current();
		if (e) e->AddTitle(selectedEntity.hashName);
	}

	Engine* engine = Engine::Current();

	switch (selectedEntity.type)
	{
	case EntityType::PROP:
	{
		const bool bIsAFav = FavouritesManagement::IsPropAFavourite(selectedEntity.hashName, selectedEntityModel.hash);
		const bool pressed = engine
			? engine->AddCheckbox("Model Is A Favourite", bIsAFav, Checkbox::BOXTICK, Checkbox::BOXBLANK)
			: false;
		if (pressed)
		{
			if (!bIsAFav)
				FavouritesManagement::AddPropToFavourites(selectedEntity.hashName, selectedEntityModel.hash);
			else
				FavouritesManagement::RemovePropFromFavourites(selectedEntity.hashName, selectedEntityModel.hash);
		}
		break;
	}
	case EntityType::VEHICLE:
	{
		const bool bIsAFav = FavouritesManagement::IsVehicleAFavourite(selectedEntityModel);
		const bool pressed = engine
			? engine->AddCheckbox("Model Is A Favourite", bIsAFav, Checkbox::BOXTICK, Checkbox::BOXBLANK)
			: false;
		if (pressed)
		{
			if (!bIsAFav)
			{
				std::string inputStr = Game::InputBox("", 28U, "Enter custom name:",
					selectedEntityModel.VehicleDisplayName(true));
				if (inputStr.length() > 0)
					FavouritesManagement::AddVehicleToFavourites(selectedEntityModel, inputStr);
			}
			else
				FavouritesManagement::RemoveVehicleFromFavourites(selectedEntityModel);
		}
		break;
	}
	default:
		break;
	}

	{
		const bool pressed = engine
			? engine->AddCheckbox("Entity Is In Database", isThisEntityInDb,
				Checkbox::BOXTICK, Checkbox::BOXBLANK)
			: false;
		if (pressed)
		{
			if (!isThisEntityInDb)
			{
				AddEntityToDb(selectedEntity);
				Game::Print::PrintBottomLeft(selectedEntity.hashName + " added to database.");
			}
			else
			{
				RemoveEntityFromDb(selectedEntity);
				Game::Print::PrintBottomLeft(selectedEntity.hashName +
					" removed from database. Properties will no longer be stored in Spooner's memory.");
			}
		}
	}

	{
		int idx = g_copyEntTexterValue;
		const ::Menu::InputResult res = engine
			? engine->AddTextList("Copy", idx,
				std::vector<std::string>{ "This Entity Only", "With Attachments" })
			: ::Menu::InputResult{};
		if (res.rightPressed) { if (g_copyEntTexterValue < 1U) g_copyEntTexterValue++; }
		if (res.leftPressed)  { if (g_copyEntTexterValue > 0)  g_copyEntTexterValue--; }
		if (res.accepted)
		{
			const SpoonerEntity& copiedEntity = CopyEntity(selectedEntity, isThisEntityInDb,
				true, g_copyEntTexterValue);
			selectedEntity = copiedEntity;
		}
	}

	if (DrawOption("Delete"))
	{
		selectedEntity.handle.RequestControl(600);
		DeleteEntity(selectedEntity);
		Engine* e = Engine::Current();
		if (e) e->GoBack();
		return;
	}

	if (DrawToggleExternal("Dynamic", selectedEntity.dynamic))
	{
		selectedEntity.dynamic = !selectedEntity.dynamic;
		selectedEntity.handle.SetDynamic(selectedEntity.dynamic);
		selectedEntity.handle.FreezePosition(!selectedEntity.dynamic);
	}

	if (DrawToggleExternal("Frozen In Place", selectedEntity.handle.IsPositionFrozen()))
		selectedEntity.handle.FreezePosition(!selectedEntity.handle.IsPositionFrozen());

	if (selectedEntity.type == EntityType::VEHICLE && selectedEntity.handle.GetLandingGearState() != -1)
	{
		bool bLandingGearDown = (selectedEntity.handle.GetLandingGearState() == 0);
		if (DrawToggle("Landing Gear", bLandingGearDown))
			selectedEntity.handle.SetLandingGear(bLandingGearDown);
	}

	if (selectedEntity.type == EntityType::PROP)
	{
		auto& thisTextureVariation = selectedEntity.textureVariation;
		int tvInt = thisTextureVariation;
		if (DrawNumber("Texture Variation", tvInt, 1, 0, UINT8_MAX))
		{
			thisTextureVariation = static_cast<UINT8>(tvInt);
			SET_OBJECT_TINT_INDEX(selectedEntity.handle.Handle(), thisTextureVariation);
		}
	}

	{
		int thisHealth = selectedEntity.handle.GetHealth();
		if (DrawNumber("Health", thisHealth, 1, 0, INT_MAX))
		{
			if (selectedEntity.handle.GetMaxHealth() < thisHealth)
				selectedEntity.handle.SetMaxHealth(thisHealth);
			else if (selectedEntity.handle.GetMaxHealth() > thisHealth + 30)
				selectedEntity.handle.SetMaxHealth(thisHealth + 30);
			selectedEntity.handle.SetHealth(thisHealth);
		}
		if (IsCurrentRowSelected() && MenuPressTimer::IsButtonTapped(MenuPressTimer::Button::Accept))
		{
			GTAentity& ent = selectedEntity.handle;
			std::string inputStr = Game::InputBox("", 5U, "", std::to_string(thisHealth));
			if (inputStr.length() > 0)
			{
				try
				{
					thisHealth = std::stoi(inputStr);
					if (ent.GetMaxHealth() < thisHealth) ent.SetMaxHealth(thisHealth);
					ent.SetHealth(thisHealth);
				}
				catch (...) { Game::Print::PrintErrorInvalidInput(inputStr); }
			}
		}
	}

	if (DrawToggleExternal("Invincible", selectedEntity.handle.IsInvincible()))
	{
		bool bEnable = !selectedEntity.handle.IsInvincible();
		selectedEntity.handle.SetInvincible(bEnable);
		selectedEntity.handle.SetExplosionProof(bEnable);
		selectedEntity.handle.SetMeleeProof(bEnable);
	}
	if (DrawToggleExternal("Fireproof", selectedEntity.handle.IsFireProof()))
		selectedEntity.handle.SetFireProof(!selectedEntity.handle.IsFireProof());
	if (DrawToggleExternal("Is On Fire (For Peds)", selectedEntity.handle.IsOnFire()))
		selectedEntity.handle.SetOnFire(!selectedEntity.handle.IsOnFire());
	if (DrawToggleExternal("Visible", selectedEntity.handle.IsVisible()))
		selectedEntity.handle.SetVisible(!selectedEntity.handle.IsVisible());

	if (DrawToggleExternal("Collision", selectedEntity.handle.GetIsCollisionEnabled()))
	{
		GTAentity attBaseEnt;
		GetEntityThisEntityIsAttachedTo(selectedEntity.handle, attBaseEnt);
		selectedEntity.handle.SetIsCollisionEnabled(!selectedEntity.handle.GetIsCollisionEnabled());
		if (selectedEntity.attachmentArgs.isAttached)
		{
			AttachEntity(selectedEntity, attBaseEnt, selectedEntity.attachmentArgs.boneIndex,
				selectedEntity.attachmentArgs.offset, selectedEntity.attachmentArgs.rotation);
		}
	}

	{
		const bool bHasGravity = selectedEntity.handle.GetHasGravity();
		if (DrawToggleExternal("Gravity", bHasGravity))
		{
			switch (selectedEntity.type)
			{
			case EntityType::PED:     GTAped(selectedEntity.handle).SetHasGravity(!bHasGravity); break;
			case EntityType::VEHICLE: GTAvehicle(selectedEntity.handle).SetHasGravity(!bHasGravity); break;
			case EntityType::PROP:    GTAprop(selectedEntity.handle).SetHasGravity(!bHasGravity); break;
			default:                  selectedEntity.handle.SetHasGravity(!bHasGravity); break;
			}
		}
	}

	if (DrawOption("Go To Entity"))
	{
		if (SpoonerMode::spoonerModeCamera.IsActive())
		{
			auto& cam = SpoonerMode::spoonerModeCamera;
			cam.SetPosition(selectedEntity.handle.GetOffsetInWorldCoords(
				0, -5.0f - selectedEntity.handle.Dim2().y, 0));
		}
		else
		{
			GTAped myPed = PLAYER_PED_ID();
			myPed.SetPosition(selectedEntity.handle.GetPosition());
		}
	}

	if (DrawOption(std::string("Bring Entity To Self") +
		(selectedEntity.handle.IsAttached() ? " (And Detach)" : "")))
	{
		if (selectedEntity.handle.IsAttached())
			DetachEntity(selectedEntity);

		if (SpoonerMode::spoonerModeCamera.IsActive())
		{
			auto& cam = SpoonerMode::spoonerModeCamera;
			selectedEntity.handle.SetPosition(cam.GetOffsetInWorldCoords(
				0, 5.0f + selectedEntity.handle.Dim2().y, 0));
		}
		else
		{
			GTAped myPed = PLAYER_PED_ID();
			selectedEntity.handle.SetPosition(myPed.GetPosition() +
				(myPed.ForwardVector() * (selectedEntity.handle.Dim1().y + 4.0f)));
		}
	}

	if (DrawOption("Place On Ground"))
		selectedEntity.handle.PlaceOnGround();

	if (DrawOption("Reset rotation"))
		selectedEntity.handle.SetRotation(Vector3::Zero());

	if (DrawOption("TriggerFX"))
	{
		SetEnt241();
		NavigateTo("ptfx");
	}

	{
		const int idxNow = selectedEntity.taskSequence.IsActive() ? 1 : 0;
		int idx = idxNow;
		const ::Menu::InputResult res = Engine::Current()
			? Engine::Current()->AddTextList("Task Sequence", idx,
				std::vector<std::string>{ "Inactive", "Active" })
			: ::Menu::InputResult{};
		if (res.accepted)
			NavigateTo("spooner_tasksequence_task_list");
	}

	if (selectedEntity.type == EntityType::PED)
	{
		if (DrawOption("Ped Options")) NavigateTo("spooner_ped_ops");
	}
	else if (selectedEntity.type == EntityType::VEHICLE)
	{
		if (DrawOption("Menyoo Customs"))
		{
			SetEnt12();
			NavigateTo("vehicle_modshop");
		}
	}

	if (DrawOption("Attachment Options"))   NavigateTo("spooner_attachment_ops");
	if (DrawOption("Manual Placement"))     NavigateTo("spooner_manual_placement");
	if (DrawOption("Manual Resize (beta)")) NavigateTo("spooner_size_manipulation");
}

// ============================================================================
// SpoonerAttachmentOpsSubmenu (Sub_AttachmentOps)
// ============================================================================

void SpoonerAttachmentOpsSubmenu::Draw()
{
	using namespace sub::Spooner;
	using namespace sub::Spooner::EntityManagement;

	if (!selectedEntity.handle.Exists())
	{
		Engine* e = Engine::Current(); if (e) e->GoBack(); return;
	}
	selectedEntity.handle.RequestControlOnce();
	auto thisEntityIndexInDb = GetEntityIndexInDb(selectedEntity);
	bool isThisEntityInDb = thisEntityIndexInDb >= 0;
	if (isThisEntityInDb)
	{
		selectedEntity.taskSequence = Databases::EntityDb[thisEntityIndexInDb].taskSequence;
		Databases::EntityDb[thisEntityIndexInDb] = selectedEntity;
	}

	GTAentity parentEntity;
	bool seIsAttached = GetEntityThisEntityIsAttachedTo(selectedEntity.handle, parentEntity);
	EntityType parentEntityType = (EntityType)parentEntity.Type();

	DrawTitle();

	if (!seIsAttached)
	{
		if (DrawOption("Attach To Something")) NavigateTo("spooner_attachment_ops_attach_to");
	}
	else
	{
		if (DrawOption("Detach")) DetachEntity(selectedEntity);
	}

	if (DrawNumber("Scroll Sensitivity", g_manualPlacementPrecision, 0.0001f, 4))
	{

	}
	{
		Engine* engine = Engine::Current();
		(void)engine; // engine row already emitted by DrawNumber above.
	}

	if (seIsAttached)
	{
		int nextBoneIndex = selectedEntity.attachmentArgs.boneIndex;
		Vector3 nextOffset = selectedEntity.attachmentArgs.offset;
		Vector3 nextRot    = selectedEntity.attachmentArgs.rotation;

		HandleEntityEditingLogic(nextOffset, nextRot, &parentEntity);

		if (parentEntityType == EntityType::PED)
		{
			int obj_currentPedBoneArrayIndex = 17; // SKEL_ROOT default
			for (int i = 0; i < (int)Bone::vBoneNames.size(); i++)
			{
				if (nextBoneIndex == GTAped(parentEntity).GetBoneIndex(Bone::vBoneNames[i].boneid))
				{
					obj_currentPedBoneArrayIndex = i;
					break;
				}
			}
			std::vector<std::string> vCurrentPedBoneNameStr{ Bone::vBoneNames[obj_currentPedBoneArrayIndex].name };

			int boneTextIdx = 0;
			Engine* engine = Engine::Current();
			const ::Menu::InputResult res = engine
				? engine->AddTextList("Bone", boneTextIdx, vCurrentPedBoneNameStr)
				: ::Menu::InputResult{};
			if (res.rightPressed && obj_currentPedBoneArrayIndex < (int)Bone::vBoneNames.size() - 1)
			{
				obj_currentPedBoneArrayIndex++;
				nextBoneIndex = GTAped(parentEntity).GetBoneIndex(
					Bone::vBoneNames[obj_currentPedBoneArrayIndex].boneid);
			}
			if (res.leftPressed && obj_currentPedBoneArrayIndex > 0)
			{
				obj_currentPedBoneArrayIndex--;
				nextBoneIndex = GTAped(parentEntity).GetBoneIndex(
					Bone::vBoneNames[obj_currentPedBoneArrayIndex].boneid);
			}
			if (res.accepted)
				NavigateTo("spooner_attachment_ops_select_bone");
		}
		else if (parentEntityType == EntityType::VEHICLE)
		{
			int obj_currentVehBoneArrayIndex = 10; // bodyshell default
			for (int i = 0; i < (int)VBone::vNames.size(); i++)
			{
				if (nextBoneIndex == GTAvehicle(parentEntity).GetBoneIndex(VBone::vNames[i]))
				{
					obj_currentVehBoneArrayIndex = i;
					break;
				}
			}
			std::vector<std::string> vCurrentVehBoneNameStr{ VBone::vNames[obj_currentVehBoneArrayIndex] };

			int boneTextIdx = 0;
			Engine* engine = Engine::Current();
			const ::Menu::InputResult res = engine
				? engine->AddTextList("Bone", boneTextIdx, vCurrentVehBoneNameStr)
				: ::Menu::InputResult{};
			if (res.rightPressed && obj_currentVehBoneArrayIndex < (int)VBone::vNames.size() - 1)
			{
				obj_currentVehBoneArrayIndex++;
				nextBoneIndex = GTAvehicle(parentEntity).GetBoneIndex(
					VBone::vNames[obj_currentVehBoneArrayIndex]);
			}
			if (res.leftPressed && obj_currentVehBoneArrayIndex > 0)
			{
				obj_currentVehBoneArrayIndex--;
				nextBoneIndex = GTAvehicle(parentEntity).GetBoneIndex(
					VBone::vNames[obj_currentVehBoneArrayIndex]);
			}
			if (res.accepted)
				NavigateTo("spooner_attachment_ops_select_bone");
		}

		// Numeric offset / rotation rows: each step uses g_manualPlacementPrecision.
		auto numField = [&](const std::string& label, float& val)
		{
			Engine* engine = Engine::Current();
			const ::Menu::InputResult res = engine
				? engine->AddNumber(label, val, 4)
				: ::Menu::InputResult{};
			if (res.rightPressed) val += g_manualPlacementPrecision;
			if (res.leftPressed)  val -= g_manualPlacementPrecision;
		};
		numField("X", nextOffset.x);
		numField("Y", nextOffset.y);
		numField("Z", nextOffset.z);
		numField("Pitch", nextRot.x);
		numField("Roll", nextRot.y);
		numField("Yaw", nextRot.z);

		WrapAngle(nextRot.x);
		WrapAngle(nextRot.y);
		WrapAngle(nextRot.z);

		if (nextOffset != selectedEntity.attachmentArgs.offset ||
			nextRot != selectedEntity.attachmentArgs.rotation ||
			nextBoneIndex != selectedEntity.attachmentArgs.boneIndex)
		{
			AttachEntity(selectedEntity, parentEntity, nextBoneIndex, nextOffset, nextRot);
		}
	}
}

void SpoonerAttachmentOpsAttachToSubmenu::Draw()
{
	using namespace sub::Spooner;
	using namespace sub::Spooner::EntityManagement;

	if (!selectedEntity.handle.Exists())
	{
		Engine* e = Engine::Current(); if (e) e->GoBack(); return;
	}
	GTAped myPed = PLAYER_PED_ID();
	GTAvehicle myVehicle = g_myVeh;

	DrawTitle();

	{
		Engine* engine = Engine::Current();
		const bool pressed = engine
			? engine->AddCheckbox("Keep World Position When Attaching",
				Settings::bKeepPositionWhenAttaching, Checkbox::BOXTICK, Checkbox::BOXBLANK)
			: false;
		if (pressed) Settings::bKeepPositionWhenAttaching = !Settings::bKeepPositionWhenAttaching;
	}

	DrawBreak("---Available Entities---");

	if (DrawOption("Self"))
	{
		AttachEntityInit(selectedEntity, myPed, Settings::bKeepPositionWhenAttaching);
		Engine* e = Engine::Current(); if (e) e->GoBack();
		return;
	}

	if (myVehicle.Exists())
	{
		if (DrawOption(std::string(myPed.IsInVehicle() ? "Current" : "Last Seated") + " Vehicle"))
		{
			AttachEntityInit(selectedEntity, myVehicle, Settings::bKeepPositionWhenAttaching);
			Engine* e = Engine::Current(); if (e) e->GoBack();
			return;
		}
	}

	if (!Databases::EntityDb.empty())
	{
		if (Databases::EntityDb.size() > 1 ||
			Databases::EntityDb.front().handle != selectedEntity.handle)
		{
			DrawBreak("---Database---");
			for (auto& e : Databases::EntityDb)
			{
				if (e.handle == selectedEntity.handle) continue;
				if (!e.handle.Exists())
				{
					DrawOption(e.hashName + " (Invalid)");
					continue;
				}
				if (!selectedEntity.handle.IsAttachedTo(e.handle))
				{
					if (DrawOption(e.hashName))
					{
						AttachEntityInit(selectedEntity, e.handle, Settings::bKeepPositionWhenAttaching);
						Engine* eng = Engine::Current(); if (eng) eng->GoBack();
						return;
					}
					if (IsCurrentRowSelected())
						ShowArrowAboveEntity(e.handle, RGBA(0, 255, 0, 200));
				}
				else
				{
					DrawOption(e.hashName + " (already attached)");
					if (IsCurrentRowSelected())
						ShowArrowAboveEntity(e.handle, RGBA(255, 0, 0, 200));
				}
			}
		}
	}
}

void SpoonerAttachmentOpsSelectBoneSubmenu::Draw()
{
	using namespace sub::Spooner;
	using namespace sub::Spooner::EntityManagement;

	if (!selectedEntity.handle.Exists())
	{
		Engine* e = Engine::Current(); if (e) e->GoBack(); return;
	}
	selectedEntity.handle.RequestControlOnce();
	auto thisEntityIndexInDb = GetEntityIndexInDb(selectedEntity);
	bool isThisEntityInDb = thisEntityIndexInDb >= 0;
	if (isThisEntityInDb)
	{
		selectedEntity.taskSequence = Databases::EntityDb[thisEntityIndexInDb].taskSequence;
		Databases::EntityDb[thisEntityIndexInDb] = selectedEntity;
	}

	GTAentity baseEntity;
	bool seIsAttached = GetEntityThisEntityIsAttachedTo(selectedEntity.handle, baseEntity);
	EntityType parentEntityType = (EntityType)baseEntity.Type();
	if (!baseEntity.Exists())
	{
		Engine* e = Engine::Current(); if (e) e->GoBack(); return;
	}

	int nextBoneIndex = selectedEntity.attachmentArgs.boneIndex;
	Vector3 nextOffset = selectedEntity.attachmentArgs.offset;
	Vector3 nextRot    = selectedEntity.attachmentArgs.rotation;

	bool bSelectedBoneIndexExists = false;

	DrawTitle();

	if (parentEntityType == EntityType::PED)
	{
		GTAped baseEntityPed = baseEntity;
		bSelectedBoneIndexExists = nextBoneIndex == baseEntityPed.GetBoneIndex(Bone::vBoneNames.front().boneid);
		for (auto& b : Bone::vBoneNames)
		{
			if (bSelectedBoneIndexExists && nextBoneIndex == baseEntityPed.GetBoneIndex(b.boneid))
				continue;
			bSelectedBoneIndexExists = (nextBoneIndex == baseEntityPed.GetBoneIndex(b.boneid));
			if (DrawSelectionItem(b.name, bSelectedBoneIndexExists))
			{
				nextBoneIndex = baseEntityPed.GetBoneIndex(b.boneid);
				break;
			}
		}
	}
	else if (parentEntityType == EntityType::VEHICLE)
	{
		GTAvehicle baseEntityVeh = baseEntity;
		bSelectedBoneIndexExists = nextBoneIndex == baseEntityVeh.GetBoneIndex(VBone::vNames.front());
		for (auto& b : VBone::vNames)
		{
			if (!baseEntityVeh.HasBone(b) ||
				(bSelectedBoneIndexExists && nextBoneIndex == baseEntityVeh.GetBoneIndex(b)))
				continue;
			bSelectedBoneIndexExists = (nextBoneIndex == baseEntityVeh.GetBoneIndex(b));
			if (DrawSelectionItem(b, bSelectedBoneIndexExists))
			{
				nextBoneIndex = baseEntityVeh.GetBoneIndex(b);
				break;
			}
		}
	}

	if (nextBoneIndex != selectedEntity.attachmentArgs.boneIndex)
		AttachEntity(selectedEntity, baseEntity, nextBoneIndex, nextOffset, nextRot);
}

void SpoonerManualPlacementSubmenu::Draw()
{
	using namespace sub::Spooner;
	using namespace sub::Spooner::EntityManagement;

	if (!selectedEntity.handle.Exists())
	{
		Engine* e = Engine::Current(); if (e) e->GoBack(); return;
	}
	selectedEntity.handle.RequestControlOnce();

	Vector3 currPos = selectedEntity.handle.GetPosition();
	Vector3 currRot = selectedEntity.handle.Rotation_get();
	Vector3 nextPos = currPos;
	Vector3 nextRot = currRot;

	DrawTitle();

	// Scroll sensitivity: precision *10 / /10 per step.
	{
		Engine* engine = Engine::Current();
		const ::Menu::InputResult res = engine
			? engine->AddNumber("Scroll Sensitivity", g_manualPlacementPrecision, 4)
			: ::Menu::InputResult{};
		if (res.rightPressed) { if (g_manualPlacementPrecision < 10.0f) g_manualPlacementPrecision *= 10; }
		if (res.leftPressed)  { if (g_manualPlacementPrecision > 0.0001f) g_manualPlacementPrecision /= 10; }
	}

	auto numField = [&](const std::string& label, float& current, float& next)
	{
		Engine* engine = Engine::Current();
		const ::Menu::InputResult res = engine
			? engine->AddNumber(label, current, 4)
			: ::Menu::InputResult{};
		if (res.rightPressed) next += g_manualPlacementPrecision;
		if (res.leftPressed)  next -= g_manualPlacementPrecision;
	};
	numField("X", currPos.x, nextPos.x);
	numField("Y", currPos.y, nextPos.y);
	numField("Z", currPos.z, nextPos.z);
	numField("Pitch", currRot.x, nextRot.x);
	numField("Roll",  currRot.y, nextRot.y);
	numField("Yaw",   currRot.z, nextRot.z);

	HandleEntityEditingLogic(nextPos, nextRot, nullptr);

	WrapAngle(nextRot.x);
	WrapAngle(nextRot.y);
	WrapAngle(nextRot.z);

	if (nextPos != currPos)
	{
		selectedEntity.handle.SetPosition(nextPos);
		currPos = selectedEntity.handle.GetPosition();
		GTAentity attBase;
		if (GetEntityThisEntityIsAttachedTo(selectedEntity.handle, attBase))
			World::DrawLine(attBase.GetPosition(), currPos, RGBA::AllWhite());
	}
	if (nextRot != currRot)
	{
		selectedEntity.handle.SetRotation(nextRot);
		currRot = selectedEntity.handle.Rotation_get();
		GTAentity attBase;
		if (GetEntityThisEntityIsAttachedTo(selectedEntity.handle, attBase))
			World::DrawLine(attBase.GetPosition(), currPos, RGBA::AllWhite());
	}
}

void SpoonerSizeManipulationSubmenu::Draw()
{
	using namespace sub::Spooner;

	if (!selectedEntity.handle.Exists())
	{
		Engine* e = Engine::Current(); if (e) e->GoBack(); return;
	}
	selectedEntity.handle.RequestControlOnce();

	DrawTitle();

	UINT64 ptr = GTAmemory::_entityAddressFunc(selectedEntity.handle.Handle());
	if (!ptr) return;

	float length = GTAmemory::ReadFloat(ptr + 0x60);
	float width  = GTAmemory::ReadFloat(ptr + 0x74);
	float height = GTAmemory::ReadFloat(ptr + 0x88);

	{
		Engine* engine = Engine::Current();
		const ::Menu::InputResult res = engine
			? engine->AddNumber("Scroll Sensitivity", g_manualPlacementPrecision, 4)
			: ::Menu::InputResult{};
		if (res.rightPressed) { if (g_manualPlacementPrecision < 10.0f) g_manualPlacementPrecision *= 10; }
		if (res.leftPressed)  { if (g_manualPlacementPrecision > 0.0001f) g_manualPlacementPrecision /= 10; }
	}

	auto dimField = [&](const std::string& label, float& val)
	{
		Engine* engine = Engine::Current();
		const ::Menu::InputResult res = engine
			? engine->AddNumber(label, val, 4)
			: ::Menu::InputResult{};
		if (res.rightPressed) val += (g_manualPlacementPrecision * 25.0f);
		if (res.leftPressed)  val -= (g_manualPlacementPrecision * 25.0f);
	};
	dimField("Length (Y)", length);
	dimField("Width (X)",  width);
	dimField("Height (Z)", height);

	if (length < 0.01f) length = 0.01f;
	if (width  < 0.01f) width  = 0.01f;
	if (height < 0.01f) height = 0.01f;

	GTAmemory::WriteFloat(ptr + 0x60, length);
	GTAmemory::WriteFloat(ptr + 0x74, width);
	GTAmemory::WriteFloat(ptr + 0x88, height);
}

void SpoonerQuickManualPlacementSubmenu::Draw()
{
	using namespace sub::Spooner;

	if (SpoonerMode::bIsSomethingHeld)
	{
		Game::Print::PrintBottomCentre("~r~Error:~s~ There is an entity held in Spooner Mode.");
		Engine* e = Engine::Current(); if (e) e->GoBack();
		return;
	}

	auto currIndexInDb = GetEntityIndexInDb(selectedEntity);
	if (currIndexInDb < 0 || !selectedEntity.handle.Exists())
	{
		bool bFound = false;
		for (UINT i = 0; i < Databases::EntityDb.size(); i++)
		{
			if (Databases::EntityDb[i].handle.Exists())
			{
				currIndexInDb = i;
				selectedEntity = Databases::EntityDb[currIndexInDb];
				bFound = true;
				break;
			}
		}
		if (!bFound)
		{
			Game::Print::PrintBottomCentre("~r~Error:~s~ No valid entities found in the database.");
			Engine* e = Engine::Current(); if (e) e->GoBack();
			return;
		}
	}

	DrawTitle();

	{
		int idx = 0;
		Engine* engine = Engine::Current();
		const ::Menu::InputResult res = engine
			? engine->AddTextList(selectedEntity.hashName, idx, std::vector<std::string>{ selectedEntity.hashName })
			: ::Menu::InputResult{};
		if (res.accepted) NavigateTo("spooner_selected_entity_ops");
		else if (res.rightPressed)
		{
			auto newIndexInDb = currIndexInDb + 1;
			while (newIndexInDb < (int)Databases::EntityDb.size())
			{
				if (Databases::EntityDb[newIndexInDb].handle.Exists())
				{
					currIndexInDb = newIndexInDb;
					selectedEntity = Databases::EntityDb[currIndexInDb];
					break;
				}
				newIndexInDb++;
			}
		}
		else if (res.leftPressed)
		{
			auto newIndexInDb = currIndexInDb - 1;
			while (newIndexInDb >= 0)
			{
				if (Databases::EntityDb[newIndexInDb].handle.Exists())
				{
					currIndexInDb = newIndexInDb;
					selectedEntity = Databases::EntityDb[currIndexInDb];
					break;
				}
				newIndexInDb--;
			}
		}
	}

	selectedEntity.handle.RequestControlOnce();
	Vector3 currPos = selectedEntity.handle.GetPosition();
	Vector3 currRot = selectedEntity.handle.Rotation_get();
	Vector3 nextPos = currPos;
	Vector3 nextRot = currRot;

	{
		Engine* engine = Engine::Current();
		const ::Menu::InputResult res = engine
			? engine->AddNumber("Scroll Sensitivity", g_manualPlacementPrecision, 4)
			: ::Menu::InputResult{};
		if (res.rightPressed) { if (g_manualPlacementPrecision < 10.0f) g_manualPlacementPrecision *= 10; }
		if (res.leftPressed)  { if (g_manualPlacementPrecision > 0.0001f) g_manualPlacementPrecision /= 10; }
	}

	auto numField = [&](const std::string& label, float& current, float& next)
	{
		Engine* engine = Engine::Current();
		const ::Menu::InputResult res = engine
			? engine->AddNumber(label, current, 4)
			: ::Menu::InputResult{};
		if (res.rightPressed) next += g_manualPlacementPrecision;
		if (res.leftPressed)  next -= g_manualPlacementPrecision;
	};
	numField("X", currPos.x, nextPos.x);
	numField("Y", currPos.y, nextPos.y);
	numField("Z", currPos.z, nextPos.z);
	numField("Pitch", currRot.x, nextRot.x);
	numField("Roll",  currRot.y, nextRot.y);
	numField("Yaw",   currRot.z, nextRot.z);

	if (DrawOption("Other Properites")) NavigateTo("spooner_selected_entity_ops");

	HandleEntityEditingLogic(nextPos, nextRot, nullptr);

	WrapAngle(nextRot.x);
	WrapAngle(nextRot.y);
	WrapAngle(nextRot.z);

	if (nextPos != currPos) selectedEntity.handle.SetPosition(nextPos);
	if (nextRot != currRot) selectedEntity.handle.SetRotation(nextRot);
}

void SpoonerVector3ManualPlacementSubmenu::Draw()
{
	if (std::get<1>(g_v3ManualPlacementPtrs) == nullptr &&
		std::get<2>(g_v3ManualPlacementPtrs) == nullptr)
	{
		Engine* e = Engine::Current(); if (e) e->GoBack(); return;
	}

	Vector3 markerPos = *std::get<1>(g_v3ManualPlacementPtrs);
	if (std::get<0>(g_v3ManualPlacementPtrs).Exists())
		markerPos = std::get<0>(g_v3ManualPlacementPtrs).GetOffsetInWorldCoords(markerPos);
	World::DrawLightWithRange(markerPos, g_fadedRGB, 2.3f, 1.5f);
	World::DrawMarker(MarkerType::DebugSphere, markerPos, Vector3(), Vector3(),
		Vector3(0.1f, 0.1f, 0.1f), g_fadedRGB.ToRGBA(190));

	DrawTitle();

	{
		Engine* engine = Engine::Current();
		const ::Menu::InputResult res = engine
			? engine->AddNumber("Scroll Sensitivity", g_manualPlacementPrecision, 4)
			: ::Menu::InputResult{};
		if (res.rightPressed) { if (g_manualPlacementPrecision < 10.0f) g_manualPlacementPrecision *= 10; }
		if (res.leftPressed)  { if (g_manualPlacementPrecision > 0.0001f) g_manualPlacementPrecision /= 10; }
	}

	if (std::get<1>(g_v3ManualPlacementPtrs) != nullptr)
	{
		Vector3& nextPos = *std::get<1>(g_v3ManualPlacementPtrs);
		auto numField = [&](const std::string& label, float& val)
		{
			Engine* engine = Engine::Current();
			const ::Menu::InputResult res = engine
				? engine->AddNumber(label, val, 4)
				: ::Menu::InputResult{};
			if (res.rightPressed) val += g_manualPlacementPrecision;
			if (res.leftPressed)  val -= g_manualPlacementPrecision;
		};
		numField("X", nextPos.x);
		numField("Y", nextPos.y);
		numField("Z", nextPos.z);
	}
	if (std::get<2>(g_v3ManualPlacementPtrs) != nullptr)
	{
		Vector3& nextRot = *std::get<2>(g_v3ManualPlacementPtrs);
		auto numField = [&](const std::string& label, float& val)
		{
			Engine* engine = Engine::Current();
			const ::Menu::InputResult res = engine
				? engine->AddNumber(label, val, 4)
				: ::Menu::InputResult{};
			if (res.rightPressed) val += g_manualPlacementPrecision;
			if (res.leftPressed)  val -= g_manualPlacementPrecision;
		};
		numField("Pitch", nextRot.x);
		numField("Roll",  nextRot.y);
		numField("Yaw",   nextRot.z);

		WrapAngle(nextRot.x);
		WrapAngle(nextRot.y);
		WrapAngle(nextRot.z);
	}
}

void SpoonerGroupSpoonSubmenu::Draw()
{
	using namespace sub::Spooner;
	using namespace sub::Spooner::EntityManagement;

	auto& vGroup = selectedSpoonGroup;
	SpoonerEntity refEnt;
	bool bEntitiesExist = false;
	for (auto it = vGroup.begin(); it != vGroup.end();)
	{
		if (!it->handle.Exists()) { it = vGroup.erase(it); }
		else
		{
			if (!bEntitiesExist) { bEntitiesExist = true; refEnt = *it; }
			++it;
		}
	}

	DrawTitle();

	if (DrawOption("Select Entities")) NavigateTo("spooner_group_spoon_select_entities");

	if (!bEntitiesExist) return;

	DrawBreak("---Place---");

	const bool isOnTheLine = NETWORK_IS_IN_SESSION() != 0;
	Vector3 refPos = refEnt.handle.GetPosition();
	Vector3 refRot = refEnt.handle.Rotation_get();
	Vector3 nextPosOffset;
	Vector3 nextRotOffset;

	{
		Engine* engine = Engine::Current();
		const ::Menu::InputResult res = engine
			? engine->AddNumber("Scroll Sensitivity", g_manualPlacementPrecision, 4)
			: ::Menu::InputResult{};
		if (res.rightPressed) { if (g_manualPlacementPrecision < 10.0f) g_manualPlacementPrecision *= 10; }
		if (res.leftPressed)  { if (g_manualPlacementPrecision > 0.0001f) g_manualPlacementPrecision /= 10; }
	}

	auto numField = [&](const std::string& label, float& current, float& offset)
	{
		Engine* engine = Engine::Current();
		const ::Menu::InputResult res = engine
			? engine->AddNumber(label, current, 4)
			: ::Menu::InputResult{};
		if (res.rightPressed) offset += g_manualPlacementPrecision;
		if (res.leftPressed)  offset -= g_manualPlacementPrecision;
	};
	numField("X", refPos.x, nextPosOffset.x);
	numField("Y", refPos.y, nextPosOffset.y);
	numField("Z", refPos.z, nextPosOffset.z);
	numField("Pitch", refRot.x, nextRotOffset.x);
	numField("Roll",  refRot.y, nextRotOffset.y);
	numField("Yaw",   refRot.z, nextRotOffset.z);

	WrapAngle(nextRotOffset.x);
	WrapAngle(nextRotOffset.y);
	WrapAngle(nextRotOffset.z);

	if (!nextPosOffset.IsZero())
	{
		for (auto& e : vGroup)
		{
			if (isOnTheLine) e.handle.RequestControl(100);
			e.handle.SetPosition(e.handle.GetPosition() + nextPosOffset);
		}
	}
	if (!nextRotOffset.IsZero())
	{
		for (auto& e : vGroup)
		{
			if (isOnTheLine) e.handle.RequestControl(100);
			e.handle.SetRotation(e.handle.Rotation_get() + nextRotOffset);
		}
	}

	DrawBreak("---Task Sequences---");

	if (DrawOption("Start Task Sequences"))
	{
		for (auto& e : vGroup)
		{
			auto eiidb = GetEntityIndexInDb(e);
			if (eiidb >= 0)
			{
				if (isOnTheLine) e.handle.RequestControl();
				Databases::EntityDb[eiidb].taskSequence.Start();
			}
		}
	}
	if (DrawOption("Stop Task Sequences"))
	{
		for (auto& e : vGroup)
		{
			auto eiidb = GetEntityIndexInDb(e);
			if (eiidb >= 0)
			{
				if (e.handle.IsPed())
				{
					if (isOnTheLine) e.handle.RequestControl();
					TASK_CLEAR_LOOK_AT(e.handle.Handle());
					CLEAR_PED_TASKS_IMMEDIATELY(e.handle.Handle());
				}
				Databases::EntityDb[eiidb].taskSequence.Reset();
			}
		}
	}

	DrawBreak("---Edit---");

	{
		int opacityLevel = refEnt.handle.GetAlpha();
		Engine* engine = Engine::Current();
		const ::Menu::InputResult res = engine
			? engine->AddNumber("Opacity (Local)", opacityLevel, 0)
			: ::Menu::InputResult{};
		if (res.rightPressed)
		{
			if (opacityLevel < 255) opacityLevel++; else opacityLevel = 0;
			for (auto& e : vGroup) { if (isOnTheLine) e.handle.RequestControl(); e.handle.SetAlpha(opacityLevel); }
		}
		if (res.leftPressed)
		{
			if (opacityLevel > 0) opacityLevel--; else opacityLevel = 255;
			for (auto& e : vGroup) { if (isOnTheLine) e.handle.RequestControl(); e.handle.SetAlpha(opacityLevel); }
		}
	}

	if (DrawOption("Attach To Something")) NavigateTo("spooner_group_spoon_attach_to");

	if (DrawOption("Detach"))
	{
		for (auto& e : vGroup)
		{
			if (isOnTheLine) e.handle.RequestControl(400);
			DetachEntity(e);
		}
	}

	{
		int idx = g_copyEntTexterValue;
		Engine* engine = Engine::Current();
		const ::Menu::InputResult res = engine
			? engine->AddTextList("Copy", idx, std::vector<std::string>{ "Selected Only", "Copy With Attachments" })
			: ::Menu::InputResult{};
		if (res.rightPressed) { if (g_copyEntTexterValue < 1U) g_copyEntTexterValue++; }
		if (res.leftPressed)  { if (g_copyEntTexterValue > 0)  g_copyEntTexterValue--; }
		if (res.accepted)
		{
			for (auto& e : vGroup)
				CopyEntity(e, true, true, g_copyEntTexterValue);
		}
	}

	if (DrawOption("Delete"))
	{
		for (auto& e : vGroup)
		{
			if (isOnTheLine) e.handle.RequestControl(400);
			DeleteEntity(e);
		}
		vGroup.clear();
	}
}

void SpoonerGroupSpoonSelectEntitiesSubmenu::Draw()
{
	using namespace sub::Spooner;
	using namespace sub::Spooner::EntityManagement;

	auto& vGroup = selectedSpoonGroup;

	DrawTitle();

	if (DrawSelectionItem("CLEAR SELECTION", true, Checkbox::CROSS, Checkbox::CROSS))
		vGroup.clear();

	if (DrawSelectionItem("SELECT ALL", vGroup == Databases::EntityDb,
		Checkbox::TICK2, Checkbox::NONE))
	{
		if (vGroup != Databases::EntityDb)
			vGroup = Databases::EntityDb;
		else
			vGroup.clear();
	}

	for (auto& e : Databases::EntityDb)
	{
		auto grpIt = std::find(vGroup.begin(), vGroup.end(), e);
		const bool bEntityIsInGroup = grpIt != vGroup.end();
		Engine* engine = Engine::Current();
		const bool pressed = engine
			? engine->AddCheckbox(e.hashName + (e.handle.Exists() ? "" : " (Invalid)"),
				bEntityIsInGroup, Checkbox::BOXTICK, Checkbox::BOXBLANK)
			: false;
		if (IsCurrentRowSelected())
			ShowArrowAboveEntity(e.handle, RGBA(127, 0, 255, 200));
		if (pressed)
		{
			if (bEntityIsInGroup)
				vGroup.erase(grpIt);
			else
				vGroup.push_back(e);
		}
	}
}

void SpoonerGroupSpoonAttachToSubmenu::Draw()
{
	using namespace sub::Spooner;
	using namespace sub::Spooner::EntityManagement;

	auto& vGroup = selectedSpoonGroup;
	GTAped myPed = PLAYER_PED_ID();
	GTAvehicle myVehicle = g_myVeh;
	const bool isOnTheLine = NETWORK_IS_IN_SESSION() != 0;

	DrawTitle();

	{
		Engine* engine = Engine::Current();
		const bool pressed = engine
			? engine->AddCheckbox("Keep World Position When Attaching",
				Settings::bKeepPositionWhenAttaching, Checkbox::BOXTICK, Checkbox::BOXBLANK)
			: false;
		if (pressed) Settings::bKeepPositionWhenAttaching = !Settings::bKeepPositionWhenAttaching;
	}

	DrawBreak("---Available Entities---");

	if (DrawOption("Self"))
	{
		for (auto& e : vGroup)
		{
			if (e.handle.Exists())
			{
				if (isOnTheLine) e.handle.RequestControl(400);
				AttachEntityInit(e, myPed, Settings::bKeepPositionWhenAttaching);
			}
		}
		Engine* eng = Engine::Current(); if (eng) eng->GoBack();
		return;
	}

	if (myVehicle.Exists())
	{
		if (DrawOption(std::string(myPed.IsInVehicle() ? "Current" : "Last Seated") + " Vehicle"))
		{
			for (auto& e : vGroup)
			{
				if (e.handle.Exists())
				{
					if (isOnTheLine) e.handle.RequestControl(400);
					AttachEntityInit(e, myVehicle, Settings::bKeepPositionWhenAttaching);
				}
			}
			Engine* eng = Engine::Current(); if (eng) eng->GoBack();
			return;
		}
	}

	if (!Databases::EntityDb.empty())
	{
		if (Databases::EntityDb.size() > 1 ||
			std::find(vGroup.begin(), vGroup.end(), Databases::EntityDb.front()) == vGroup.end())
		{
			DrawBreak("---Database---");
			for (auto& e : Databases::EntityDb)
			{
				if (std::find(vGroup.begin(), vGroup.end(), e) != vGroup.end())
					continue;
				if (!e.handle.Exists())
				{
					DrawOption(e.hashName + " (Invalid)");
					continue;
				}
				if (DrawOption(e.hashName))
				{
					for (auto& eig : vGroup)
					{
						if (eig.handle.Exists())
						{
							if (isOnTheLine) eig.handle.RequestControl(400);
							AttachEntityInit(eig, e.handle, Settings::bKeepPositionWhenAttaching);
						}
					}
					Engine* eng = Engine::Current(); if (eng) eng->GoBack();
					return;
				}
				if (IsCurrentRowSelected())
					ShowArrowAboveEntity(e.handle, RGBA(0, 255, 0, 200));
			}
		}
	}
}

void SpoonerPedOpsSubmenu::Draw()
{
	using namespace sub::Spooner;
	using namespace sub::Spooner::EntityManagement;

	if (!selectedEntity.handle.Exists())
	{
		Engine* e = Engine::Current(); if (e) e->GoBack(); return;
	}
	selectedEntity.handle.RequestControlOnce();
	auto thisEntityIndexInDb = GetEntityIndexInDb(selectedEntity);
	bool isThisEntityInDb = thisEntityIndexInDb >= 0;
	if (isThisEntityInDb)
	{
		selectedEntity.taskSequence = Databases::EntityDb[thisEntityIndexInDb].taskSequence;
		Databases::EntityDb[thisEntityIndexInDb] = selectedEntity;
	}

	GTAped myPed = PLAYER_PED_ID();
	GTAped thisPed = selectedEntity.handle;
	const bool butAmIOnline = NETWORK_IS_IN_SESSION() != 0;
	const bool isPedMyPed = thisPed.Handle() == myPed.Handle();
	const bool bIsPedShortHeighted = GET_PED_CONFIG_FLAG(thisPed.Handle(), ePedConfigFlags::_Shrink, false) != 0;
	PedGroup myPedGroup = myPed.GetCurrentPedGroup();

	DrawTitle();

	if (!isPedMyPed)
		DrawRelationshipTextScroller();

	if (DrawToggle("Is Still (Block Fleeing)", selectedEntity.isStill))
		thisPed.SetBlockPermanentEvent(selectedEntity.isStill);

	if (DrawToggleExternal("Can Ragdoll", thisPed.GetCanRagdoll()))
	{
		bool ns = !thisPed.GetCanRagdoll();
		thisPed.SetCanRagdoll(ns);
		SET_PED_RAGDOLL_ON_COLLISION(thisPed.Handle(), ns);
	}
	if (DrawToggleExternal("Is Short Heighted (Small)", bIsPedShortHeighted))
	{
		SET_PED_CONFIG_FLAG(selectedEntity.handle.Handle(),
			ePedConfigFlags::_Shrink, bIsPedShortHeighted ? 0 : 1);
	}

	{
		int thisArmour = thisPed.GetArmour();
		if (DrawNumber("Armour", thisArmour, 1, 0, INT_MAX))
			thisPed.SetArmour(thisArmour);
		if (IsCurrentRowSelected() && MenuPressTimer::IsButtonTapped(MenuPressTimer::Button::Accept))
		{
			std::string inputStr = Game::InputBox("", 5U, "", std::to_string(thisArmour));
			if (inputStr.length() > 0)
			{
				try { thisArmour = std::stoi(inputStr); thisPed.SetArmour(thisArmour); }
				catch (...) { Game::Print::PrintErrorInvalidInput(inputStr); }
			}
		}
	}

	if (DrawOption("Wardrobe"))
	{
		SetEnt241();
		NavigateTo("ped_components");
	}
	if (sub::g_cam_componentChanger.Exists())
	{
		sub::g_cam_componentChanger.SetActive(false);
		sub::g_cam_componentChanger.Destroy();
		World::SetRenderingCamera(0);
	}

	if (DrawOption("Animations"))         { SetEnt241(); NavigateTo("ped_animation"); }
	if (DrawOption("Scenario Actions"))   { SetEnt241(); NavigateTo("ped_animation_task_scenarios"); }
	if (DrawOption("Moods"))              { SetEnt241(); NavigateTo("ped_animation_facial_mood"); }
	if (DrawOption("Movement Styles"))    { SetEnt241(); NavigateTo("ped_animation_movement_group"); }
	if (DrawOption("Weapon"))             NavigateTo("spooner_ped_ops_weapon");
	if (DrawOption("Speech Player  (Doesn't Save)")) { SetEnt241(); NavigateTo("ped_speech_player"); }
	if (DrawOption("Voice Changer  (Doesn't Save)")) { SetEnt241(); NavigateTo("ped_voice_changer"); }

	if (DrawToggleExternal("Companion (7 Max) (Doesn't Save) (Obsolete)",
		myPedGroup.Contains(thisPed)))
	{
		NETWORK_REQUEST_CONTROL_OF_NETWORK_ID(thisPed.NetID());
		if (myPedGroup.Exists())
		{
			if (!myPedGroup.Contains(thisPed))
			{
				myPedGroup.Add(thisPed, false);
				myPedGroup.SetSeparationRange(100.0f);
				myPedGroup.SetFormationSpacing(1.5f);
				thisPed.Task().FightAgainstHatedTargets(400.0f);
			}
			else
				myPedGroup.Remove(thisPed);
		}
		else
		{
			myPedGroup = PedGroup::CreateNewGroup();
			myPedGroup.Add(myPed, true);
			myPedGroup.SetSeparationRange(100.0f);
			myPedGroup.SetFormationSpacing(1.5f);
		}
	}

	if (DrawToggleExternal("Burn Ped", thisPed.IsOnFire()))
		thisPed.SetOnFire(!thisPed.IsOnFire());

	if (!isPedMyPed)
	{
		const bool isPiggyback = (GET_ENTITY_ATTACHED_TO(myPed.Handle() == thisPed.Handle())
			&& IS_ENTITY_PLAYING_ANIM(myPed.Handle(), "mini@prostitutes@sexnorm_veh", "bj_loop_male", 3));
		if (DrawToggleExternal("Piggyback Ride  (Doesn't Save)", isPiggyback))
		{
			if (thisPed == myPed)
				Game::Print::PrintBottomCentre("~r~Error:~s~ Can't do that to yourself.");
			else
			{
				if (GET_ENTITY_ATTACHED_TO(myPed.Handle()) != thisPed.Handle())
				{
					thisPed.RequestControl();
					myPed.AttachTo(thisPed, -1, false, Vector3(0.0f, -0.3f, 0.0f), Vector3(0, 0, 0));
					myPed.Task().PlayAnimation("mini@prostitutes@sexnorm_veh", "bj_loop_male",
						8.0f, 0.0f, -1, 9, 0, false);
				}
				else
				{
					myPed.Detach();
					myPed.Task().ClearAllImmediately();
				}
			}
		}

		const bool isShoulderRide = (GET_ENTITY_ATTACHED_TO(myPed.Handle() == thisPed.Handle())
			&& IS_ENTITY_PLAYING_ANIM(myPed.Handle(),
				"amb@prop_human_seat_chair@male@elbows_on_knees@idle_a", "idle_a", 3));
		if (DrawToggleExternal("Shoulder Ride  (Doesn't Save)", isShoulderRide))
		{
			if (thisPed == myPed)
				Game::Print::PrintBottomCentre("~r~Error:~s~ Can't do that to yourself.");
			else
			{
				if (GET_ENTITY_ATTACHED_TO(myPed.Handle()) != thisPed.Handle())
				{
					myPed.AttachTo(thisPed, Bone::SKEL_ROOT, Vector3(0, -0.25, 0.35f),
						Vector3(45.0f, 0, 0), true, true, false, true, 1, true);
					myPed.Task().PlayAnimation(
						"amb@prop_human_seat_chair@male@elbows_on_knees@idle_a", "idle_a",
						1000.0f, -1.5f, -1, AnimFlag::Loop, 0.445f, false);
				}
				else
				{
					myPed.Detach();
					myPed.Task().ClearAllImmediately();
				}
			}
		}
	}

	if (DrawOption("Travel To Waypoint"))
	{
		if (!IS_WAYPOINT_ACTIVE())
			Game::Print::PrintBottomCentre("~r~Error:~s~ No Waypoint Set.");
		else
		{
			Vector3 coord = GET_BLIP_INFO_ID_COORD(GET_FIRST_BLIP_INFO_ID(BlipIcon::Waypoint));
			coord.z = World::GetGroundHeight(coord);
			TASK_GO_TO_COORD_ANY_MEANS(thisPed.Handle(), coord.x, coord.y, coord.z,
				3.0f, 0, 0, 786603, -1082130432);
		}
	}

	if (!isPedMyPed)
	{
		if (DrawOption("Become This Ped (Soul-Steal) (SP)"))
		{
			if (!butAmIOnline) SetBecomePed(thisPed);
		}
	}
}

void SpoonerPedOpsWeaponSubmenu::Draw()
{
	using namespace sub::Spooner;

	GTAped myPed = PLAYER_PED_ID();
	GTAped thisPed = selectedEntity.handle;
	Hash pedCurrWeapon = thisPed.GetWeapon();
	const bool isPedMyPed = thisPed.Handle() == myPed.Handle();

	DrawTitle();

	if (DrawSelectionItem("None", pedCurrWeapon == WEAPON_UNARMED))
	{
		pedCurrWeapon = WEAPON_UNARMED;
		SET_CURRENT_PED_WEAPON(thisPed.Handle(), pedCurrWeapon, true);
	}

	if (!isPedMyPed)
	{
		if (DrawOption("Your Current Weapon"))
		{
			Hash weaponHash = myPed.GetWeapon();
			GIVE_DELAYED_WEAPON_TO_PED(thisPed.Handle(), weaponHash, 9999, true);
			GIVE_WEAPON_TO_PED(thisPed.Handle(), weaponHash, 1, true, false);
			int ammo;
			GET_MAX_AMMO(thisPed.Handle(), weaponHash, &ammo);
			SET_PED_AMMO(thisPed.Handle(), weaponHash, ammo, 0);
			SET_AMMO_IN_CLIP(thisPed.Handle(), weaponHash,
				GET_MAX_AMMO_IN_CLIP(thisPed.Handle(), weaponHash, true));
			SET_CURRENT_PED_WEAPON(thisPed.Handle(), weaponHash, true);
		}
	}

	for (UINT i = 0; i < WeaponIndivs::vCategoryNames.size(); i++)
	{
		if (DrawOption(WeaponIndivs::vCategoryNames[i]))
		{
			msCurrentPaintIndex = i;
			NavigateTo("spooner_ped_ops_weapon_in_category");
		}
	}
}

void SpoonerPedOpsWeaponInCategorySubmenu::Draw()
{
	using namespace sub::Spooner;

	Engine* engine = Engine::Current();
	auto& selectedCategoryIndex = msCurrentPaintIndex;
	if (engine && selectedCategoryIndex >= 0 &&
		selectedCategoryIndex < (int)WeaponIndivs::vCategoryNames.size())
	{
		engine->AddTitle(WeaponIndivs::vCategoryNames[selectedCategoryIndex]);
	}
	else
	{
		DrawTitle();
	}

	GTAped thisPed = selectedEntity.handle;
	Hash currentWeapon = thisPed.GetWeapon();

	for (auto& wc : *WeaponIndivs::vAllWeapons[selectedCategoryIndex])
	{
		if (DrawSelectionItem(GetWeaponLabel(wc.weaponHash, true), currentWeapon == wc.weaponHash))
		{
			GIVE_DELAYED_WEAPON_TO_PED(thisPed.Handle(), wc.weaponHash, 9999, true);
			GIVE_WEAPON_TO_PED(thisPed.Handle(), wc.weaponHash, 1, true, false);
			int ammo;
			GET_MAX_AMMO(thisPed.Handle(), wc.weaponHash, &ammo);
			SET_PED_AMMO(thisPed.Handle(), wc.weaponHash, ammo, 0);
			SET_AMMO_IN_CLIP(thisPed.Handle(), wc.weaponHash,
				GET_MAX_AMMO_IN_CLIP(thisPed.Handle(), wc.weaponHash, true));
			SET_CURRENT_PED_WEAPON(thisPed.Handle(), wc.weaponHash, true);
		}
	}
}

void SpoonerManageMarkersSubmenu::Draw()
{
	using namespace sub::Spooner;
	using namespace sub::Spooner::MarkerManagement;

	int markerIndexInDbToDelete = -1;

	DrawTitle();

	if (DrawOption("Removal")) NavigateTo("spooner_manage_markers_removal");

	DrawBreak("---Database---");

	for (UINT i = 0; i < Databases::MarkerDb.size(); i++)
	{
		auto& m = Databases::MarkerDb[i];
		if (DrawOption(m.m_name))
		{
			SelectedMarker = &m;
			NavigateTo("spooner_manage_markers_in_marker");
		}
		if (IsCurrentRowSelected())
		{
			m.m_selectedInSub = true;
			if (DrawDeleteEntityShortcut("Delete Marker"))
				markerIndexInDbToDelete = i;
		}
	}

	if (DrawSelectionItem("ADD NEW MARKER", true, Checkbox::SMALLNEWSTAR, Checkbox::NONE))
	{
		auto& spoocam = SpoonerMode::spoonerModeCamera;
		if (!spoocam.IsActive())
		{
			GTAentity myPed = PLAYER_PED_ID();
			Vector3 myPos = myPed.GetPosition();
			SelectedMarker = AddMarker(myPos, Vector3(0, 0, myPed.GetHeading()));
		}
		else
		{
			Vector3 spawnPos = spoocam.RaycastForCoord(Vector2(0.0f, 0.0f), 0, 120.0f,
				30.0f + SpoonerMarker().m_scale / 2);
			spawnPos.z += SpoonerMarker().m_scale / 2;
			SelectedMarker = AddMarker(spawnPos, Vector3(0, 0, spoocam.GetRotation().z));
		}
		NavigateTo("spooner_manage_markers_in_marker");
	}

	if (markerIndexInDbToDelete != -1)
		RemoveMarker(markerIndexInDbToDelete);
}

void SpoonerManageMarkersRemovalSubmenu::Draw()
{
	using namespace sub::Spooner;
	using namespace sub::Spooner::EntityManagement;
	using sub::Spooner::MarkerManagement::RemoveAllMarkersInRange;
	using sub::Spooner::MarkerManagement::RemoveAllMarkers;

	GTAentity myPed = PLAYER_PED_ID();
	const Vector3& myPos = myPed.GetPosition();

	DrawTitle();

	{
		Engine* engine = Engine::Current();
		const ::Menu::InputResult res = engine
			? engine->AddNumber("Delete Markers In Range", g_fSaveRangeRadius, 0)
			: ::Menu::InputResult{};
		if (IsCurrentRowSelected())
			DrawRadiusDisplayingMarker(myPos, g_fSaveRangeRadius);
		if (res.rightPressed) { if (g_fSaveRangeRadius < FLT_MAX) g_fSaveRangeRadius += 1.0f; }
		if (res.leftPressed)  { if (g_fSaveRangeRadius > 0.0f)   g_fSaveRangeRadius -= 1.0f; }
		if (res.accepted)
			RemoveAllMarkersInRange(myPos, g_fSaveRangeRadius);
	}

	if (DrawOption("Delete All Markers (" + std::to_string(Databases::MarkerDb.size()) + ")"))
		RemoveAllMarkers();
}

void SpoonerManageMarkersInMarkerSubmenu::Draw()
{
	using namespace sub::Spooner;
	using namespace sub::Spooner::MarkerManagement;

	if (SelectedMarker == nullptr)
	{
		Engine* e = Engine::Current(); if (e) e->GoBack(); return;
	}

	Engine* engine = Engine::Current();
	if (engine) engine->AddTitle(SelectedMarker->m_name);

	Vector3 finalPosition, finalRotation;
	if (SelectedMarker->m_attachmentArgs.attachedTo.Exists())
	{
		finalPosition = SelectedMarker->m_attachmentArgs.attachedTo.GetOffsetInWorldCoords(
			SelectedMarker->m_attachmentArgs.offset);
		finalRotation = SelectedMarker->m_attachmentArgs.attachedTo.Rotation_get() +
			SelectedMarker->m_attachmentArgs.rotation;
	}
	else
	{
		finalPosition = SelectedMarker->m_position;
		finalRotation = SelectedMarker->m_rotation;
	}

	SpoonerMarkerPosition* dest = SelectedMarker->m_destinationPtr == nullptr
		? &SelectedMarker->m_destinationVal : SelectedMarker->m_destinationPtr;
	Vector3 finalDest;
	float finalDestHeading;
	if (dest->m_attachmentArgs.attachedTo.Exists())
	{
		finalDest = dest->m_attachmentArgs.attachedTo.GetOffsetInWorldCoords(
			dest->m_attachmentArgs.offset);
		finalDestHeading = dest->m_attachmentArgs.attachedTo.Rotation_get().z +
			SelectedMarker->m_destinationHeading;
	}
	else
	{
		finalDest = dest->m_position;
		finalDestHeading = SelectedMarker->m_destinationHeading;
	}

	if (!dest->m_position.IsZero())
	{
		World::DrawLine(finalPosition, finalDest, RGBA(SelectedMarker->m_colour, 200));
		World::DrawLightWithRange(finalDest, RgbS(SelectedMarker->m_colour), 2.3f, 1.5f);

		const Vector3& helpingSpherePos = finalDest.PointOnCircle(1.16f, finalDestHeading + 90.0f);
		World::DrawLine(finalDest, helpingSpherePos, RGBA(SelectedMarker->m_colour, 200));
		World::DrawMarker(MarkerType::DebugSphere, helpingSpherePos, Vector3(), Vector3(),
			Vector3(0.1f, 0.1f, 0.1f), RGBA(SelectedMarker->m_colour, 200));
	}

	auto& spoocam = SpoonerMode::spoonerModeCamera;

	// Name (free-text edit on accept).
	{
		int idx = 0;
		const ::Menu::InputResult res = engine
			? engine->AddTextList("Name", idx, std::vector<std::string>{ SelectedMarker->m_name })
			: ::Menu::InputResult{};
		if (res.accepted)
		{
			SelectedMarker->m_name = Game::InputBox(SelectedMarker->m_name, 26U,
				"Enter custom marker name:", SelectedMarker->m_name);
		}
	}

	// Type (cycles through MarkerType::vNames).
	{
		int typeIdx = SelectedMarker->m_type;
		if (DrawTextList("Type", typeIdx, MarkerType::vNames))
			SelectedMarker->m_type = static_cast<UINT8>(typeIdx);
	}

	{
		const ::Menu::InputResult res = engine
			? engine->AddNumber("Size", SelectedMarker->m_scale, 2)
			: ::Menu::InputResult{};
		if (res.rightPressed) { if (SelectedMarker->m_scale < 10.0f) SelectedMarker->m_scale += 0.05f; }
		if (res.leftPressed)  { if (SelectedMarker->m_scale > 0.0f)  SelectedMarker->m_scale -= 0.05f; }
	}

	{
		const bool pressed = engine
			? engine->AddCheckbox("Show Name", SelectedMarker->m_showName,
				Checkbox::BOXTICK, Checkbox::BOXBLANK)
			: false;
		if (pressed) SelectedMarker->m_showName = !SelectedMarker->m_showName;
	}
	{
		const bool pressed = engine
			? engine->AddCheckbox("Rotate Continuously", SelectedMarker->m_rotateContinuously,
				Checkbox::BOXTICK, Checkbox::BOXBLANK)
			: false;
		if (pressed) SelectedMarker->m_rotateContinuously = !SelectedMarker->m_rotateContinuously;
	}
	{
		const bool pressed = engine
			? engine->AddCheckbox("Allow Vehicle Teleportation", SelectedMarker->m_allowVehicles,
				Checkbox::BOXTICK, Checkbox::BOXBLANK)
			: false;
		if (pressed) SelectedMarker->m_allowVehicles = !SelectedMarker->m_allowVehicles;
	}

	// Colour: shows preview swatch on the highlighted row; tap to navigate to
	// the settings_colours2 picker with the marker colour bound.
	{
		const bool pressed = DrawOption("Colour");
		if (engine && IsCurrentRowSelected())
		{
			engine->AddPresetColourOptionsPreview(
				static_cast<unsigned char>(SelectedMarker->m_colour.R),
				static_cast<unsigned char>(SelectedMarker->m_colour.G),
				static_cast<unsigned char>(SelectedMarker->m_colour.B));
		}
		if (pressed)
		{
			sub::g_settingsRGBA = &SelectedMarker->m_colour;
			NavigateTo("settings_colours2");
		}
	}

	DrawBreak("---Position---");
	{
		DrawOption("~italic~" + finalPosition.ToString());

		if (!spoocam.IsActive())
		{
			if (DrawOption("Set To Player Position"))
			{
				Vector3 myPos = GTAentity(PLAYER_PED_ID()).GetPosition();
				SelectedMarker->m_position = myPos;
				SelectedMarker->m_attachmentArgs.attachedTo = 0;
			}
		}
		else
		{
			if (DrawOption("Set To Camera Target"))
			{
				Vector3 hitCoords = spoocam.RaycastForCoord(Vector2(0.0f, 0.0f), 0, 160.0f, 3.0f);
				SelectedMarker->m_position = hitCoords;
				SelectedMarker->m_attachmentArgs.attachedTo = 0;
			}
		}
		if (IS_WAYPOINT_ACTIVE())
		{
			if (DrawOption("Set To Waypoint"))
			{
				GTAblip wpBlip = GET_FIRST_BLIP_INFO_ID(BlipIcon::Waypoint);
				Vector3 wpCoords = wpBlip.GetPosition();
				wpCoords.z = World::GetGroundHeight(wpCoords);
				SelectedMarker->m_position = wpCoords;
				SelectedMarker->m_attachmentArgs.attachedTo = 0;
			}
		}

		if (!Databases::EntityDb.empty())
		{
			if (DrawOption("Attach To Entity")) NavigateTo("spooner_manage_markers_in_marker_attach");
		}

		if (SelectedMarker->m_attachmentArgs.attachedTo.Exists())
		{
			if (DrawOption("Adjust Attachment"))
			{
				g_v3ManualPlacementPtrs = std::make_tuple<GTAentity, Vector3*, Vector3*>(
					0, &SelectedMarker->m_attachmentArgs.offset,
					&SelectedMarker->m_attachmentArgs.rotation);
				NavigateTo("spooner_vector3_manual_placement");
			}
		}
		else
		{
			if (DrawOption("Manual Placement"))
			{
				g_v3ManualPlacementPtrs = std::make_tuple<GTAentity, Vector3*, Vector3*>(
					0, &SelectedMarker->m_position, &SelectedMarker->m_rotation);
				NavigateTo("spooner_vector3_manual_placement");
			}
		}
	}

	DrawBreak("---Destination---");
	{
		DrawOption("~italic~" + finalDest.ToString());

		if (DrawSelectionItem("No Destination",
			SelectedMarker->m_destinationVal.m_position.IsZero()))
		{
			SelectedMarker->m_destinationVal.m_position.clear();
			SelectedMarker->m_destinationVal.m_attachmentArgs.attachedTo = 0;
			SelectedMarker->m_destinationPtr = nullptr;
		}

		if (!spoocam.IsActive())
		{
			if (DrawOption("Set To Player Position"))
			{
				Vector3 myPos = GTAentity(PLAYER_PED_ID()).GetPosition();
				SelectedMarker->m_destinationVal.m_position = myPos;
				SelectedMarker->m_destinationVal.m_attachmentArgs.attachedTo = 0;
				SelectedMarker->m_destinationPtr = nullptr;
			}
		}
		else
		{
			if (DrawOption("Set To Camera Target"))
			{
				Vector3 hitCoords = spoocam.RaycastForCoord(Vector2(0.0f, 0.0f), 0, 160.0f, 3.0f);
				SelectedMarker->m_destinationVal.m_position = hitCoords;
				SelectedMarker->m_destinationVal.m_attachmentArgs.attachedTo = 0;
				SelectedMarker->m_destinationPtr = nullptr;
			}
		}
		if (IS_WAYPOINT_ACTIVE())
		{
			if (DrawOption("Set To Waypoint"))
			{
				GTAblip wpBlip = GET_FIRST_BLIP_INFO_ID(BlipIcon::Waypoint);
				Vector3 wpCoords = wpBlip.GetPosition();
				wpCoords.z = World::GetGroundHeight(wpCoords);
				SelectedMarker->m_destinationVal.m_position = wpCoords;
				SelectedMarker->m_destinationVal.m_attachmentArgs.attachedTo = 0;
				SelectedMarker->m_destinationPtr = nullptr;
			}
		}

		if (Databases::MarkerDb.size() > 1)
		{
			if (DrawOption("Link With Marker" +
				(SelectedMarker->m_destinationPtr == nullptr ? std::string()
					: " ('" + SelectedMarker->m_destinationPtr->m_name + "')")))
			{
				NavigateTo("spooner_manage_markers_in_marker_dest2marker");
			}
		}

		if (SelectedMarker->m_destinationPtr == nullptr)
		{
			if (SelectedMarker->m_destinationVal.m_attachmentArgs.attachedTo.Exists())
			{
				if (DrawOption("Adjust Attachment"))
				{
					g_v3ManualPlacementPtrs = std::make_tuple<GTAentity, Vector3*, Vector3*>(
						0, &SelectedMarker->m_destinationVal.m_attachmentArgs.offset, nullptr);
					NavigateTo("spooner_vector3_manual_placement");
				}
			}
			else
			{
				if (DrawOption("Manual Placement"))
				{
					g_v3ManualPlacementPtrs = std::make_tuple<GTAentity, Vector3*, Vector3*>(
						0, &SelectedMarker->m_destinationVal.m_position, nullptr);
					NavigateTo("spooner_vector3_manual_placement");
				}
			}
		}

		{
			const ::Menu::InputResult res = engine
				? engine->AddNumber("Post-Teleport Direction (To Face)",
					SelectedMarker->m_destinationHeading, 1)
				: ::Menu::InputResult{};
			if (res.rightPressed) SelectedMarker->m_destinationHeading += 1.0f;
			if (res.leftPressed)  SelectedMarker->m_destinationHeading -= 1.0f;
		}
	}

	DrawBreak("---Other---");

	if (DrawOption("Copy Marker"))
	{
		SelectedMarker = CopyMarker(*SelectedMarker);
		SelectedMarker->m_name = Game::InputBox(SelectedMarker->m_name, 26U,
			"Enter custom marker name:", SelectedMarker->m_name);
	}
}

void SpoonerManageMarkersInMarkerDest2MarkerSubmenu::Draw()
{
	using namespace sub::Spooner;

	if (SelectedMarker == nullptr)
	{
		Engine* e = Engine::Current(); if (e) e->GoBack(); return;
	}

	DrawTitle();

	for (UINT i = 0; i < Databases::MarkerDb.size(); i++)
	{
		auto& m = Databases::MarkerDb[i];
		if (&m == SelectedMarker) continue;

		if (DrawOption(m.m_name))
		{
			SelectedMarker->m_destinationPtr = &m;
			SelectedMarker->m_destinationVal = m;
			Engine* e = Engine::Current(); if (e) e->GoBack();
			return;
		}

		if (IsCurrentRowSelected())
		{
			m.m_selectedInSub = true;

			Vector3 finalPosition;
			if (SelectedMarker->m_attachmentArgs.attachedTo.Exists())
				finalPosition = SelectedMarker->m_attachmentArgs.attachedTo
					.GetOffsetInWorldCoords(SelectedMarker->m_attachmentArgs.offset);
			else
				finalPosition = SelectedMarker->m_position;

			Vector3 finalDest;
			if (m.m_attachmentArgs.attachedTo.Exists())
				finalDest = m.m_attachmentArgs.attachedTo
					.GetOffsetInWorldCoords(m.m_attachmentArgs.offset);
			else
				finalDest = m.m_position;

			World::DrawLine(finalPosition, finalDest, RGBA(SelectedMarker->m_colour, 200));
			World::DrawLightWithRange(finalDest, RgbS(SelectedMarker->m_colour), 2.3f, 1.5f);
		}
	}
}

void SpoonerManageMarkersInMarkerAttachSubmenu::Draw()
{
	using namespace sub::Spooner;
	using namespace sub::Spooner::EntityManagement;

	if (SelectedMarker == nullptr)
	{
		Engine* e = Engine::Current(); if (e) e->GoBack(); return;
	}

	DrawTitle();

	{
		Engine* engine = Engine::Current();
		const bool pressed = engine
			? engine->AddCheckbox("Keep World Position When Attaching",
				Settings::bKeepPositionWhenAttaching, Checkbox::BOXTICK, Checkbox::BOXBLANK)
			: false;
		if (pressed) Settings::bKeepPositionWhenAttaching = !Settings::bKeepPositionWhenAttaching;
	}

	if (DrawSelectionItem("Detach",
		!SelectedMarker->m_attachmentArgs.attachedTo.GetHandle(),
		Checkbox::TICK2, Checkbox::NONE))
	{
		if (SelectedMarker->m_attachmentArgs.attachedTo.Exists())
		{
			SelectedMarker->m_position = SelectedMarker->m_attachmentArgs.attachedTo
				.GetOffsetInWorldCoords(SelectedMarker->m_attachmentArgs.offset);
			SelectedMarker->m_rotation = SelectedMarker->m_attachmentArgs.attachedTo
				.Rotation_get() + SelectedMarker->m_attachmentArgs.rotation;
		}
		SelectedMarker->m_attachmentArgs.attachedTo = 0;
		SelectedMarker->m_attachmentArgs.offset.clear();
		SelectedMarker->m_attachmentArgs.rotation.clear();
	}

	if (!Databases::EntityDb.empty())
	{
		DrawBreak("---Database---");
		for (auto& e : Databases::EntityDb)
		{
			if (!e.handle.Exists())
			{
				DrawOption(e.hashName + " (Invalid)");
				continue;
			}
			if (DrawSelectionItem(e.hashName,
				SelectedMarker->m_attachmentArgs.attachedTo == e.handle,
				Checkbox::TICK2, Checkbox::NONE))
			{
				SelectedMarker->m_attachmentArgs.attachedTo = e.handle;
				if (Settings::bKeepPositionWhenAttaching)
				{
					SelectedMarker->m_attachmentArgs.offset =
						e.handle.GetOffsetGivenWorldCoords(SelectedMarker->m_position);
					SelectedMarker->m_attachmentArgs.rotation =
						SelectedMarker->m_rotation - e.handle.Rotation_get();
				}
				else
				{
					SelectedMarker->m_attachmentArgs.offset.clear();
					SelectedMarker->m_attachmentArgs.rotation.clear();
				}
				Engine* eng = Engine::Current(); if (eng) eng->GoBack();
				return;
			}
			if (IsCurrentRowSelected())
				ShowArrowAboveEntity(e.handle, RGBA(0, 255, 0, 200));
		}
	}
}

void SpoonerSpawnCategoriesSubmenu::Draw()
{
	SearchStr().clear();

	DrawTitle();

	if (DrawOption("Object"))  NavigateTo("spooner_spawn_prop");
	if (DrawOption("Ped"))     NavigateTo("spooner_spawn_ped");
	if (DrawOption("Vehicle")) NavigateTo("spooner_spawn_vehicle");
}

void SpoonerSpawnPropSubmenu::Draw()
{
	using namespace sub::Spooner;

	DrawTitle();

	if (DrawOption("Favourites")) NavigateTo("spooner_spawn_prop_favourites");

	if (DrawOption(SearchStr().empty() ? "SEARCH" : toUpperCopy(SearchStr())))
	{
		SearchStr() = Game::InputBox(SearchStr(), 126U, "SEARCH", SearchStr());
		toLowerInPlace(SearchStr());
	}

	for (const auto& current : objectModels)
	{
		if (!SearchStr().empty())
		{
			if (current.find(SearchStr()) == std::string::npos) continue;
		}

		Model currentModel = GET_HASH_KEY(current);

		DrawAddProp(current, currentModel);

		if (IsCurrentRowSelected())
		{
			const bool bIsAFav = FavouritesManagement::IsPropAFavourite(current, currentModel.hash);
			const std::string label = (!bIsAFav ? "Add to" : "Remove from")
				+ (std::string)" favourites";
			Engine* engine = Engine::Current();
			if (!engine) continue;
			if (Menu::bitController)
			{
				engine->AddInstructionalButton(INPUT_SCRIPT_RLEFT, label, /*isKey=*/false);
				if (IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_SCRIPT_RLEFT))
				{
					if (!bIsAFav) FavouritesManagement::AddPropToFavourites(current, currentModel.hash);
					else          FavouritesManagement::RemovePropFromFavourites(current, currentModel.hash);
				}
			}
			else
			{
				engine->AddInstructionalButton(static_cast<int>(VirtualKey::B), label, /*isKey=*/true);
				if (IsKeyJustUp(VirtualKey::B))
				{
					if (!bIsAFav) FavouritesManagement::AddPropToFavourites(current, currentModel.hash);
					else          FavouritesManagement::RemovePropFromFavourites(current, currentModel.hash);
				}
			}
		}
	}

	if (DrawOption("INPUT MODEL"))
		EntityManagement::InputEntityIntoDb(EntityType::PROP);
}

void SpoonerSpawnPropFavouritesSubmenu::Draw()
{
	using namespace sub::Spooner;
	using sub::Spooner::FavouritesManagement::xmlFavouriteProps;

	DrawTitle();

	pugi::xml_document doc;
	const std::string xmlPath = (std::string)GetPathffA(Pathff::Main, true) + xmlFavouriteProps;
	if (doc.load_file(xmlPath.c_str()).status != pugi::status_ok)
	{
		doc.reset();
		auto nodeDecleration = doc.append_child(pugi::node_declaration);
		nodeDecleration.append_attribute("version") = "1.0";
		nodeDecleration.append_attribute("encoding") = "ISO-8859-1";
		doc.append_child("FavouriteProps");
		doc.save_file(xmlPath.c_str());
		return;
	}
	pugi::xml_node nodeRoot = doc.child("FavouriteProps");

	if (DrawOption("Add New Object Model"))
	{
		std::string inputStr = Game::InputBox("", 40U, "Enter model name:");
		if (inputStr.length() > 0)
		{
			if (FavouritesManagement::AddPropToFavourites(inputStr, GET_HASH_KEY(inputStr)))
				Game::Print::PrintBottomLeft("Model ~b~added~s~.");
			else
				Game::Print::PrintBottomLeft("~r~Error:~s~ Unable to add model.");
		}
	}

	if (!nodeRoot.first_child()) return;

	DrawBreak("---Added Object Models---");

	for (auto nodeLocToLoad = nodeRoot.first_child(); nodeLocToLoad;
		nodeLocToLoad = nodeLocToLoad.next_sibling())
	{
		const std::string modelName = nodeLocToLoad.attribute("modelName").as_string();
		Model model = nodeLocToLoad.attribute("modelHash").as_uint(0);
		if (model.hash == 0) model = GET_HASH_KEY(modelName);

		DrawAddProp(modelName, model);

		if (IsCurrentRowSelected())
		{
			Engine* engine = Engine::Current();
			if (!engine) continue;
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

void SpoonerSpawnPedSubmenu::Draw()
{
	using namespace sub::Spooner;

	DrawTitle();

	if (DrawOption("Favourites")) NavigateTo("ped_model_changer_favourites");

	if (DrawOption(SearchStr().empty() ? "SEARCH" : toUpperCopy(SearchStr())))
	{
		SearchStr() = Game::InputBox(SearchStr(), 126U, "SEARCH", SearchStr());
		toLowerInPlace(SearchStr());
	}

	if (!SearchStr().empty())
	{
		for (auto& current : g_pedModels)
		{
			if (current.first.find(SearchStr()) == std::string::npos &&
				current.second.find(SearchStr()) == std::string::npos)
				continue;

			Model currentModel = GET_HASH_KEY(current.first);
			if (currentModel.IsInCdImage())
			{
				DrawAddPed(current.second, currentModel);
				if (IsCurrentRowSelected())
					sub::PedFavourites::ShowInstructionalButton(currentModel);
			}
		}
	}
	else
	{
		if (DrawOption("Player"))                 NavigateTo("ped_model_changer_player");
		if (DrawOption("Animals"))                NavigateTo("ped_model_changer_animal");
		if (DrawOption("Ambient Females"))        NavigateTo("ped_model_changer_amb_females");
		if (DrawOption("Ambient Males"))          NavigateTo("ped_model_changer_amb_males");
		if (DrawOption("Cutscene Models"))        NavigateTo("ped_model_changer_cs");
		if (DrawOption("Gang Females"))           NavigateTo("ped_model_changer_gang_females");
		if (DrawOption("Gang Males"))             NavigateTo("ped_model_changer_gang_males");
		if (DrawOption("Story Models"))           NavigateTo("ped_model_changer_story");
		if (DrawOption("Multiplayer Models"))     NavigateTo("ped_model_changer_mp");
		if (DrawOption("Scenario Females"))       NavigateTo("ped_model_changer_scenario_females");
		if (DrawOption("Scenario Males"))         NavigateTo("ped_model_changer_scenario_males");
		if (DrawOption("Story Scenario Females")) NavigateTo("ped_model_changer_st_scenario_females");
		if (DrawOption("Story Scenario Males"))   NavigateTo("ped_model_changer_st_scenario_males");
		if (DrawOption("Others"))                 NavigateTo("ped_model_changer_others");
	}

	if (DrawOption("INPUT MODEL"))
		EntityManagement::InputEntityIntoDb(EntityType::PED);
}

void SpoonerSpawnVehicleSubmenu::Draw()
{
	using namespace sub::Spooner;

	g_Ped1 = PLAYER_PED_ID();

	DrawTitle();

	if (DrawOption("Favourites")) NavigateTo("vehicle_spawner_favourites");

	if (DrawOption("Vehicle Spawner")) NavigateTo("vehicle_spawner");

	if (DrawOption("INPUT MODEL"))
		EntityManagement::InputEntityIntoDb(EntityType::VEHICLE);
}

}

REGISTER_SUBMENU(::Menu::SpoonerSubmenu)
REGISTER_SUBMENU(::Menu::SpoonerSettingsSubmenu)
REGISTER_SUBMENU(::Menu::SpoonerSpawnCategoriesSubmenu)
REGISTER_SUBMENU(::Menu::SpoonerSpawnPropSubmenu)
REGISTER_SUBMENU(::Menu::SpoonerSpawnPropFavouritesSubmenu)
REGISTER_SUBMENU(::Menu::SpoonerSpawnPedSubmenu)
REGISTER_SUBMENU(::Menu::SpoonerSpawnVehicleSubmenu)
REGISTER_SUBMENU(::Menu::SpoonerManageMarkersSubmenu)
REGISTER_SUBMENU(::Menu::SpoonerManageMarkersRemovalSubmenu)
REGISTER_SUBMENU(::Menu::SpoonerManageMarkersInMarkerSubmenu)
REGISTER_SUBMENU(::Menu::SpoonerManageMarkersInMarkerDest2MarkerSubmenu)
REGISTER_SUBMENU(::Menu::SpoonerManageMarkersInMarkerAttachSubmenu)
REGISTER_SUBMENU(::Menu::SpoonerManageDbSubmenu)
REGISTER_SUBMENU(::Menu::SpoonerManageDbRemovalSubmenu)
REGISTER_SUBMENU(::Menu::SpoonerSelectedEntityOpsSubmenu)
REGISTER_SUBMENU(::Menu::SpoonerPedOpsSubmenu)
REGISTER_SUBMENU(::Menu::SpoonerPedOpsWeaponSubmenu)
REGISTER_SUBMENU(::Menu::SpoonerPedOpsWeaponInCategorySubmenu)
REGISTER_SUBMENU(::Menu::SpoonerAttachmentOpsSubmenu)
REGISTER_SUBMENU(::Menu::SpoonerAttachmentOpsAttachToSubmenu)
REGISTER_SUBMENU(::Menu::SpoonerAttachmentOpsSelectBoneSubmenu)
REGISTER_SUBMENU(::Menu::SpoonerManualPlacementSubmenu)
REGISTER_SUBMENU(::Menu::SpoonerSizeManipulationSubmenu)
REGISTER_SUBMENU(::Menu::SpoonerQuickManualPlacementSubmenu)
REGISTER_SUBMENU(::Menu::SpoonerVector3ManualPlacementSubmenu)
REGISTER_SUBMENU(::Menu::SpoonerGroupSpoonSubmenu)
REGISTER_SUBMENU(::Menu::SpoonerGroupSpoonSelectEntitiesSubmenu)
REGISTER_SUBMENU(::Menu::SpoonerGroupSpoonAttachToSubmenu)
