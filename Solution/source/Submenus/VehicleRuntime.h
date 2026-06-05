#pragma once

#include <string>
#include <map>
#include <unordered_set>
#include <vector>

#include "../Menu/Routine.h"

typedef unsigned __int8 UINT8;
typedef signed short INT16;
typedef int INT, Entity, Ped, Vehicle;
typedef unsigned long DWORD, Hash;
typedef float FLOAT;

class RgbS;
class Vector3;
class GTAplayer;
class GTAentity;
class GTAvehicle;
class GTAped;
namespace GTAmodel
{
	class Model;
}
namespace PTFX
{
	class sFxData;
}
namespace EXPLOSION
{
	enum EXPLOSION : UINT8;
}

extern Vehicle g_myVeh;
extern GTAmodel::Model g_myVehModel;

extern FLOAT vehicleDamageAndDefense;
extern FLOAT vehicleSlam;

extern INT accelMult;
extern INT brakeMult;
extern INT handlingMult;

extern INT16 g_frozenRadioStation;

extern bool bitVehicleGravity;
extern bool bitFreezeVehicle;
extern bool bitVehicleSlippyTires;

extern bool multiPlatNeons;
extern bool multiPlatNeonsRainbow;
extern RgbS g_multiPlatNeonsColor;
extern std::vector<GTAvehicle> g_multiPlatNeonsList;
extern float g_multiPlatNeonsIntensity;

extern UINT8 carJump;
extern bool carHydraulics;

extern bool vehiclePopulation;

extern bool vehicleInvincibility;
extern bool vehicleHeavyMass;

extern bool raceBoost;
extern bool superGrip;
extern bool superCarMode;
extern bool unlimitedVehicleBoost;

extern bool vehicleWeaponLines;
extern bool vehicleRPG;
extern bool vehicleFireworks;
extern bool vehicleGuns;
extern bool vehicleSnowballs;
extern bool vehicleBalls;

extern bool vehicleWaterHydrant;
extern bool vehicleLaserGreen;
extern bool vehicleFlameLeak;
extern bool vehicleLaserRed;

extern bool vehicleTurretsValkyrie;
extern bool vehicleFlaregun;
extern bool vehicleHeavySniper;
extern bool vehicleTazerWeapon;
extern bool vehicleMolotovWeapon;
extern bool vehicleCombatPDW;

extern bool carColorChange;
extern bool vehicleInvisibility;
extern bool selfEngineOn;

extern bool vehicleFixLoop;
extern bool vehicleFlipLoop;

extern bool vehicleDisableSiren;

extern GTAvehicle pvSubVehicleID;

extern std::map<Vehicle, float> g_multListRPM;
extern std::map<Vehicle, float> g_multListTorque;
extern std::map<Vehicle, float> g_multListMaxSpeed;
extern std::map<Vehicle, float> g_multListHeadLights;

extern std::map<Vehicle, std::string> g_vehListEngineSounds;

extern std::unordered_set<Vehicle> g_vehWheelsInvisForRussian;

void SetVehicleNosPTFXThisFrame(GTAvehicle vehicle);
void SetSuperCarModeSelf();
void SetLocalCarJump();
void SetLocalCarHydraulics();
void DrawVehicleAmbientLightNeons(const GTAvehicle& vehicle, const RgbS& colour);
void SetMultiPlatNeons();
void SetVehicleInvincibleOn(Vehicle vehicle);
void SetVehicleInvincibleOff(Vehicle vehicle);
void SetVehicleFlip(GTAvehicle vehicle);
void SetVehicleRainbowMode(GTAvehicle vehicle, bool useFader);
void SetVehicleNeonAnim(GTAvehicle vehicle);
void SetVehicleHeavyMass(GTAvehicle vehicle);
void SetSelfVehicleBoost();
void SetPVOpsVehicleTextWorld2Screen();

void VehicleTorqueMultiplier();
void VehicleMaxSpeedMultiplier();

std::string GetVehicleEngineSoundName(const GTAvehicle& vehicle);
void SetVehicleEngineSoundName(GTAvehicle vehicle, const std::string& name);

bool AreVehicleWheelsInvisible(const GTAvehicle& vehicle);
void SetVehicleWheelsInvisible(GTAvehicle vehicle, bool enable);

extern Vector3 vehicleWeaponsOriginR;
extern Vector3 vehicleWeaponsTargetR;
extern Vector3 vehicleWeaponsOriginL;
extern Vector3 vehicleWeaponsTargetL;

void SetExplosionWP(UINT8 mode);
void SetHandlingMultiplier();
void StoreVehicleWeaponPos(const GTAentity& vehicle);
void SetVehicleWeaponFire(Hash whash, float speed = 2000.0f);
void SetVehicleWeaponExplosion(const EXPLOSION::EXPLOSION& type);
void SetVehicleWeaponLines();
void SetVehicleWeapons();
void SetSelfVehicleNativeBoost();
