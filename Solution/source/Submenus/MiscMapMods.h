#pragma once

#include "../Menu/Submenu.h"

#include "../macros.h"
#include "../Menu/Menu.h"
#include "../Natives/natives2.h"
#include "../Util/GTAmath.h"
#include "../Util/ExePath.h"
#include "../Scripting/GTAentity.h"
#include "../Scripting/GTAprop.h"
#include "../Scripting/GTAped.h"
#include "../Scripting/Game.h"
#include "../Scripting/Model.h"
#include "../Scripting/World.h"
#include "Teleport/TeleMethods.h"

#include <pugixml/src/pugixml.hpp>
#include <string>
#include <vector>

typedef int Object;
typedef unsigned long DWORD, Hash;

namespace sub
{
	namespace MapMods
	{
		void LoadMapModsFromXmlIfEmpty();
		int  GetMapModCount();
		const std::string& GetMapModName(int index);
		void SetCurrentMapModIndex(int index);
		const std::string& GetCurrentMapModName();
		bool IsCurrentMapModLoaded();
		void CurrentMapModTeleport();
		void CurrentMapModLoad();
		void CurrentMapModUnload();
	}
}

namespace Menu {

class MapModsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "misc_map_mods"; }
	const char* Title() const override { return "Map Mods"; }
	void Draw() override;
};

class MapMods2Submenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "misc_map_mods_2"; }
	const char* Title() const override { return "Map Mod"; }
	void Draw() override;
};

}