/*
* Menyoo PC - Grand Theft Auto V single-player trainer mod
*
* See Ticks.h for the migration rationale. This translation unit owns the
* per-frame tick functions previously living in Routine.cpp, and is the sole
* place where the legacy submenu Tick headers may be included; every other
* Routine caller goes through `Menu::Ticks::*`.
*/
#include "Ticks.h"

#include "..\macros.h"

#include "Menu.h"
#include "MenuConfig.h"
#include "Routine.h"

#include "..\Natives\types.h"
#include "..\Natives\natives2.h"
#include "..\Scripting\enums.h"
#include "..\Scripting\GTAplayer.h"
#include "..\Scripting\GTAentity.h"
#include "..\Scripting\GTAvehicle.h"
#include "..\Scripting\GTAped.h"
#include "..\Scripting\Game.h"
#include "..\Scripting\GameplayCamera.h"
#include "..\Scripting\Camera.h"
#include "..\Scripting\World.h"
#include "..\Scripting\Model.h"
#include "..\Scripting\PTFX.h"
#include "..\Scripting\WeaponIndivs.h"
#include "..\Scripting\Raycast.h"
#include "..\Scripting\ModelNames.h"
#include "..\Scripting\GTAblip.h"
#include "..\Scripting\TimecycleModification.h"
#include "..\Scripting\CustomHelpText.h"
#include "..\Memory\GTAmemory.h"

#include "..\Misc\FpsCounter.h"
#include "..\Misc\FlameThrower.h"
#include "..\Misc\Gta2Cam.h"
#include "..\Misc\JumpAroundMode.h"
#include "..\Misc\MagnetGun.h"
#include "..\Misc\ManualRespawn.h"
#include "..\Misc\MeteorShower.h"
#include "..\Misc\RopeGun.h"
#include "..\Misc\SmashAbility.h"
#include "..\Misc\VehicleCruise.h"
#include "..\Misc\VehicleFly.h"
#include "..\Misc\VehicleTow.h"

#include "..\Submenus\AnimalRiding.h"
#include "..\Submenus\BreatheStuff.h"
#include "..\Submenus\GhostRider.h"
#include "..\Submenus\Misc.h"      // WaterHack, TVChannelStuff
#include "..\Submenus\Time.h"      // Clock
#include "..\Submenus\Vehicle.h"   // VehicleAutoDrive, Speedo
#include "..\Submenus\WeaponRuntime.h"    // GravityGun_catind, LaserSight_catind
#include "..\Submenus\Spooner\SpoonerMode.h"
#include "..\Submenus\Spooner\SpoonerEntity.h"
#include "..\Submenus\Spooner\SpoonerSettings.h"
#include "..\Submenus\Spooner\EntityManagement.h"
#include "..\Submenus\TeleportYachts.h"
#include "..\Submenus\Teleport\TeleMethods.h"
#include "..\Submenus\PedAnimationRuntime.h"
#include "..\Submenus\PedComponentRuntime.h"
#include "..\Submenus\PedSpeech.h"
#include "..\Submenus\PlayerRuntime.h"
#include "..\Submenus\VehicleRuntime.h"
#include "..\Submenus\VehicleSpawnerRuntime.h"
#include "..\Submenus\VehicleModShopRuntime.h"
#include "..\Submenus\Neons.h"
#include "..\Submenus\Weather.h"
#include "..\Submenus\PtfxData.h"
#include "..\Submenus\CutscenePlayer.h"

#include "..\Util\keyboard.h"
#include "..\Util\FileLogger.h"

#include <Windows.h>
#include <string>
#include <vector>
#include <list>
#include <array>

// ---- HUD-related globals migrated from Routine.cpp --------------------------
bool hideHUD = false;
bool showFullHUD = false;

// Bootstrap global owned by Routine.cpp; TickMenyooLoops gates the
// one-time-per-load "apply default outfit" step on this flag.
extern bool defaultPedSet;

namespace Menu { namespace Ticks {

	// --- Delegating wrappers ------------------------------------------
	// Each of these is intentionally trivial; the legacy `*::Tick()` body
	// still owns the per-frame state. Promote to a native impl by
	// inlining the legacy body here and switching to locally-owned state
	// once the corresponding accessor surface exists.

	void GhostRider()
	{
		sub::GhostRiderMode::Tick();
	}

	void VehicleAutoDrive()
	{
		sub::VehicleAutoDrive::Tick();
	}

	void GravityGun()
	{
		sub::GravityGun_catind::Tick();
	}

	void WaterHack()
	{
		sub::WaterHack::Tick();
	}

	void LaserSight()
	{
		sub::LaserSight_catind::Tick();
	}

	void AnimalRiding()
	{
		sub::AnimalRiding::Tick();
	}

	void SpoonerMode()
	{
		sub::Spooner::SpoonerMode::Tick();
	}

	void TeleportYachts()
	{
		sub::TeleportLocations_catind::Yachts::Tick();
	}

	void Clock()
	{
		if (sub::Clock::loopClock)
		{
			sub::Clock::DisplayClock();
		}
	}

	void BreatheStuff()
	{
		if (sub::BreatheStuff::playerBreatheStuff != sub::BreatheStuff::BreathePtfxType::None)
		{
			sub::BreatheStuff::SetSelfBreathePTFX(sub::BreatheStuff::playerBreatheStuff);
		}
	}

	void Speedometer()
	{
		if (sub::Speedo::loopSpeedo != sub::Speedo::SPEEDOMODE_OFF)
		{
			sub::Speedo::SpeedoTick();
		}
	}

	void Tv()
	{
		if (sub::TVChannelStuff::loopBasicTV)
		{
			sub::TVChannelStuff::DrawTvWhereItsSupposedToBe();
		}
	}

	void TickSubsystems()
	{
		// Order preserved from legacy Routine.cpp::TickSubsystems(). The
		// previous Ticks.cpp version called only a 6-wrapper subset; the
		// non-audit-set subsystems (Spooner, MagnetGun, snow, VehicleTow,
		// VehicleCruise, VehicleFly, MeteorShower, SmashAbility, RopeGun,
		// GTA2Cam, SetPTFXLopTick) are now driven from here too.
		SpoonerMode();
		GhostRider();
		VehicleAutoDrive();
		GravityGun();
		MagnetGun::g_magnetGun.Tick();
		g_spSnow.Tick();
		VehicleTow::g_vehicleTow.Tick();
		VehicleCruise::g_vehicleCruise.Tick();
		VehicleFly::g_vehicleFly.Tick();
		WaterHack();
		LaserSight();
		MeteorShower::g_meteorShower.Tick();
		SmashAbility::g_smashAbility.Tick();
		RopeGun::g_ropeGun.Tick();
		AnimalRiding();
		GTA2Cam::g_gta2Cam.Tick();
		SetPTFXLopTick();
	}

}}

// ---- HUD helpers migrated from Routine.cpp ----------------------------------

// Game - HUD
void DisplayFullHUDThisFrame(bool bEnabled)
{
	DISPLAY_AMMO_THIS_FRAME(bEnabled);
	DISPLAY_CASH(bEnabled);

	std::list<HudComponent> comps
	{
		HudComponent::Cash,
		HudComponent::MpCash,
		HudComponent::MpRankBar,
		HudComponent::WantedStars,
		HudComponent::WeaponIcon,
		HudComponent::VehicleName,
		HudComponent::AreaName,
		HudComponent::StreetName,
		HudComponent::Reticle,
	};

	if (bEnabled)
	{
		for (auto& x : comps)
		{
			SHOW_HUD_COMPONENT_THIS_FRAME((int)x);
		}
	}
	else
	{
		for (auto& x : comps)
		{
			HIDE_HUD_COMPONENT_THIS_FRAME((int)x);
		}
	}
}

// World - Entities
void UpdateNearbyStuffArraysTick()
{
	nearbyPeds.clear();
	nearbyVehicles.clear();
	worldPeds.clear();
	worldVehicles.clear();
	worldObjects.clear();
	worldEntities.clear();

	GTAmemory::GetVehicleHandles(worldVehicles);
	GTAmemory::GetPedHandles(worldPeds);
	GTAmemory::GetPropHandles(worldObjects);
	GTAmemory::GetEntityHandles(worldEntities);

	Ped me = PLAYER_PED_ID();
	INT i, offsettedID, count = 100;

	std::vector<Ped> peds(count * 2 + 2);
	peds[0] = count;
	INT found = GET_PED_NEARBY_PEDS(me, (Any*)peds.data(), -1);
	for (i = 0; i < found; i++)
	{
		offsettedID = i * 2 + 2;
		if (!DOES_ENTITY_EXIST(peds[offsettedID])) continue;
		nearbyPeds.push_back(peds[offsettedID]);
	}

	std::vector<Vehicle> vehicles(count * 2 + 2);
	vehicles[0] = count;
	found = GET_PED_NEARBY_VEHICLES(me, (Any*)vehicles.data());
	for (i = 0; i < found; i++)
	{
		offsettedID = i * 2 + 2;
		if (!DOES_ENTITY_EXIST(vehicles[offsettedID])) continue;
		nearbyVehicles.push_back(vehicles[offsettedID]);
	}
}

static void DrawGameInfo()
{
	constexpr float HUD_LINE_HEIGHT = 0.025f;
	const Vector2 HUD_FONT_SIZE(0.35f, 0.35f);

	float hudY = 0.09f; // automatically increments with each drawn line

	// automatically move the HUD to the left if menyoo menus are on the right to make sure that neither is obstructed
	const bool bRightJustified = GetXCoordAtMenuLeftEdge(0.0f, false) < 0.5f;
	float hudX = bRightJustified ? 0.98f : 0.02f;
	Vector2 hudTextWrap = bRightJustified ? Vector2(0.0f, 0.98f) : Vector2(0.0f, 1.0f);


	auto drawText = [&](const std::string& text, RGBA colour = {255, 255, 255, 255})
	{
		Game::Print::SetupDraw(font_hud, HUD_FONT_SIZE, false, bRightJustified, true, colour, hudTextWrap);
		Game::Print::drawstring(text, hudX, hudY);
		hudY += HUD_LINE_HEIGHT;
	};

	auto drawFloat = [&](float value, UINT8 decimals, RGBA colour = {255, 255, 255, 255})
	{
		Game::Print::SetupDraw(font_hud, HUD_FONT_SIZE, false, bRightJustified, true, colour, hudTextWrap);
		Game::Print::drawfloat(value, decimals, hudX, hudY);
		hudY += HUD_LINE_HEIGHT;
	};

	if (FPSCounter::bDisplayFps)
	{
		auto fps = FPSCounter::g_fpsCounter.Get();
		drawText(std::to_string(fps) + " FPS");
		hudY += HUD_LINE_HEIGHT/2; // for visual separation between hud elements
	}

	if (bDisplayXyzhCoords)
	{
		Vector3 pos = GET_ENTITY_COORDS(PLAYER_PED_ID(), 1);
		float heading = GET_ENTITY_HEADING(PLAYER_PED_ID());
		drawText("Coords:");
		drawFloat(pos.x, 4);
		drawFloat(pos.y, 4);
		drawFloat(pos.z, 4);
		drawFloat(heading, 4);
		hudY += HUD_LINE_HEIGHT/2; // for visual separation between hud elements
	}

	if (sub::Spooner::SpoonerMode::bEnabled && sub::Spooner::Settings::bDisplaySpoonerInfo)
	{
		auto stats = sub::Spooner::SpoonerMode::GetSpoonerStats();
		drawText("Total Entities Spawned: " + std::to_string(stats.totalNumEntities));
		drawText("Objects Spawned: " + std::to_string(stats.totalNumProps));
		drawText("Peds Spawned: " + std::to_string(stats.totalNumPeds));
		drawText("Vehicles Spawned: " + std::to_string(stats.totalNumVehicles));
		hudY += HUD_LINE_HEIGHT/2; // for visual separation between hud elements
	}
}

// ---- Per-frame tick implementations (migrated from Routine.cpp) -------------

static void TickWorldState()
{
	if (restrictedAreasAccess)
	{
		TERMINATE_ALL_SCRIPTS_WITH_THIS_NAME("am_armybase");
		TERMINATE_ALL_SCRIPTS_WITH_THIS_NAME("restrictedareas");
		TERMINATE_ALL_SCRIPTS_WITH_THIS_NAME("re_armybase");
		TERMINATE_ALL_SCRIPTS_WITH_THIS_NAME("re_lossantosintl");
		TERMINATE_ALL_SCRIPTS_WITH_THIS_NAME("re_prison");
		TERMINATE_ALL_SCRIPTS_WITH_THIS_NAME("re_prisonvanbreak");
		TERMINATE_ALL_SCRIPTS_WITH_THIS_NAME("am_doors");
	}

	if (!IS_PLAYER_SWITCH_IN_PROGRESS())
	{
		if (pauseClock)
		{
			NETWORK_OVERRIDE_CLOCK_TIME(pauseClockH, pauseClockM, 0);
		}
		if (syncClock)
		{
			SetSyncClockTime();
		}
		Menu::Ticks::Clock();

		if (hideHUD)
		{
			HIDE_HUD_AND_RADAR_THIS_FRAME();
		}
		if (showFullHUD)
		{
			DisplayFullHUDThisFrame(true);
		}

		if (massacreMode)
		{
			SetMassacreModeTick();
		}
		if (blackoutMode)
		{
			SetBlackoutEMPMode();
		}
		if (simpleBlackoutMode)
		{
			SetBlackoutMode();
		}
		if (JumpAroundMode::bEnabled)
		{
			JumpAroundMode::Tick();
		}
	}

	if (neverWanted)
	{
		SET_MAX_WANTED_LEVEL(0);
		SET_WANTED_LEVEL_MULTIPLIER(0.0f);
	}

	if (vehiclePopulation)
	{
		SET_VEHICLE_DENSITY_MULTIPLIER_THIS_FRAME(0.0);
		SET_RANDOM_VEHICLE_DENSITY_MULTIPLIER_THIS_FRAME(0.0);
		SET_PARKED_VEHICLE_DENSITY_MULTIPLIER_THIS_FRAME(0.0);
		SET_VEHICLE_POPULATION_BUDGET(0);
	}

	if (pedPopulation)
	{
		SET_PED_POPULATION_BUDGET(0);
		SET_PED_DENSITY_MULTIPLIER_THIS_FRAME(0.0);
	}

	if (g_rainFXIntensity > 0.0f)
	{
		SET_RAIN(g_rainFXIntensity);
	}

	if (g_frozenRadioStation != -1)
	{
		if (GET_PLAYER_RADIO_STATION_INDEX() != g_frozenRadioStation)
		{
			SET_RADIO_TO_STATION_NAME(GET_RADIO_STATION_NAME(g_frozenRadioStation));
		}
	}

	if (multiPlatNeons)
	{
		SetMultiPlatNeons();
	}

	if (explostionWP != 0)
	{
		SetExplosionWP(explostionWP);
	}
}

static void TickPlayerState(int player, int iped, GTAplayer& player2)
{
	if (ignoredByEveryone)
	{
		NetworkSetEveryoneIgnorePlayer(player);
		SetSelfNearbyPedsCalm();
	}

	if (selfFreezeWantedLevel)
	{
		SET_PLAYER_WANTED_LEVEL(player, selfFreezeWantedLevel, 0);
		SET_PLAYER_WANTED_LEVEL_NOW(player, 0);
	}

	if (playerUnlimitedAbility)
	{
		if (!IS_SPECIAL_ABILITY_ENABLED(player, 0))
		{
			ENABLE_SPECIAL_ABILITY(player, TRUE, 0);
		}
		SET_SPECIAL_ABILITY_MULTIPLIER(FLT_MAX);
		SPECIAL_ABILITY_FILL_METER(player, TRUE, 0);
	}

	if (playerAutoClean)
	{
		sub::PedDamageTextures::ClearAllBloodDamage(iped);
		sub::PedDamageTextures::ClearAllVisibleDamage(iped);
	}

	if (fireworksDisplay)
	{
		StartFireworksAtCoords(GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(player2.GetPed().Handle(), GET_RANDOM_FLOAT_IN_RANGE(-10.0f, 10.0f), GET_RANDOM_FLOAT_IN_RANGE(-6.0f, 27.0f), GET_RANDOM_FLOAT_IN_RANGE(-9.0f, 3.5f)), Vector3(0, 0, GET_RANDOM_FLOAT_IN_RANGE(-90.0f, 90.0f)), GET_RANDOM_FLOAT_IN_RANGE(0.4f, 2.45f));
		if (rand() % (INT)2)
		{
			SetExplosionAtCoords(iped, Vector3(GET_RANDOM_FLOAT_IN_RANGE(9.0f, 25.0f), GET_RANDOM_FLOAT_IN_RANGE(5.0f, 25.0f), GET_RANDOM_FLOAT_IN_RANGE(0.4f, 20.0f)), EXPLOSION::DIR_WATER_HYDRANT, 8.0f, 0.0f, true, false, 0);
		}
		else
		{
			SetExplosionAtCoords(iped, Vector3(GET_RANDOM_FLOAT_IN_RANGE(-9.0f, -25.0f), GET_RANDOM_FLOAT_IN_RANGE(5.0f, 25.0f), GET_RANDOM_FLOAT_IN_RANGE(0.4f, 20.0f)), EXPLOSION::DIR_WATER_HYDRANT, 8.0f, 0.0f, true, false, 0);
		}
	}

	if (clearWeaponPickups)
	{
		static const Hash weaponPickups[] = {
			PICKUP_WEAPON_BULLPUPSHOTGUN, PICKUP_WEAPON_ASSAULTSMG, PICKUP_VEHICLE_WEAPON_ASSAULTSMG,
			PICKUP_WEAPON_PISTOL50, PICKUP_WEAPON_ASSAULTRIFLE, PICKUP_WEAPON_CARBINERIFLE,
			PICKUP_WEAPON_ADVANCEDRIFLE, PICKUP_WEAPON_MG, PICKUP_WEAPON_COMBATMG,
			PICKUP_WEAPON_SNIPERRIFLE, PICKUP_WEAPON_HEAVYSNIPER, PICKUP_WEAPON_MICROSMG,
			PICKUP_WEAPON_SMG, PICKUP_WEAPON_RPG, PICKUP_WEAPON_MINIGUN,
			PICKUP_WEAPON_PUMPSHOTGUN, PICKUP_WEAPON_SAWNOFFSHOTGUN, PICKUP_WEAPON_ASSAULTSHOTGUN,
			PICKUP_WEAPON_GRENADE, PICKUP_WEAPON_MOLOTOV, PICKUP_WEAPON_SMOKEGRENADE,
			PICKUP_WEAPON_STICKYBOMB, PICKUP_WEAPON_PISTOL, PICKUP_WEAPON_COMBATPISTOL,
			PICKUP_WEAPON_APPISTOL, PICKUP_WEAPON_GRENADELAUNCHER, PICKUP_WEAPON_STUNGUN,
			PICKUP_WEAPON_FIREEXTINGUISHER, PICKUP_WEAPON_PETROLCAN, PICKUP_WEAPON_KNIFE,
			PICKUP_WEAPON_NIGHTSTICK, PICKUP_WEAPON_HAMMER, PICKUP_WEAPON_BAT,
			PICKUP_WEAPON_GOLFCLUB, PICKUP_WEAPON_CROWBAR, PICKUP_WEAPON_BULLPUPRIFLE,
			PICKUP_WEAPON_BOTTLE, PICKUP_WEAPON_SNSPISTOL, PICKUP_WEAPON_GUSENBERG,
			PICKUP_WEAPON_HEAVYPISTOL, PICKUP_WEAPON_SPECIALCARBINE, PICKUP_WEAPON_DAGGER,
			PICKUP_WEAPON_VINTAGEPISTOL, PICKUP_WEAPON_FIREWORK, PICKUP_WEAPON_MUSKET,
			PICKUP_WEAPON_HEAVYSHOTGUN, PICKUP_WEAPON_MARKSMANRIFLE, PICKUP_WEAPON_PROXMINE,
			PICKUP_WEAPON_HOMINGLAUNCHER, PICKUP_WEAPON_FLAREGUN,
			PICKUP_WEAPON_PISTOL_MK2, PICKUP_WEAPON_SMG_MK2, PICKUP_WEAPON_ASSAULTRIFLE_MK2,
			PICKUP_WEAPON_CARBINERIFLE_MK2, PICKUP_WEAPON_COMBATMG_MK2, PICKUP_WEAPON_HEAVYSNIPER_MK2,
		};
		for (Hash pickup : weaponPickups)
		{
			REMOVE_ALL_PICKUPS_OF_TYPE(pickup);
		}
	}
}

static void TickPlayerAbilities()
{
	Ped myPed = PLAYER_PED_ID();
	Player myPlayer = PLAYER_ID();

	if (playerWalkUnderwater)
	{
		SetWalkUnderwater(myPed);
	}

	if (playerInvincibility)
	{
		if (!GET_PLAYER_INVINCIBLE(myPlayer))
		{
			SET_PLAYER_INVINCIBLE(myPlayer, 1);
		}
		SetPedInvincibleOn(myPed);
	}

	if (playerNoRagdoll)
	{
		SetPedNoRagdollOn(myPed);
	}

	if (playerSeatbelt)
	{
		SetPedSeatbeltOn(myPed);
	}

	if (superJump)
	{
		SET_SUPER_JUMP_THIS_FRAME(myPlayer);
	}

	if (noClip)
	{
		SetNoclip();
	}

	if (superRun)
	{
		SetLocalButtonSuperRun();
	}

	if (selfRefillHealthInCover)
	{
		SetSelfRefillHealthWhenInCover();
	}

	if (superman)
	{
		SetLocalSupermanManual();
	}

	if (supermanAuto)
	{
		SetPedSupermanAuto(myPed);
	}

	if (forceField)
	{
		SetLocalForcefield();
	}

	if (driveOnWater)
	{
		DriveOnWater(myPed, g_driveWaterObject);
	}

	if (playerBurn)
	{
		SetPedBurnMode(myPed, true);
	}

	if (selfSweatMult > 0.0f)
	{
		SET_PED_SWEAT(myPed, selfSweatMult);
		if (selfSweatMult > 4.0f)
		{
			SET_PED_WETNESS_ENABLED_THIS_FRAME(myPed);
			SET_PED_WETNESS_HEIGHT(myPed, selfSweatMult / 6);
		}
	}

	if (swimSpeedMult)
	{
		SET_SWIM_MULTIPLIER_FOR_PLAYER(myPlayer, swimSpeedMult);
		SET_RUN_SPRINT_MULTIPLIER_FOR_PLAYER(myPlayer, swimSpeedMult);
	}

	if (g_playerVerticalElongationMultiplier != 1.0f)
	{
		GeneralGlobalHax::SetPlayerHeight(g_playerVerticalElongationMultiplier);
	}

	if (playerNoiseMult != 1)
	{
		SET_PLAYER_NOISE_MULTIPLIER(myPlayer, playerNoiseMult);
	}
}

static void TickWeaponEffects()
{
	Ped myPed = PLAYER_PED_ID();
	Player myPlayer = PLAYER_ID();

	SetTargetIntoSlot();

	if (forgeGun)
	{
		SetForgeGun();
	}

	Menu::Ticks::BreatheStuff();

	if (explosiveRounds)
	{
		SET_EXPLOSIVE_AMMO_THIS_FRAME(myPlayer);
	}

	if (explosiveMelee)
	{
		SET_EXPLOSIVE_MELEE_THIS_FRAME(myPlayer);
	}

	if (flamingRounds)
	{
		SET_FIRE_AMMO_THIS_FRAME(myPlayer);
	}

	if (bitInfiniteAmmo && (bitInfiniteAmmoEnth != myPed || GET_TIME_SINCE_LAST_DEATH() < 10000))
	{
		bitInfiniteAmmoEnth = myPed;
		SET_PED_INFINITE_AMMO_CLIP(bitInfiniteAmmoEnth, true);
	}

	if (selfInfiniteParachutes)
	{
		GivePedParachute(myPed);
	}

	if (weaponDamageIncrease != 1.0f)
	{
		SET_PLAYER_WEAPON_DAMAGE_MODIFIER(myPlayer, weaponDamageIncrease);
		SET_PLAYER_MELEE_WEAPON_DAMAGE_MODIFIER(myPlayer, weaponDamageIncrease, true);
	}

	if (IS_PED_SHOOTING(myPed))
	{
		if (kaboomGun)
		{
			SetExplosionAtBulletHit(myPed, kaboomGunHash, kaboomGunInvis);
		}
		if (triggerFXGun)
		{
			SetTriggerFXAtBulletHit(myPed, triggerFXGunData.asset, triggerFXGunData.effect, Vector3::RandomXYZ() * 180.0f, GET_RANDOM_FLOAT_IN_RANGE(0.63f, 1.40f));
		}
		if (bulletGun)
		{
			SetBulletGun();
		}
		if (teleportGun)
		{
			SetTeleportGun();
		}
		if (pedGun)
		{
			SetPedGun();
		}
		if (objectGun)
		{
			SetObjectGun();
		}
		if (bulletTime)
		{
			SET_TIME_SCALE(0.2f);
		}
		if (tripleBullets)
		{
			SetTripleBullets();
		}
	}
	if (GET_GAME_TIMER() >= Menu::delayedTimer && bulletTime)
	{
		SET_TIME_SCALE(currentTimescale);
	}
}

static void TickVehicleEffects(bool gameIsPaused)
{
	if (!IS_PED_IN_ANY_VEHICLE(PLAYER_PED_ID(), 0))
	{
		bitVehicleGravity = bitFreezeVehicle = bitVehicleSlippyTires = false;
		return;
	}

	// Acceleration and brake multipliers
	if (!g_myVehModel.IsHeli())
	{
		if (g_myVehModel.IsPlane() || IS_VEHICLE_ON_ALL_WHEELS(g_myVeh))
		{
			if (GET_PED_IN_VEHICLE_SEAT(g_myVeh, VehicleSeat::SEAT_DRIVER, 0) == PLAYER_PED_ID())
			{
				if (IS_CONTROL_PRESSED(2, INPUT_VEH_ACCELERATE) && accelMult != 0)
				{
					APPLY_FORCE_TO_ENTITY(g_myVeh, 1, 0.0, (float)(accelMult) / 69.0f, 0.0, 0.0, 0.0, 0.0, 0, 1, 1, 1, 0, 1);
				}
				if (brakeMult != 0 && IS_CONTROL_PRESSED(2, INPUT_VEH_BRAKE))
				{
					APPLY_FORCE_TO_ENTITY(g_myVeh, 0, 0.0, (float)(0 - brakeMult), 0.0, 0.0, 0.0, 0.0, 0, 1, 1, 1, 0, 1);
				}
				if (GET_ENTITY_SPEED_VECTOR(g_myVeh, true).y > 2)
				{
					SetHandlingMultiplier();
				}
			}
		}
	}

	if (vehicleDamageAndDefense != 1.0f)
	{
		SET_PLAYER_VEHICLE_DAMAGE_MODIFIER(PLAYER_ID(), vehicleDamageAndDefense);
		SET_PLAYER_VEHICLE_DEFENSE_MODIFIER(PLAYER_ID(), vehicleDamageAndDefense);
	}

	if (selfEngineOn)
	{
		GTAvehicle veh = g_myVeh;
		if (veh.Exists())
		{
			if (!veh.GetEngineRunning())
			{
				veh.SetEngineRunning(true);
			}
			if (!veh.GetLightsOn())
			{
				veh.SetLightsOn(true);
			}
		}
	}

	if (vehicleInvincibility)
	{
		SetVehicleInvincibleOn(g_myVeh);
	}

	if (vehicleFixLoop)
	{
		GTAvehicle veh = g_myVeh;
		if (veh.IsDamaged())
		{
			static int vehicleOpsFixCarTexterValue = 0;
			auto& fixCarTexterVal = vehicleOpsFixCarTexterValue;
			std::array<bool, (int)VehicleWindow::Last> windowsIntact;
			if (fixCarTexterVal == 1)
			{
				for (int i = 0; i < windowsIntact.size(); i++)
				{
					windowsIntact[i] = veh.IsWindowIntact((VehicleWindow)i);
				}
			}

			SET_VEHICLE_FIXED(veh.Handle());
			SET_VEHICLE_DIRT_LEVEL(veh.Handle(), 0.0f);
			SET_VEHICLE_ENGINE_HEALTH(veh.Handle(), 2000.0f);
			SET_VEHICLE_PETROL_TANK_HEALTH(veh.Handle(), 2000.0f);
			SET_VEHICLE_BODY_HEALTH(veh.Handle(), 2000.0f);
			SET_VEHICLE_UNDRIVEABLE(veh.Handle(), false);

			if (fixCarTexterVal == 1)
			{
				for (int i = 0; i < windowsIntact.size(); i++)
				{
					if (!windowsIntact[i])
					{
						veh.RollDownWindow((VehicleWindow)i);
					}
				}
			}
		}
	}

	if (vehicleFlipLoop)
	{
		SetVehicleFlip(g_myVeh);
	}

	if (vehicleInvisibility)
	{
		BOOL pedWasVisible = IS_ENTITY_VISIBLE(PLAYER_PED_ID());
		SET_ENTITY_VISIBLE(g_myVeh, false, false);
		SET_ENTITY_VISIBLE(PLAYER_PED_ID(), pedWasVisible, false);
		vehicleInvisibility = false;
	}

	if (carColorChange)
	{
		SetVehicleRainbowMode(g_myVeh, true);
	}

	if (s_neonDirty) {
		SetVehicleNeonAnim(g_myVeh);
		s_neonDirty = false;
	}

	if (vehicleDisableSiren)
	{
		if (GTAvehicle(g_myVeh).GetHasSiren())
		{
			SET_VEHICLE_HAS_MUTED_SIRENS(g_myVeh, TRUE);
		}
	}

	if (vehicleSlam)
	{
		if (vehicleSlam <= -0.35f || IS_VEHICLE_ON_ALL_WHEELS(g_myVeh))
		{
			APPLY_FORCE_TO_ENTITY(g_myVeh, 1, 0.0, 0.0, vehicleSlam, 0.0, 0.0, 0.0, 1, 1, 1, 1, 0, 1);
		}
	}

	if (vehicleHeavyMass)
	{
		SetVehicleHeavyMass(g_myVeh);
	}

	// Vehicle controls (only when game is not paused)
	if (!gameIsPaused)
	{
		if (raceBoost && IS_CONTROL_PRESSED(2, INPUT_VEH_HORN))
		{
			SetSelfVehicleBoost();
		}

		if (carJump != 0)
		{
			SetLocalCarJump();
		}

		if (carHydraulics)
		{
			SetLocalCarHydraulics();
		}

		if (superGrip)
		{
			SET_VEHICLE_ON_GROUND_PROPERLY(g_myVeh, 0.0f);
		}

		if (superCarMode)
		{
			SetSuperCarModeSelf();
		}

		SetVehicleWeapons();

		Menu::Ticks::Speedometer();
	}
}

// Main loop

void TickMenyooLoops()
{
	bool gameIsPaused = IS_PAUSE_MENU_ACTIVE() != 0;

	// Apply default outfit on first load
	if (!GET_IS_LOADING_SCREEN_ACTIVE() && !defaultPedSet)
	{
		sub::ComponentChangerOutfit::Apply(PLAYER_PED_ID(), "menyooStuff/defaultPed.xml", true, false, false, false, false, false);
		sub::ComponentChangerOutfit::Apply(PLAYER_PED_ID(), "menyooStuff/defaultPed.xml", false, true, true, true, true, true);
		defaultPedSet = true;
	}

	Game::CustomHelpText::Tick();
	UpdateNearbyStuffArraysTick();

	if (gameIsPaused)
	{
		SetPauseMenuTeleToWpCommand();
	}

	Menu::Ticks::TickSubsystems();
	MenuInput::UpdateDeltaCursorNormal();

	// Cache current vehicle and weapon
	if (IS_PED_IN_ANY_VEHICLE(PLAYER_PED_ID(), 0))
	{
		g_myVeh = GET_VEHICLE_PED_IS_IN(PLAYER_PED_ID(), false);
		g_myVehModel = GET_ENTITY_MODEL(g_myVeh);
	}
	else
	{
		g_myVeh = 0;
	}

	if (!IS_PLAYER_DEAD(PLAYER_ID()))
	{
		GET_CURRENT_PED_WEAPON(PLAYER_PED_ID(), &g_myWeap, 1);
	}

	if (rainbowBoxes)
	{
		titlebox = { g_fadedRGB.R, g_fadedRGB.G, g_fadedRGB.B, titlebox.A };
	}

	if (checkSelfDeathModel)
	{
		ManualRespawn::CheckSelfDealthModel();
	}

	// Tick all categories
	TickWorldState();

	if (GET_GAME_TIMER() >= Menu::delayedTimer)
	{
		int player = PLAYER_ID();
		GTAplayer player2;
		player2.Handle() = player;
		int iped = PLAYER_PED_ID();
		TickPlayerState(player, iped, player2);
	}

	TickWeaponEffects();
	TickPlayerAbilities();

	// HUD overlays
	DrawGameInfo();

	TickVehicleEffects(gameIsPaused);
	SetPVOpsVehicleTextWorld2Screen();
}

// Secondary thread

static void TickSpectatePlayer()
{
	if (spectatePlayer >= 0 && spectatePlayer < GAME_PLAYERCOUNT)
	{
		if (!NETWORK_IS_PLAYER_ACTIVE(spectatePlayer))
		{
			NETWORK_SET_IN_SPECTATOR_MODE_EXTENDED(false, spectatePlayer, 1);
			int p = GET_PLAYER_PED(spectatePlayer);
			if (DOES_ENTITY_EXIST(p))
			{
				NETWORK_SET_IN_SPECTATOR_MODE(false, p);
			}
			NETWORK_SET_ACTIVITY_SPECTATOR(false);
			spectatePlayer = -1;
		}
		else
		{
			NETWORK_SET_IN_SPECTATOR_MODE(true, GET_PLAYER_PED(spectatePlayer));
		}
	}
}

void ThreadMenuLoops2()
{
	for (;;)
	{
		WAIT(0);

		VehicleTorqueMultiplier();
		VehicleMaxSpeedMultiplier();

		FlameThrower::Tick();

		Menu::Ticks::Tv();

		TickSpectatePlayer();

		if (lightGun)
		{
			SetLightGun();
		}

		switch (autoKillEnemies)
		{
		case 1:
			World::KillNearbyPeds(PLAYER_PED_ID(), FLT_MAX, PedRelationship::Hate);
			World::KillNearbyPeds(PLAYER_PED_ID(), FLT_MAX, PedRelationship::Dislike);
			break;
		case 2:
			World::KillMyEnemies();
			break;
		}

		Menu::Ticks::TeleportYachts();
		ManualRespawn::g_manualRespawn.Tick();

		if (unlimitedVehicleBoost)
		{
			SetSelfVehicleNativeBoost();
		}
	}
}
