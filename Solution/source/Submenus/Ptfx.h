#pragma once

#include "../Menu/Submenu.h"

namespace Menu {

enum class PtfxSource
{
	LoopOnEntity = 0,
	OneShotOnPed,
	TriggerFxGun,
};

void SetPtfxSource(PtfxSource source);
PtfxSource GetPtfxSource();

class PtfxSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ptfx"; }
	const char* Title() const override { return "FX"; }
	void Draw() override;
};

class PtfxFavouritesSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ptfx_favourites"; }
	const char* Title() const override { return "Favorite FX's"; }
	void Draw() override;
};

}