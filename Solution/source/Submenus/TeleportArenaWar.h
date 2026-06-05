#pragma once

#include "../Menu/Submenu.h"

#include "Teleport/TeleLocation.h"

#include <vector>

namespace Menu {

class TeleportArenaWarSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "teleport_arena_war"; }
	const char* Title() const override { return "Arena War"; }
	void Draw() override;

private:
	static const std::vector<sub::TeleportLocations_catind::TeleLocation> vOtherArenaWarRelatedTeleports;
};

}