#include "Vehicle.h"

#include "../Menu/Engine.h"
#include "../Menu/SubmenuRegistry.h"
#include "../Menu/Menu.h"
#include "../Menu/Routine.h"
#include "PlayerRuntime.h"
#include "VehicleRuntime.h"
#include "VehicleSpawnerRuntime.h"
#include "VehicleModShopRuntime.h"

#include "../Natives/natives2.h"
#include "../Scripting/enums.h"
#include "../Scripting/Game.h"
#include "../Scripting/World.h"
#include "../Scripting/Model.h"
#include "../Scripting/GTAvehicle.h"
#include "../Scripting/GTAped.h"
#include "../Scripting/GTAplayer.h"
#include "../Scripting/GTAprop.h"
#include "../Scripting/GTAblip.h"
#include "../Util/GTAmath.h"
#include "../Util/FileLogger.h"

#include "../Misc/VehicleCruise.h"
#include "../Misc/VehicleTow.h"
#include "../Misc/VehicleFly.h"

#include "../Scripting/Raycast.h"
#include "../Scripting/DxHookIMG.h"
#include "../Util/ExePath.h"

#include <math.h>
#include <vector>
#include <string>

namespace Menu {

void VehicleSubmenu::Draw()
{
	DrawTitle();

	g_Ped1 = PLAYER_PED_ID();
	g_Ped2 = PLAYER_ID();

	GTAped myPed = g_Ped1;
	GTAvehicle myVehicle = g_myVeh;
	const Model& myVehicleModel = myVehicle.Model();
	const bool myPedIsInVehicle = myPed.IsInVehicle();

	Engine* engine = Engine::Current();

	if (myPedIsInVehicle && engine)
	{
		static const std::vector<std::string> fixOptions{
			"Full", "Keep Dirt", "Keep windows open", "Keep windows open with Dirt"
		};
		int idx = fixCarTexterValue;
		const ::Menu::InputResult res = engine->AddTextList("CMOD_MOD_MNT", idx, fixOptions, /*gxt=*/true);

		if (res.rightPressed && idx < static_cast<int>(fixOptions.size()) - 1)
			++idx;
		else if (res.leftPressed && idx > 0)
			--idx;
		fixCarTexterValue = idx;

		if (res.accepted)
		{
			if (!IS_PED_IN_ANY_VEHICLE(PLAYER_PED_ID(), 0))
			{
				Game::Print::PrintBottomCentre("~r~Error:~s~ You are not in a vehicle.");
			}
			else
			{
				std::vector<VehicleWindow> windowsToOpen;
				if (fixCarTexterValue == 1 || fixCarTexterValue == 3)
				{
					for (int i = static_cast<int>(VehicleWindow::FrontLeftWindow);
						i < static_cast<int>(VehicleWindow::Last); ++i)
					{
						if (!myVehicle.IsWindowIntact(static_cast<VehicleWindow>(i)))
							windowsToOpen.push_back(static_cast<VehicleWindow>(i));
					}
				}

				myVehicle.RequestControlOnce();
				SET_VEHICLE_FIXED(g_myVeh);
				if (fixCarTexterValue == 0 || fixCarTexterValue == 2)
					SET_VEHICLE_DIRT_LEVEL(g_myVeh, 0.0f);
				SET_VEHICLE_ENGINE_CAN_DEGRADE(g_myVeh, 0);
				SET_VEHICLE_ENGINE_HEALTH(g_myVeh, 1250.0f);
				SET_VEHICLE_PETROL_TANK_HEALTH(g_myVeh, 1250.0f);
				SET_VEHICLE_BODY_HEALTH(g_myVeh, 1250.0f);
				SET_VEHICLE_UNDRIVEABLE(g_myVeh, 0);
				if (!GET_IS_VEHICLE_ENGINE_RUNNING(g_myVeh))
					SET_VEHICLE_ENGINE_ON(g_myVeh, 1, 1, 0);

				if (fixCarTexterValue == 1 || fixCarTexterValue == 3)
				{
					for (VehicleWindow w : windowsToOpen)
						myVehicle.RollDownWindow(w);
				}
			}
			return;
		}
	}

	if (DrawOption("Vehicle Spawner")) NavigateTo("vehicle_spawner");

	if (DrawOption("Menyoo Customs"))
	{
		if (DOES_ENTITY_EXIST(g_myVeh))
		{
			g_Ped4 = g_myVeh;
			NavigateTo("vehicle_modshop");
		}
		else
		{
			Game::Print::PrintBottomCentre("~r~Error:~s~ You are not in a vehicle.");
		}
	}

	if (myVehicleModel.IsCargobob())
	{
		if (DrawToggleExternal("Cargobob Magnet", myVehicle.IsCargobobHookActive(CargobobHook::Magnet)))
		{
			if (myVehicle.IsCargobobHookActive(CargobobHook::Magnet))
			{
				myVehicle.RequestControl();
				myVehicle.RetractCargobobHook();
				WAIT(50);
				myVehicle.DropCargobobHook(CargobobHook::Hook);
			}
			else
			{
				myVehicle.RequestControl();
				myVehicle.DropCargobobHook(CargobobHook::Magnet);
				myVehicle.CargoBobMagnetGrabVehicle();
			}
		}
	}

	if (DOES_VEHICLE_ALLOW_RAPPEL(g_myVeh))
	{
		if (DrawOption("Rappel From Helicopter") && myPed.IsInVehicle())
		{
			sub::TaskRappel(myPed.Handle(), myPed.CurrentVehicle());
			return;
		}
	}

	if (myVehicle.GetHasSiren())
	{
		if (DrawToggle("Disable Vehicle Siren", vehicleDisableSiren) && !vehicleDisableSiren)
			SET_VEHICLE_HAS_MUTED_SIRENS(g_myVeh, FALSE);
	}

	if (myVehicleModel.IsBoat())
	{
		if (DrawToggleExternal("Anchor Boat", myVehicle.IsBoatAnchored()))
			myVehicle.AnchorBoat(!myVehicle.IsBoatAnchored());
	}

	if (DrawOption("Teleport Into Closest Vehicle"))
	{
		const GTAvehicle& closest = World::GetClosestVehicle(myPed.GetPosition(), FLT_MAX);
		if (closest.Exists())
		{
			myPed.SetIntoVehicle(closest, closest.FirstFreeSeat(SEAT_DRIVER));
		}
		return;
	}

	if (DrawOption("Vehicle Weapons"))      NavigateTo("vehicle_weapons");
	if (DrawOption("Vehicle Multipliers"))  NavigateTo("vehicle_multipliers");
	if (DrawOption("Speedometers"))         NavigateTo("vehicle_speedos");
	if (DrawOption("Multi-Platform Neons")) NavigateTo("vehicle_neons_multi");
	if (DrawOption("PV Options"))           NavigateTo("vehicle_pv");
	if (DrawOption("Auto Drive"))           NavigateTo("vehicle_auto_drive");

	if (DrawOption(std::string("Slam It (") + Game::GetGXTEntry("CMOD_MOD_22_D") + ")"))
	{
		sub::VehicleSlam::InitSub(g_myVeh, &vehicleSlam);
		NavigateTo("vehicle_slam");
		Game::Print::PrintBottomCentre("~b~Note:~s~ If you try hard enough, you can drive on walls too!");
	}

	if (DrawNumber("Damage & Defense", vehicleDamageAndDefense, 0.2f, 2))
	{
		SET_PLAYER_VEHICLE_DAMAGE_MODIFIER(PLAYER_ID(), vehicleDamageAndDefense);
		SET_PLAYER_VEHICLE_DEFENSE_MODIFIER(PLAYER_ID(), vehicleDamageAndDefense);
		return;
	}
	if (engine && IsCurrentRowSelected()
		&& MenuPressTimer::IsButtonTapped(MenuPressTimer::Button::Accept))
	{
		std::string inputStr = Game::InputBox("", 10U);
		if (!inputStr.empty())
		{
			try
			{
				vehicleDamageAndDefense = std::stof(inputStr);
				SET_PLAYER_VEHICLE_DAMAGE_MODIFIER(PLAYER_ID(), vehicleDamageAndDefense);
				SET_PLAYER_VEHICLE_DEFENSE_MODIFIER(PLAYER_ID(), vehicleDamageAndDefense);
			}
			catch (...) { /* leave value unchanged */ }
		}
	}

	if (DrawToggle("Invincibility (Looped)", vehicleInvincibility) && !vehicleInvincibility)
	{
		if (IS_PED_IN_ANY_VEHICLE(PLAYER_PED_ID(), 0))
		{
			myVehicle.RequestControlOnce();
			SetVehicleInvincibleOff(g_myVeh);
		}
		return;
	}

	if (DrawToggleExternal("Invisibility", !myVehicle.IsVisible()))
	{
		if (!IS_PED_IN_ANY_VEHICLE(PLAYER_PED_ID(), 0))
		{
			Game::Print::PrintBottomCentre("~r~Error:~s~ You are not in a vehicle.");
		}
		else
		{
			myVehicle.RequestControlOnce();
			myVehicle.SetVisible(true);
		}
		return;
	}

	DrawToggle("Auto-Repair", vehicleFixLoop);
	DrawToggle("Auto-Flip", vehicleFlipLoop);
	DrawToggle("Keep Engine & Lights On", selfEngineOn);

	if (engine && engine->AddCheckbox("Kill Engine",
		GET_VEHICLE_ENGINE_HEALTH(g_myVeh) < 0.0f,
		Checkbox::PERCENTAGESTICKER, Checkbox::NONE))
	{
		if (!IS_PED_IN_ANY_VEHICLE(PLAYER_PED_ID(), 0))
		{
			Game::Print::PrintBottomCentre("~r~Error:~s~ You are not in a vehicle.");
		}
		else if (GET_VEHICLE_ENGINE_HEALTH(g_myVeh) > 0.0f)
		{
			myVehicle.RequestControlOnce();
			SET_VEHICLE_ENGINE_HEALTH(g_myVeh, -1.0f);
			SET_VEHICLE_UNDRIVEABLE(g_myVeh, 1);
			Game::Print::PrintBottomCentre("Engine Killed");
		}
		else
		{
			myVehicle.RequestControlOnce();
			SET_VEHICLE_ENGINE_HEALTH(g_myVeh, 1250.0f);
			SET_VEHICLE_UNDRIVEABLE(g_myVeh, 0);
			Game::Print::PrintBottomCentre("Engine Revived");
		}
		return;
	}

	DrawToggle("Rainbow Mode", carColorChange);

	if (DrawToggleExternal("Slidy Tyres", bitVehicleSlippyTires))
	{
		if (!IS_PED_IN_ANY_VEHICLE(PLAYER_PED_ID(), 0))
		{
			Game::Print::PrintBottomCentre("~r~Error:~s~ You are not in a vehicle.");
		}
		else
		{
			bitVehicleSlippyTires = !bitVehicleSlippyTires;
			myVehicle.RequestControlOnce();
			SET_VEHICLE_REDUCE_GRIP(g_myVeh, bitVehicleSlippyTires);
		}
		return;
	}

	if (DrawToggleExternal("Cruise Control", VehicleCruise::g_vehicleCruise.Enabled()))
		VehicleCruise::ToggleOnOff();
	if (DrawToggleExternal("Tow Mode (ALPHA)", VehicleTow::g_vehicleTow.Enabled()))
		VehicleTow::ToggleOnOff();

	if (DrawToggle("Race Boost On Horn", raceBoost) && raceBoost)
	{
		Game::Print::PrintBottomLeft("Use the horn for a boost.");
		return;
	}
	DrawToggle("Infinite Native Boost (for e.g. Oppressor)", unlimitedVehicleBoost);

	if (DrawToggle("SuprKar Mode", superCarMode))
	{
		if (superCarMode)
		{
			Game::Print::PrintBottomLeft("Use driving controls and handbrake.");
		}
		else
		{
			SET_VEHICLE_BOOST_ACTIVE(g_myVeh, 0);
		}
		return;
	}

	if (DrawToggleExternal("Fly Mode", VehicleFly::g_vehicleFly.Enabled()))
		VehicleFly::ToggleOnOff();

	DrawToggle("Glue to Ground", superGrip);

	{
		const std::vector<std::string> jumpOpts = Menu::bitController
			? std::vector<std::string>{ "Off", "Tap/Press A/X", "Hold A/X" }
			: std::vector<std::string>{ "Off", "Tap/Press Space", "Hold Space" };
		int carJumpIdx = carJump;
		const ::Menu::InputResult jres = engine
			? engine->AddTextList("Vehicle Jump", carJumpIdx, jumpOpts)
			: ::Menu::InputResult{};
		if (jres.rightPressed && carJump < 2) ++carJump;
		else if (jres.leftPressed && carJump > 0) --carJump;
	}

	if (DrawToggle("Hydraulics", carHydraulics) && carHydraulics)
	{
		Game::Print::PrintBottomLeft(oss_ "Use ~b~" << (Menu::bitController
			? "LS/L1 + stick movement" : "LeftShift + WASD") << "~s~ for hydraulics.");
		return;
	}

	if (DrawToggle("Drive On Water", driveOnWater))
	{
		if (driveOnWater)
		{
			Game::Print::PrintBottomLeft("You can drive and walk on water now!");
		}
		else
		{
			GTAprop(g_driveWaterObject).Delete(true);
		}
		return;
	}

	if (DrawToggle("Increased Mass", vehicleHeavyMass) && !vehicleHeavyMass)
	{
		if (myVehicle.Exists())
		{
			SetVehicleInvincibleOff(myVehicle.GetHandle());
			myVehicle.SetFrictionOverride(1.0f);
		}
		return;
	}

	// Child Locks / Door Locks — BOXTICK / BOXBLANK checkboxes.
	if (engine && engine->AddCheckbox("Child Locks",
		GET_VEHICLE_DOOR_LOCK_STATUS(g_myVeh) == 4,
		Checkbox::BOXTICK, Checkbox::BOXBLANK))
	{
		myVehicle.RequestControlOnce();
		SET_VEHICLE_DOORS_LOCKED(g_myVeh, GET_VEHICLE_DOOR_LOCK_STATUS(g_myVeh) == 4 ? 0 : 4);
		return;
	}
	if (engine && engine->AddCheckbox("Door Locks",
		GET_VEHICLE_DOOR_LOCK_STATUS(g_myVeh) == 2,
		Checkbox::BOXTICK, Checkbox::BOXBLANK))
	{
		myVehicle.RequestControlOnce();
		SET_VEHICLE_DOORS_LOCKED(g_myVeh, GET_VEHICLE_DOOR_LOCK_STATUS(g_myVeh) == 2 ? 0 : 2);
		return;
	}

	if (DrawToggleExternal("No Gravity", bitVehicleGravity))
	{
		if (!IS_PED_IN_ANY_VEHICLE(PLAYER_PED_ID(), 0))
		{
			Game::Print::PrintBottomCentre("~r~Error:~s~ You are not in a vehicle.");
		}
		else
		{
			myVehicle.RequestControlOnce();
			SET_VEHICLE_GRAVITY(g_myVeh, bitVehicleGravity ? 1 : 0);
			bitVehicleGravity = !bitVehicleGravity;
			Game::Print::PrintBottomCentre(bitVehicleGravity
				? "Vehicle Gravity ~g~Disabled" : "Vehicle Gravity ~r~Enabled");
		}
		return;
	}

	if (DrawToggleExternal("Freeze Vehicle", bitFreezeVehicle))
	{
		if (!IS_PED_IN_ANY_VEHICLE(PLAYER_PED_ID(), 0))
		{
			Game::Print::PrintBottomCentre("~r~Error:~s~ You are not in a vehicle.");
		}
		else
		{
			bitFreezeVehicle = !bitFreezeVehicle;
			myVehicle.RequestControlOnce();
			if (bitFreezeVehicle)
			{
				SET_VEHICLE_FORWARD_SPEED(g_myVeh, 0.0f);
				FREEZE_ENTITY_POSITION(g_myVeh, 1);
				Game::Print::PrintBottomCentre("Vehicle Freeze ~g~On");
			}
			else
			{
				SET_VEHICLE_FORWARD_SPEED(g_myVeh, 8.0f);
				FREEZE_ENTITY_POSITION(g_myVeh, 0);
				Game::Print::PrintBottomCentre("Vehicle Freeze ~r~Off");
			}
		}
		return;
	}

	if (DrawToggleExternal("Set on Fire", myVehicle.IsOnFire()))
	{
		if (!IS_PED_IN_ANY_VEHICLE(PLAYER_PED_ID(), 0))
		{
			Game::Print::PrintBottomCentre("~r~Error:~s~ You are not in a vehicle.");
		}
		else
		{
			if (!myVehicle.IsOnFire())
			{
				myVehicle.RequestControl(200);
				myVehicle.SetProofs(true, false, true, false, false, false, false, true);
				myVehicle.SetFireProof(false);
				WAIT(40);
				myVehicle.SetOnFire(true);
			}
			else
			{
				myVehicle.RequestControl(200);
				myVehicle.SetProofs(true, true, true, false, false, false, false, true);
				myVehicle.SetFireProof(true);
				WAIT(40);
				myVehicle.SetOnFire(false);
			}
		}
		return;
	}

	if (DrawToggleExternal("Collision", myVehicle.GetIsCollisionEnabled()))
		myVehicle.SetIsCollisionEnabled(!myVehicle.GetIsCollisionEnabled());

	if (DrawOption("Delete Vehicle"))
	{
		if (!IS_PED_IN_ANY_VEHICLE(PLAYER_PED_ID(), 0))
		{
			Game::Print::PrintBottomCentre("~r~Error:~s~ You are not in a vehicle.");
		}
		else
		{
			myVehicle.Delete(false);
			Game::Print::PrintBottomCentre("Poof!");
			if (engine) engine->currentOption = 1;
		}
		return;
	}
}

void PvSubmenu::Draw()
{
	DrawTitle();

	GTAped myPed = PLAYER_PED_ID();
	GTAvehicle myVehicle = g_myVeh;
	GTAvehicle& pv = pvSubVehicleID;

	const bool currentlySaved = (pv == myVehicle);
	if (DrawSelectionItem("Remember Vehicle", currentlySaved))
	{
		if (currentlySaved)
		{
			pv.SetMissionEntity(false);
			GTAblip blip = pv.CurrentBlip();
			if (blip.Exists() && blip.Colour() == 61)
				blip.Remove();
			pv = 0;
			return;
		}

		if (!myPed.IsInVehicle())
		{
			Game::Print::PrintBottomCentre("~r~Error:~s~ You are not in a vehicle.");
		}
		else
		{
			pv = myVehicle;
			pv.SetMissionEntity(true);

			GTAblip blip = pv.CurrentBlip();
			if (!blip.Exists())
			{
				blip = pv.AddBlip();
				if (blip.Exists())
				{
					Model model = pv.Model();
					BlipIcon::BlipIcon sprite = BlipIcon::PersonalVehicleCar;
					if (model.IsHeli())          sprite = BlipIcon::EnemyHelicopter;
					else if (model.IsPlane())    sprite = BlipIcon::Plane;
					else if (model.IsBoat())     sprite = BlipIcon::Boat;
					else if (model.IsBike())     sprite = BlipIcon::PersonalVehicleBike;
					else if (model.hash == VEHICLE_RHINO) sprite = BlipIcon::Tank;

					blip.SetIcon(sprite);
					blip.SetScale(0.7f);
					blip.SetColour(BlipColour::Pink);
					blip.SetFriendly(true);
				}
			}
		}
		return;
	}

	if (DrawOption("Teleport Into Seat"))
	{
		if (!pv.Exists())
			Game::Print::PrintBottomCentre("~r~Error:~s~ No longer in memory.");
		else
			myPed.SetIntoVehicle(pv, pv.FirstFreeSeat(SEAT_DRIVER));
		return;
	}

	if (DrawOption("Teleport to Vehicle"))
	{
		if (!pv.Exists())
		{
			Game::Print::PrintBottomCentre("~r~Error:~s~ No longer in memory.");
		}
		else
		{
			if (myPed.IsInVehicle())
			{
				myVehicle.SetPosition(pv.GetOffsetInWorldCoords(0, pv.Dim1().y + myVehicle.Dim2().y + 0.3f, 0));
			}
			else
			{
				myPed.SetPosition(pv.GetOffsetInWorldCoords(0, 0, pv.Dim2().z + myPed.Dim1().z));
			}
		}
		return;
	}

	if (DrawOption("Teleport Vehicle to Self"))
	{
		if (!pv.Exists())
		{
			Game::Print::PrintBottomCentre("~r~Error:~s~ No longer in memory.");
		}
		else
		{
			pv.RequestControl(600);
			pv.SetPosition(myPed.GetPosition());
		}
		return;
	}
}

void VehicleWeaponsSubmenu::Draw()
{
	DrawTitle();

	DrawToggle("Display Projectile Path", vehicleWeaponLines);

	if (DrawOption("Turn All Off"))
	{
		vehicleRPG = false;
		vehicleFireworks = false;
		vehicleGuns = false;
		vehicleSnowballs = false;
		vehicleBalls = false;
		vehicleWaterHydrant = false;
		vehicleFlameLeak = false;
		vehicleLaserGreen = false;
		vehicleLaserRed = false;
		vehicleTurretsValkyrie = false;
		vehicleFlaregun = false;
		vehicleHeavySniper = false;
		vehicleTazerWeapon = false;
		vehicleMolotovWeapon = false;
		vehicleCombatPDW = false;
		return;
	}

	struct WRow { const char* label; bool* flag; Hash hash; };
	const WRow rows[] = {
		{ "RPG",              &vehicleRPG,             3204302209 },
		{ "Fireworks",        &vehicleFireworks,       2138347493 },
		{ "Guns",             &vehicleGuns,            3220176749 },
		{ "Snowballs",        &vehicleSnowballs,       126349499  },
		{ "Balls",            &vehicleBalls,           600439132  },
		{ "Water Hydrants",   &vehicleWaterHydrant,    0          },
		{ "Flame Leaks",      &vehicleFlameLeak,       0          },
		{ "Green Laser",      &vehicleLaserGreen,      4026335563 },
		{ "Red Laser",        &vehicleLaserRed,        1566990507 },
		{ "Valkyrie Turrets", &vehicleTurretsValkyrie, 1097917585 },
		{ "Flares",           &vehicleFlaregun,        static_cast<Hash>(WEAPON_FLARE) },
		{ "Heavy Snipers",    &vehicleHeavySniper,     205991906  },
		{ "Tazers",           &vehicleTazerWeapon,     911657153  },
		{ "Molotovs",         &vehicleMolotovWeapon,   615608432  },
		{ "Combat PDWs",      &vehicleCombatPDW,       171789620  }
	};
	for (const WRow& r : rows)
	{
		if (DrawToggle(r.label, *r.flag))
		{
			if (*r.flag && r.hash)
			{
				if (!HAS_WEAPON_ASSET_LOADED(r.hash))
					REQUEST_WEAPON_ASSET(r.hash, 31, 0);
				Game::Print::PrintBottomLeft("Press ~b~LS/L1/NUM_PLUS~s~ for hax!");
			}
		}
	}
}

void VehicleMultipliersSubmenu::Draw()
{
	DrawTitle();

	DrawNumber("CMOD_STAT_1", accelMult, 1, 0, 200);          // Acceleration
	DrawNumber(Game::GetGXTEntry("CMOD_STAT_2") + " & Reverse",
		brakeMult, 1, 0, 100);                                // Braking & Reverse
	DrawNumber("CMOD_STAT_3", handlingMult, 1, 0, 100);       // Handling/Traction
}

void VehicleMultiPlatNeonsSubmenu::Draw()
{
	DrawTitle();

	Engine* engine = Engine::Current();

	bool toggleChanged = DrawToggle("Toggle", multiPlatNeons);
	if (toggleChanged && !multiPlatNeons)
	{
		g_multiPlatNeonsList.clear();
	}

	if (DrawNumber("Intensity", g_multiPlatNeonsIntensity, 0.05f, 2))
	{
		// pass-through; nothing else to do per frame
	}
	if (engine && IsCurrentRowSelected()
		&& MenuPressTimer::IsButtonTapped(MenuPressTimer::Button::Accept))
	{
		std::string inputStr = Game::InputBox(std::to_string(g_multiPlatNeonsIntensity), 10U,
			"Enter Value:", std::to_string(g_multiPlatNeonsIntensity));
		if (!inputStr.empty())
		{
			try { g_multiPlatNeonsIntensity = std::stof(inputStr); }
			catch (...) { Game::Print::PrintErrorInvalidInput(inputStr); }
		}
	}

	DrawToggle("Rainbow Mode", multiPlatNeonsRainbow);

	if (DrawOption("Set Colour"))
	{
		bitMSPaintsRGBMode = 3;
		NavigateTo("vehicle_modshop_paints_rgb");
	}
	if (engine && IsCurrentRowSelected())
	{
		engine->AddPresetColourOptionsPreview(g_multiPlatNeonsColor);
	}
}

void VehicleSlamSubmenu::Draw()
{
	DrawTitle();

	float* target = sub::VehicleSlam::slamValue ? sub::VehicleSlam::slamValue : &vehicleSlam;

	Engine* engine = Engine::Current();
	for (const sub::VehicleSlam::NamedSlamValueS& sl : sub::VehicleSlam::vValues_VehicleSlam)
	{
		const bool active = (*target == sl.value);
		const bool pressed = engine
			? engine->AddCheckbox(sl.name, active, Checkbox::CARTHING, Checkbox::NONE)
			: false;
		if (pressed)
		{
			*target = sl.value;
			if (sl.value <= -0.35f)
				Game::Print::PrintBottomCentre("~b~Note:~s~ You can even drive on walls with this value.");
		}
	}
}

void AutoDriveSubmenu::Draw()
{
	DrawTitle();

	GTAped myPed = Game::PlayerPed();
	using namespace sub::VehicleAutoDrive;

	std::vector<std::string> styleNames;
	for (const auto& d : DrivingStyle::nameArray)
		styleNames.push_back(d.name);

	bool& rand = RandomDestinationMode();
	if (DrawToggle("Random Destination", rand))
		ToggleOnOff();

	if (DrawToggleExternal("Go To Waypoint", Enabled() && !rand))
		ToggleOnOff();

	float& speed = Speed();
	float speedDisplay = speed * 3.6f;
	if (DrawNumber("Speed (KMPH)", speedDisplay, 0.5f, 1, 0.0f, 1e9f))
	{
		speed = speedDisplay / 3.6f;
		myPed.SetDrivingSpeed(speed);
	}

	{
		Engine* engine = Engine::Current();
		unsigned char& dsIdx = DrivingStyleIndex();
		int idx = dsIdx;
		const ::Menu::InputResult res = engine
			? engine->AddTextList("Driving Style", idx, styleNames)
			: ::Menu::InputResult{};
		if (res.rightPressed && dsIdx + 1 < styleNames.size())
		{
			++dsIdx;
			myPed.SetDrivingStyle(DrivingStyle::nameArray[dsIdx].style);
		}
		else if (res.leftPressed && dsIdx > 0)
		{
			--dsIdx;
			myPed.SetDrivingStyle(DrivingStyle::nameArray[dsIdx].style);
		}
	}

	bool& push = PushEnemiesAway();
	DrawToggle("Push Other Vehicles Away", push);

	float& pushRadius = PushRadius();
	if (DrawNumber("Forcefield Radius", pushRadius, 0.5f, 1, 1.0f, 1e9f))
	{
		// nothing extra to do
	}
}

void SpeedoMainSubmenu::Draw()
{
	DrawTitle();

	using namespace sub::Speedo;
	Engine* engine = Engine::Current();

	{
		const std::vector<std::string> modeNames{ "Off", "Digital", "Analogue" };
		int idx = loopSpeedo;
		const ::Menu::InputResult res = engine
			? engine->AddTextList("Speedo", idx, modeNames) : ::Menu::InputResult{};
		if (res.rightPressed && loopSpeedo < static_cast<unsigned char>(modeNames.size()) - 1)
			++loopSpeedo;
		else if (res.leftPressed && loopSpeedo > 0)
			--loopSpeedo;
	}

	{
		const std::vector<std::string> unitNames{ "Metric", "Imperial" };
		int idx = speedoMPH ? 1 : 0;
		const ::Menu::InputResult res = engine
			? engine->AddTextList("Unit", idx, unitNames) : ::Menu::InputResult{};
		if (res.rightPressed || res.leftPressed)
			speedoMPH = !speedoMPH;
	}

	if (loopSpeedo == SPEEDOMODE_ANALOGUE)
	{
		if (DrawOption("Light Themes")) NavigateTo("vehicle_speedos_light");
		if (DrawOption("Dark Themes"))  NavigateTo("vehicle_speedos_dark");
		DrawNumber("X Position", speedoPosition.x, 0.005f, 3);
		DrawNumber("Y Position", speedoPosition.y, 0.005f, 3);
	}
}

void SpeedoThemesLightSubmenu::Draw()
{
	DrawTitle();
	for (sub::Speedo::NamedSpeedoImage& img : sub::Speedo::speedoImagesNames[1])
	{
		const bool active = (sub::Speedo::currentSpeedoBG.id.ID() == img.id.ID());
		if (DrawSelectionItem(img.displayName, active))
			sub::Speedo::currentSpeedoBG = { img.fileName, img.id };
	}
}

void SpeedoThemesDarkSubmenu::Draw()
{
	DrawTitle();
	for (sub::Speedo::NamedSpeedoImage& img : sub::Speedo::speedoImagesNames[2])
	{
		const bool active = (sub::Speedo::currentSpeedoBG.id.ID() == img.id.ID());
		if (DrawSelectionItem(img.displayName, active))
			sub::Speedo::currentSpeedoBG = { img.fileName, img.id };
	}
}

}
namespace sub
{
	void TaskRappel(GTAped ped, GTAvehicle vehicle)
	{
		ped.RequestControl();
		ped.Task().ClearAll();
		ped.SetIntoVehicle(vehicle, VehicleSeat::SEAT_LEFTREAR);

		GTAped newPed;
		if (vehicle.IsSeatFree(VehicleSeat::SEAT_DRIVER))
		{
			newPed = World::CreatePedInsideVehicle(PedHash::PrologueDriver, vehicle, VehicleSeat::SEAT_DRIVER);
		}

		WAIT(100);

		if (newPed.Exists())
		{
			const Vector3& vehPos = vehicle.GetPosition();
			newPed.SetBlockPermanentEvent(true);
			TASK_HELI_MISSION(newPed.Handle(), vehicle.Handle(), 0, 0, vehPos.x, vehPos.y, vehPos.z, 4, 0.0f, 50.0f, -1.0f, 10000, 100, -1082130432, 0);
			newPed.SetAlwaysKeepTask(true);
		}

		vehicle.SetVelocity(Vector3::Zero());

		ped.RequestControl();
		TASK_RAPPEL_FROM_HELI(ped.Handle(), 1); // 1092616192 0x41200000

		WAIT(100);
		if (newPed.Exists())
		{
			newPed.NoLongerNeeded(); // Too soon, maybe?
		}
	}

	namespace VehicleAutoDrive
	{
		class MethodsClass final : public GenericLoopedMode
		{
		private:
			GTAped myPed;
			GTAvehicle vehicle;
			Model vehicleModel;
			Vector3 destination;
		public:
			float speed = 20;
			int drivingStyle = 5;
			unsigned char drivingStyleIndex = 0;
			bool bPushEmAway = true;
			bool bRandDestinationMode = false;
			float pushRadius = 4.0f;
			bool initialSet = false;

			void TurnOn() override
			{
				GenericLoopedMode::TurnOn();

				initialSet = false;
			}
			void TurnOff() override
			{
				GenericLoopedMode::TurnOff();

				initialSet = false;

				CLEAR_PED_TASKS(myPed.Handle());

			}

			void Tick() override
			{
				if (bEnabled)
				{
					MainTick();
				}
			}
			inline void MainTick()
			{
				myPed = PLAYER_PED_ID();

				if (myPed.IsInVehicle() && (IS_WAYPOINT_ACTIVE() || bRandDestinationMode))
				{
					if (!initialSet)
					{
						destination = GET_BLIP_COORDS(GET_FIRST_BLIP_INFO_ID(BlipIcon::Waypoint));
					}

					vehicle = myPed.CurrentVehicle();
					vehicleModel = vehicle.Model();
					Vector3 currentPos = vehicle.GetPosition();

					// If we are within 300m of destination OR outside map bounds, pick a new target
					bool outOfBounds = (currentPos.x > 4000.0f || currentPos.x < -4000.0f || currentPos.y > 7000.0f || currentPos.y < -3500.0f);
					bool arrived = currentPos.DistanceTo(destination) < 300.0f;

					if (bRandDestinationMode && (vehicleModel.IsHeli() || vehicleModel.IsPlane()))
					{
						if (arrived || outOfBounds || !initialSet)
						{
							if (outOfBounds) { // out of bounds, target the center of the map (0,0)
								destination = { 0.0f, 0.0f, 400.0f };
							}
							else {
								int currentDir = MISC::GET_RANDOM_INT_IN_RANGE(1, 5);
								float offset = 4000.0f;
								float targetZ = 400.0f; // Sane cruising altitude to avoid mountains

								if (currentDir == 1) destination = { currentPos.x, currentPos.y + offset, targetZ }; // N
								else if (currentDir == 2) destination = { currentPos.x + offset, currentPos.y, targetZ }; // E
								else if (currentDir == 3) destination = { currentPos.x, currentPos.y - offset, targetZ }; // S
								else destination = { currentPos.x - offset, currentPos.y, targetZ }; // W
							}
							initialSet = false;
						}
					}
					else if (!initialSet && IS_WAYPOINT_ACTIVE())
					{
						destination = GET_BLIP_COORDS(GET_FIRST_BLIP_INFO_ID(BlipIcon::Waypoint));
					}

					if (vehicleModel.IsHeli())
						HeliTick();
					else if (vehicleModel.IsPlane())
						PlaneTick();
					else if (vehicleModel.IsBoat())
						BoatTick();
					else
						NormalTick();

					if (bPushEmAway)
					{
						PushEmAway();
					}
				}
				else
				{
					TurnOff();
				}

			}

			inline void NormalTick()
			{
				if (!initialSet)
				{
					ScrHandle tsk;
					OPEN_SEQUENCE_TASK(&tsk);

					if (bRandDestinationMode) {
						TASK_VEHICLE_DRIVE_WANDER(0, vehicle.Handle(), speed, drivingStyle);
					}
					else {
						addlog(ige::LogType::LOG_DEBUG, "Driving to: " + std::to_string(destination.x) + ", " + std::to_string(destination.y) + ", " + std::to_string(destination.z));
							TASK_VEHICLE_DRIVE_TO_COORD_LONGRANGE(0, vehicle.Handle(), destination.x, destination.y, destination.z, speed, drivingStyle, 7.0f);
					}
					CLOSE_SEQUENCE_TASK(tsk);
					TASK_PERFORM_SEQUENCE(myPed.Handle(), tsk);
					CLEAR_SEQUENCE_TASK(&tsk);

					initialSet = true;
				}
			}

			inline void HeliTick()
			{
				if (!initialSet)
				{
					ScrHandle tsk;
					OPEN_SEQUENCE_TASK(&tsk);
					addlog(ige::LogType::LOG_DEBUG, "Heli Flying to: " + std::to_string(destination.x) + ", " + std::to_string(destination.y) + ", " + std::to_string(destination.z));

					TASK_HELI_MISSION(0, vehicle.Handle(), 0, 0, destination.x, destination.y, destination.z, 9, speed, 250.0f, -1.0f, 100, 200, 0xbf800000, 0);

					CLOSE_SEQUENCE_TASK(tsk);
					TASK_PERFORM_SEQUENCE(myPed.Handle(), tsk);
					CLEAR_SEQUENCE_TASK(&tsk);

					initialSet = true;
				}


			}

			inline void PlaneTick()
			{
				if (!initialSet)
				{
					ScrHandle tsk;
					OPEN_SEQUENCE_TASK(&tsk);
					addlog(ige::LogType::LOG_DEBUG, "Plane Flying to: " + std::to_string(destination.x) + ", " + std::to_string(destination.y) + ", " + std::to_string(destination.z));

					TASK_PLANE_MISSION(0, vehicle.Handle(), 0, 0, destination.x, destination.y, destination.z, 4, speed, 50.0f, -1.0f, 100.0f, 200.0f, false);

					CLOSE_SEQUENCE_TASK(tsk);
					TASK_PERFORM_SEQUENCE(myPed.Handle(), tsk);
					CLEAR_SEQUENCE_TASK(&tsk);

					initialSet = true;
				}

			}

			inline void BoatTick()
			{
				if (!initialSet)
				{
					ScrHandle tsk;
					OPEN_SEQUENCE_TASK(&tsk);

					if (bRandDestinationMode) {
						TASK_VEHICLE_DRIVE_WANDER(0, vehicle.Handle(), speed, drivingStyle);
					}
					else {
						addlog(ige::LogType::LOG_DEBUG, "Sailing to: " + std::to_string(destination.x) + ", " + std::to_string(destination.y) + ", " + std::to_string(destination.z));
						TASK_BOAT_MISSION(0, vehicle.Handle(), 0, 0, destination.x, destination.y, destination.z, 4, speed, 786469, 10.0f, 1071);
					}
					CLOSE_SEQUENCE_TASK(tsk);
					TASK_PERFORM_SEQUENCE(myPed.Handle(), tsk);
					CLEAR_SEQUENCE_TASK(&tsk);

					initialSet = true;
				}

			}

			inline void PushEmAway()
			{
				const auto& md = vehicleModel.Dimensions();
				const auto& pos = vehicle.GetPosition();
				const auto& rot = vehicle.Rotation_get();
				const auto& dir = Vector3::RotationToDirection(rot);

				auto ray = RaycastResult::RaycastCapsule(pos, dir, 3.2f + md.Dim1.y, 2.3f, IntersectOptions::Everything, vehicle);

				if (ray.DidHitEntity())
				{
					GTAentity thingInFront = ray.HitEntity();
					thingInFront.ApplyForce(dir * 10.0f);
				}

				const Vector3& myFrontBumper = pos + (dir * md.Dim1.y);

				for (GTAvehicle v : nearbyVehicles)
				{
					if (v.Handle() == vehicle.Handle())
					{
						continue;
					}
					if (v.IsInRangeOf(myFrontBumper, pushRadius))
					{
						v.ApplyForce(dir * 10.0f);
					}
				}
			}
		};
		MethodsClass Methods;

		void ToggleOnOff()
		{
			Methods.Toggle();
		}
		void Tick()
		{
			Methods.Tick();
		}

		// MenuMenu accessors.
		bool& Enabled() { return Methods.Enabled(); }
		float& Speed() { return Methods.speed; }
		unsigned char& DrivingStyleIndex() { return Methods.drivingStyleIndex; }
		bool& PushEnemiesAway() { return Methods.bPushEmAway; }
		float& PushRadius() { return Methods.pushRadius; }
		bool& RandomDestinationMode() { return Methods.bRandDestinationMode; }
	}

	namespace VehicleSlam
	{
		Vehicle& slamVehicle = g_Ped4;
		float* slamValue;
		void InitSub(GTAvehicle veh, float* val)
		{
			slamVehicle = veh.Handle();
			slamValue = val;
		}

		std::vector<NamedSlamValueS> vValues_VehicleSlam
		{
			{ "Off", 0.0f },

			{ "I'm A Skyscraper", 0.35f },
			{ "Such High", 0.30f },
			{ "I'm Tall, Aren't I", 0.25f },
			{ "I'm Growing", 0.20f },
			{ "Tip Toeing", 0.10f },

			{ "I Got The Crabs", -0.10f },
			{ "Get Get Get, Get Low", -0.20f },
			{ "Short Is Cute", -0.25f },
			{ "I'm A Ninja Turtle", -0.30f },
			{ "Apple Bottom Jeans", -0.38f }
		};
	}

	namespace Speedo
	{
		std::vector<NamedSpeedoImage> speedoImagesNames[]
		{
			{
				{ "Orange", "needle_orange", 0 },
				{ "Orange Night", "night_needle_orange", 0 },
				{ "Red", "needle_red", 0 },
				{ "Red Night", "night_needle_red", 0 }
			},
			{
				{ "Cyan", "bg_cyan2", 0 },
				{ "Green", "bg_green2", 0 },
				{ "Yellow", "bg_yellow2", 0 },
				{ "Orange", "bg_orange2", 0 },
				{ "Purple", "bg_purple2", 0 },
				{ "Pink", "bg_pink2", 0 }
			},
			{
				{ "Cyan", "bg_cyan", 0 },
				{ "Green", "bg_green", 0 },
				{ "Yellow", "bg_yellow", 0 },
				{ "Orange", "bg_orange", 0 },
				{ "Purple", "bg_purple", 0 },
				{ "Pink", "bg_pink", 0 }
			}
		};

		SpeedoImage currentSpeedoBG = { speedoImagesNames[2].at(0).fileName, 0 };
		SpeedoImage currentSpeedoNeedle = { speedoImagesNames[0].at(0).fileName, 0 };
		unsigned __int8 speedoAlpha = 0;
		Vector2 speedoPosition = { 0.85f, 0.86f };

		unsigned __int8 loopSpeedo = SPEEDOMODE_OFF;
		bool speedoMPH = false;

		void SetCurrentBgIdFromBgNameForConfig()
		{
			for (auto& sic : speedoImagesNames)
			{
				for (auto& si : sic)
				{
					if (currentSpeedoBG.fileName == si.fileName)
					{
						currentSpeedoBG.id = si.id;
						return;
					}
				}
			}
		}
		void LoadSpeedoImages()
		{
			for (auto& sic : speedoImagesNames)
			{
				for (auto& si : sic)
				{
					si.id.Load(GetPathffA(Pathff::Speedo, true) + si.fileName + ".png");
				}
			}

			SetCurrentBgIdFromBgNameForConfig();
		}

		inline void DrawSpeedoImage(SpeedoImage& bg, SpeedoImage& needle, float speedf, float alpha)
		{
			Vector2 size = { 0.1540f, 0.164f };
			Vector2& pos = Speedo::speedoPosition;

			bg.id.Draw(0, pos, size, 0.0f, RGBA(255, 255, 255, alpha < 170 ? alpha : 170));
			needle.id.Draw(0, pos, size, (speedf > 270.0f ? 270.6f : speedf), RGBA(255, 255, 255, alpha));

			Game::Print::SetupDraw(font_speedo, Vector2(0.33, 0.33), true, false, false, RGBA(0, 153, 153, alpha));
			Game::Print::drawfloat(speedf, 0, pos.x, pos.y - 0.073f);
		}

		void SpeedoTick()
		{
			if (!IS_PLAYER_CONTROL_ON(PLAYER_ID()))
			{
				speedoAlpha = 0;
			}
			else
			{
				float speedf = abs(3.6f * GET_ENTITY_SPEED_VECTOR(g_myVeh, true).y);
				if (speedoMPH)
				{
					speedf *= 0.6214f;
				}

				if (loopSpeedo == SPEEDOMODE_DIGITAL)
				{
					std::string speedStr;
					if (speedoMPH)
					{
						speedStr = std::to_string((int)speedf) + " ~b~MPH";
					}
					else
					{
						speedStr = std::to_string((int)speedf) + " ~b~KMPH";
					}
					Game::Print::SetupDraw(font_speedo, Vector2(0.8f, 0.8f), true, false, false);
					Game::Print::drawstring(speedStr, 0.915f, 0.8f);

				}
				else
				{
					if (speedoAlpha < 255)
					{
						speedoAlpha += 5;
					}

					unsigned __int8 clockHour = GET_CLOCK_HOURS();
					if (clockHour < 19 && clockHour > 7)
					{
						currentSpeedoNeedle = { speedoImagesNames[0].at(0).fileName, speedoImagesNames[0].at(0).id }; // Day
					}
					else
					{
						currentSpeedoNeedle = { speedoImagesNames[0].at(1).fileName, speedoImagesNames[0].at(1).id }; // Night
					}

					DrawSpeedoImage(currentSpeedoBG, currentSpeedoNeedle, speedf, speedoAlpha);
				}
			}
		}
	}
}

REGISTER_SUBMENU(::Menu::VehicleSubmenu)
REGISTER_SUBMENU(::Menu::PvSubmenu)
REGISTER_SUBMENU(::Menu::VehicleWeaponsSubmenu)
REGISTER_SUBMENU(::Menu::VehicleMultipliersSubmenu)
REGISTER_SUBMENU(::Menu::VehicleMultiPlatNeonsSubmenu)
REGISTER_SUBMENU(::Menu::VehicleSlamSubmenu)
REGISTER_SUBMENU(::Menu::AutoDriveSubmenu)
REGISTER_SUBMENU(::Menu::SpeedoMainSubmenu)
REGISTER_SUBMENU(::Menu::SpeedoThemesLightSubmenu)
REGISTER_SUBMENU(::Menu::SpeedoThemesDarkSubmenu)
