#pragma once

#include "../Menu/Submenu.h"

#include <string>
#include <vector>

namespace Menu {

class CreditsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "credits"; }
	const char* Title() const override { return "Credits"; }

	void OnEnter() override;
	void Draw() override;

private:
	struct Tier
	{
		std::string name;
		std::vector<std::string> members;
	};

	std::vector<Tier> tiers;
	bool loaded = false;

	void LoadFromXml();
};

}