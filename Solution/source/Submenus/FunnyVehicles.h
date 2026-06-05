#pragma once

#include "../Menu/Submenu.h"

namespace Menu {

class FunnyVehiclesSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "funny_vehicles"; }
	const char* Title() const override { return "Badly Constructed Vehicles"; }
	void Draw() override;
};

}