#pragma once

#include "../Menu/Submenu.h"

namespace Menu {

class PlayersSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "players"; }
	const char* Title() const override { return "Players"; }
	void Draw() override;
};

class PlayersSubSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "players_sub"; }
	const char* Title() const override { return "Player"; }
	void Draw() override;
};

}