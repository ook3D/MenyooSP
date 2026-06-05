#include "WeaponRuntime.h"

#include "..\macros.h"

#include "..\Menu\Menu.h"
#include "..\Menu\Routine.h"
#include "PlayerRuntime.h"
#include "Misc.h"

#include "..\Natives\natives2.h"
#include "..\Scripting\WeaponIndivs.h"
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
#include "..\Util\ExePath.h"
#include "..\Util\StringManip.h"
#include "..\Util\keyboard.h"
#include "..\Menu\FolderPreviewBmps.h"
#include "..\Menu\Ticks.h"
#include "..\Natives\types.h" //RGBA
#include "..\Scripting\Raycast.h"

#include "..\Misc\RopeGun.h"
#include "..\Misc\MagnetGun.h"
#include "..\Misc\FlameThrower.h"
#include "..\Misc\Gta2Cam.h"

#include "PtfxData.h"
#include "PedAnimationRuntime.h"
#include "VehicleRuntime.h"
#include "VehicleSpawnerRuntime.h"
#include "SettingsRuntime.h"
#include "Spooner\SpoonerEntity.h"
#include "Spooner\EntityManagement.h"

#include <Shlwapi.h> //PathIsDirectory
#pragma comment(lib, "Shlwapi.lib")
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <set>
#include <pugixml\src\pugixml.hpp>
#include <dirent\include\dirent.h>

int g_WeaponOpsPedOverride = 0;
int g_WeaponOpsPlayerOverride = -1;
Ped g_WeaponMenuPedOverride = 0;

namespace sub
{
	namespace GravityGun_catind
	{
		class GravityGun
		{
		private:
			const Hash _whash = WEAPON_MARKSMANPISTOL;
			bool bEnabled = false;
			bool bGunActive = true;
			bool bMultipleEntities = true;
			float distanceFromCam = 0;
			float shootForce = 69.0f;
			EntityType typeToPickUp = EntityType::ALL;
			std::set<GTAentity> entityArray;
		public:
			GravityGun()
			{
			}
			bool& Enabled()
			{
				return bEnabled;
			}
			const Hash& WHASH()
			{
				return _whash;
			}
			bool& MultipleEntities()
			{
				return bMultipleEntities;
			}
			float& ShootForce()
			{
				return shootForce;
			}
			EntityType& TypeToPickUp()
			{
				return typeToPickUp;
			}

			void Tick()
			{
				if (bEnabled)
				{
					DoGravityGunTick();
				}
			}
			inline void DoGravityGunTick()
			{
				GTAplayer player = PLAYER_ID();
				GTAped ped = PLAYER_PED_ID();

				if (g_myWeap == _whash && (player.IsFreeAiming() || player.IsTargetingAnything()))
				{
					if (bGunActive && StoreEntities())
					{
						Vector3 camPos = GameplayCamera::GetPosition();
						GTAentity firstEntity = *std::next(entityArray.begin(), 0);

						if (distanceFromCam == 0) distanceFromCam = camPos.DistanceTo(firstEntity.GetPosition());

						SetForgeGunDist(distanceFromCam); // Use buttons to change the hold distance value

						Vector3 targetPos = camPos + (GameplayCamera::GetDirection() * distanceFromCam);

						if (entityArray.size() == 1)
						{
							GTAentity entity = *entityArray.begin();
							entity.RequestControlOnce();
							entity.Oscillate(targetPos, 0.395f, 0.1f);
						}
						else
						{
							for (GTAentity entity : entityArray)
							{
								entity.RequestControlOnce();
								entity.Oscillate(targetPos, 0.5f, 0.3f);
							}
						}

						if (IS_DISABLED_CONTROL_JUST_PRESSED(0, INPUT_ATTACK))
						{
							PLAY_SOUND_FROM_ENTITY(-1, "Chopper_Destroyed", ped.Handle(), "FBI_HEIST_FIGHT_CHOPPER_SOUNDS", 0, 0);

							Vector3 force = GameplayCamera::GetDirection() * shootForce;

							for (GTAentity entity : entityArray)
							{
								entity.ApplyForce(force);
							}

							entityArray.clear();
							distanceFromCam = 0;
							bGunActive = false;
						}
					}
				}
				else
				{
					if (!entityArray.empty())
						entityArray.clear();
					distanceFromCam = 0;
					bGunActive = true;
				}

			}

			bool StoreEntities()
			{
				GTAplayer player = PLAYER_ID();
				GTAentity myPed = PLAYER_PED_ID();
				EntityType type;

				if (entityArray.empty())
				{
					GTAentity aimedEntity = World::EntityFromAimCamRay();
					if (!aimedEntity.Handle())
						return false;

					type = (EntityType)aimedEntity.Type();
					if (typeToPickUp == EntityType::ALL || typeToPickUp == type)
					{
						aimedEntity.FreezePosition(false);
						entityArray.insert(aimedEntity.Handle());
					}
					else
						return false;
				}

				if (bMultipleEntities)
				{
					for (GTAentity outside : worldEntities)
					{
						if (outside.Equals(myPed))
							continue;
						for (GTAentity inside : entityArray)
						{
							if (inside.Equals(outside))
								continue;

							type = (EntityType)outside.Type();

							if (inside.IsTouching(outside) && (typeToPickUp == EntityType::ALL || typeToPickUp == type))
							{
								outside.FreezePosition(false);
								entityArray.insert(outside);
								break;
							}
						}
					}
				}

				return true;
			}
		};
		GravityGun g_gravityGun;

		void Tick()
		{
			g_gravityGun.Tick();
		}
		bool& Enabled()
		{
			return g_gravityGun.Enabled();
		}
		float& ShootForce()
		{
			return g_gravityGun.ShootForce();
		}
		bool& MultipleEntities()
		{
			return g_gravityGun.MultipleEntities();
		}
		unsigned char& TypeToPickUpIndex()
		{
			return *reinterpret_cast<unsigned char*>(&g_gravityGun.TypeToPickUp());
		}
		Hash WHash()
		{
			return g_gravityGun.WHASH();
		}
	}

	namespace WeaponFavourites_catind
	{
		std::string xmlFavouriteWeapons = "FavouriteWeapons.xml";
		bool PopulateFavouritesInfo()
		{
			pugi::xml_document doc;
			if (doc.load_file((const char*)(GetPathffA(Pathff::Main, true) + xmlFavouriteWeapons).c_str()).status != pugi::status_ok)
				return false;
			pugi::xml_node nodeRoot = doc.document_element();
			for (auto nodeWeap = nodeRoot.child("Weapon"); nodeWeap; nodeWeap = nodeWeap.next_sibling("Weapon"))
			{
				Hash whash = nodeWeap.attribute("hash").as_uint();
				std::string customName = nodeWeap.attribute("customName").as_string();
				bool bIsPresentAlready = WeaponIndivs::vWeaponLabels.count(whash) != 0;
				WeaponIndivs::vWeaponLabels[whash] = customName;
				if (!bIsPresentAlready)
				{
					WeaponAndComponents wac(whash, {}, &WeaponIndivs::vCaptions_Tints);
					WeaponIndivs::vAllWeapons.back()->push_back(wac);
				}
			}
			return true;
		}
		bool IsWeaponAFavourite(Hash whash)
		{
			pugi::xml_document doc;
			if (doc.load_file((const char*)(GetPathffA(Pathff::Main, true) + xmlFavouriteWeapons).c_str()).status != pugi::status_ok)
				return false;
			pugi::xml_node nodeRoot = doc.document_element();
			return nodeRoot.find_child_by_attribute("hash", IntToHexString(whash, true).c_str()) != NULL;
		}
		bool AddWeaponToFavourites(Hash whash, const std::string& customName)
		{
			if (customName.empty())
				return false;

			pugi::xml_document doc;
			if (doc.load_file((const char*)(GetPathffA(Pathff::Main, true) + xmlFavouriteWeapons).c_str()).status != pugi::status_ok)
			{
				doc.reset();
				auto nodeDecleration = doc.append_child(pugi::node_declaration);
				nodeDecleration.append_attribute("version") = "1.0";
				nodeDecleration.append_attribute("encoding") = "ISO-8859-1";
				auto nodeRoot = doc.append_child("FavouriteWeapons");
				doc.save_file((const char*)(GetPathffA(Pathff::Main, true) + xmlFavouriteWeapons).c_str());
			}
			pugi::xml_node nodeRoot = doc.document_element();

			auto nodeOldLoc = nodeRoot.find_child_by_attribute("hash", IntToHexString(whash, true).c_str());
			if (nodeOldLoc) // If not null
			{
				nodeOldLoc.parent().remove_child(nodeOldLoc);
				for (auto wit = WeaponIndivs::vAllWeapons.back()->begin(); wit != WeaponIndivs::vAllWeapons.back()->end(); ++wit)
				{
					if (wit->weaponHash == whash)
					{
						WeaponIndivs::vAllWeapons.back()->erase(wit);
						break;
					}
				}
			}
			auto nodeNewLoc = nodeRoot.append_child("Weapon");
			nodeNewLoc.append_attribute("hash") = IntToHexString(whash, true).c_str();
			nodeNewLoc.append_attribute("customName") = customName.c_str();

			WeaponIndivs::vWeaponLabels[whash] = customName;
			WeaponAndComponents wac(whash, {}, &WeaponIndivs::vCaptions_Tints);
			WeaponIndivs::vAllWeapons.back()->push_back(wac);

			return (doc.save_file((const char*)(GetPathffA(Pathff::Main, true) + xmlFavouriteWeapons).c_str()));
		}
		bool RemoveWeaponFromFavourites(Hash whash)
		{
			pugi::xml_document doc;
			if (doc.load_file((const char*)(GetPathffA(Pathff::Main, true) + xmlFavouriteWeapons).c_str()).status != pugi::status_ok)
				return false;
			pugi::xml_node nodeRoot = doc.document_element();
			auto nodeOldLoc = nodeRoot.find_child_by_attribute("hash", IntToHexString(whash, true).c_str());
			if (nodeOldLoc) // If not null
			{
				nodeOldLoc.parent().remove_child(nodeOldLoc);
			}

			for (auto wit = WeaponIndivs::vAllWeapons.back()->begin(); wit != WeaponIndivs::vAllWeapons.back()->end(); ++wit)
			{
				if (wit->weaponHash == whash)
				{
					WeaponIndivs::vAllWeapons.back()->erase(wit);
					break;
				}
			}

			return (doc.save_file((const char*)(GetPathffA(Pathff::Main, true) + xmlFavouriteWeapons).c_str()));
		}
	}

	namespace WeaponIndivs_catind
	{
		using WeaponIndivs::WEAPONTYPE;
		using WeaponIndivs::vCategoryNames;
		using WeaponIndivs::vAllWeapons;
		using WeaponIndivs::get_weapon;
		using WeaponIndivs::vCaptions_ChuteTints;

		INT& selectedCategory = g_Ped4;
		INT& selectedWeapon = msCurrentPaintIndex;
	}

	namespace WeaponsLoadouts_catind
	{
		std::string& _searchStr = dict2;
		std::string& _name = dict;
		std::string& _dir = dict3;
		auto& _ped = g_Ped1;

		bool Create(GTAped ped, const std::string& filePath)
		{
			pugi::xml_document doc;

			auto nodeDecleration = doc.append_child(pugi::node_declaration);
			nodeDecleration.append_attribute("version") = "1.0";
			nodeDecleration.append_attribute("encoding") = "ISO-8859-1";

			auto nodeLoadout = doc.append_child("Loadout"); // Root
			nodeLoadout.append_attribute("menyoo_ver") = MENYOO_CURRENT_VER_;

			std::vector<s_Weapon_Components_Tint> wct;
			ped.StoreWeaponsInArray(wct);

			for (auto& c : wct)
			{
				auto nodeWeapon = nodeLoadout.append_child("Weapon");
				nodeWeapon.append_attribute("hash") = IntToHexString(c.weaponHash, true).c_str();
				nodeWeapon.append_attribute("tint") = c.tint;
				auto nodeComponents = nodeWeapon.append_child("Components");
				for (auto& cc : c.componentHashes)
				{
					auto nodeComponent = nodeComponents.append_child("Component");
					nodeComponent.append_attribute("hash") = IntToHexString(cc.first, true).c_str();
					nodeComponent.append_attribute("livery") = IntToHexString(cc.second, true).c_str();
				}
			}

			return (doc.save_file((const char*)filePath.c_str()));
		}
		bool Apply(GTAped ped, const std::string& filePath)
		{
			pugi::xml_document doc;
			if (doc.load_file((const char*)filePath.c_str()).status != pugi::status_ok)
				return false;

			auto nodeLoadout = doc.child("Loadout"); // Root

			std::vector<s_Weapon_Components_Tint> wct;
			for (auto nodeWeapon = nodeLoadout.child("Weapon"); nodeWeapon; nodeWeapon = nodeWeapon.next_sibling("Weapon"))
			{
				s_Weapon_Components_Tint c;
				c.weaponHash = nodeWeapon.attribute("hash").as_uint();
				c.tint = nodeWeapon.attribute("tint").as_int();
				auto nodeComponents = nodeWeapon.child("Components");
				for (auto nodeComponent = nodeComponents.first_child(); nodeComponent; nodeComponent = nodeComponent.next_sibling())
				{
					auto componentHashAttr = nodeComponent.attribute("hash");
					auto componentLiveryAttr = nodeComponent.attribute("livery");
					if (componentHashAttr) c.componentHashes.push_back({ componentHashAttr.as_uint(), componentLiveryAttr ? componentLiveryAttr.as_int() : -1 });
					else c.componentHashes.push_back({ nodeComponent.text().as_uint(), componentLiveryAttr ? componentLiveryAttr.as_int() : -1 });
				}
				wct.push_back(c);
			}

			if (NETWORK_IS_IN_SESSION())
				ped.RequestControl(600);
			ped.RemoveAllWeapons();
			ped.GiveWeaponsFromArray(wct);
			return true;
		}
	}

	namespace LaserSight_catind
	{
		bool bEnabled = false;
		RGBA _colour(255, 0, 0, 255);

		inline void DoSight()
		{
			GTAped myPed = PLAYER_PED_ID();
			if (!myPed.Exists())
				return;
			GTAplayer myPlayer = PLAYER_ID();

			Hash weap = myPed.GetWeapon();
			switch (GET_WEAPONTYPE_GROUP(weap))
			{
			case WeaponGroupHash::FireExtinguisher:
			case WeaponGroupHash::Melee:
			case WeaponGroupHash::Spillable:
			case WeaponGroupHash::Throwable:
				return; break;
			}

			RaycastResult ray;
			Vector3 hitCoord;
			GTAentity gun = WEAPON::GET_CURRENT_PED_WEAPON_ENTITY_INDEX(myPed.Handle(), 0);

			if (gun.Handle() != NULL)
			{
				if (!myPed.IsReloading() && (myPlayer.IsFreeAiming() || myPlayer.IsTargetingAnything()))
				{
					const Vector3& camDir = GameplayCamera::GetDirectionFromScreenCentre();
					const Vector3& camCoord = GameplayCamera::GetPosition();

					ray = RaycastResult::Raycast(camCoord, camDir, 300.0f, IntersectOptions::Everything, myPed);

					if (ray.DidHitAnything())
						hitCoord = ray.HitCoords() + Vector3(0, 0, -0.07f);
					else
					{
						hitCoord = camCoord + (camDir * 1000.0f);
					}


					Vector3 laserLaunchPos;
					Vector3 dotSize(0.016f, 0.016f, 0.016f);

					float distanceToEnd = myPed.GetPosition().DistanceTo(hitCoord);
					if (distanceToEnd < 7.0f)
						dotSize = Vector3(0.009f, 0.009f, 0.009f);
					if (myPed.IsShooting())
						hitCoord = hitCoord + Vector3(0, 0, distanceToEnd * sin(RadianToDegree(0.7f)));


					if (GET_WEAPONTYPE_GROUP(weap) == WeaponGroupHash::Sniper)
					{
						laserLaunchPos = gun.GetOffsetInWorldCoords(0, 0, -0.006f);
					}
					else
					{
						laserLaunchPos = gun.GetOffsetInWorldCoords(0, 0, -0.01f);
					}

					World::DrawLine(laserLaunchPos, hitCoord, _colour);
					World::DrawMarker(MarkerType::DebugSphere, hitCoord, Vector3(), Vector3(), dotSize, _colour);
				}
			}
		}
		void Tick()
		{
			if (LaserSight_catind::bEnabled && !GTA2Cam::g_gta2Cam.Enabled()) // This does not work well with the top down camera
			{
				DoSight();
			}
		}
	}
}

Hash g_myWeap = 0U;
PTFX::sFxData triggerFXGunData = { "scr_fbi4", "scr_fbi4_trucks_crash" };
Hash kaboomGunHash = EXPLOSION::DIR_WATER_HYDRANT;
Hash bullet_gun_hash = WEAPON_FLARE;
GTAmodel::Model pedGunHash = PedHash::KillerWhale;
GTAmodel::Model objectGunHash = VEHICLE_BUS;

bool kaboomGunInvis = false;
bool kaboomGunRandBit = false;
bool pedGunRandBit = false;
bool objectGunRandBitO = false;
bool objectGunRandBitV = false;

UINT8 explostionWP = 0;

UINT8 autoKillEnemies = 0;
float weaponDamageIncrease = 1.0f;

Entity bitInfiniteAmmoEnth = 0;

bool forgeGun = false;
bool explosiveRounds = false;
bool flamingRounds = false;
bool teleportGun = false;
bool kaboomGun = false;
bool triggerFXGun = false;
bool bulletGun = false;
bool pedGun = false;
bool objectGun = false;
bool lightGun = false;
bool bulletTime = false;
bool selfTriggerbot = false;
bool explosiveMelee = false;
bool clearWeaponPickups = false;
bool tripleBullets = false;
bool rapidFire = false;
bool selfResurrectionGun = false;
bool soulSwitchGun = false;
bool selfDeleteGun = false;
bool hvSnipers = false;
bool bitInfiniteAmmo = false;
bool selfInfiniteParachutes = false;

Entity targetSlotEntity = 0;
bool targetEntityLocked = false;
bool bitGravityGunDisabled = false;

float forgeDist = 6.0f;
float g_forgeGunPrecision = 0.2f;
float g_forgeGunShootForce = 300.0f;
bool objectSpawnForgeAssistance = false;

void SetPTFXLopTick()
{
	using sub::PtfxSubs::fxLoops;

	if (GET_GAME_TIMER() > Menu::delayedTimer)
	{
		for (auto it = fxLoops.begin(); it != fxLoops.end();)
		{
			if (!it->entity.Exists())
			{
				it = fxLoops.erase(it);
				continue;
			}

			switch ((EntityType)it->entity.Type())
			{
			case EntityType::PED:
				if (IS_PED_A_PLAYER(it->entity.Handle()) && it->entity.Handle() != PLAYER_PED_ID())
				{
					PTFX::TriggerPTFX(it->asset, it->fx, NULL, GET_PED_BONE_COORDS(it->entity.Handle(), Bone::SKEL_Head, 0.0f, 0.0f, 0.0f), it->entity.Rotation_get(), GET_RANDOM_FLOAT_IN_RANGE(0.76f, 1.4f));
				}
				else
				{
					PTFX::TriggerPTFX(it->asset, it->fx, it->entity, Vector3(), Vector3(), GET_RANDOM_FLOAT_IN_RANGE(0.76f, 1.4f), Bone::SKEL_Head);
				}
				break;
			default:
				PTFX::TriggerPTFX(it->asset, it->fx, it->entity, Vector3(), Vector3(), GET_RANDOM_FLOAT_IN_RANGE(0.76f, 1.4f));
				break;
			}
			++it;
		}
	}
}

void SetExplosionAtCoords(GTAentity entity, Vector3 pos, UINT8 type, float radius, float camshake, bool sound, bool invis, GTAentity owner)
{
	const Vector3& Pos = (entity.Handle() == 0) ? pos : entity.GetOffsetInWorldCoords(pos);

	if (owner.Handle() != 0 && owner.IsPed())
	{
		ADD_OWNED_EXPLOSION(owner.Handle(), Pos.x, Pos.y, Pos.z, type, radius, sound, invis, camshake);
	}
	else
	{
		ADD_EXPLOSION(Pos.x, Pos.y, Pos.z, type, radius, sound, invis, camshake, false);
	}
}

void SetTargetIntoSlot()
{
	GTAplayer player = PLAYER_ID();

	if (player.IsTargetingAnything() || player.IsFreeAiming())
	{
		if (rapidFire)
		{
			SetRapidFire();
		}
		if (selfTriggerbot)
		{
			SetPlayerTriggerbot(player);
		}
		if (soulSwitchGun)
		{
			SetSoulSwitchGun();
		}
		if (selfDeleteGun)
		{
			SetSelfDeleteGun();
		}
		if (selfResurrectionGun)
		{
			SetSelfResurrectionGun();
		}
		if (hvSnipers)
		{
			SetHVSnipers(true);
		}

		if (!targetEntityLocked)
		{
			targetSlotEntity = player.AimedEntity().GetHandle();
			if (targetSlotEntity == 0)
			{
				GTAentity aimedEntity = World::EntityFromAimCamRay();
				if (aimedEntity.Handle() != 0)
				{
					targetSlotEntity = aimedEntity.Handle();
				}
				else
				{
					targetSlotEntity = 0;
					return;
				}
			}

			if (IS_ENTITY_A_PED(targetSlotEntity))
			{
				if (IS_PED_SITTING_IN_ANY_VEHICLE(targetSlotEntity))
				{
					targetSlotEntity = GET_VEHICLE_PED_IS_IN(targetSlotEntity, 0);
				}
			}

			targetEntityLocked = true;
		}
	}
	else
	{
		if (hvSnipers)
		{
			SetHVSnipers(false);
		}
		targetEntityLocked = false;
		targetSlotEntity = 0;
	}
}

void SetPlayerTriggerbot(GTAplayer player)
{
	GTAentity playerPed = player.GetPed();

	if (player.IsTargetingAnything() || player.IsFreeAiming())
	{
		GTAentity target;
		if (GET_ENTITY_PLAYER_IS_FREE_AIMING_AT(player.Handle(), &target.Handle()))
		{
			if (target.IsPed() && target.IsAlive())
			{
				Hash weap = 0;
				GET_CURRENT_PED_WEAPON(playerPed.Handle(), &weap, true);

				if (IS_WEAPON_VALID(weap))
				{
					static const std::array<int, 1> triggerbotBoneList
					{
						{
							Bone::SKEL_Head,
						}
					};

					const Vector3& targetPos = GET_PED_BONE_COORDS(target.Handle(), triggerbotBoneList[rand() % triggerbotBoneList.size()], 0.0f, 0.0f, 0.0f);
					if (player.Handle() == PLAYER_ID())
					{
						SET_PED_SHOOTS_AT_COORD(playerPed.Handle(), targetPos.x, targetPos.y, targetPos.z, 0);
					}
					else
					{
						GTAentity gunObj = GET_CURRENT_PED_WEAPON_ENTITY_INDEX(playerPed.Handle(), 0);
						const Vector3& launchPos = gunObj.GetOffsetInWorldCoords(0, gunObj.Dim1().y, 0);
						CLEAR_AREA_OF_PROJECTILES(launchPos.x, launchPos.y, launchPos.z, 4.0f, 0);
						World::ShootBullet(launchPos, targetPos, playerPed, weap, 5, -1, true, true);
					}
				}
			}
		}
	}
}

void SetRapidFire()
{
	if (GET_WEAPONTYPE_GROUP(g_myWeap) == WeaponGroupHash::Melee)
	{
		return;
	}

	DISABLE_CONTROL_ACTION(0, INPUT_ATTACK, TRUE);
	DISABLE_CONTROL_ACTION(2, INPUT_ATTACK2, TRUE);

	if (IS_DISABLED_CONTROL_PRESSED(0, INPUT_ATTACK))
	{
		Player playerPed = PLAYER_PED_ID();
		GTAentity gunObj = GET_CURRENT_PED_WEAPON_ENTITY_INDEX(playerPed, 0);
		const Vector3& camDir = GameplayCamera::GetDirectionFromScreenCentre();
		const Vector3& camPos = GameplayCamera::GetPosition();
		const Vector3& launchPos = camPos + (camDir * (camPos.DistanceTo(gunObj.GetPosition()) + 0.4f));
		const Vector3& targPos = camPos + (camDir * 200.0f);

		CLEAR_AREA_OF_PROJECTILES(launchPos.x, launchPos.y, launchPos.z, 6.0f, 0);

		SET_CONTROL_SHAKE(0, 250, 125);
		World::ShootBullet(launchPos, targPos, playerPed, g_myWeap, 5, 24000.0f, true, true);
		World::ShootBullet(launchPos, targPos, playerPed, g_myWeap, 5, 24000.0f, true, true);
		STOP_CONTROL_SHAKE(0);
	}
}

void SetSoulSwitchGun()
{
	if (g_myWeap != WEAPON_COMBATPISTOL)
	{
		return;
	}

	DISABLE_CONTROL_ACTION(0, INPUT_ATTACK, TRUE);
	DISABLE_CONTROL_ACTION(2, INPUT_ATTACK2, TRUE);
	DISABLE_CONTROL_ACTION(2, INPUT_VEH_ATTACK, TRUE);
	DISABLE_CONTROL_ACTION(2, INPUT_VEH_ATTACK2, TRUE);

	GTAplayer player = PLAYER_ID();
	GTAentity playerPed = PLAYER_PED_ID();
	ScrHandle eHandle;
	if (GET_ENTITY_PLAYER_IS_FREE_AIMING_AT(player.Handle(), &eHandle))
	{
		GTAentity soulSwitchEntity = eHandle;
		if (soulSwitchEntity.IsPed() && soulSwitchEntity.IsAlive() && (IS_DISABLED_CONTROL_JUST_PRESSED(0, INPUT_ATTACK) || (IS_PED_IN_ANY_VEHICLE(playerPed.Handle(), false) && IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_VEH_ATTACK))))
		{
			if (IS_SPECIAL_ABILITY_ACTIVE(PLAYER_ID(), 0))
			{
				SPECIAL_ABILITY_DEACTIVATE_FAST(PLAYER_ID(), 0);
				WAIT(16);
			}

			Game::Sound::PlayFrontend("Knuckle_Crack_Hard_Cel", "MP_SNACKS_SOUNDSET");
			ANIMPOSTFX_PLAY("MinigameEndNeutral", 0, 0); // FocusIn
			SetBecomePed(soulSwitchEntity);

			SET_CONTROL_SHAKE(0, 4000, 210);
			STOP_CONTROL_SHAKE(0);

			if (g_Ped1 == playerPed.Handle())
			{
				g_Ped1 = PLAYER_PED_ID();
			}

			soulSwitchEntity.Handle() = 0;
			sub::Spooner::SpoonerEntity spe;
			spe.handle = playerPed;
			if (!(sub::Spooner::EntityManagement::GetEntityIndexInDb(spe) >= 0))
			{
				playerPed.NoLongerNeeded();
			}

			WAIT(64);

			playerPed = PLAYER_PED_ID();

			SET_CURRENT_PED_WEAPON(playerPed.Handle(), WEAPON_COMBATPISTOL, true);
			SET_PED_CURRENT_WEAPON_VISIBLE(playerPed.Handle(), 1, 1, 1, 0);
		}
	}
}

void SetSelfDeleteGun()
{
	if (g_myWeap == WEAPON_SNSPISTOL)
	{
		if (IS_PED_SHOOTING(PLAYER_PED_ID()))
		{
			GTAentity targEntity = World::EntityFromAimCamRay();

			if (targEntity.Handle())
			{
				sub::Spooner::SpoonerEntity ent;
				ent.handle = targEntity;
				sub::Spooner::EntityManagement::DeleteEntity(ent);
			}
		}
	}
}

void SetSelfResurrectionGun()
{
	if (g_myWeap == WEAPON_STUNGUN)
	{
		GTAentity myPed = PLAYER_PED_ID();
		if (IS_PED_SHOOTING(myPed.Handle()))
		{
			GTAped targPed = World::EntityFromAimCamRay();

			if (targPed.IsPed() && targPed.IsDead())
			{
				targPed.RequestControl();
				targPed.SetHealth(200);
				RESURRECT_PED(targPed.Handle());
				REVIVE_INJURED_PED(targPed.Handle());
				targPed.SetMaxHealth(400);
				targPed.SetHealth(200);
				SET_PED_GENERATES_DEAD_BODY_EVENTS(targPed.Handle(), false);
				SET_PED_CONFIG_FLAG(targPed.Handle(), ePedConfigFlags::IsInjured, 0);
				SET_PED_CONFIG_FLAG(targPed.Handle(), ePedConfigFlags::HasHurtStarted, 0);

				targPed.Task().ClearAllImmediately();
				TaskSequence seq;
				seq.AddTask().PlayAnimation("get_up@directional@movement@from_knees@standard", rand() % 3 == 1 ? "getup_r_90" : "getup_l_90", 8.0f, -8.0f, -1, AnimFlag::SecondTask, 0, false);
				seq.AddTask().WanderAround();
				seq.Close();
				seq.MakePedPerform(targPed);
				seq.Clear();
			}
		}
	}
}

void SetHVSnipers(bool set)
{
	if (set && g_myWeap == WEAPON_UNARMED)
	{
		return;
	}
	SET_SEETHROUGH(set);
}

void SetTeleportGun()
{
	GTAentity myPed = PLAYER_PED_ID();
	Hash weap;
	GET_CURRENT_PED_WEAPON(myPed.Handle(), &weap, true);
	if (weap == WEAPON_HEAVYPISTOL)
	{
		GTAentity ent = IS_PED_IN_ANY_VEHICLE(myPed.Handle(), false) ? g_myVeh : myPed;
		Vector3 targetPos;
		const Vector3& camPos = GameplayCamera::GetPosition();
		const Vector3& camDir = GameplayCamera::GetDirectionFromScreenCentre();
		auto ray = RaycastResult::Raycast(camPos, camDir, 15000.0f, IntersectOptions::Everything, myPed);
		if (ray.DidHitAnything())
		{
			if (ray.DidHitEntity())
			{
				const GTAentity& hitEntity = ray.HitEntity();
				if (hitEntity.IsVehicle() || !hitEntity.MissionEntity_get())
				{
					targetPos = hitEntity.GetPosition() + Vector3(0, 0, hitEntity.Dim2().z + ent.Dim1().z);
				}
				else
				{
					targetPos = ray.HitCoords();
				}
			}
			else
			{
				targetPos = ray.HitCoords();
			}

			if (!targetPos.IsZero())
			{
				ent.RequestControl();
				ent.SetPosition(targetPos);
				Game::Sound::PlayFrontend("Knuckle_Crack_Hard_Cel", "MP_SNACKS_SOUNDSET");
				ANIMPOSTFX_PLAY("ExplosionJosh3", 0, 0);
			}
		}
	}
}

void SetBulletGun()
{
	GTAentity playerPed = PLAYER_PED_ID();

	GTAentity gunObj = GET_CURRENT_PED_WEAPON_ENTITY_INDEX(playerPed.Handle(), 0);
	const Vector3& camDir = GameplayCamera::GetDirectionFromScreenCentre();
	const Vector3& camPos = GameplayCamera::GetPosition();
	const Vector3& launchPos = camPos + (camDir * (camPos.DistanceTo(gunObj.GetPosition()) + 0.4f));
	const Vector3& targPos = camPos + (camDir * 200.0f);

	CLEAR_AREA_OF_PROJECTILES(launchPos.x, launchPos.y, launchPos.z, 6.0f, 0);
	World::ShootBullet(launchPos, targPos, playerPed, bullet_gun_hash, 5, 2500.0f, true, true);
}

void SetPedGun()
{
	if (pedGunRandBit)
	{
		pedGunHash = GET_HASH_KEY(g_pedModels[rand() % (INT)g_pedModels.size()].first); pedGunHash.Load(200);
	}

	if (pedGunHash.IsLoaded())
	{
		GTAentity myPed = PLAYER_PED_ID();
		const Vector3& launchPos = GetCoordsFromCam(GameplayCamera::GetPosition().DistanceTo(myPed.GetPosition()) + pedGunHash.Dim2().y + 0.5f);
		const Vector3& Rot = GameplayCamera::GetRotation();

		GTAentity spawnedPed = CREATE_PED(PedType::Human, pedGunHash.hash, launchPos.x, launchPos.y, launchPos.z, Rot.z, 1, 1);
		spawnedPed.SetRotation(Rot);
		spawnedPed.ApplyForceRelative(Vector3(0, 300.0f, 0), Vector3(0, -0.65f, 0), ForceType::MaxForceRot);
		spawnedPed.NoLongerNeeded();
	}
	if (pedGunRandBit)
	{
		pedGunHash.Unload();
	}
}

void SetObjectGun()
{
	Entity tempPed = PLAYER_PED_ID();
	Entity tempEntity;

	if (objectGunRandBitO)
	{
		objectGunHash = GET_HASH_KEY(objectModels[rand() % (INT)objectModels.size()]);
	}
	else if (objectGunRandBitV)
	{
		objectGunHash = g_vehHashes[rand() % (INT)g_vehHashes.size()];
	}

	if (objectGunHash.IsInCdImage())
	{
		if (objectGunRandBitO || objectGunRandBitV)
		{
			objectGunHash.Load(160);
		}

		const Vector3& launchPos = GetCoordsFromCam(GameplayCamera::GetPosition().DistanceTo(GET_ENTITY_COORDS(tempPed, 1)) + objectGunHash.Dim2().y + 1.355f);
		const Vector3& Rot = GET_GAMEPLAY_CAM_ROT(2);

		if (objectGunHash.IsVehicle())
		{
			tempEntity = CREATE_VEHICLE(objectGunHash.hash, launchPos.x, launchPos.y, launchPos.z, GET_ENTITY_HEADING(tempPed), 1, 1, 0);
		}
		else
		{
			tempEntity = CREATE_OBJECT(objectGunHash.hash, launchPos.x, launchPos.y, launchPos.z, 1, 1, 1);
		}

		SET_ENTITY_ROTATION(tempEntity, Rot.x, Rot.y, Rot.z, 2, 1);
		APPLY_FORCE_TO_ENTITY(tempEntity, 1, 0.0f, 350.0f, 0.0f, 0.0f, -0.65f, 0.0f, 0, 1, 1, 1, 0, 1);

		if (objectGunRandBitO || objectGunRandBitV)
		{
			objectGunHash.Unload();
		}

		if (DOES_ENTITY_EXIST(tempEntity))
		{
			SET_ENTITY_AS_NO_LONGER_NEEDED(&tempEntity);
		}
	}
}

void SetLightGun()
{
	GTAentity myPed = PLAYER_PED_ID();

	if (IS_PED_SHOOTING(myPed.Handle()))
	{
		const auto& colour = RgbS::Random();

		Game::Sound::GameSound soundEffect("EPSILONISM_04_SOUNDSET", "IDLE_BEEP");
		soundEffect.Play(myPed);

		const Vector3& camPos = GameplayCamera::GetPosition();
		const Vector3& camDir = GameplayCamera::GetDirectionFromScreenCentre();
		float launchDist;

		GTAentity myWeaponEntity = GET_CURRENT_PED_WEAPON_ENTITY_INDEX(myPed.Handle(), 0);
		if (myWeaponEntity.Exists())
		{
			launchDist = camPos.DistanceTo(myWeaponEntity.GetPosition()) + 0.26f;
		}
		else
		{
			launchDist = camPos.DistanceTo(myPed.GetPosition()) + 0.44f;
		}

		for (float i = launchDist; i < 260.0f; i += 0.02f)
		{
			World::DrawLightWithRange(camPos + (camDir * i), colour, 1.0f, 3.0f);
		}
	}
}

void SetTripleBullets()
{
	if (GET_WEAPONTYPE_GROUP(g_myWeap) == WeaponGroupHash::Melee)
	{
		return;
	}

	Player playerPed = PLAYER_PED_ID();
	GTAentity gunObj = GET_CURRENT_PED_WEAPON_ENTITY_INDEX(playerPed, 0);
	const Vector3& launchPos = gunObj.GetOffsetInWorldCoords(0, gunObj.Dim1().y, 0);
	Vector3 targPos[]
	{
		{ -2, 2, 2 },
		{ -1.5, 1, 1 },
		{ -1, 0, 0 },
		{ 1, 0, 0 },
		{ 1.5, 1, 1 },
		{ 2, 2, 2 }
	};

	CLEAR_AREA_OF_PROJECTILES(launchPos.x, launchPos.y, launchPos.z, 6.0f, 0);
	float maxDist = GET_MAX_RANGE_OF_CURRENT_PED_WEAPON(playerPed);

	for (auto& pos : targPos)
	{
		World::ShootBullet(launchPos, GameplayCamera::RaycastForCoord(Vector2(0.0f, 0.0f), playerPed, maxDist, maxDist) + pos, playerPed, g_myWeap, 5, 0xbf800000, true, true);
	}

}

void SetForgeGunDist(float& distance)
{
	DISABLE_CONTROL_ACTION(2, INPUT_LOOK_BEHIND, TRUE);
	DISABLE_CONTROL_ACTION(2, INPUT_WEAPON_WHEEL_NEXT, TRUE);
	DISABLE_CONTROL_ACTION(2, INPUT_WEAPON_WHEEL_PREV, TRUE);
	if (Menu::bitController)
	{
		if (IS_DISABLED_CONTROL_PRESSED(2, INPUT_FRONTEND_RS))
		{
			distance += 0.166f;
		}
		if (IS_DISABLED_CONTROL_PRESSED(2, INPUT_FRONTEND_LS))
		{
			if (distance > 3.0f)
			{
				distance -= 0.166f;
			}

			auto ped = PLAYER_PED_ID();
			if (GET_PED_STEALTH_MOVEMENT(ped))
			{
				SET_PED_STEALTH_MOVEMENT(ped, 0, 0);
			}
			if (GET_PED_COMBAT_MOVEMENT(ped))
			{
				SET_PED_COMBAT_MOVEMENT(ped, 0);
			}
		}
	}
	else
	{
		if (IS_DISABLED_CONTROL_PRESSED(2, INPUT_CURSOR_SCROLL_UP))
		{
			distance += 0.32f;
		}
		if (IS_DISABLED_CONTROL_PRESSED(2, INPUT_CURSOR_SCROLL_DOWN))
		{
			if (distance > 3.0f) distance -= 0.32f;
		}
	}

	ENABLE_CONTROL_ACTION(2, INPUT_WEAPON_WHEEL_NEXT, TRUE);
	ENABLE_CONTROL_ACTION(2, INPUT_WEAPON_WHEEL_PREV, TRUE);
}

static inline void SetForgeGunRotationHotKeys()
{
	Vector3 Rot = GET_ENTITY_ROTATION(targetSlotEntity, 2);
	FLOAT& precision = g_forgeGunPrecision;

	if (!Menu::bitController)
	{
		if (IsKeyDown(VK_OEM_4))
		{
			Rot.x -= precision;
		}
		if (IsKeyDown(VK_OEM_6))
		{
			Rot.x += precision;
		}
		if (IsKeyDown(VK_OEM_1))
		{
			Rot.y -= precision;
		}
		if (IsKeyDown(VK_OEM_7))
		{
			Rot.y += precision;
		}
		if (IsKeyDown(VK_OEM_COMMA))
		{
			Rot.z -= precision;
		}
		if (IsKeyDown(VK_OEM_PERIOD))
		{
			Rot.z += precision;
		}

	}
	else
	{
		DISABLE_CONTROL_ACTION(0, INPUT_WEAPON_WHEEL_NEXT, TRUE);
		DISABLE_CONTROL_ACTION(0, INPUT_WEAPON_WHEEL_PREV, TRUE);
		DISABLE_CONTROL_ACTION(0, INPUT_WEAPON_WHEEL_LR, TRUE);
		DISABLE_CONTROL_ACTION(2, INPUT_MAP, TRUE);
		DISABLE_CONTROL_ACTION(0, INPUT_VEH_SELECT_NEXT_WEAPON, TRUE);
		DISABLE_CONTROL_ACTION(0, INPUT_VEH_FLY_SELECT_NEXT_WEAPON, TRUE);
		DISABLE_CONTROL_ACTION(0, INPUT_SELECT_CHARACTER_FRANKLIN, TRUE);
		DISABLE_CONTROL_ACTION(0, INPUT_SELECT_CHARACTER_MICHAEL, TRUE);
		DISABLE_CONTROL_ACTION(0, INPUT_SELECT_CHARACTER_TREVOR, TRUE);
		DISABLE_CONTROL_ACTION(0, INPUT_SELECT_CHARACTER_MULTIPLAYER, TRUE);
		DISABLE_CONTROL_ACTION(0, INPUT_CHARACTER_WHEEL, TRUE);
		DISABLE_CONTROL_ACTION(2, INPUT_CELLPHONE_CANCEL, TRUE);
		DISABLE_CONTROL_ACTION(2, INPUT_CELLPHONE_SELECT, TRUE);
		DISABLE_CONTROL_ACTION(2, INPUT_CELLPHONE_UP, TRUE);
		DISABLE_CONTROL_ACTION(2, INPUT_CELLPHONE_DOWN, TRUE);


		if (IS_CONTROL_PRESSED(2, INPUT_FRONTEND_DOWN))
		{
			Rot.x -= precision;
		}
		if (IS_CONTROL_PRESSED(2, INPUT_FRONTEND_UP))
		{
			Rot.x += precision;
		}
		if (IS_CONTROL_PRESSED(2, INPUT_FRONTEND_LEFT))
		{
			Rot.y -= precision;
		}
		if (IS_CONTROL_PRESSED(2, INPUT_FRONTEND_RIGHT))
		{
			Rot.y += precision;
		}
		if (IS_CONTROL_PRESSED(2, INPUT_FRONTEND_LB))
		{
			Rot.z -= precision;
		}
		if (IS_CONTROL_PRESSED(2, INPUT_FRONTEND_RB))
		{
			Rot.z += precision;
		}
	}

	SET_ENTITY_ROTATION(targetSlotEntity, Rot.x, Rot.y, Rot.z, 2, 1);
}

void SetForgeGun()
{
	Ped tempPed = PLAYER_ID();
	if (g_myWeap == WEAPON_PISTOL && (IS_PLAYER_FREE_AIMING(tempPed) || IS_PLAYER_TARGETTING_ANYTHING(tempPed)))
	{
		if (!DOES_ENTITY_EXIST(targetSlotEntity) || bitGravityGunDisabled)
		{
			return;
		}

		if (forgeDist == 0) forgeDist = GameplayCamera::GetPosition().DistanceTo(GET_ENTITY_COORDS(targetSlotEntity, 1));
		SetForgeGunDist(forgeDist);
		SetForgeGunRotationHotKeys();

		Vector3 Coord, dim2;
		dim2 = GTAentity(targetSlotEntity).Dim2();
		float heading = GET_ENTITY_HEADING(targetSlotEntity);
		float playerHeading = GET_ENTITY_HEADING(PLAYER_PED_ID());
		if (heading > (playerHeading - 50) && heading < (playerHeading + 50))
		{
			Coord = GetCoordsFromCam(forgeDist + abs(dim2.y));
		}
		else
		{
			Coord = GetCoordsFromCam(forgeDist + abs(dim2.x));
		}

		GTAentity(targetSlotEntity).RequestControl();
		SET_ENTITY_COORDS_NO_OFFSET(targetSlotEntity, Coord.x, Coord.y, Coord.z, 0, 0, 0);
		FREEZE_ENTITY_POSITION(targetSlotEntity, objectSpawnForgeAssistance);

		if (IS_DISABLED_CONTROL_JUST_PRESSED(0, INPUT_ATTACK))
		{
			FREEZE_ENTITY_POSITION(targetSlotEntity, 0);
			PLAY_SOUND_FROM_ENTITY(-1, "Foot_Swish", targetSlotEntity, "docks_heist_finale_2a_sounds", 0, 0);
			PLAY_SOUND_FROM_ENTITY(-1, "SUSPENSION_SCRIPT_FORCE", targetSlotEntity, 0, 0, 0);
			PLAY_SOUND_FROM_ENTITY(-1, "Chopper_Destroyed", PLAYER_PED_ID(), "FBI_HEIST_FIGHT_CHOPPER_SOUNDS", 0, 0);
			dim2 = GET_GAMEPLAY_CAM_ROT(2);
			SET_ENTITY_ROTATION(targetSlotEntity, dim2.x, dim2.y, dim2.z, 2, 1);
			APPLY_FORCE_TO_ENTITY(targetSlotEntity, 1, 0.0f, g_forgeGunShootForce, 0.0f, 0.0f, 0.0f, 0.0f, 0, 1, 1, 1, 0, 1);
			PTFX::TriggerPTFX("scr_carsteal4", "scr_carsteal4_wheel_burnout", 0, Coord, Vector3(), 0.66f);

			targetSlotEntity = 0;
			forgeDist = 0;
			bitGravityGunDisabled = true;
			targetEntityLocked = false;
		}
	}
	else
	{
		forgeDist = 0;
		bitGravityGunDisabled = false;
	}

}

void SetExplosionAtBulletHit(Ped ped, Hash type, bool invisible)
{
	Vector3_t Pos;
	if (!GET_PED_LAST_WEAPON_IMPACT_COORD(ped, &Pos) && ped == PLAYER_PED_ID())
	{
		const Vector3& camDir = GameplayCamera::GetDirection();
		const Vector3& camCoord = GameplayCamera::GetPosition();
		Vector3 hitCoord = (camDir * 1000.0f) + camCoord;

		RaycastResult ray = RaycastResult::Raycast(camCoord, hitCoord, IntersectOptions::Everything);
		if (ray.DidHitAnything())
		{
			Pos = hitCoord.ToTypeStruct();
		}
		else
		{
			hitCoord = (camDir * 100.0f) + camCoord;
			Pos = hitCoord.ToTypeStruct();
		}
	}


	if (kaboomGun && ped == PLAYER_PED_ID() && kaboomGunRandBit)
	{
		ADD_EXPLOSION(Pos.x, Pos.y, Pos.z, GET_RANDOM_INT_IN_RANGE(0, 40), 5.0, 1, invisible, 0.5, false);
	}
	else if (type < 70)
	{
		ADD_EXPLOSION(Pos.x, Pos.y, Pos.z, type, 5.0, 1, invisible, 0.5, false);
	}
	else
	{
		Entity ent;
		if (IS_MODEL_A_VEHICLE(type))
		{
			ent = CREATE_VEHICLE(type, Pos.x, Pos.y, Pos.z + 0.16f, GET_ENTITY_HEADING(ped), 1, 1, 0);
		}
		else
		{
			ent = CREATE_PED(PedType::Human, type, Pos.x, Pos.y, Pos.z + 0.16f, GET_ENTITY_HEADING(ped), 1, 1);
		}

		SET_ENTITY_AS_NO_LONGER_NEEDED(&ent);
	}
}

void SetTriggerFXAtBulletHit(Ped ped, const std::string& fxAsset, const std::string& fxName, const Vector3& Rot, float scale)
{
	Vector3_t Pos;
	if (!GET_PED_LAST_WEAPON_IMPACT_COORD(ped, &Pos) && ped == PLAYER_PED_ID())
	{
		const Vector3& camDir = GameplayCamera::GetDirection();
		const Vector3& camCoord = GameplayCamera::GetPosition();
		Vector3 hitCoord = (camDir * 1000.0f) + camCoord;

		RaycastResult ray = RaycastResult::Raycast(camCoord, hitCoord, IntersectOptions::Everything);
		if (ray.DidHitAnything())
		{
			Pos = hitCoord.ToTypeStruct();
		}
		else
		{
			hitCoord = (camDir * 100.0f) + camCoord;
			Pos = hitCoord.ToTypeStruct();
		}
	}

	PTFX::TriggerPTFX(fxAsset, fxName, 0, Pos, Rot, scale);
}
