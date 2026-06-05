#include "Player.h"

#include "../Menu/Engine.h"
#include "../Menu/SubmenuRegistry.h"
#include "../Menu/Menu.h"
#include "../Menu/Routine.h"
#include "PlayerRuntime.h"
#include "Misc.h"              // dict2

#include "../Natives/natives2.h"
#include "../Util/keyboard.h"
#include "../Util/FileLogger.h"
#include "../Scripting/Camera.h"
#include "../Scripting/Game.h"
#include "../Scripting/World.h"
#include "../Scripting/GTAped.h"
#include "../Scripting/GTAplayer.h"
#include "../Scripting/enums.h"
#include "../Memory/GTAmemory.h"

#include "../Misc/SmashAbility.h"

#include "PedComponentRuntime.h"
#include "Spooner/SpoonerEntity.h"
#include "Spooner/EntityManagement.h"

#include <string>
#include <vector>

namespace Menu {

void PlayerOpsSubmenu::Draw()
{
	DrawTitle();

	dict2.clear();

	g_Ped1 = PLAYER_PED_ID();
	g_Ped2 = PLAYER_ID();

	GTAped myPed = g_Ped1;
	GTAplayer myPlayer = g_Ped2;
	float& height = g_playerVerticalElongationMultiplier;

	Engine* engine = Engine::Current();

	if (DrawOption("Model Changer"))   NavigateTo("ped_model_changer");
	if (DrawOption("Wardrobe"))
	{
		if (sub::g_cam_componentChanger.Exists())
		{
			sub::g_cam_componentChanger.SetActive(false);
			sub::g_cam_componentChanger.Destroy();
			World::SetRenderingCamera(0);
		}
		NavigateTo("ped_components");
	}
	if (DrawOption("Animations"))       NavigateTo("ped_animation");
	if (DrawOption("Scenario Actions")) NavigateTo("ped_animation_task_scenarios");
	if (DrawOption("Moods"))            NavigateTo("ped_animation_facial_mood");
	if (DrawOption("Movement Styles"))  NavigateTo("ped_animation_movement_group");
	if (DrawOption("Speech Player"))    NavigateTo("ped_speech_player");
	if (DrawOption("Voice Changer"))    NavigateTo("ped_voice_changer");
	if (DrawOption("Ped Flags"))        NavigateTo("player_ped_flags");
	if (DrawOption("TriggerFX"))        NavigateTo("ptfx");
	if (DrawOption("Breathe Stuff"))    NavigateTo("breathe_stuff");
	if (DrawOption("Ghost Rider Mode")) NavigateTo("ghost_rider_mode");

	if (DrawOption("Cloning Options"))  NavigateTo("player_clone_companion");

	if (DrawOption("Replenish Player"))
	{
		addlog(ige::LogType::LOG_TRACE, "Replenishing Player");
		myPed.SetHealth(myPed.GetMaxHealth());
		myPed.SetArmour(myPlayer.MaxArmour_get());
		sub::PedDamageTextures::ClearAllBloodDamage(myPed);
		sub::PedDamageTextures::ClearAllVisibleDamage(myPed);
		return;
	}

	DrawToggle("Refill Health When In Cover", selfRefillHealthInCover);
	if (DrawToggle("Invincibility", playerInvincibility) && !playerInvincibility)
	{
		addlog(ige::LogType::LOG_TRACE, "Turning Off Invincibility");
		SET_PLAYER_INVINCIBLE(PLAYER_ID(), 0);
		SetPedInvincibleOff(PLAYER_PED_ID());
		return;
	}

	if (DrawToggleExternal("Invisibility", !myPed.IsVisible()))
	{
		addlog(ige::LogType::LOG_TRACE, "Turning Off Invisibility");
		myPed.SetVisible(!myPed.IsVisible());
	}

	if (DrawToggle("No Ragdoll", playerNoRagdoll) && !playerNoRagdoll)
	{
		addlog(ige::LogType::LOG_TRACE, "Turning Off No Ragdoll");
		SetPedNoRagdollOff(PLAYER_PED_ID());
		return;
	}
	if (DrawToggle("Seatbelt", playerSeatbelt) && !playerSeatbelt)
	{
		addlog(ige::LogType::LOG_TRACE, "Turning Off Seatbelt");
		SetPedSeatbeltOff(PLAYER_PED_ID());
		return;
	}

	DrawToggle("Unlimited Special Ability (SP)", playerUnlimitedAbility);
	DrawToggle("Auto-Clean", playerAutoClean);
	DrawToggle("Super Run", superRun);
	DrawToggle("Super Jump", superJump);
	DrawToggle("Walk underwater", playerWalkUnderwater);

	{
		const std::vector<std::string> forceFieldNames{ "Off", "Push Out", "Destroy" };
		int idx = forceField;
		const ::Menu::InputResult res = engine
			? engine->AddTextList("Forcefield", idx, forceFieldNames)
			: ::Menu::InputResult{};
		if (res.rightPressed && forceField < static_cast<unsigned char>(forceFieldNames.size() - 1))
			++forceField;
		else if (res.leftPressed && forceField > 0)
			--forceField;
	}

	if (DrawToggleExternal("Smash Ability", SmashAbility::g_smashAbility.Enabled()))
		SmashAbility::ToggleOnOff();

	if (DrawToggle("Fly Manual", superman) && superman)
	{
		if (Menu::bitController)
			Game::Print::PrintBottomLeft("RT for Up. LT for Down. RB for Boost. A for Brake.");
		else
			Game::Print::PrintBottomLeft("Num7 for Up. Num1 for Down. NumPlus for Boost. NumMinus for Brake.");
		return;
	}

	if (DrawToggle("Fly Auto", supermanAuto) && supermanAuto)
	{
		Vector3 pos = GET_ENTITY_COORDS(g_Ped1, 1);
		CREATE_AMBIENT_PICKUP(PICKUP_PARACHUTE, pos.x, pos.y, pos.z, 0, 300, 1, 0, 1);
		TASK_PARACHUTE(g_Ped1, true, false);
		APPLY_FORCE_TO_ENTITY(PLAYER_PED_ID(), 1, 0.0f, 0.0f, 10.0f, 0.0, 0.0, 0.0, 1, 1, 1, 1, 0, 1);
		if (Menu::bitController)
			Game::Print::PrintBottomLeft("Press ~b~A~s~ for temporary brake.");
		else
			Game::Print::PrintBottomLeft("Press ~b~NUMPLUS~s~ for temporary brake.");
		return;
	}

	if (DrawToggle("Ignored By Everyone", ignoredByEveryone) && !ignoredByEveryone)
	{
		addlog(ige::LogType::LOG_TRACE, "Ignore By Everyone Off");
		Player temp = PLAYER_ID();
		SET_POLICE_IGNORE_PLAYER(temp, neverWanted);
		SET_EVERYONE_IGNORE_PLAYER(temp, 0);
		SET_PLAYER_CAN_BE_HASSLED_BY_GANGS(temp, 1);
		SET_IGNORE_LOW_PRIORITY_SHOCKING_EVENTS(temp, 0);
		return;
	}

	int wantedLevel = myPlayer.GetWantedLevel();
	if (DrawNumber("Wanted Level", wantedLevel, 1, 0, 6))
	{
		addlog(ige::LogType::LOG_TRACE, "Changed Wanted Level");
		SET_MAX_WANTED_LEVEL(6);
		SET_PLAYER_WANTED_LEVEL(PLAYER_ID(), wantedLevel, 0);
		SET_PLAYER_WANTED_LEVEL_NOW(PLAYER_ID(), 0);
		neverWanted = false;
		if (selfFreezeWantedLevel != 0)
			selfFreezeWantedLevel = static_cast<unsigned char>(wantedLevel);
		return;
	}

	if (wantedLevel > 0)
	{
		if (DrawToggleExternal("Freeze Wanted Level", selfFreezeWantedLevel != 0))
		{
			if (selfFreezeWantedLevel != 0)
			{
				addlog(ige::LogType::LOG_TRACE, "Freeze Wanted Level Off");
				selfFreezeWantedLevel = 0;
			}
			else
			{
				addlog(ige::LogType::LOG_TRACE, "Freeze Wanted Level On");
				selfFreezeWantedLevel = static_cast<unsigned char>(Game::Player().GetWantedLevel());
			}
		}
	}
	else
	{
		// Never Wanted toggle (on AND off side effects).
		if (DrawToggle("Never Wanted", neverWanted))
		{
			if (neverWanted)
			{
				addlog(ige::LogType::LOG_TRACE, "Never Wanted On");
				SET_PLAYER_WANTED_LEVEL(PLAYER_ID(), 0, 0);
				SET_PLAYER_WANTED_LEVEL_NOW(PLAYER_ID(), 0);
			}
			else
			{
				addlog(ige::LogType::LOG_TRACE, "Never Wanted Off");
				SET_MAX_WANTED_LEVEL(6);
				SET_WANTED_LEVEL_MULTIPLIER(1.0f);
			}
			return;
		}
	}

	if (DrawToggle("Burn Mode", playerBurn))
	{
		if (playerBurn)
		{
			addlog(ige::LogType::LOG_TRACE, "Burn Mode On");
			if (GET_PLAYER_INVINCIBLE(g_Ped2))
				SET_PLAYER_INVINCIBLE(g_Ped2, 0);
			SetPedInvincibleOff(g_Ped1);
			WAIT(130);
			if (!IS_ENTITY_ON_FIRE(g_Ped1))
				START_ENTITY_FIRE(g_Ped1);
			Game::Print::PrintBottomCentre("~b~Note:~s~ If you're not on fire yet, kill yourself.");
		}
		else
		{
			addlog(ige::LogType::LOG_TRACE, "Burn Mode Off");
			if (IS_ENTITY_ON_FIRE(g_Ped1))
				STOP_ENTITY_FIRE(g_Ped1);
		}
		return;
	}

	if (DrawNumber("Height (Elongation) - Experimental", height, 0.1f, 2, -2.5f, 2.5f))
	{
		addlog(ige::LogType::LOG_TRACE, "Changed Height");
		GeneralGlobalHax::SetPlayerHeight(height);
	}

	DrawNumber("Movement Speed (Alt)", swimSpeedMult, 0.1f, 2, 0.0f, 1.4f);

	if (DrawNumber("Sweat Level", selfSweatMult, 0.1f, 2, 0.0f, 5.5f))
	{
		if (selfSweatMult == 0.0f)
		{
			SET_PED_SWEAT(g_Ped1, selfSweatMult);
			CLEAR_PED_WETNESS(g_Ped1);
		}
	}

	DrawNumber("Noise Level", playerNoiseMult, 0.1f, 2, 0.0f, 10.0f);

	if (DrawToggleExternal("Collision", myPed.GetIsCollisionEnabled()))
		myPed.SetIsCollisionEnabled(!myPed.GetIsCollisionEnabled());
}

void CloneCompanionSubmenu::Draw()
{
	DrawTitle();

	GTAplayer player = g_Ped2;
	GTAped playerPed = g_Ped1;

	if (!playerPed.Exists())
	{
		Game::Print::PrintBottomCentre("~r~Error:~s~ No longer in memory.");
		addlog(ige::LogType::LOG_WARNING, "Cannot start clone menu, playerPed No longer in memory");
		Engine* engine = Engine::Current();
		if (engine) engine->GoBack();
		return;
	}

	if (DrawOption("Clone As Neutral"))
	{
		GTAped clone = playerPed.Clone(playerPed.GetHeading(), true, true);
		auto cloneNetId = clone.NetID();
		Game::RequestControlOfId(cloneNetId);
		SET_NETWORK_ID_CAN_MIGRATE(cloneNetId, true);
		SET_NETWORK_ID_EXISTS_ON_ALL_MACHINES(cloneNetId, true);
		clone.SetBlockPermanentEvent(true);
		SET_PED_COMBAT_ABILITY(cloneNetId, 100);
		clone.SetMissionEntity(true);
		clone.NoLongerNeeded();

		Game::Print::PrintBottomLeft(oss_ << "Cloned ~b~" << player.GetName());
	}

	if (DrawOption("Clone As Companion (7 Max)"))
	{
		GTAped clone = playerPed.Clone(playerPed.GetHeading(), true, true);
		clone.RequestControl(300);
		auto cloneNetId = clone.NetID();
		Game::RequestControlOfId(cloneNetId);
		SET_NETWORK_ID_CAN_MIGRATE(cloneNetId, true);
		SET_NETWORK_ID_EXISTS_ON_ALL_MACHINES(cloneNetId, true);
		SET_PED_COMBAT_ABILITY(cloneNetId, 200);
		Hash weaponToGive = WEAPON_APPISTOL;
		GIVE_DELAYED_WEAPON_TO_PED(clone.Handle(), weaponToGive, 9999, true);
		clone.SetWeapon(weaponToGive);
		clone.SetFiringPattern(FiringPattern::FullAuto);
		clone.SetShootRate(100);

		sub::Spooner::SpoonerEntity cloneEnt;
		cloneEnt.dynamic = true;
		cloneEnt.handle = clone;
		cloneEnt.hashName = player.GetName() + "\'s Clone Companion";
		cloneEnt.isStill = false;
		cloneEnt.type = EntityType::PED;
		sub::Spooner::EntityManagement::AddEntityToDb(cloneEnt);

		PedGroup grp;
		if (playerPed.IsInGroup())
		{
			grp = playerPed.GetCurrentPedGroup();
		}
		else
		{
			grp = PedGroup::CreateNewGroup();
			grp.Add(playerPed, true);
		}
		grp.Add(clone, false);
		grp.SetSeparationRange(100.0f);
		grp.SetFormationSpacing(1.5f);

		Game::Print::PrintBottomLeft(oss_ << "Cloned ~b~" << player.GetName() << "~s~ and made the clone "
			<< (playerPed.GetGender() == Gender::Female ? "her" : "his") << " companion.");
		Game::Print::PrintBottomLeft("Clone added to Spooner Database as a persistent entity.");
	}

	if (DrawOption("Clone As Enemy"))
	{
		GTAped clone = playerPed.Clone(playerPed.GetHeading(), true, true);
		clone.RequestControl(300);
		auto cloneNetId = clone.NetID();
		Game::RequestControlOfId(cloneNetId);
		SET_NETWORK_ID_CAN_MIGRATE(cloneNetId, true);
		SET_NETWORK_ID_EXISTS_ON_ALL_MACHINES(cloneNetId, true);
		clone.SetBlockPermanentEvent(true);
		SET_PED_COMBAT_ABILITY(cloneNetId, 100);
		Hash weaponToGive = WEAPON_APPISTOL;
		GIVE_DELAYED_WEAPON_TO_PED(clone.Handle(), weaponToGive, 9999, true);
		clone.SetWeapon(weaponToGive);
		clone.SetFiringPattern(FiringPattern::FullAuto);
		clone.SetShootRate(100);

		sub::Spooner::SpoonerEntity cloneEnt;
		cloneEnt.dynamic = true;
		cloneEnt.handle = clone;
		cloneEnt.hashName = player.GetName() + "\'s Clone Enemy";
		cloneEnt.isStill = false;
		cloneEnt.type = EntityType::PED;
		sub::Spooner::EntityManagement::AddEntityToDb(cloneEnt);
		TASK_COMBAT_PED(clone.Handle(), playerPed.Handle(), 0, 16);

		TaskSequence squ;
		TASK_COMBAT_PED_TIMED(0, playerPed.Handle(), 0, 10000);
		TASK_SHOOT_AT_ENTITY(0, playerPed.Handle(), 10000, FiringPattern::FullAuto);
		TASK_PUT_PED_DIRECTLY_INTO_MELEE(0, playerPed.Handle(), 0.0f, -1.0f, 0.0f, 0);
		TASK_COMBAT_HATED_TARGETS_AROUND_PED(0, 25.0f, 0);
		squ.Close(true);
		squ.MakePedPerform(clone);
		clone.SetAlwaysKeepTask(true);
		squ.Clear();

		Game::Print::PrintBottomLeft(oss_ << "Cloned ~b~" << player.GetName() << "~s~ and made the clone "
			<< (playerPed.GetGender() == Gender::Female ? "her" : "his") << " enemy.");
		Game::Print::PrintBottomLeft("Clone added to Spooner Database as a persistent entity.");
	}
}

void PedFlagsListSubmenu::Draw()
{
	DrawTitle();

	GTAped ped = g_Ped1;

	if (DrawOption("Custom"))
	{
		NavigateTo("player_ped_flags_custom");
	}

	for (auto& f : sub::PedConfigFlagManager::pedFlags)
	{
		const BOOL flagStatus = GET_PED_CONFIG_FLAG(ped.Handle(), f.id, true);
		if (DrawToggleExternal(f.title, flagStatus != 0))
		{
			SET_PED_CONFIG_FLAG(ped.Handle(), f.id, !flagStatus);
		}
	}
}

void PedFlagsCustomSubmenu::Draw()
{
	DrawTitle();

	GTAped ped = g_Ped1;
	int& flagID = sub::PedConfigFlagManager::flagID;
	const BOOL flagStatus = GET_PED_CONFIG_FLAG(ped.Handle(), flagID, true);

	Engine* engine = Engine::Current();

	// ID — int field; accept opens a text input box (legacy customInput).
	const bool idChanged = DrawNumber("ID", flagID, 1, 0, INT_MAX);
	(void)idChanged;

	if (engine && IsCurrentRowSelected()
		&& MenuPressTimer::IsButtonTapped(MenuPressTimer::Button::Accept))
	{
		std::string inputStr = Game::InputBox("", 10U);
		if (inputStr.length() > 0)
		{
			try
			{
				flagID = std::stoi(inputStr);
			}
			catch (...)
			{
				Game::Print::PrintErrorInvalidInput(inputStr);
				addlog(ige::LogType::LOG_ERROR, "Invalid flagID entered: " + inputStr);
			}
		}
	}

	if (DrawToggleExternal("Status", flagStatus != 0))
	{
		SET_PED_CONFIG_FLAG(ped.Handle(), flagID, !flagStatus);
	}
}

}
REGISTER_SUBMENU(::Menu::PlayerOpsSubmenu)
REGISTER_SUBMENU(::Menu::CloneCompanionSubmenu)
REGISTER_SUBMENU(::Menu::PedFlagsListSubmenu)
REGISTER_SUBMENU(::Menu::PedFlagsCustomSubmenu)

namespace sub
{
	namespace PedConfigFlagManager
	{
		std::vector<NamedPedFlagS> pedFlags
		{
			{ ePedConfigFlags::WillFlyThruWindscreen, "Can Fall Out Through Windscreen" },
			{ ePedConfigFlags::InVehicle, "Is (Nearby) Car (MAY CRASH)" },
			{ ePedConfigFlags::IsAimingGun, "Is Aiming Check" },
			{ ePedConfigFlags::ForcedAim, "Is Aiming" },
			{ ePedConfigFlags::_0x5FED6BFD, "Has Overflowing Diaper (MAY CRASH)" },
			{ ePedConfigFlags::IsInjured, "Is Injured" },
			{ ePedConfigFlags::HasHurtStarted, "Is Injured In Combat" },
			{ ePedConfigFlags::_Shrink, "Is Short Heighted (Small)" },
		};

		int flagID = 0;
	}
}
