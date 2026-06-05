#pragma once

#include "../Menu/Submenu.h"

#include <string>
#include <vector>

typedef unsigned short UINT16;

namespace sub
{
	namespace PedConfigFlagManager
	{
		struct NamedPedFlagS
		{
			UINT16 id;
			std::string title;
		};
		extern std::vector<NamedPedFlagS> pedFlags;
		extern int flagID;
	}
}

namespace Menu {

class PlayerOpsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "player_ops"; }
	const char* Title() const override { return "Player Options"; }
	void Draw() override;
};

class CloneCompanionSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "player_clone_companion"; }
	const char* Title() const override { return "Cloning"; }
	void Draw() override;
};

class PedFlagsListSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "player_ped_flags"; }
	const char* Title() const override { return "Ped Flags"; }
	void Draw() override;
};

class PedFlagsCustomSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "player_ped_flags_custom"; }
	const char* Title() const override { return "Custom"; }
	void Draw() override;
};

}