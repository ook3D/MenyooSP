#pragma once

#include "../Menu/Submenu.h"

namespace Menu {

class MainMenuSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "main_menu"; }
	const char* Title() const override { return "MENYOO"; }
	void Draw() override;
};

}