#include "Bodyguard.h"

#include "../Menu/Engine.h"
#include "../Menu/SubmenuRegistry.h"
#include "../Menu/Menu.h"      // Menu::bitController, VirtualKey, Checkbox
#include "../Menu/Routine.h"   // g_Ped1, g_Ped2

#include "../Natives/natives.h"
#include "../Natives/natives2.h"
#include "../Util/keyboard.h"
#include "../Util/StringManip.h"

#include "../Scripting/Camera.h"
#include "../Scripting/Game.h"
#include "../Scripting/GTAped.h"
#include "../Scripting/GTAentity.h"
#include "../Scripting/Model.h"
#include "../Scripting/ModelNames.h"
#include "../Scripting/World.h"

#include "BodyguardRuntime.h"
#include "PedComponentRuntime.h"  // g_cam_componentChanger
#include "PedModelRuntime.h"      // PedFavourites::ShowInstructionalButton
#include "PlayerRuntime.h"        // g_Ped1, g_Ped2, SetPedInvincibleOn/Off
#include "Spooner/SpoonerShared.h" // _searchStr
#include "WeaponRuntime.h"           // g_WeaponOpsPedOverride, etc.

#include <algorithm>
#include <cctype>
#include <string>
#include <utility>
#include <vector>

namespace Menu {

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

static void SpawnBodyguardFromModel(const std::string& text, const Model& model)
{
	using namespace sub::BodyguardMenu;

	if (BodyguardDb.size() >= BodyguardManagement::MAX_BODYGUARDS)
	{
		Game::Print::PrintBottomLeft("Maximum of 7 bodyguards reached.");
		return;
	}
	if (!model.IsInCdImage())
		return;
	if (!model.Load(4000))
	{
		Game::Print::PrintBottomLeft("Failed to load model.");
		model.Unload();
		return;
	}

	const int pedType = 26;

	GTAped player(PLAYER::PLAYER_PED_ID());
	Vector3 pos = player.GetOffsetInWorldCoords(Vector3(2.f, 0.f, 0.f));

	Ped ped = PED::CREATE_PED(pedType, model.hash, pos.x, pos.y, pos.z, 0.f, true, true);

	ENTITY::SET_ENTITY_MAX_HEALTH(ped, health);
	ENTITY::SET_ENTITY_HEALTH(ped, health, 0);
	PED::SET_PED_ARMOUR(ped, armor);
	if (godmode) SetPedInvincibleOn(ped);
	else         SetPedInvincibleOff(ped);

	PED::SET_PED_AS_GROUP_MEMBER(ped, PLAYER::GET_PLAYER_GROUP(PLAYER::PLAYER_ID()));
	PED::SET_PED_NEVER_LEAVES_GROUP(ped, true);
	PED::SET_PED_COMBAT_ABILITY(ped, 2);
	PED::SET_PED_COMBAT_MOVEMENT(ped, 2);
	PED::SET_PED_COMBAT_ATTRIBUTES(ped, 46, true);

	BodyguardEntity ent{};
	ent.Handle = GTAentity(ped);
	ent.Type = EntityType::PED;
	ent.Name = text;
	ent.HashName = IntToHexString(model.hash, true);

	BodyguardManagement::AddBodyguardToDb(ent);
	for (auto& bg : BodyguardDb)
	{
		if (bg.Handle.Exists())
			ApplyBodyguardBlip(bg.Handle.GetHandle(), blipIcon);
	}
	BodyguardManagement::s_bodyguards.push_back(ped);

	Game::Print::PrintBottomLeft("Bodyguard spawned");
	model.Unload();
}

void BodyguardSubmenu::Draw()
{
	Engine* engine = Engine::Current();
	if (!engine) return;
	engine->GoBack();
	engine->NavigateTo("bodyguard_main");
}

void BodyguardMenuSubmenu::Draw()
{
	using namespace sub::BodyguardMenu;

	DrawTitle();

	static const std::vector<std::pair<int, std::string>> blipOptions =
	{
		{ 1,   "Standard" },
		{ 280, "Friend"   },
		{ 480, "VIP"      }
	};
	static const std::vector<std::pair<int, std::string>> formationOptions =
	{
		{ 0, "Default Formation" },
		{ 1, "Circle (Inward)"   },
		{ 2, "Circle (North)"    },
		{ 3, "Line"              }
	};

	if (DrawOption("Spawn Bodyguard")) NavigateTo("bodyguard_spawn");
	if (DrawOption("Bodyguard List"))  NavigateTo("bodyguard_list");

	DrawNumber("Default Health", health, 1, 0, INT_MAX);
	if (IsCurrentRowSelected() && MenuPressTimer::IsButtonTapped(MenuPressTimer::Button::Accept))
	{
		std::string inputStr = Game::InputBox("", 5U, "", std::to_string(health));
		if (!inputStr.empty())
		{
			try { health = std::stoi(inputStr); }
			catch (...) { Game::Print::PrintErrorInvalidInput(inputStr); }
		}
	}

	DrawNumber("Default Armor", armor, 1, 0, INT_MAX);
	if (IsCurrentRowSelected() && MenuPressTimer::IsButtonTapped(MenuPressTimer::Button::Accept))
	{
		std::string inputStr = Game::InputBox("", 5U, "", std::to_string(armor));
		if (!inputStr.empty())
		{
			try { armor = std::stoi(inputStr); }
			catch (...) { Game::Print::PrintErrorInvalidInput(inputStr); }
		}
	}

	const bool oldGodmode = godmode;
	if (DrawToggle("Godmode", godmode) && oldGodmode != godmode)
	{
		for (auto& bg : BodyguardDb)
		{
			if (!bg.Handle.Exists()) continue;
			Ped ped = bg.Handle.GetHandle();
			if (godmode) SetPedInvincibleOn(ped);
			else         SetPedInvincibleOff(ped);
		}
	}

	// Bodyguard Blip — single-entry texter showing currently-selected label;
	// right/left advances + refreshes blips. Legacy uses wrap-around.
	if (DrawTextList("Bodyguard Blip", blipIndex,
		std::vector<std::string>{ blipOptions[0].second, blipOptions[1].second, blipOptions[2].second }))
	{
		blipIcon = blipOptions[blipIndex].first;
		RefreshAllBodyguardBlips();
	}

	// Formation — single-entry texter; right/left cycles + applies formation.
	{
		const int prevIdx = formationIndex;
		if (DrawTextList("Formation", formationIndex,
			std::vector<std::string>{ formationOptions[0].second, formationOptions[1].second,
				formationOptions[2].second, formationOptions[3].second }))
		{
			if (formationIndex != prevIdx)
			{
				const int playerGroup = PLAYER::GET_PLAYER_GROUP(PLAYER::PLAYER_ID());
				PED::SET_GROUP_FORMATION(playerGroup, formationOptions[formationIndex].first);
			}
		}
	}

	if (DrawOption("Bring Bodyguards To Self"))
	{
		Ped playerPed = PLAYER_PED_ID();
		if (ENTITY::DOES_ENTITY_EXIST(playerPed))
		{
			Vector3 playerPos = ENTITY::GET_ENTITY_COORDS(playerPed, true);
			Vector3 forward = ENTITY::GET_ENTITY_FORWARD_VECTOR(playerPed);

			const float baseDist = 3.0f;
			const float spacing = 0.75f;
			int placed = 0;

			for (auto& bg : BodyguardDb)
			{
				if (!bg.Handle.Exists()) continue;
				Ped ped = bg.Handle.GetHandle();

				Vector3 targetPos = playerPos
					+ (forward * baseDist)
					+ Vector3(0.0f, 0.0f, 0.2f)
					+ (forward * (spacing * placed));

				ENTITY::SET_ENTITY_COORDS_NO_OFFSET(ped, targetPos.x, targetPos.y, targetPos.z,
					false, false, false);
				++placed;
			}

			Game::Print::PrintBottomLeft("Bodyguards teleported.");
		}
	}
}

// ============================================================
// BodyguardSpawnSubmenu (sub::BodyguardMenu::BodyguardSpawn)
// ============================================================

void BodyguardSpawnSubmenu::Draw()
{
	using namespace sub::BodyguardMenu;
	using sub::Spooner::Submenus::_searchStr;
	using sub::PedFavourites::ShowInstructionalButton;

	DrawTitle();

	if (DrawOption("Favourites")) { NavigateTo("ped_model_changer_favourites"); return; }

	// SEARCH row.
	{
		const std::string searchLabel = _searchStr.empty()
			? std::string("SEARCH") : toUpperCopy(_searchStr);
		if (DrawOption(searchLabel))
		{
			_searchStr = Game::InputBox(_searchStr, 126U, "SEARCH", _searchStr);
			toLowerInPlace(_searchStr);
		}
	}

	if (!_searchStr.empty())
	{
		for (auto& current : g_pedModels)
		{
			if (current.first.find(_searchStr) == std::string::npos
				&& current.second.find(_searchStr) == std::string::npos)
				continue;

			Model currentModel = GET_HASH_KEY(current.first);
			if (!currentModel.IsInCdImage())
				continue;

			if (DrawOption(current.second))
				SpawnBodyguardFromModel(current.second, currentModel);

			if (IsCurrentRowSelected())
				ShowInstructionalButton(currentModel);
		}
	}
	else
	{
		if (DrawOption("Player"))                  NavigateTo("ped_model_changer_player");
		if (DrawOption("Animals"))                 NavigateTo("ped_model_changer_animal");
		if (DrawOption("Ambient Females"))         NavigateTo("ped_model_changer_amb_females");
		if (DrawOption("Ambient Males"))           NavigateTo("ped_model_changer_amb_males");
		if (DrawOption("Cutscene Models"))         NavigateTo("ped_model_changer_cs");
		if (DrawOption("Gang Females"))            NavigateTo("ped_model_changer_gang_females");
		if (DrawOption("Gang Males"))              NavigateTo("ped_model_changer_gang_males");
		if (DrawOption("Story Models"))            NavigateTo("ped_model_changer_story");
		if (DrawOption("Multiplayer Models"))      NavigateTo("ped_model_changer_mp");
		if (DrawOption("Scenario Females"))        NavigateTo("ped_model_changer_scenario_females");
		if (DrawOption("Scenario Males"))          NavigateTo("ped_model_changer_scenario_males");
		if (DrawOption("Story Scenario Females"))  NavigateTo("ped_model_changer_st_scenario_females");
		if (DrawOption("Story Scenario Males"))    NavigateTo("ped_model_changer_st_scenario_males");
		if (DrawOption("Others"))                  NavigateTo("ped_model_changer_others");
	}
}

// ============================================================
// BodyguardListSubmenu (sub::BodyguardMenu::BodyguardList)
// ============================================================

void BodyguardListSubmenu::Draw()
{
	using namespace sub::BodyguardMenu;

	DrawTitle();

	if (BodyguardDb.empty())
	{
		DrawOption("No bodyguards spawned");
		return;
	}

	Engine* engine = Engine::Current();
	BodyguardEntity* pBodyguardToDelete = nullptr;

	for (unsigned int i = 0; i < BodyguardDb.size(); ++i)
	{
		auto& bg = BodyguardDb[i];
		if (!bg.Handle.Exists())
			continue;

		const std::string label = !bg.Name.empty() ? bg.Name : bg.HashName;

		if (DrawOption(label))
		{
			SelectedBodyguard = &bg;
			NavigateTo("bodyguard_entity_ops");
		}

		if (IsCurrentRowSelected())
		{
			if (bg.Handle.Exists())
				ENTITY::SET_ENTITY_HAS_GRAVITY(bg.Handle.GetHandle(), true);
			BodyguardManagement::ShowArrowAboveEntity(bg.Handle);

			if (Menu::bitController)
			{
				if (engine) engine->AddInstructionalButton(INPUT_SCRIPT_RLEFT, "Delete Bodyguard", false);
				if (IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_SCRIPT_RLEFT))
					pBodyguardToDelete = &bg;
			}
			else
			{
				if (engine) engine->AddInstructionalButton(static_cast<int>(VirtualKey::B), "Delete Bodyguard", true);
				if (IsKeyJustUp(VirtualKey::B))
					pBodyguardToDelete = &bg;
			}
		}
	}

	if (pBodyguardToDelete)
		BodyguardManagement::DeleteBodyguard(*pBodyguardToDelete);
}

// ============================================================
// BodyguardSettingsSubmenu (sub::BodyguardMenu::BodyguardOps_)
// ============================================================

void BodyguardSettingsSubmenu::Draw()
{
	// Legacy body was just AddTitle("Bodyguard Settings"); — empty page.
	DrawTitle();
}

// ============================================================
// BodyguardEntityOpsSubmenu (sub::BodyguardMenu::BodyguardEntityOps)
// ============================================================

void BodyguardEntityOpsSubmenu::Draw()
{
	using namespace sub::BodyguardMenu;
	using sub::g_cam_componentChanger;

	// Dynamic title: prefer Name, else HashName, else hex model, else missing.
	std::string title = "Bodyguard";
	if (SelectedBodyguard)
	{
		if (SelectedBodyguard->Handle.Exists())
		{
			if (!SelectedBodyguard->Name.empty())
				title = SelectedBodyguard->Name;
			else if (!SelectedBodyguard->HashName.empty())
				title = SelectedBodyguard->HashName;
			else
				title = IntToHexString(SelectedBodyguard->Handle.Model().hash, true);
		}
		else
		{
			title = "Bodyguard (missing)";
		}
	}
	Engine* engine = Engine::Current();
	if (engine) engine->AddTitle(title);

	if (!SelectedBodyguard)
	{
		DrawOption("No bodyguard selected");
		return;
	}
	if (!SelectedBodyguard->Handle.Exists())
	{
		DrawOption("Bodyguard no longer exists");
		return;
	}

	// "Wardrobe" — sets g_Ped1 to the selected bodyguard then routes to
	// the components page. Legacy also tore down g_cam_componentChanger
	// before nav.
	if (DrawOption("Wardrobe"))
	{
		g_Ped1 = SelectedBodyguard->Handle.Handle();
		if (g_cam_componentChanger.Exists())
		{
			g_cam_componentChanger.SetActive(false);
			g_cam_componentChanger.Destroy();
			World::SetRenderingCamera(0);
		}
		NavigateTo("ped_components");
	}

	if (DrawOption("Voice Changer"))
	{
		g_Ped1 = SelectedBodyguard->Handle.Handle();
		NavigateTo("ped_voice_changer");
	}

	if (DrawOption("Weapons"))
	{
		NavigateTo("bodyguard_weapon_ops");
	}

	if (DrawOption("Loadouts"))
	{
		// Mirrors legacy BodyguardWeaponLoadoutOps: set weapon-ops overrides
		// so the loadouts page operates on the bodyguard ped, then nav.
		Ped ped = SelectedBodyguard->Handle.GetHandle();
		g_WeaponOpsPedOverride = ped;
		g_WeaponOpsPlayerOverride = -1;
		g_WeaponMenuPedOverride = ped;
		g_Ped1 = ped;
		g_Ped2 = -1;
		NavigateTo("weapon_loadouts");
	}
}

// ============================================================
// BodyguardWeaponOpsSubmenu (sub::BodyguardMenu::BodyguardWeaponOps)
// ============================================================
//
// Legacy was a per-frame wrapper: set overrides, call Sub_CategoriesList()
// inline, clear overrides. The port does the same by setting the overrides each
// frame while this submenu is the active one and forwarding navigation to
// "weapon_categories" on first frame. The override is read by the
// weapon submenus directly (see WeaponSubmenu::Draw), so re-asserting it
// each frame is the safe path.

void BodyguardWeaponOpsSubmenu::Draw()
{
	using namespace sub::BodyguardMenu;

	if (!SelectedBodyguard || !SelectedBodyguard->Handle.Exists())
	{
		Engine* engine = Engine::Current();
		if (engine) engine->GoBack();
		return;
	}

	Ped ped = SelectedBodyguard->Handle.GetHandle();
	g_WeaponOpsPedOverride = ped;
	g_WeaponOpsPlayerOverride = -1;
	g_WeaponMenuPedOverride = ped;

	// Redirect to the weapon categories list. Pop ourselves off the nav
	// stack first so Back from the weapon list returns to the entity ops
	// menu, not into this redirector (which would loop).
	Engine* engine = Engine::Current();
	if (!engine) return;
	engine->GoBack();
	engine->NavigateTo("weapon_categories");
}

}
REGISTER_SUBMENU(::Menu::BodyguardSubmenu)
REGISTER_SUBMENU(::Menu::BodyguardMenuSubmenu)
REGISTER_SUBMENU(::Menu::BodyguardSpawnSubmenu)
REGISTER_SUBMENU(::Menu::BodyguardListSubmenu)
REGISTER_SUBMENU(::Menu::BodyguardSettingsSubmenu)
REGISTER_SUBMENU(::Menu::BodyguardEntityOpsSubmenu)
REGISTER_SUBMENU(::Menu::BodyguardWeaponOpsSubmenu)
