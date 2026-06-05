#pragma once

#include <string>
#include "../Menu/Routine.h"

typedef unsigned __int8 UINT8;
typedef int INT, Entity, Ped, Vehicle, Object, ScrHandle;
typedef unsigned long DWORD, Hash;
typedef float FLOAT;
typedef char *PCHAR;

class RGBA;
class GTAped;
namespace GTAmodel 
{
	class Model;
}

extern Ped g_WeaponOpsPedOverride;
extern Player g_WeaponOpsPlayerOverride;
extern Ped g_WeaponMenuPedOverride;

class Vector3;
class GTAplayer;
class GTAentity;
class GTAvehicle;
namespace PTFX
{
	class sFxData;
}

extern Hash kaboomGunHash;
extern Hash bullet_gun_hash;
extern GTAmodel::Model pedGunHash;
extern GTAmodel::Model objectGunHash;
extern PTFX::sFxData triggerFXGunData;
extern Hash g_myWeap;

extern bool kaboomGunInvis;
extern bool kaboomGunRandBit;
extern bool pedGunRandBit;
extern bool objectGunRandBitO;
extern bool objectGunRandBitV;

extern bool forgeGun;
extern float forgeDist;
extern float g_forgeGunPrecision;
extern float g_forgeGunShootForce;
extern bool objectSpawnForgeAssistance;

extern bool kaboomGun;
extern bool bulletGun;
extern bool pedGun;
extern bool objectGun;
extern bool lightGun;
extern bool triggerFXGun;
extern bool teleportGun;

extern bool explosiveRounds;
extern bool flamingRounds;
extern bool explosiveMelee;
extern bool bulletTime;
extern bool selfTriggerbot;

extern bool tripleBullets;
extern bool rapidFire;
extern bool selfResurrectionGun;
extern bool soulSwitchGun;
extern bool selfDeleteGun;

extern float weaponDamageIncrease;

extern bool bitInfiniteAmmo;
extern Entity bitInfiniteAmmoEnth;
extern bool selfInfiniteParachutes;

extern UINT8 autoKillEnemies;

extern Entity targetSlotEntity;
extern bool targetEntityLocked;

extern bool bitGravityGunDisabled;

extern bool hvSnipers;

extern UINT8 explostionWP;

extern bool clearWeaponPickups;

enum class WeaponTargetType
{
	TargetPlayer,
	TargetPed
};

inline WeaponTargetType g_WeaponTargetType = WeaponTargetType::TargetPlayer;
inline Ped g_WeaponTargetPed = 0;

struct ScopedWeaponTargetOverride
{
	WeaponTargetType oldType;
	Ped oldPed;

	ScopedWeaponTargetOverride(Ped ped)
	{
		oldType = g_WeaponTargetType;
		oldPed = g_WeaponTargetPed;

		g_WeaponTargetType = WeaponTargetType::TargetPed;
		g_WeaponTargetPed = ped;
	}

	~ScopedWeaponTargetOverride()
	{
		g_WeaponTargetType = oldType;
		g_WeaponTargetPed = oldPed;
	}
};

void SetExplosionAtCoords(GTAentity entity, Vector3 pos, UINT8 type, float radius, float camshake, bool sound, bool invis, GTAentity owner);
void SetTeleportGun();
void SetBulletGun();
void SetPedGun();
void SetObjectGun();
void SetLightGun();
void SetTripleBullets();
void SetRapidFire();
void SetSoulSwitchGun();
void SetSelfDeleteGun();
void SetSelfResurrectionGun();
void SetHVSnipers(bool set);
void SetForgeGunDist(float& distance);
void SetForgeGun();
void SetExplosionAtBulletHit(Ped ped, Hash type, bool invisible);
void SetTriggerFXAtBulletHit(Ped ped, const std::string& fxAsset, const std::string& fxName, const Vector3& rot, float scale);
void SetTargetIntoSlot();
void SetPlayerTriggerbot(GTAplayer player);
void SetPTFXLopTick();

namespace sub 
{ 
	namespace PtfxSubs 
	{ 
		struct PtfxS; 
	}
}

namespace sub
{
	namespace GravityGun_catind
	{

		void Tick();
		bool& Enabled();
		float& ShootForce();

		bool& MultipleEntities();
		unsigned char& TypeToPickUpIndex();
		Hash WHash();
	}

	namespace WeaponFavourites_catind
	{
		bool PopulateFavouritesInfo();
		bool IsWeaponAFavourite(Hash whash);
		bool AddWeaponToFavourites(Hash whash, const std::string& customName);
		bool RemoveWeaponFromFavourites(Hash whash);
	}

	namespace WeaponIndivs_catind
	{
	}

	namespace WeaponsLoadouts_catind
	{

		bool Create(GTAped ped, const std::string& filePath);
		bool Apply(GTAped ped, const std::string& filePath);
	}

	namespace LaserSight_catind
	{
		extern bool bEnabled;
		extern RGBA _colour;

		inline void DoSight();
		void Tick();
	}
}



