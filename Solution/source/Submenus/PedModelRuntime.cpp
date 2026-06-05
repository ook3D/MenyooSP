#include "PedModelRuntime.h"

#include "..\macros.h"

#include "..\Menu\Menu.h"
#include "..\Menu\Routine.h"

#include "..\Natives\natives2.h"
#include "..\Scripting\Model.h"
#include "..\Util\ExePath.h"
#include "..\Util\FileLogger.h"
#include "..\Util\StringManip.h"
#include "..\Util\keyboard.h"
#include "..\Scripting\Game.h"
#include "..\Scripting\ModelNames.h"
#include "..\Scripting\GTAped.h"
#include "..\Scripting\GTAvehicle.h"
#include "..\Scripting\Model.h"
#include "..\Scripting\PTFX.h"
#include "..\Scripting\WeaponIndivs.h"

#include "PedComponentRuntime.h"
#include "WeaponRuntime.h"
#include "Spooner\SpoonerEntity.h"
#include "Spooner\Databases.h"
#include "Spooner\EntityManagement.h"

#include <string>
#include <vector>
#include <pugixml\src\pugixml.hpp>

namespace sub
{
	namespace PedFavourites
	{
		std::string xmlFavouritePeds = "FavouritePeds.xml";
		std::string searchStr = std::string();

		void ClearSearchStr()
		{
			searchStr.clear();
		}

		bool IsPedAFavourite(GTAmodel::Model model)
		{
			pugi::xml_document doc;
			if (doc.load_file((const char*)(GetPathffA(Pathff::Main, true) + xmlFavouritePeds).c_str()).status != pugi::status_ok)
			{
				return false;
			}
			pugi::xml_node nodeRoot = doc.document_element();
			return nodeRoot.find_child_by_attribute("hash", IntToHexString(model.hash, true).c_str()) != NULL;
		}
		bool AddPedToFavourites(GTAmodel::Model model, const std::string& customName)
		{
			if (customName.empty())
			{
				return false;
			}
			pugi::xml_document doc;
			if (doc.load_file((const char*)(GetPathffA(Pathff::Main, true) + xmlFavouritePeds).c_str()).status != pugi::status_ok)
			{
				doc.reset();
				auto nodeDecleration = doc.append_child(pugi::node_declaration);
				nodeDecleration.append_attribute("version") = "1.0";
				nodeDecleration.append_attribute("encoding") = "ISO-8859-1";
				auto nodeRoot = doc.append_child("FavouriteWeapons");
				doc.save_file((const char*)(GetPathffA(Pathff::Main, true) + xmlFavouritePeds).c_str());
			}
			pugi::xml_node nodeRoot = doc.document_element();
			auto nodeOldLoc = nodeRoot.find_child_by_attribute("hash", IntToHexString(model.hash, true).c_str());
			if (nodeOldLoc) // If not null
			{
				nodeOldLoc.parent().remove_child(nodeOldLoc);
			}
			auto nodeNewLoc = nodeRoot.append_child("Ped");
			nodeNewLoc.append_attribute("hash") = IntToHexString(model.hash, true).c_str();
			nodeNewLoc.append_attribute("customName") = customName.c_str();
			return (doc.save_file((const char*)(GetPathffA(Pathff::Main, true) + xmlFavouritePeds).c_str()));
		}

		bool RemovePedFromFavourites(GTAmodel::Model model)
		{
			pugi::xml_document doc;
			if (doc.load_file((const char*)(GetPathffA(Pathff::Main, true) + xmlFavouritePeds).c_str()).status != pugi::status_ok)
			{
				return false;
			}
			pugi::xml_node nodeRoot = doc.document_element();
			auto nodeOldLoc = nodeRoot.find_child_by_attribute("hash", IntToHexString(model.hash, true).c_str());
			if (nodeOldLoc) // If not null
			{
				nodeOldLoc.parent().remove_child(nodeOldLoc);
			}
			return (doc.save_file((const char*)(GetPathffA(Pathff::Main, true) + xmlFavouritePeds).c_str()));
		}

		void ShowInstructionalButton(GTAmodel::Model model)
		{
			bool bIsAFav = IsPedAFavourite(model);
			if (Menu::bitController)
			{
				Menu::AddIB(INPUT_SCRIPT_RLEFT, (!bIsAFav ? "Add to" : "Remove from") + (std::string)" favourites");
				if (IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_SCRIPT_RLEFT))
				{
					!bIsAFav ? AddPedToFavourites(model, Game::InputBox("", 28U, "Enter custom name:", GetPedModelLabel(model, true))) : RemovePedFromFavourites(model);
				}
			}
			else
			{
				Menu::AddIB(VirtualKey::B, (!bIsAFav ? "Add to" : "Remove from") + (std::string)" favourites");
				if (IsKeyJustUp(VirtualKey::B))
				{
					!bIsAFav ? AddPedToFavourites(model, Game::InputBox("", 28U, "Enter custom name:", GetPedModelLabel(model, true))) : RemovePedFromFavourites(model);
				}
			}
		}
	}

	void ChangeModel(GTAmodel::Model model)
	{
		if (model.IsInCdImage())
		{
			if (model.Load(4000))
			{
				GTAped playerPed = PLAYER_PED_ID();
				int oldPlayerPed = playerPed.Handle();

				if (sub::PedDamageTextures::vPedsAndDamagePacks.count(playerPed.Handle()))
				{
					sub::PedDamageTextures::vPedsAndDamagePacks.erase(playerPed.Handle());
				}
				if (sub::PedDecals::vPedsAndDecals.count(playerPed.Handle()))
				{
					sub::PedDecals::vPedsAndDecals.erase(playerPed.Handle());
				}

				std::vector<s_Weapon_Components_Tint> weaponsBackup;
				playerPed.StoreWeaponsInArray(weaponsBackup);
				Hash currWeaponHash = playerPed.GetWeapon();

				GTAentity att;
				auto spi = sub::Spooner::EntityManagement::GetEntityIndexInDb(playerPed);
				if (spi >= 0)
				{
					auto& spe = sub::Spooner::Databases::EntityDb[spi];
					sub::Spooner::EntityManagement::GetEntityThisEntityIsAttachedTo(spe.handle, att);
				}

				bool wasInVehicle = playerPed.IsInVehicle();
				GTAvehicle vehicle;
				VehicleSeat currentVehSeat;
				if (wasInVehicle)
				{
					vehicle = playerPed.CurrentVehicle();
					currentVehSeat = playerPed.GetCurrentVehicleSeat();
				}

				bool hasCollision = playerPed.GetIsCollisionEnabled();
				SET_PLAYER_MODEL(PLAYER_ID(), model.hash);

				playerPed = PLAYER_PED_ID();
				playerPed.SetIsCollisionEnabled(hasCollision);

				SET_PED_DEFAULT_COMPONENT_VARIATION(playerPed.Handle());
				model.Unload();


				if (wasInVehicle)
				{
					playerPed.SetIntoVehicle(vehicle, currentVehSeat);
				}

				if (playerPed.PedType() == PedType::Animal && !HAS_ANIM_DICT_LOADED("creatures@rottweiler@melee@streamed_core@"))
				{
					REQUEST_ANIM_DICT("creatures@rottweiler@melee@streamed_core@");
					REQUEST_ANIM_DICT("creatures@cougar@melee@streamed_core@");
				}

				playerPed.GiveWeaponsFromArray(weaponsBackup);
				if (IS_WEAPON_VALID(currWeaponHash))
				{
					playerPed.SetWeapon(currWeaponHash);
				}

				SET_PED_INFINITE_AMMO_CLIP(playerPed.Handle(), bitInfiniteAmmo);

				if (spi >= 0)
				{
					auto& spe = sub::Spooner::Databases::EntityDb[spi];
					GTAentity oldPlayerPed = spe.handle;
					spe.handle = playerPed;
					spe.hashName = GetPedModelLabel(model, true);
					if (spe.hashName.length() == 0)
					{
						IntToHexString(model.hash, true);
					}
					spe.lastAnimation.dict.clear();
					spe.lastAnimation.name.clear();
					if (att.Exists() && spe.attachmentArgs.isAttached)
					{
						spe.handle.AttachTo(att, spe.attachmentArgs.boneIndex, spe.handle.GetIsCollisionEnabled(), spe.attachmentArgs.offset, spe.attachmentArgs.rotation);
					}
					spe.taskSequence.Reset();
					if (sub::Spooner::selectedEntity.handle.Equals(oldPlayerPed))
					{
						sub::Spooner::selectedEntity = spe;
					}
				}
			}
		}
	}

	std::pair<std::string, std::string> rngped;

	GTAmodel::Model ModelChangerRandom(std::vector<std::pair<std::string, std::string>> pedModels)
	{
		addlog(ige::LogType::LOG_TRACE, "Getting Random Ped Model");
		rngped = pedModels[std::rand() % pedModels.size()];
		addlog(ige::LogType::LOG_TRACE, "Got Random Ped Model: " + rngped.first + ", " + rngped.second);
		return rngped.first;
	}
}