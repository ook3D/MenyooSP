#pragma once

#include "../Menu/Submenu.h"

namespace Menu {

class SettingsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "settings"; }
	const char* Title() const override { return "Settings"; }
	void Draw() override;
};

class SettingsMenuPosSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "settings_menupos"; }
	const char* Title() const override { return "Menu Position"; }
	void Draw() override;
};

class SettingsThemesSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "settings_themes"; }
	const char* Title() const override { return "Themes"; }
	void Draw() override;
};

class SettingsColoursSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "settings_colours"; }
	const char* Title() const override { return "Menu Colours"; }
	void Draw() override;
};

class SettingsColours2Submenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "settings_colours2"; }
	const char* Title() const override { return "Set Colour"; }
	void Draw() override;
};

class SettingsFontsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "settings_fonts"; }
	const char* Title() const override { return "Menu Fonts"; }
	void Draw() override;
};

class SettingsFonts2Submenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "settings_fonts2"; }
	const char* Title() const override { return "Set Font"; }
	void Draw() override;
};

class SettingsLanguageSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "settings_language"; }
	const char* Title() const override { return "Language"; }
	void Draw() override;
};

}