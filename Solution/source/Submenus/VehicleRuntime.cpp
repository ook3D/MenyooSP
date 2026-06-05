#include "VehicleRuntime.h"

#include "..\macros.h"

#include "..\Menu\Menu.h"
#include "..\Menu\Routine.h"
#include "PlayerRuntime.h"
#include "VehicleSpawnerRuntime.h"
#include "Neons.h"

#include "..\Natives\natives2.h"
#include "..\Natives\types.h" // RGBA/RgbS & types
#include "..\Scripting\enums.h"
#include "..\Scripting\GTAplayer.h"
#include "..\Scripting\GTAentity.h"
#include "..\Scripting\GTAvehicle.h"
#include "..\Scripting\GTAped.h"
#include "..\Scripting\Game.h"
#include "..\Scripting\GameplayCamera.h"
#include "..\Scripting\World.h"
#include "..\Scripting\PTFX.h"
#include "..\Scripting\Model.h"
#include "..\Scripting\ModelNames.h"
#include "..\Scripting\GTAblip.h"
#include "..\Scripting\WeaponIndivs.h"
#include "..\Util\keyboard.h"
#include "..\Util\FileLogger.h"

#include "PtfxData.h"

#include <string>
#include <map>
#include <unordered_set>
#include <vector>

Vehicle g_myVeh = 0;
GTAmodel::Model g_myVehModel;

FLOAT vehicleDamageAndDefense = 1.0f;
FLOAT vehicleSlam = 0.0f;

INT accelMult = 0;
INT brakeMult = 0;
INT handlingMult = 0;

INT16 g_frozenRadioStation = -1;

bool bitVehicleGravity = false;
bool bitFreezeVehicle = false;
bool bitVehicleSlippyTires = false;

bool multiPlatNeons = false;
bool multiPlatNeonsRainbow = false;
RgbS g_multiPlatNeonsColor(0, 255, 0);
std::vector<GTAvehicle> g_multiPlatNeonsList;
float g_multiPlatNeonsIntensity = 3.5f;

UINT8 carJump = 0;

bool vehiclePopulation = false;

bool vehicleInvincibility = false;
bool vehicleHeavyMass = false;
bool raceBoost = false;
bool carHydraulics = false;
bool superGrip = false;
bool superCarMode = false;
bool unlimitedVehicleBoost = true;

bool vehicleWeaponLines = false;
bool vehicleRPG = false;
bool vehicleFireworks = false;
bool vehicleGuns = false;
bool vehicleSnowballs = false;
bool vehicleBalls = false;
bool vehicleWaterHydrant = false;
bool vehicleLaserGreen = false;
bool vehicleFlameLeak = false;
bool vehicleLaserRed = false;
bool vehicleTurretsValkyrie = false;
bool vehicleFlaregun = false;
bool vehicleHeavySniper = false;
bool vehicleTazerWeapon = false;
bool vehicleMolotovWeapon = false;
bool vehicleCombatPDW = false;
bool carColorChange = false;
bool vehicleInvisibility = false;
bool selfEngineOn = false;
bool vehicleFixLoop = false;
bool vehicleFlipLoop = false;
bool vehicleDisableSiren = false;

GTAvehicle pvSubVehicleID;

std::map<Vehicle, float> g_multListRPM;
std::map<Vehicle, float> g_multListTorque;
std::map<Vehicle, float> g_multListMaxSpeed;
std::map<Vehicle, float> g_multListHeadLights;

std::map<Vehicle, std::string> g_vehListEngineSounds;

std::unordered_set<Vehicle> g_vehWheelsInvisForRussian;

void SetVehicleNosPTFXThisFrame(GTAvehicle vehicle)
{
	WeaponS ptfx1 = { "scr_rcbarry1", "scr_alien_teleport" };
	if (!HAS_NAMED_PTFX_ASSET_LOADED(ptfx1.label.c_str()))
	{
		REQUEST_NAMED_PTFX_ASSET(ptfx1.label.c_str());
	}
	else
	{
		Vector3 dim1, dim2;
		vehicle.ModelDimensions(dim1, dim2);
		USE_PARTICLE_FX_ASSET(ptfx1.label.c_str());
		SET_PARTICLE_FX_NON_LOOPED_COLOUR(float(titlebox.R) / 255.0f, float(titlebox.G) / 255.0f, float(titlebox.B) / 255.0f);
		START_NETWORKED_PARTICLE_FX_NON_LOOPED_ON_ENTITY(ptfx1.name.c_str(), vehicle.Handle(), dim1.x - 0.2f, 0.4 - dim2.y, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0, 0, 0);
		USE_PARTICLE_FX_ASSET(ptfx1.label.c_str());
		SET_PARTICLE_FX_NON_LOOPED_COLOUR(float(titlebox.R) / 255.0f, float(titlebox.G) / 255.0f, float(titlebox.B) / 255.0f);
		START_NETWORKED_PARTICLE_FX_NON_LOOPED_ON_ENTITY(ptfx1.name.c_str(), vehicle.Handle(), 0.2f - dim2.x, 0.4 - dim2.y, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0, 0, 0);
	}

	PTFX::NonLoopedPTFX muzzleFlash("scr_carsteal4", "scr_carsteal5_car_muzzle_flash");
	if (!muzzleFlash.IsAssetLoaded())
	{
		muzzleFlash.LoadAsset();
	}
	else
	{
		const Vector3& otherWayRot = vehicle.Rotation_get() + Vector3(0, 0, -90.0f);
		for (auto& exh : { VBone::exhaust, VBone::exhaust_2 })
		{
			muzzleFlash.Start(vehicle.GetBoneCoords(vehicle.GetBoneIndex(exh)), 1.0f, otherWayRot);
		}
	}
}

void SetSuperCarModeSelf()
{
	if (IS_VEHICLE_ON_ALL_WHEELS(g_myVeh))
	{
		if (IS_CONTROL_PRESSED(2, INPUT_VEH_ACCELERATE))
		{
			SET_VEHICLE_BOOST_ACTIVE(g_myVeh, 1);
			SET_VEHICLE_BOOST_ACTIVE(g_myVeh, 0);
			SET_VEHICLE_FORWARD_SPEED(g_myVeh, GET_ENTITY_SPEED_VECTOR(g_myVeh, true).y + 0.46f);
			SetVehicleNosPTFXThisFrame(GTAvehicle(g_myVeh));
		}
		if (IS_CONTROL_JUST_PRESSED(2, INPUT_VEH_BRAKE))
		{
			SET_VEHICLE_FORWARD_SPEED(g_myVeh, 0.0f);
		}
		else if (IS_CONTROL_PRESSED(2, INPUT_VEH_BRAKE))
		{
			APPLY_FORCE_TO_ENTITY(g_myVeh, 1, 0.0, -0.4, 0.0, 0.0, 0.0, 0.0, 0, 1, 1, 1, 0, 1);
		}
	}
}

void SetLocalCarJump()
{
	if (!IS_MP_TEXT_CHAT_TYPING())
	{
		Model& model = g_myVehModel;

		if (model.IsPlane() || model.IsHeli())
		{
			return;
		}

		GTAvehicle vehicle = g_myVeh;
		bool bPressed;

		switch (carJump)
		{
		case 1: // Tap
			if (vehicle.Model().IsBicycle())
			{
				bPressed = IS_CONTROL_JUST_PRESSED(2, INPUT_VEH_JUMP);
			}
			else
			{
				bPressed = Menu::bitController ? IS_CONTROL_JUST_PRESSED(2, INPUT_FRONTEND_RDOWN) : IS_CONTROL_JUST_PRESSED(2, INPUT_VEH_HANDBRAKE);
			}
			if (bPressed)
			{
				vehicle.ApplyForceCustom(Vector3(0, 0, 3.6f), Vector3(0, 1.2f, 0), ForceType::MaxForceRot, true, false, true, true, true, true);
				vehicle.ApplyForceCustom(Vector3(0, 0, 3.6f), Vector3(0, -1.0f, 0), ForceType::MaxForceRot, true, false, true, true, true, true);
			}
			break;
		case 2: // Hold
			if (vehicle.Model().IsBicycle())
			{
				bPressed = IS_CONTROL_PRESSED(2, INPUT_VEH_JUMP);
			}
			else
			{
				bPressed = Menu::bitController ? IS_CONTROL_PRESSED(2, INPUT_FRONTEND_RDOWN) : IS_CONTROL_PRESSED(2, INPUT_VEH_HANDBRAKE);
			}
			if (bPressed)
			{
				vehicle.ApplyForceCustom(Vector3(0, 0, 0.6f), Vector3(0, 0.06f, 0), ForceType::MaxForceRot, true, false, true, true, true, true);
			}
			break;
		}
	}
}

void SetLocalCarHydraulics()
{
	GTAvehicle vehicle = g_myVeh;

	if ((Menu::bitController ? IS_DISABLED_CONTROL_PRESSED(2, INPUT_FRONTEND_LS) : get_key_pressed(VirtualKey::LeftShift)) && vehicle.IsOnAllWheels())
	{
		Vector2 normal;
		if (Menu::bitController)
		{
			DISABLE_CONTROL_ACTION(2, INPUT_VEH_HORN, true);
			normal.x = GET_CONTROL_NORMAL(2, INPUT_SCRIPT_LEFT_AXIS_X);
			normal.y = GET_CONTROL_NORMAL(2, INPUT_SCRIPT_LEFT_AXIS_Y);
		}
		else
		{
			if (IS_DISABLED_CONTROL_PRESSED(2, INPUT_SCRIPT_PAD_RIGHT) || IsKeyDown('D'))
			{
				normal.x = 1.0f;
			}
			else if (IS_DISABLED_CONTROL_PRESSED(2, INPUT_SCRIPT_PAD_LEFT) || IsKeyDown('A'))
			{
				normal.x = -1.0f;
			}
			else if (IS_DISABLED_CONTROL_PRESSED(2, INPUT_SCRIPT_PAD_UP) || IsKeyDown('W'))
			{
				normal.y = 1.0f;
			}
			else if (IS_DISABLED_CONTROL_PRESSED(2, INPUT_SCRIPT_PAD_DOWN) || IsKeyDown('S'))
			{
				normal.y = -1.0f;
			}
		}

		if (normal.x > 0.4f || normal.x < -0.4f)
		{
			vehicle.ApplyForceCustom(Vector3(0, 0, abs(normal.x) * 0.294f), Vector3((normal.x > 0 ? 1 : -1) * 0.98f, 0, 0), ForceType::MaxForceRot, true, true, true, true, false, true);
		}
		if (normal.y > 0.4f || normal.y < -0.4f)
		{
			vehicle.ApplyForceCustom(Vector3(0, 0, abs(normal.y) * 0.164f), Vector3(0, (normal.y > 0 ? 1 : -1) * 3.05f, 0), ForceType::MaxForceRot, true, true, true, true, false, true);
		}
	}
}

void DrawVehicleAmbientLightNeons(const GTAvehicle& vehicle, const RgbS& colour)
{
	Vector3 dim1, dim2;
	vehicle.ModelDimensions(dim1, dim2);

	World::DrawLightWithRange(vehicle.GetOffsetInWorldCoords(0.0f, dim1.y - 0.3f, -0.4f), colour, 5.0f, g_multiPlatNeonsIntensity);
	World::DrawLightWithRange(vehicle.GetOffsetInWorldCoords(0.0f, 0.3f - dim2.y, -0.4f), colour, 5.0f, g_multiPlatNeonsIntensity);
	World::DrawLightWithRange(vehicle.GetOffsetInWorldCoords(dim1.x - 0.3f, 0.64f, -0.4f), colour, 5.0f, g_multiPlatNeonsIntensity);
	World::DrawLightWithRange(vehicle.GetOffsetInWorldCoords(0.3f - dim2.x, 0.64f, -0.4f), colour, 5.0f, g_multiPlatNeonsIntensity);

}

void SetMultiPlatNeons()
{
	bool already = false;
	auto& theList = g_multiPlatNeonsList;
	auto& colour = g_multiPlatNeonsColor;

	if (multiPlatNeonsRainbow) // rainbow loop
	{
		colour = g_fadedRGB;
	}

	if (IS_PED_IN_ANY_VEHICLE(PLAYER_PED_ID(), 0))
	{
		for (auto& veh : theList)
		{
			if (veh.Equals(g_myVeh)) { already = true; break; }
		}
		if (!already)
		{
			theList.push_back((g_myVeh));
		}
	}

	for (auto it = theList.begin(); it != theList.end();)
	{
		if (!it->Exists() || it->IsDead())
		{
			it = theList.erase(it);
			continue;
		}
		DrawVehicleAmbientLightNeons(*it, g_multiPlatNeonsColor);
		++it;
	}

}

void SetVehicleInvincibleOn(Vehicle vehicle)
{
	SET_VEHICLE_CAN_BREAK(vehicle, 0);
	SET_VEHICLE_CAN_BE_VISIBLY_DAMAGED(vehicle, 0);
	SET_VEHICLE_TYRES_CAN_BURST(vehicle, 0);
	SET_ENTITY_CAN_BE_DAMAGED(vehicle, 0);
	SET_ENTITY_INVINCIBLE(vehicle, 1);
	SET_ENTITY_PROOFS(vehicle, 1, 0, 1, 1, 1, 1, 1, 1);
}

void SetVehicleInvincibleOff(Vehicle vehicle)
{
	SET_VEHICLE_CAN_BREAK(vehicle, 1);
	SET_VEHICLE_CAN_BE_VISIBLY_DAMAGED(vehicle, 1);
	SET_ENTITY_CAN_BE_DAMAGED(vehicle, 1);
	SET_ENTITY_INVINCIBLE(vehicle, 0);
	SET_ENTITY_PROOFS(vehicle, 0, 0, 0, 0, 0, 0, 0, 0);
	SET_ENTITY_ONLY_DAMAGED_BY_PLAYER(vehicle, 0);
}

void SetVehicleFlip(GTAvehicle vehicle)
{
	FLOAT roll = GET_ENTITY_ROLL(vehicle.Handle());
	if (vehicle.IsUpsideDown() && (roll > 160 || roll < -160))
	{
		const Model& model = vehicle.Model();
		if (!vehicle.IsInAir() && !vehicle.IsInWater() && !model.IsPlane() && !model.IsHeli())
		{
			vehicle.RequestControlOnce();
			vehicle.SetRotation(Vector3(0, 0, vehicle.Rotation_get().z));
		}
	}
}

void SetVehicleRainbowMode(GTAvehicle vehicle, bool useFader)
{
	vehicle.RequestControlOnce();
	if (useFader)
	{
		vehicle.SetCustomPrimaryColour(g_fadedRGB);
		vehicle.SetCustomSecondaryColour(g_fadedRGB);
	}
	else
	{
		if (vehicle.IsPrimaryColorCustom())
		{
			vehicle.ClearCustomPrimaryColour();
		}
		if (vehicle.IsSecondaryColorCustom())
		{
			vehicle.ClearCustomSecondaryColour();
		}
		vehicle.SetPrimaryColour(rand() % 160);
		vehicle.SetSecondaryColour(rand() % 160);
	}
}
void SetVehicleNeonAnim(GTAvehicle vehicle)
{
	addlog(ige::LogType::LOG_TRACE, "SetVehicleNeonAnim called");
	if (g_Ped4 != g_myVeh)
	{
		loop_neon_fade = 0;
		loop_neon_flash = 0;
		loop_neon_rgb = 0;
		for (int i = 0; i < 4; i++) {
			neonstate[i] = 0;
		}
	}
	vehicle.RequestControlOnce();
	if (loop_neon_rgb)
	{
		g_setNeonColour = g_fadedRGB;
		vehicle.SetNeonLightsColour(g_setNeonColour);
		s_neonDirty = true;
	}
	static constexpr VehicleNeonLight kNeonLights[] = {
		VehicleNeonLight::Left,
		VehicleNeonLight::Right,
		VehicleNeonLight::Front,
		VehicleNeonLight::Back
	};
	switch (loop_neon_flash)
	{
		case 0:
		{
			for (auto light : kNeonLights)
				vehicle.SetNeonLightOn(light, neonstate[static_cast<int>(light)]);
			s_neonDirty = true;
			break;
		}
		case 1:
		{
			for (auto light : kNeonLights)
				vehicle.SetNeonLightOn(light, g_neonFlash * neonstate[static_cast<int>(light)]);
			s_neonDirty = true;
			break;
		}
		case 2:
		{
			for (auto light : kNeonLights)
				if (static_cast<int>(light) == g_neonSpin)
					vehicle.SetNeonLightOn(light, true);
				else
					vehicle.SetNeonLightOn(light, false);
			s_neonDirty = true;
			break;
		}
		case 3:
		{
			for (auto light : kNeonLights)
				if (static_cast<int>(light) == g_neonSpinBack)
					vehicle.SetNeonLightOn(light, true);
				else
					vehicle.SetNeonLightOn(light, false);
			s_neonDirty = true;
			break;
		}
		case 4:
		{
			for (auto light : kNeonLights)
				vehicle.SetNeonLightOn(light, g_neonFwk[static_cast<int>(light)]);
			s_neonDirty = true;
			break;
		}
	}

	switch (loop_neon_fade)
	{
	case 0:
	{
		vehicle.SetNeonLightsColour(g_setNeonColour);
		s_neonDirty = true;
		break;
	}
	case 1:
	{
		vehicle.SetNeonLightsColour(g_neonFade);
		s_neonDirty = true;
		break;
	}
	case 2:
	{
		vehicle.SetNeonLightsColour(g_neonHeart);
		s_neonDirty = true;
		break;
	}
	case 3:
	{
		vehicle.SetNeonLightsColour(g_neonShift);
		s_neonDirty = true;
		break;
	}
	case 4:
	{
		vehicle.SetNeonLightsColour(g_neonSlide);
		s_neonDirty = true;
		break;
	}
	}
}

void SetVehicleHeavyMass(GTAvehicle vehicle)
{
	if (!vehicle.Exists())
	{
		return;
	}

	float speed = vehicle.GetSpeed();
	if (speed < 0.5f)
	{
		return;
	}

	vehicle.RequestControlOnce();

	SetVehicleInvincibleOn(vehicle.Handle());
	vehicle.Repair(true); // Only if it needs a repair obv
	vehicle.SetFrictionOverride(100.0f);
	if (!vehicle.IsSeatFree(VehicleSeat::SEAT_DRIVER))
	{
		SetPedSeatbeltOn(vehicle.GetPedOnSeat(VehicleSeat::SEAT_DRIVER).Handle());
	}

	float pushForce = speed * 3.5f; // More speed === More bleed
	auto& vehicleArray = vehicle == g_myVeh ? nearbyVehicles : worldVehicles;

	for (auto& veh : vehicleArray)
	{
		if (vehicle != veh && vehicle.IsTouching(veh))
		{
			GTAvehicle(veh).ApplyForce(vehicle.CollisionNormal() * pushForce);
			vehicle.SetForwardSpeed(speed);
		}
	}

}

void SetSelfVehicleBoost()
{
	GTAvehicle vehicle(g_myVeh);

	if (!vehicle.Exists())
	{
		return;
	}

	vehicle.RequestControlOnce();
	SET_VEHICLE_BOOST_ACTIVE(vehicle.Handle(), true);
	APPLY_FORCE_TO_ENTITY(vehicle.Handle(), 1, 0.0f, 1.7f, 0.0f, 0.0f, 0.0f, 0.0f, 0, 1, 1, 1, 0, 1);
	SET_VEHICLE_BOOST_ACTIVE(vehicle.Handle(), false);

	SetVehicleNosPTFXThisFrame(vehicle);
}

void SetPVOpsVehicleTextWorld2Screen()
{
	if (pvSubVehicleID.Exists())
	{
		GTAblip carblip = pvSubVehicleID.CurrentBlip();
		GTAped playerPed = PLAYER_PED_ID();

		if (IS_PED_SITTING_IN_VEHICLE(playerPed.Handle(), pvSubVehicleID.Handle()))
		{
			if (carblip.Exists())
			{
				carblip.SetAlpha(0);
			}
			return;
		}

		if (carblip.Exists())
		{
			carblip.SetAlpha(255);
		}

		const Vector3& carpos = pvSubVehicleID.GetPosition();

		if (carpos.DistanceTo(playerPed.GetPosition()) < 40.0f)
		{
			Vector2 newScreenPos;
			if (GET_SCREEN_COORD_FROM_WORLD_COORD(carpos.x, carpos.y, carpos.z, &newScreenPos.x, &newScreenPos.y))
			{
				Game::Print::SetupDraw(GTAfont::Impact, Vector2(0.64f, 0.64f), true, false, false);
				if (g_vehiclePVOpsName)
					Game::Print::drawstring(pvSubVehicleID.Model().VehicleDisplayName(true) + " - PV", newScreenPos.x, newScreenPos.y);
			}
		}
	}
}

void VehicleTorqueMultiplier()
{
	for (auto it = g_multListTorque.begin(); it != g_multListTorque.end();)
	{
		if (DOES_ENTITY_EXIST(it->first))
		{
			SET_VEHICLE_CHEAT_POWER_INCREASE(it->first, it->second);
			++it;
		}
		else
		{
			it = g_multListTorque.erase(it);
		}
	}
}

void VehicleMaxSpeedMultiplier()
{
	for (auto it = g_multListMaxSpeed.begin(); it != g_multListMaxSpeed.end();)
	{
		if (DOES_ENTITY_EXIST(it->first))
		{
			SET_ENTITY_MAX_SPEED(it->first, it->second);
			++it;
		}
		else
		{
			it = g_multListMaxSpeed.erase(it);
		}
	}
}

std::string GetVehicleEngineSoundName(const GTAvehicle& vehicle)
{
	auto it = g_vehListEngineSounds.find(vehicle.GetHandle());
	if (it != g_vehListEngineSounds.end())
	{
		return it->second;
	}
	else
	{
		return std::string();
	}
}

void SetVehicleEngineSoundName(GTAvehicle vehicle, const std::string& name)
{
	g_vehListEngineSounds[vehicle.GetHandle()] = name;
	vehicle.SetEngineSound(name);
}

bool AreVehicleWheelsInvisible(const GTAvehicle& vehicle)
{
	return (g_vehWheelsInvisForRussian.find(vehicle.GetHandle()) != g_vehWheelsInvisForRussian.end());
}

void SetVehicleWheelsInvisible(GTAvehicle vehicle, bool enable)
{
	if (enable)
	{
		if (g_vehWheelsInvisForRussian.find(vehicle.Handle()) == g_vehWheelsInvisForRussian.end())
		{
			g_vehWheelsInvisForRussian.insert(vehicle.Handle());
		}

		vehicle.RequestControl(800);
		vehicle.SetForwardSpeed(DBL_MAX * DBL_MAX);
		WAIT(100);
		SET_VEHICLE_CHEAT_POWER_INCREASE(vehicle.Handle(), DBL_MAX * DBL_MAX);
		MODIFY_VEHICLE_TOP_SPEED(vehicle.Handle(), DBL_MAX * DBL_MAX);
		vehicle.ApplyForceRelative(Vector3(0, 0, -DBL_MAX * DBL_MAX));
		WAIT(100);
		if (g_multListRPM.count(vehicle.Handle()))
		{
			MODIFY_VEHICLE_TOP_SPEED(vehicle.Handle(), g_multListRPM[vehicle.Handle()]);
		}
		else
		{
			MODIFY_VEHICLE_TOP_SPEED(vehicle.Handle(), 1.0f);
		}
		if (g_multListTorque.count(vehicle.Handle()))
		{
			SET_VEHICLE_CHEAT_POWER_INCREASE(vehicle.Handle(), g_multListTorque[vehicle.Handle()]);
		}
		else
		{
			SET_VEHICLE_CHEAT_POWER_INCREASE(vehicle.Handle(), 1.0f);
		}
	}
	else
	{
		if (g_vehWheelsInvisForRussian.find(vehicle.Handle()) != g_vehWheelsInvisForRussian.end())
		{
			g_vehWheelsInvisForRussian.erase(vehicle.Handle());
		}

		vehicle.RequestControl(800);
		for (UINT i = 0; i <= 8; i++)
		{
			vehicle.FixTyre(i);
		}
		vehicle.Repair(false);

		SET_VEHICLE_CHEAT_POWER_INCREASE(vehicle.Handle(), 0.0f);
		MODIFY_VEHICLE_TOP_SPEED(vehicle.Handle(), 0.0f);
		WAIT(100);
		if (g_multListRPM.count(vehicle.Handle()))
		{
			MODIFY_VEHICLE_TOP_SPEED(vehicle.Handle(), g_multListRPM[vehicle.Handle()]);
		}
		else
		{
			MODIFY_VEHICLE_TOP_SPEED(vehicle.Handle(), 1.0f);
		}
		if (g_multListTorque.count(vehicle.Handle()))
		{
			SET_VEHICLE_CHEAT_POWER_INCREASE(vehicle.Handle(), g_multListTorque[vehicle.Handle()]);
		}
		else
		{
			SET_VEHICLE_CHEAT_POWER_INCREASE(vehicle.Handle(), 1.0f);
		}
	}
}

Vector3 vehicleWeaponsOriginR;
Vector3 vehicleWeaponsTargetR;
Vector3 vehicleWeaponsOriginL;
Vector3 vehicleWeaponsTargetL;

void SetExplosionWP(UINT8 mode)
{
	if (!IS_WAYPOINT_ACTIVE())
	{
		return;
	}

	float camshake = 0.0f;
	bool visible = true;

	switch (mode)
	{
	case 0: // off
		return;
		break;
	case 1: default: // visible + shaky
		camshake = 3.0f;
		break;
	case 2: // visible
		visible = true;
		break;
	case 3: // invisible
		visible = false;
		break;
	}

	Vector3 pos = GET_BLIP_INFO_ID_COORD(GET_FIRST_BLIP_INFO_ID(BlipIcon::Waypoint));
	ADD_EXPLOSION(pos.x, pos.y, pos.z, 16, 36.0f, 1, !visible, camshake, false);
	GET_GROUND_Z_FOR_3D_COORD(pos.x, pos.y, 600.0f, &pos.z, 0, 0);
	ADD_EXPLOSION(pos.x, pos.y, pos.z + 5.0f, 29, 36.0f, 1, !visible, camshake, false);
	ADD_EXPLOSION(pos.x, pos.y, pos.z + 20.0f, 29, 36.0f, 1, !visible, camshake, false);

}

void SetHandlingMultiplier()
{
	if (handlingMult == 0)
	{
		return;
	}
	if (!Menu::bitController)
	{
		if (IS_DISABLED_CONTROL_PRESSED(2, INPUT_SCRIPT_PAD_RIGHT) || IsKeyDown('D'))
		{
			APPLY_FORCE_TO_ENTITY(g_myVeh, 1, handlingMult / 220, 0.0, 0.0, 0.0, 0.0, 0.0, 0, 1, 1, 1, 0, 1);
		}
		if (IS_DISABLED_CONTROL_PRESSED(2, INPUT_SCRIPT_PAD_LEFT) || IsKeyDown('A'))
		{
			APPLY_FORCE_TO_ENTITY(g_myVeh, 1, (0 - handlingMult) / 220, 0.0, 0.0, 0.0, 0.0, 0.0, 0, 1, 1, 1, 0,1);
		}
	}
	else
	{
		FLOAT handling_mult69_7_control_normal = GET_DISABLED_CONTROL_NORMAL(2, INPUT_SCRIPT_LEFT_AXIS_X);
		if (handling_mult69_7_control_normal > 0.5f || handling_mult69_7_control_normal < -0.5f)
		{
			APPLY_FORCE_TO_ENTITY(g_myVeh, 1, (handlingMult * handling_mult69_7_control_normal) / 220, 0.0, 0.0, 0.0, 0.0f, 0.0, 0, 1, 1, 1, 0, 1);
		}
	}
}

void StoreVehicleWeaponPos(const GTAentity& vehicle)
{
	Vector3_t dim1, dim2;
	vehicle.Model().Dimensions(dim1, dim2);

	if (IS_DISABLED_CONTROL_PRESSED(0, INPUT_LOOK_BEHIND))
	{
		vehicleWeaponsOriginR = vehicle.GetOffsetInWorldCoords(dim1.x - 0.22f, 0.5f - dim2.y, 0.5f);
		vehicleWeaponsTargetR = vehicle.GetOffsetInWorldCoords(dim1.x - 0.22f, -(dim1.y + 350.0f), 0.5f);

		vehicleWeaponsOriginL = vehicle.GetOffsetInWorldCoords(0.22f - dim2.x, 0.5f - dim2.y, 0.5f);
		vehicleWeaponsTargetL = vehicle.GetOffsetInWorldCoords(0.22f - dim2.x, -(dim2.y + 350.0f), 0.5f);
	}
	else
	{
		vehicleWeaponsOriginR = vehicle.GetOffsetInWorldCoords(dim1.x - 0.22f, dim1.y - 0.5f, 0.5f);
		vehicleWeaponsTargetR = vehicle.GetOffsetInWorldCoords(dim1.x - 0.22f, dim1.y + 350.0f, 0.5f);

		vehicleWeaponsOriginL = vehicle.GetOffsetInWorldCoords(0.22f - dim2.x, dim1.y - 0.5f, 0.5f);
		vehicleWeaponsTargetL = vehicle.GetOffsetInWorldCoords(0.22f - dim2.x, dim1.y + 350.0f, 0.5f);
	}
}

void SetVehicleWeaponFire(Hash whash, float speed)
{
	const auto& owner = Game::PlayerPed();
	World::ShootBullet(vehicleWeaponsOriginR, vehicleWeaponsTargetR, owner, whash, 200, speed, true, true);
	World::ShootBullet(vehicleWeaponsOriginL, vehicleWeaponsTargetL, owner, whash, 200, speed, true, true);
}

void SetVehicleWeaponExplosion(const EXPLOSION::EXPLOSION& type)
{
	Vector3 targPosR;
	Vector3 targPosL;
	GTAvehicle vehicle(g_myVeh);

	if (IS_DISABLED_CONTROL_PRESSED(0, INPUT_LOOK_BEHIND))
	{
		targPosR = vehicle.GetOffsetInWorldCoords(1.24f, -40.0f, 0.0f);
		targPosL = vehicle.GetOffsetInWorldCoords(-1.24f, -40.0f, 0.0f);
	}
	else
	{
		targPosR = vehicle.GetOffsetInWorldCoords(1.24f, 40.0f, 0.0f);
		targPosL = vehicle.GetOffsetInWorldCoords(-1.24f, 40.0f, 0.0f);
	}

	World::AddExplosion(targPosR, type, 5.0f, 0.3f, true, true);
	World::AddExplosion(targPosL, type, 5.0f, 0.3f, true, true);
}

void SetVehicleWeaponLines()
{
	World::DrawLine(vehicleWeaponsOriginR, vehicleWeaponsTargetR, RGBA(titlebox.R, titlebox.G, titlebox.B, 255));
	World::DrawLine(vehicleWeaponsOriginL, vehicleWeaponsTargetL, RGBA(titlebox.R, titlebox.G, titlebox.B, 255));
}

void SetVehicleWeapons()
{
	StoreVehicleWeaponPos(g_myVeh);
	if (vehicleWeaponLines)
	{
		SetVehicleWeaponLines();
	}

	if (Menu::bitController ? IS_CONTROL_PRESSED(2, INPUT_FRONTEND_LS) : IsKeyDown(VirtualKey::Add))
	{
		if (vehicleRPG
			|| vehicleFireworks
			|| vehicleGuns
			|| vehicleSnowballs
			|| vehicleBalls
			|| vehicleWaterHydrant
			|| vehicleFlameLeak
			|| vehicleLaserGreen
			|| vehicleLaserRed
			|| vehicleTurretsValkyrie
			|| vehicleFlaregun
			|| vehicleHeavySniper
			|| vehicleTazerWeapon
			|| vehicleMolotovWeapon
			|| vehicleCombatPDW)
			CLEAR_AREA_OF_PROJECTILES(vehicleWeaponsOriginR.x, vehicleWeaponsOriginR.y, vehicleWeaponsOriginR.z, 8.0f, 0);

		// RPG
		if (vehicleRPG)
		{
			SetVehicleWeaponFire(WEAPON_VEHICLE_ROCKET);
		}

		// Fireworks
		if (vehicleFireworks)
		{
			SetVehicleWeaponFire(WEAPON_FIREWORK);
		}

		// Guns
		if (vehicleGuns)
		{
			SetVehicleWeaponFire(WEAPON_ASSAULTRIFLE);
		}

		// Snowballs
		if (vehicleSnowballs)
		{
			SetVehicleWeaponFire(WEAPON_SNOWBALL, 2970.0f);
		}

		// Balls
		if (vehicleBalls)
		{
			SetVehicleWeaponFire(WEAPON_BALL, 2970.0f);
		}

		// Water hydrant explosions
		if (vehicleWaterHydrant)
		{
			SetVehicleWeaponExplosion(EXPLOSION::DIR_WATER_HYDRANT);
		}

		// Flame Explosions
		if (vehicleFlameLeak)
		{
			SetVehicleWeaponExplosion(EXPLOSION::DIR_FLAME_EXPLODE);
		}

		// Green laser
		if (vehicleLaserGreen)
		{
			SetVehicleWeaponFire(VEHICLE_WEAPON_PLAYER_LASER);
		}

		// Red laser
		if (vehicleLaserRed)
		{
			SetVehicleWeaponFire(VEHICLE_WEAPON_ENEMY_LASER);
		}

		// Valkyrie turrets
		if (vehicleTurretsValkyrie)
		{
			SetVehicleWeaponFire(VEHICLE_WEAPON_NOSE_TURRET_VALKYRIE);
		}

		// Flare gun/flare
		if (vehicleFlaregun)
		{
			SetVehicleWeaponFire(WEAPON_FLARE, 2970.0f);
		}

		// Heavy sniper
		if (vehicleHeavySniper)
		{
			SetVehicleWeaponFire(WEAPON_HEAVYSNIPER);
		}

		// Tazer
		if (vehicleTazerWeapon)
		{
			SetVehicleWeaponFire(WEAPON_STUNGUN, 2970.0f);
		}

		// Molotov
		if (vehicleMolotovWeapon)
		{
			SetVehicleWeaponFire(WEAPON_MOLOTOV, 2970.0f);
		}

		// Combat pdw
		if (vehicleCombatPDW)
		{
			SetVehicleWeaponFire(WEAPON_COMBATPDW);
		}
	}
}

void SetSelfVehicleNativeBoost()
{
	{
		const GTAentity& myPed = Game::PlayerPed();
		if (IS_PED_SITTING_IN_ANY_VEHICLE(myPed.GetHandle()) && DOES_ENTITY_EXIST(g_myVeh) && GET_HAS_ROCKET_BOOST(g_myVeh))
		{
			if (IS_CONTROL_PRESSED(2, INPUT_VEH_HORN))
			{
					SET_ROCKET_BOOST_FILL(g_myVeh, 1.0f);
					SET_ROCKET_BOOST_ACTIVE(g_myVeh, true);
			}
			else
			{
				SET_ROCKET_BOOST_ACTIVE(g_myVeh, false);
			}
		}
	}
}
