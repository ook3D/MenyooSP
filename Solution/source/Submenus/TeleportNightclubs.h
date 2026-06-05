#pragma once

#include "../Menu/Submenu.h"

#include "Teleport/TeleLocation.h"

#include <vector>

namespace Menu {

class TeleportNightclubsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "teleport_nightclubs"; }
	const char* Title() const override { return "Nightclubs"; }
	void Draw() override;

private:
	static const std::vector<sub::TeleportLocations_catind::TeleLocation> vOtherNightclubRelatedTeleports;
};

}