#include "MiscMapMods.h"

#include "../Menu/SubmenuRegistry.h"

namespace Menu {

void MapModsSubmenu::Draw()
{
	sub::MapMods::LoadMapModsFromXmlIfEmpty();

	DrawTitle();

	const int count = sub::MapMods::GetMapModCount();
	for (int i = 0; i < count; ++i)
	{
		if (DrawOption(sub::MapMods::GetMapModName(i)))
		{
			sub::MapMods::SetCurrentMapModIndex(i);
			NavigateTo("misc_map_mods_2");
		}
	}
}

void MapMods2Submenu::Draw()
{
	DrawTitle();

	if (DrawOption("Teleport"))
		sub::MapMods::CurrentMapModTeleport();

	const bool wasLoaded = sub::MapMods::IsCurrentMapModLoaded();
	if (DrawToggleExternal("Load", wasLoaded))
	{
		if (wasLoaded)
			sub::MapMods::CurrentMapModUnload();
		else
			sub::MapMods::CurrentMapModLoad();
	}
}

}
REGISTER_SUBMENU(::Menu::MapModsSubmenu)
REGISTER_SUBMENU(::Menu::MapMods2Submenu)

namespace sub
{
	namespace MapMods
	{
		void UnloadAllMapMods();

		void MapModsPlaceObject(std::vector<GTAentity>& handles, Hash hash, float X, float Y, float Z, float pitch, float roll, float yaw, bool noLongerNeeded = false)
		{
			Model model(hash);
			GTAprop e = World::CreateProp(hash, Vector3(X, Y, Z), Vector3(pitch, roll, yaw), false, false);
			e.FreezePosition(true);
			SET_NETWORK_ID_CAN_MIGRATE(OBJ_TO_NET(e.Handle()), TRUE);
			e.SetLODDistance(1000000);
			e.SetMissionEntity(true);
			model.Unload();
			if (noLongerNeeded)
			{
				SET_OBJECT_AS_NO_LONGER_NEEDED(&e.Handle());
			}

			handles.push_back(e.GetHandle());
		}

		void MapModsPlaceRawObject(std::vector<GTAentity>& handles, Hash hash, float X, float Y, float Z, float pitch, float roll, float yaw, bool hasQuaternion, float qx, float qy, float qz, float qw)
		{
			Object tempEntity = CREATE_OBJECT(hash, X, Y, Z, 1, 1, 0);
			SET_ENTITY_COORDS(tempEntity, X, Y, Z, 1, 0, 0, 1);
			if (pitch != 0.0f || roll != 0.0f || yaw != 0.0f)
			{
				SET_ENTITY_ROTATION(tempEntity, pitch, roll, yaw, 2, 1);
			}
			if (hasQuaternion)
			{
				SET_ENTITY_QUATERNION(tempEntity, qx, qy, qz, qw);
			}
			FREEZE_ENTITY_POSITION(tempEntity, 1);
			handles.push_back(tempEntity);
		}

		class GTAMapMod
		{
		private:
			std::string name;
			std::string author;
			Vector3 location;
			pugi::xml_node xmlNode;
			std::vector<GTAentity> ohandles;
		public:
			GTAMapMod(const std::string& newName, const std::string& newAuthor, Vector3 newLocation, pugi::xml_node node)
				: name(newName), author(newAuthor), location(newLocation), xmlNode(node)
			{
			}
			void Teleport()
			{
				TeleportNetPed(Game::PlayerPed(), location.x, location.y, location.z);
			}
			bool IsLoaded()
			{
				return !ohandles.empty();
			}
			void Load(bool showMsg = true)
			{
				UnloadAllMapMods();
				LoadFromXml();
				if (showMsg)
				{
					Game::Print::PrintBottomCentre(name + " ~b~loaded~s~.");
				}
			}
			void Unload(bool showMsg = true)
			{
				for (auto& obj : ohandles)
				{
					obj.Delete(1);
				}
				ohandles.clear();

				if (showMsg)
					Game::Print::PrintBottomCentre(name + " ~p~unloaded~s~.");
			}
			const std::string& Name()
			{
				return name;
			}

		private:
			void LoadFromXml()
			{
				pugi::xml_node loopNode = xmlNode.child("Loop");
				if (loopNode)
				{
					int count = loopNode.attribute("count").as_int();
					float zStart = loopNode.attribute("zStart").as_float();
					float zIncrement = loopNode.attribute("zIncrement").as_float();

					pugi::xml_node objNode = loopNode.child("Object");
					if (objNode)
					{
						Hash hash = (Hash)objNode.attribute("hash").as_llong();
						float x = objNode.attribute("x").as_float();
						float y = objNode.attribute("y").as_float();
						float pitch = objNode.attribute("pitch").as_float();
						float roll = objNode.attribute("roll").as_float();
						float yaw = objNode.attribute("yaw").as_float();
						bool noLongerNeeded = objNode.attribute("noLongerNeeded").as_bool(false);

						float tempZ = zStart;
						for (int i = 0; i < count; i++)
						{
							tempZ += zIncrement;
							MapModsPlaceObject(ohandles, hash, x, y, tempZ, pitch, roll, yaw, noLongerNeeded);
						}
					}
				}

				// Handle regular objects
				pugi::xml_node objectsNode = xmlNode.child("Objects");
				if (objectsNode)
				{
					for (pugi::xml_node objNode = objectsNode.child("Object"); objNode; objNode = objNode.next_sibling("Object"))
					{
						Hash hash = (Hash)objNode.attribute("hash").as_llong();
						float x = objNode.attribute("x").as_float();
						float y = objNode.attribute("y").as_float();
						float z = objNode.attribute("z").as_float();
						float pitch = objNode.attribute("pitch").as_float();
						float roll = objNode.attribute("roll").as_float();
						float yaw = objNode.attribute("yaw").as_float();
						bool noLongerNeeded = objNode.attribute("noLongerNeeded").as_bool(false);
						bool raw = objNode.attribute("raw").as_bool(false);

						if (raw)
						{
							bool hasQuat = objNode.attribute("qx");
							float qx = objNode.attribute("qx").as_float();
							float qy = objNode.attribute("qy").as_float();
							float qz = objNode.attribute("qz").as_float();
							float qw = objNode.attribute("qw").as_float();
							MapModsPlaceRawObject(ohandles, hash, x, y, z, pitch, roll, yaw, hasQuat, qx, qy, qz, qw);
						}
						else
						{
							MapModsPlaceObject(ohandles, hash, x, y, z, pitch, roll, yaw, noLongerNeeded);
						}
					}
				}

				if (!author.empty())
				{
					Game::Print::PrintBottomLeft("Hax by ~b~" + author);
				}
			}
		};

		pugi::xml_document mapModsDoc;
		std::vector<GTAMapMod> allMapMods;
		GTAMapMod *currentMAPMODC;

		void UnloadAllMapMods()
		{
			for (GTAMapMod& mapMod : allMapMods)
			{
				mapMod.Unload(false);
			}
		}

		void LoadMapModsFromXml()
		{
			allMapMods.clear();

			std::string xmlPath = GetPathffA(Pathff::Main, true) + "MapMods.xml";
			if (mapModsDoc.load_file(xmlPath.c_str()).status != pugi::status_ok)
			{
				return;
			}

			pugi::xml_node root = mapModsDoc.document_element();
			for (pugi::xml_node modNode = root.child("MapMod"); modNode; modNode = modNode.next_sibling("MapMod"))
			{
				std::string name = modNode.attribute("name").as_string();
				std::string author = modNode.attribute("author").as_string();

				pugi::xml_node locNode = modNode.child("Location");
				Vector3 location;
				location.x = locNode.attribute("x").as_float();
				location.y = locNode.attribute("y").as_float();
				location.z = locNode.attribute("z").as_float();

				allMapMods.emplace_back(name, author, location, modNode);
			}
		}

		void LoadMapModsFromXmlIfEmpty()
		{
			if (allMapMods.empty())
				LoadMapModsFromXml();
		}
		int GetMapModCount()
		{
			return static_cast<int>(allMapMods.size());
		}
		const std::string& GetMapModName(int index)
		{
			return allMapMods[index].Name();
		}
		void SetCurrentMapModIndex(int index)
		{
			currentMAPMODC = &allMapMods[index];
		}
		static const std::string kEmpty;
		const std::string& GetCurrentMapModName()
		{
			return currentMAPMODC ? currentMAPMODC->Name() : kEmpty;
		}
		bool IsCurrentMapModLoaded()
		{
			return currentMAPMODC && currentMAPMODC->IsLoaded();
		}
		void CurrentMapModTeleport()
		{
			if (currentMAPMODC) currentMAPMODC->Teleport();
		}
		void CurrentMapModLoad()
		{
			if (currentMAPMODC) currentMAPMODC->Load();
		}
		void CurrentMapModUnload()
		{
			if (currentMAPMODC) currentMAPMODC->Unload();
		}

	}
}
