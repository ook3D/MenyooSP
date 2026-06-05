#pragma once

#include "../Menu/Submenu.h"

#include <string>

namespace Menu {

class SpoonerSaveFilesSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_save_files"; }
	const char* Title() const override { return "Manage Saved Files"; }
	void Draw() override;
	void OnExit() override;

private:
	std::string searchStr;
	float fSaveRangeRadius = 5.0f;
};

class SpoonerSaveFilesLoadSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_save_files_load"; }
	const char* Title() const override { return "Saved File"; }
	void Draw() override;
};

class SpoonerSaveFilesLoadLegacySubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_save_files_load_legacy"; }
	const char* Title() const override { return "Legacy SP00N File"; }
	void Draw() override;
};

}