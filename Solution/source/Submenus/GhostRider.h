#pragma once

#include "../Menu/Submenu.h"

#include <string>

#include "..\macros.h"
#include "..\Menu\Menu.h"

#include "..\Natives\natives2.h"
#include "..\Util\GTAmath.h"
#include "..\Util\ExePath.h"
#include "..\Misc\GenericLoopedMode.h"
#include "..\Scripting\GTAped.h"
#include "..\Scripting\GTAvehicle.h"
#include "..\Scripting\Model.h"
#include "..\Scripting\PTFX.h"
#include "..\Scripting\Game.h"

#include "PedComponentRuntime.h"
#include "VehicleSpawnerRuntime.h"

typedef char *PCHAR;

namespace sub
{
	namespace GhostRiderMode
	{
		void ToggleOnOff();
		void Tick();
		bool& Enabled();

		extern std::string outfitFileName;
		void ApplyGhostRiderOutfit();
		void SpawnGhostRiderRide();
	}
}

namespace Menu {

class GhostRiderSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ghost_rider_mode"; }
	const char* Title() const override { return "Ghost Rider Mode"; }
	void Draw() override;
};

}