#include "PlayerRuntime.h"

#include "..\macros.h"

#include "..\Menu\Menu.h"
#include "..\Menu\MenuConfig.h"
#include "..\Menu\Routine.h"
#include "..\Menu\GlobalEngine.h"

#include "..\Natives\natives2.h"
#include "..\Natives\types.h"
#include "..\Scripting\enums.h"
#include "..\Scripting\GTAplayer.h"
#include "..\Scripting\GTAentity.h"
#include "..\Scripting\GTAvehicle.h"
#include "..\Scripting\GTAped.h"
#include "..\Scripting\WeaponIndivs.h"
#include "..\Scripting\Game.h"
#include "..\Scripting\GameplayCamera.h"
#include "..\Scripting\World.h"
#include "..\Scripting\Camera.h"
#include "..\Scripting\Model.h"
#include "..\Scripting\CustomHelpText.h"
#include "..\Util\keyboard.h"

#include "VehicleRuntime.h"
#include "WeaponRuntime.h"
#include "Spooner\SpoonerMode.h"

#include <Windows.h>
#include <string>
#include <vector>

INT16 BindNoClip = VirtualKey::F3;

INT g_Ped1;
INT g_Ped2;
INT g_Ped3;
INT g_Ped4;
const char* g_PlayerName;

bool bitNightVision = false;

FLOAT swimSpeedMult = 0;
FLOAT playerNoiseMult = 1.0f;
FLOAT selfSweatMult = 0.0f;
FLOAT g_playerVerticalElongationMultiplier = 1.0f;

Entity g_driveWaterObject;
INT8 spectatePlayer = -1;

bool checkSelfDeathModel = false;

UINT8 forceField = 0;
UINT8 selfFreezeWantedLevel = 0;

bool playerNoRagdoll = false;
bool playerSeatbelt = false;
bool playerUnlimitedAbility = false;
bool playerAutoClean = false;
bool playerWalkUnderwater = false;
bool superJump = false;
bool selfRefillHealthInCover = false;
bool playerInvincibility = false;
bool noClip = false;
bool noClipToggle = false;
bool superRun = false;
bool ignoredByEveryone = false;
bool neverWanted = false;
bool superman = false;
bool supermanAuto = false;
bool driveOnWater = false;
bool playerBurn = false;

bool bitNoclipAlreadyInvisible = true;
bool bitNoclipAlreadyCollision = true;
bool bitNoclipShowHelp = true;
Camera g_cam_noClip;

// File-local freecam state used by SetNoclip
static bool g_freecamHeightLocked = false;
static float g_freecamLockedHeight = 0.0f;
static float g_freecamSpeed = MenuConfig::FreeCam::defaultSpeed;

static DWORD g_lastSpeedDisplayTime = 0;
static DWORD g_lastFOVDisplayTime = 0;
static float g_lastSpeedValue = 0.0f;
static float g_lastFOVValue = 0.0f;

static DWORD g_lastHeightLockMessageTime = 0;
static const char* g_lastHeightLockMessage = nullptr;

void SetSelfNearbyPedsCalm()
{
	for (auto& ped : nearbyPeds)
	{
		NETWORK_REQUEST_CONTROL_OF_ENTITY(ped);
		SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(ped, 1);
		SET_PED_FLEE_ATTRIBUTES(ped, 0, 0);
		SET_PED_COMBAT_ATTRIBUTES(ped, 17, 1);
	}
}
void NetworkSetEveryoneIgnorePlayer(Player player)
{
	SET_POLICE_IGNORE_PLAYER(player, 1);
	SET_EVERYONE_IGNORE_PLAYER(player, 1);
	SET_PLAYER_CAN_BE_HASSLED_BY_GANGS(player, 0);
	SET_IGNORE_LOW_PRIORITY_SHOCKING_EVENTS(player, 1);
}

void SetBecomePed(GTAped ped)
{
	GTAped oldPed = PLAYER_PED_ID();

	std::vector<s_Weapon_Components_Tint> weaponsBackup;
	oldPed.StoreWeaponsInArray(weaponsBackup);

	GTAvehicle vehicle(0);
	VehicleSeat currVehSeat;
	if (ped.IsInVehicle())
	{
		vehicle = ped.CurrentVehicle();
		currVehSeat = ped.GetCurrentVehicleSeat();
	}

	float camPitchRelative = GameplayCamera::GetRelativePitch();

	if (ped.IsPlayer())
	{
		return;
	}

	ped.RequestControl();
	SetPedInvincibleOff(oldPed.Handle());
	CHANGE_PLAYER_PED(PLAYER_ID(), ped.Handle(), true, true);

	GameplayCamera::SetRelativeHeading(0.0f);
	GameplayCamera::SetRelativePitch(camPitchRelative);

	ped = PLAYER_PED_ID();

	if (vehicle.Exists())
	{
		vehicle.RequestControl();
		ped.SetIntoVehicle(vehicle, currVehSeat);
	}

	ped.GiveWeaponsFromArray(weaponsBackup);

	SET_PED_INFINITE_AMMO_CLIP(ped.Handle(), bitInfiniteAmmo);

}

void SetPedInvincibleOn(Ped ped)
{
	SET_ENTITY_INVINCIBLE(ped, 1);
	SET_PED_DIES_IN_WATER(ped, 0);
	SET_ENTITY_PROOFS(ped, 1, 1, 1, 1, 1, 1, 1, 1);
}
void SetPedInvincibleOff(Ped ped)
{
	SET_ENTITY_INVINCIBLE(ped, 0);
	SET_PED_DIES_IN_WATER(ped, 1);
	SET_ENTITY_PROOFS(ped, 0, 0, 0, 0, 0, 0, 0, 0);
}
void SetPedNoRagdollOn(Ped ped)
{
	SET_PED_CAN_RAGDOLL(ped, 0);
	SET_PED_CAN_RAGDOLL_FROM_PLAYER_IMPACT(ped, 0);
}
void SetPedNoRagdollOff(Ped ped)
{
	SET_PED_CAN_RAGDOLL(ped, 1);
	SET_PED_CAN_RAGDOLL_FROM_PLAYER_IMPACT(ped, 1);
}
void SetPedSeatbeltOn(Ped ped)
{
	SET_PED_CAN_BE_KNOCKED_OFF_VEHICLE(ped, 1); // state cantFollOff
	SET_PED_CONFIG_FLAG(ped, ePedConfigFlags::WillFlyThruWindscreen, false);
}
void SetPedSeatbeltOff(Ped ped)
{
	SET_PED_CAN_BE_KNOCKED_OFF_VEHICLE(ped, 0); // state canFallOff
	SET_PED_CONFIG_FLAG(ped, ePedConfigFlags::WillFlyThruWindscreen, true);
}

void SetNoclipOff1()
{
	GTAentity myPed = PLAYER_PED_ID();
	GTAentity ent = IS_PED_IN_ANY_VEHICLE(myPed.Handle(), false) ? GET_VEHICLE_PED_IS_IN(myPed.Handle(), false) : myPed;

	ent.RequestControl();
	ent.SetVisible(!bitNoclipAlreadyInvisible);
	ent.SetIsCollisionEnabled(bitNoclipAlreadyCollision);
	ent.FreezePosition(false);
	ENABLE_CONTROL_ACTION(2, INPUT_VEH_HORN, TRUE);
	ENABLE_CONTROL_ACTION(2, INPUT_LOOK_BEHIND, TRUE);
	ENABLE_CONTROL_ACTION(2, INPUT_VEH_LOOK_BEHIND, TRUE);
	ENABLE_CONTROL_ACTION(2, INPUT_SELECT_WEAPON, TRUE);
	bitNoclipShowHelp = true;
}
void SetNoclipOff2()
{
	auto& cam = g_cam_noClip;
	if (cam.Exists())
	{
		cam.SetActive(false);
		cam.Destroy();
		World::SetRenderingCamera(0);
	}
}
void SetNoclip()
{
	if (sub::Spooner::SpoonerMode::bEnabled)
	{
		return;
	}

	auto& cam = g_cam_noClip;
	GTAentity myPed = PLAYER_PED_ID();
	GTAplayer myPlayer = PLAYER_ID();
	GTAentity ent = IS_PED_IN_ANY_VEHICLE(myPed.Handle(), false) ? GET_VEHICLE_PED_IS_IN(myPed.Handle(), false) : myPed;

	if (ent.Exists())
	{
		if (Menu::bitController ? (IS_CONTROL_PRESSED(2, INPUT_FRONTEND_X) && IS_CONTROL_JUST_PRESSED(2, INPUT_FRONTEND_LS)) : IsKeyJustUp(BindNoClip))
		{
			noClipToggle = !noClipToggle;
			if (!noClipToggle)
			{
				SetNoclipOff1();
			}
			else
			{
				if (bitNoclipShowHelp)
				{
					bitNoclipShowHelp = false;
					if (Menu::bitController)
					{
						Game::CustomHelpText::ShowTimedText(oss_ << "FreeCam:~n~~INPUT_MOVE_UD~ = " << Game::GetGXTEntry("ITEM_MOV_CAM")
							<< "~n~~INPUT_LOOK_LR~ = " << Game::GetGXTEntry("ITEM_MOVE") << "~n~~INPUT_FRONTEND_RT~/~INPUT_FRONTEND_LT~ = " << "Ascend/Descend" << "~n~~INPUT_FRONTEND_RB~ = " << "Hasten", 6000);
					}
					else
					{
						Game::CustomHelpText::ShowTimedText(oss_ << "FreeCam:~n~~INPUT_MOVE_UD~/~INPUT_MOVE_LR~ = " << Game::GetGXTEntry("ITEM_MOV_CAM")
							<< "~n~~INPUT_LOOK_LR~ = " << Game::GetGXTEntry("ITEM_MOVE") << "~n~~INPUT_PARACHUTE_BRAKE_RIGHT~/~INPUT_PARACHUTE_BRAKE_LEFT~ = " << "Ascend/Descend" << "~n~~INPUT_SPRINT~ = " << "Hasten", 6000);
					}
					bitNoclipShowHelp = false;
				}
				bitNoclipAlreadyInvisible = !ent.IsVisible();
				bitNoclipAlreadyCollision = ent.GetIsCollisionEnabled();
			}
		}

		if (!noClipToggle)
		{
			SetNoclipOff2();
			return;
		}

		DISABLE_CONTROL_ACTION(2, INPUT_VEH_HORN, TRUE);
		DISABLE_CONTROL_ACTION(2, INPUT_LOOK_BEHIND, TRUE);
		DISABLE_CONTROL_ACTION(2, INPUT_VEH_LOOK_BEHIND, TRUE);
		DISABLE_CONTROL_ACTION(2, INPUT_SELECT_WEAPON, TRUE);
		DISABLE_CONTROL_ACTION(2, INPUT_VEH_ACCELERATE, TRUE);
		DISABLE_CONTROL_ACTION(2, INPUT_VEH_BRAKE, TRUE);
		DISABLE_CONTROL_ACTION(2, INPUT_VEH_RADIO_WHEEL, TRUE);

		const Vector3& entPos = ent.GetPosition();
		const Vector3& camOffset = Vector3();

		if (!cam.Exists())
		{
			ent.RequestControl();
			cam = World::CreateCamera();
			cam.SetPosition(GameplayCamera::GetPosition());
			cam.SetRotation(GameplayCamera::GetRotation());
			cam.AttachTo(ent, camOffset);
			cam.SetFieldOfView(MenuConfig::FreeCam::defaultFov); // Use configured FOV
			cam.SetDepthOfFieldStrength(0.0f);
			World::SetRenderingCamera(cam);
		}

		ent.RequestControl();
		ent.FreezePosition(true);
		ent.SetIsCollisionEnabled(false);
		ent.SetVisible(false);
		myPed.SetVisible(false);

		Vector3 nextRot = cam.GetRotation() - Vector3(GET_DISABLED_CONTROL_NORMAL(0, INPUT_LOOK_UD), 0, GET_DISABLED_CONTROL_NORMAL(0, INPUT_LOOK_LR)) * (Menu::bitController ? 2.5f : 11.0f);
		nextRot.y = 0.0f; // No roll
		ent.SetRotation(Vector3(0, 0, nextRot.z));
		cam.SetRotation(nextRot);
		if (!myPlayer.IsFreeAiming() && !myPlayer.IsTargetingAnything())
		{
			SET_GAMEPLAY_CAM_RELATIVE_HEADING(0.0f);
		}

		if (Menu::bitController)
		{
			DISABLE_CONTROL_ACTION(0, INPUT_VEH_HORN, TRUE);

			if (ent == myPed)
			{
				if (GET_PED_STEALTH_MOVEMENT(myPed.Handle()))
				{
					SET_PED_STEALTH_MOVEMENT(myPed.Handle(), false, 0);
				}
				if (GET_PED_COMBAT_MOVEMENT(myPed.Handle()))
				{
					SET_PED_COMBAT_MOVEMENT(myPed.Handle(), 0);
				}
			}

			float noclipPrecisionLevel = IS_DISABLED_CONTROL_PRESSED(2, INPUT_FRONTEND_RB) ? 1.8f : 0.8f;
			Vector3 offset;
			offset.x = GET_CONTROL_NORMAL(0, INPUT_MOVE_LR) * noclipPrecisionLevel;
			offset.y = -GET_CONTROL_NORMAL(0, INPUT_MOVE_UD) * noclipPrecisionLevel;
			offset.z = (GET_DISABLED_CONTROL_NORMAL(2, INPUT_FRONTEND_RT) - GET_DISABLED_CONTROL_NORMAL(2, INPUT_FRONTEND_LT)) * noclipPrecisionLevel;
			if (!offset.IsZero())
			{
				ent.SetPosition(cam.GetOffsetInWorldCoords(offset - camOffset));
			}

		}
		else
		{
			// TAB to toggle height lock
			if (!IsKeyDown(VK_SPACE))
			{
				if (IsKeyJustUp(VK_TAB))
				{
					g_freecamHeightLocked = !g_freecamHeightLocked;
					if (g_freecamHeightLocked)
					{
						g_freecamLockedHeight = ent.GetPosition().z;
						g_lastHeightLockMessage = "Height Locked";
					}
					else
					{
						g_lastHeightLockMessage = "Height Unlocked";
					}
					g_lastHeightLockMessageTime = GetTickCount();
				}

				// Mouse wheel to adjust speed
				if (IS_DISABLED_CONTROL_PRESSED(2, INPUT_CURSOR_SCROLL_UP))
				{
					g_freecamSpeed = min(g_freecamSpeed + MenuConfig::FreeCam::speedAdjustStep, MenuConfig::FreeCam::maxSpeed);
					MenuConfig::FreeCam::defaultSpeed = g_freecamSpeed;
					MenuConfig::SaveConfig();
					g_lastSpeedValue = g_freecamSpeed;
					g_lastSpeedDisplayTime = GetTickCount();
				}
				if (IS_DISABLED_CONTROL_PRESSED(2, INPUT_CURSOR_SCROLL_DOWN))
				{
					g_freecamSpeed = max(g_freecamSpeed - MenuConfig::FreeCam::speedAdjustStep, MenuConfig::FreeCam::minSpeed);
					MenuConfig::FreeCam::defaultSpeed = g_freecamSpeed;
					MenuConfig::SaveConfig();
					g_lastSpeedValue = g_freecamSpeed;
					g_lastSpeedDisplayTime = GetTickCount();
				}

				if (GetTickCount() - g_lastSpeedDisplayTime < 1000)
				{
					Game::Print::SetupDraw(GTAfont::Impact, Vector2(0.4f, 0.4f), true, false, false);
					Game::Print::DrawString(oss_ << "FreeCam Speed: " << g_lastSpeedValue, 0.5f, 0.95f);
				}
			}

			float currentSpeed = IS_DISABLED_CONTROL_PRESSED(2, INPUT_VEH_ATTACK2) ? MenuConfig::FreeCam::defaultSlowSpeed : g_freecamSpeed;
			float noclipPrecisionLevel = IS_DISABLED_CONTROL_PRESSED(0, INPUT_SPRINT) ? currentSpeed * 2.0f : currentSpeed;

			Vector3 offset;
			offset.x = GET_CONTROL_NORMAL(0, INPUT_MOVE_LR) * noclipPrecisionLevel;
			offset.y = -GET_CONTROL_NORMAL(0, INPUT_MOVE_UD) * noclipPrecisionLevel;

			if (g_freecamHeightLocked)
			{
				float zOffset = IS_DISABLED_CONTROL_PRESSED(2, INPUT_PARACHUTE_BRAKE_RIGHT) ? noclipPrecisionLevel : IS_DISABLED_CONTROL_PRESSED(2, INPUT_PARACHUTE_BRAKE_LEFT) ? -noclipPrecisionLevel : 0.0f;
				if (zOffset != 0.0f)
				{
					g_freecamLockedHeight += zOffset;
				}

				Vector3 newPos = cam.GetOffsetInWorldCoords(offset - camOffset);
				newPos.z = g_freecamLockedHeight;
				ent.SetPosition(newPos);
			}
			else
			{
				offset.z = IS_DISABLED_CONTROL_PRESSED(2, INPUT_PARACHUTE_BRAKE_RIGHT) ? noclipPrecisionLevel : IS_DISABLED_CONTROL_PRESSED(2, INPUT_PARACHUTE_BRAKE_LEFT) ? -noclipPrecisionLevel : 0.0f;
				if (!offset.IsZero())
				{
					ent.SetPosition(cam.GetOffsetInWorldCoords(offset - camOffset));
				}
			}

			// Space + scroll wheel to control camera FOV
			if (IsKeyDown(VK_SPACE))
			{
				float currentFov = cam.GetFieldOfView();
				if (IS_DISABLED_CONTROL_PRESSED(2, INPUT_CURSOR_SCROLL_UP))
				{
					currentFov = min(currentFov + MenuConfig::FreeCam::fovAdjustStep, MenuConfig::FreeCam::maxFov);
					cam.SetFieldOfView(currentFov);
					MenuConfig::FreeCam::defaultFov = currentFov;
					MenuConfig::SaveConfig();
					g_lastFOVValue = currentFov;
					g_lastFOVDisplayTime = GetTickCount();
				}
				if (IS_DISABLED_CONTROL_PRESSED(2, INPUT_CURSOR_SCROLL_DOWN))
				{
					currentFov = max(currentFov - MenuConfig::FreeCam::fovAdjustStep, MenuConfig::FreeCam::minFov);
					cam.SetFieldOfView(currentFov);
					MenuConfig::FreeCam::defaultFov = currentFov;
					MenuConfig::SaveConfig();
					g_lastFOVValue = currentFov;
					g_lastFOVDisplayTime = GetTickCount();
				}

				if (GetTickCount() - g_lastFOVDisplayTime < 1000)
				{
					Game::Print::SetupDraw(GTAfont::Impact, Vector2(0.4f, 0.4f), true, false, false);
					Game::Print::DrawString(oss_ << "Camera FOV: " << g_lastFOVValue, 0.5f, 0.95f);
				}
			}
		}
	}

	// Height lock status display
	if (g_lastHeightLockMessage != nullptr && GetTickCount() - g_lastHeightLockMessageTime < 1000)
	{
		Game::Print::SetupDraw(GTAfont::Impact, Vector2(0.4f, 0.4f), true, false, false);
		Game::Print::drawstring(g_lastHeightLockMessage, 0.5f, 0.95f);
	}
}

void SetLocalButtonSuperRun()
{
	auto ped = PLAYER_PED_ID();
	bool isInAir = IS_ENTITY_IN_AIR(ped) != 0;

	if (!isInAir)
	{
		if (IS_CONTROL_PRESSED(0, INPUT_SPRINT))
		{
			APPLY_FORCE_TO_ENTITY(ped, 1, 0.0, 3.4, 0.0, 0.0, 0.0, 0.0, 1, 1, 1, 1, 0, 1);
		}

		else if (IS_CONTROL_JUST_RELEASED(0, INPUT_SPRINT))
		{
			FREEZE_ENTITY_POSITION(ped, 1);
			FREEZE_ENTITY_POSITION(ped, 0);
		}
	}

}

void SetSelfRefillHealthWhenInCover()
{
	if (GET_GAME_TIMER() >= Menu::delayedTimer - 100)
	{
		GTAped playerPed = PLAYER_PED_ID();
		auto health = playerPed.GetHealth();
		auto maxHealth = playerPed.GetMaxHealth();
		if (playerPed.IsInCover() && !playerPed.IsAimingFromCover() && health < maxHealth)
		{
			playerPed.SetHealth(health + 4);
		}
	}
}

void SetLocalSupermanManual()
{
	if (IS_PAUSE_MENU_ACTIVE() || IS_PED_IN_ANY_VEHICLE(PLAYER_PED_ID(), 0))
	{
		return;
	}

	DISABLE_CONTROL_ACTION(2, INPUT_PARACHUTE_DEPLOY, TRUE);

	Ped playerPed = PLAYER_PED_ID();
	bool isInParaFreeFall = IS_PED_IN_PARACHUTE_FREE_FALL(playerPed) != 0;

	if (isInParaFreeFall)
	{
		if (IS_CONTROL_PRESSED(2, INPUT_FRONTEND_RB) || get_key_pressed(VK_ADD))
		{
			APPLY_FORCE_TO_ENTITY(playerPed, 1, 0.0, 45.0, 0.0, 0.0, 0.0, 0.0, 1, 1, 1, 1, 0, 1);
		}

		DISABLE_CONTROL_ACTION(2, INPUT_PARACHUTE_DEPLOY, TRUE);
		if (IS_CONTROL_PRESSED(2, INPUT_FRONTEND_RDOWN) || get_key_pressed(VK_SUBTRACT))
		{
			FREEZE_ENTITY_POSITION(playerPed, true);
		}
		else if (IS_CONTROL_JUST_RELEASED(2, INPUT_FRONTEND_RDOWN) || IsKeyJustUp(VK_SUBTRACT))
		{
			FREEZE_ENTITY_POSITION(playerPed, false);
		}
	}

	if (IS_CONTROL_PRESSED(2, INPUT_FRONTEND_RT) || IsKeyDown(VK_NUMPAD7))
	{
		if (!isInParaFreeFall)
		{
			TASK_PARACHUTE(playerPed, true, false);
		}
		APPLY_FORCE_TO_ENTITY(playerPed, 1, 0.0, 0.0, 13.0, 0.0, 0.0, 0.0, 1, 1, 1, 1, 0, 1);
	}

	if (IS_CONTROL_PRESSED(2, INPUT_FRONTEND_LT) || IsKeyDown(VK_NUMPAD1))
	{
		if (!isInParaFreeFall)
		{
			TASK_PARACHUTE(playerPed, true, false);
		}
		APPLY_FORCE_TO_ENTITY(playerPed, 1, 0.0, 0.0, -13.0, 0.0, 0.0, 0.0, 1, 1, 1, 1, 0, 1);
	}

}
void SetPedSupermanAuto(Ped ped)
{
	if (IS_PED_IN_PARACHUTE_FREE_FALL(ped))
	{
		if (!NETWORK_HAS_CONTROL_OF_ENTITY(ped))
		{
			NETWORK_REQUEST_CONTROL_OF_ENTITY(ped);
		}

		APPLY_FORCE_TO_ENTITY(ped, 1, 0.0, 70.0f, 70.0f, 0.0, 0.0, 0.0, 0, 1, 0, 0, 0, 1);

		if (ped == PLAYER_PED_ID())
		{
			bool isBrakePressed, isBrakeReleased = false;
			if (Menu::bitController)
			{
				DISABLE_CONTROL_ACTION(2, INPUT_PARACHUTE_DEPLOY, TRUE);
				isBrakePressed = IS_CONTROL_PRESSED(2, INPUT_FRONTEND_RDOWN) != 0;
				if (!isBrakePressed)
				{
					isBrakeReleased = IS_CONTROL_JUST_RELEASED(2, INPUT_FRONTEND_RDOWN) != 0;
				}
			}
			else
			{
				isBrakePressed = IsKeyDown(VK_ADD);
				if (!isBrakePressed)
				{
					isBrakeReleased = IsKeyJustUp(VK_ADD);
				}
			}
			if (isBrakePressed)
			{
				FREEZE_ENTITY_POSITION(ped, true);
			}
			else if (isBrakeReleased)
			{
				FREEZE_ENTITY_POSITION(ped, false);
			}
		}

	}
}

void SetLocalForcefield()
{
	GTAentity myPed = PLAYER_PED_ID();
	const Vector3& myPos = myPed.GetPosition();
	switch (forceField)
	{
	case 1: //push out
		for (GTAentity ent : worldEntities)
		{
			if (ent.Handle() != myPed.Handle() && ent.Handle() != g_myVeh)
			{
				const Vector3& entPos = ent.GetPosition();
				if (myPos.DistanceTo(entPos) < 10.0f)
				{
					ent.ApplyForce(entPos - myPos, ForceType::MaxForceRot2);
				}
			}
		}
		break;
	case 2://explode
		myPed.SetExplosionProof(true);
		World::AddExplosion(myPos, EXPLOSION::BLIMP, 5.0f, 0.0f, false, false);
		break;
	}
}

void DriveOnWater(GTAped ped, Entity& waterobject)
{
	if (ped.IsInVehicle())
	{
		ped = ped.CurrentVehicle();
	}

	if (!DOES_ENTITY_EXIST(waterobject))
	{
		Model objModel = 0xC42C019A; // prop_ld_ferris_wheel
		objModel.LoadAndWait();
		const Vector3& Pos = ped.GetOffsetInWorldCoords(0, 4.0f, 0);
		float whh = 0.0f;
		if (GET_WATER_HEIGHT_NO_WAVES(Pos.x, Pos.y, Pos.z, &whh))
		{
			ped.RequestControl();
			SET_ENTITY_COORDS(ped.Handle(), Pos.x, Pos.y, whh, 0, 0, 0, 1);
		}
		waterobject = CREATE_OBJECT(objModel.hash, Pos.x, Pos.y, whh - 4.0f, 1, 1, 1);
		SET_NETWORK_ID_CAN_MIGRATE(OBJ_TO_NET(waterobject), ped != PLAYER_PED_ID());
		SET_ENTITY_COORDS_NO_OFFSET(waterobject, Pos.x, Pos.y, whh, 0, 0, 0);
		SET_ENTITY_ROTATION(waterobject, 0, 90, 0, 2, 1);
		FREEZE_ENTITY_POSITION(waterobject, true);
		Game::Print::PrintBottomCentre("~b~Note:~s~ Enable again if water level is incorrect/changes.");
		WAIT(65);
		return;
	}

	const Vector3& myPos = ped.GetPosition();
	const Vector3& Pos = GET_ENTITY_COORDS(waterobject, 1);

	if (ped.IsInWater())
	{
		float whh = 0.0f;
		if (GET_WATER_HEIGHT_NO_WAVES(Pos.x, Pos.y, Pos.z, &whh))
		{
			SET_ENTITY_COORDS_NO_OFFSET(waterobject, Pos.x, Pos.y, whh, 0, 0, 0);
		}
	}

	if (!NETWORK_HAS_CONTROL_OF_ENTITY(waterobject))
	{
		NETWORK_REQUEST_CONTROL_OF_ENTITY(waterobject);
	}
	SET_ENTITY_COORDS_NO_OFFSET(waterobject, myPos.x, myPos.y, Pos.z, 1, 1, 1);
	SET_ENTITY_ROTATION(waterobject, 180.0f, 90.0f, 180.0f, 2, 1);
	SET_ENTITY_VISIBLE(waterobject, false, false);
	FREEZE_ENTITY_POSITION(waterobject, true);


}

void SetPedBurnMode(GTAped ped, bool enable)
{
	auto isOnFire = ped.IsOnFire();

	if (enable && !isOnFire && !ped.IsInWater())
	{
		ped.SetOnFire(enable);
	}
	else if (!enable && isOnFire)
	{
		ped.SetOnFire(enable);
	}

}

void SetWalkUnderwater(Entity PlayerPed)
{
	if (IS_ENTITY_IN_WATER(PlayerPed))
	{
		SET_PED_CONFIG_FLAG(PlayerPed, ePedConfigFlags::IsSwimming, false);
		SET_PED_CONFIG_FLAG(PlayerPed, ePedConfigFlags::WasSwimming, false);
		SET_PED_CONFIG_FLAG(PlayerPed, ePedConfigFlags::_0xD8072639, false);

		Vector3 PlayerPos = GET_ENTITY_COORDS(PlayerPed, 0);
		DRAW_LIGHT_WITH_RANGEEX(PlayerPos.x, PlayerPos.y, (PlayerPos.z + 1.5f), 255, 255, 251, 100.0f, 1.5f, 0.0f);
		DRAW_LIGHT_WITH_RANGEEX(PlayerPos.x, PlayerPos.y, (PlayerPos.z + 50.0f), 255, 255, 251, 200.0f, 1.0f, 0.0f);

		if (IS_PED_JUMPING(PlayerPed)) // small pushup so jump feel more natural ( like when not underwater )
		{
			APPLY_FORCE_TO_ENTITY(PlayerPed, true, 0, 0, 0.7f, 0, 0, 0, true, true, true, true, false, true);
		}

		if (GET_ENTITY_HEIGHT_ABOVE_GROUND(PlayerPed) > 1) //Do falling down
		{
			SET_PED_CONFIG_FLAG(PlayerPed, ePedConfigFlags::IsStanding, false);
			SET_PED_CONFIG_FLAG(PlayerPed, ePedConfigFlags::WasStanding, false);
			SET_PED_CONFIG_FLAG(PlayerPed, ePedConfigFlags::OpenDoorArmIK, false);
			SET_PED_CONFIG_FLAG(PlayerPed, ePedConfigFlags::EdgeDetected, false);
			SET_PED_CONFIG_FLAG(PlayerPed, ePedConfigFlags::IsInTheAir, true);
			APPLY_FORCE_TO_ENTITY(PlayerPed, true, 0, 0, -0.7f, 0, 0, 0, true, true, true, true, false, true);
		}

		if (GET_IS_TASK_ACTIVE(PlayerPed, 281) || IS_PED_SWIMMING(PlayerPed) || IS_PED_SWIMMING_UNDER_WATER(PlayerPed)) // Stop Swimming
		{
			CLEAR_PED_TASKS_IMMEDIATELY(PlayerPed);
		}
	}
}
