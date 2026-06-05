#pragma once

#include "../Menu/Submenu.h"

#include "..\macros.h"

#include "..\Menu\Menu.h"
#include "..\Menu\Routine.h"

#include "..\Natives\natives2.h"
#include "..\Scripting\enums.h"
#include "..\Util\ExePath.h"
#include "..\Util\GTAmath.h"
#include "..\Scripting\Model.h"
#include "..\Misc\GenericLoopedMode.h"
#include "..\Scripting\GTAentity.h"
#include "..\Scripting\GTAped.h"
#include "..\Scripting\WeaponIndivs.h"
#include "..\Scripting\World.h"
#include "..\Scripting\Game.h"
#include "..\Scripting\CustomHelpText.h"
#include "..\Scripting\ModelNames.h"

#include <string>
#include <vector>
#include <pugixml\src\pugixml.hpp>

namespace sub
{
	namespace AnimalRiding
	{
		struct AnimalAndSeat
		{
			Model model;
			int attachBone;
			Vector3 position;
			Vector3 rotation;
		};
		extern std::vector<AnimalAndSeat> vAnimals;

		void PopulateAnimals();
		void ToggleOnOff();
		void Tick();
		bool& Enabled();
		void SpawnAnimalRide(const Model& model);
	}
}

namespace Menu {

class AnimalRidingSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "animal_riding"; }
	const char* Title() const override { return "Animal Riding"; }
	void Draw() override;
};

}