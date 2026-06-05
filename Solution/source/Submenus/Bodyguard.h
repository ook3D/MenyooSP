#pragma once

#include "../Menu/Submenu.h"

namespace Menu {

class BodyguardSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "bodyguard"; }
	const char* Title() const override { return "Bodyguards"; }
	void Draw() override;
};

class BodyguardMenuSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "bodyguard_main"; }
	const char* Title() const override { return "Bodyguards"; }
	void Draw() override;
	void OnEnter() override { formationIndex = 0; blipIndex = 0; }

private:
	int formationIndex = 0;
	int blipIndex = 0;
};

class BodyguardSpawnSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "bodyguard_spawn"; }
	const char* Title() const override { return "Spawn Ped"; }
	void Draw() override;
};

class BodyguardListSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "bodyguard_list"; }
	const char* Title() const override { return "Bodyguard List"; }
	void Draw() override;
};

class BodyguardSettingsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "bodyguard_settings"; }
	const char* Title() const override { return "Bodyguard Settings"; }
	void Draw() override;
};

class BodyguardEntityOpsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "bodyguard_entity_ops"; }
	const char* Title() const override { return "Bodyguard"; }
	void Draw() override;
};

class BodyguardWeaponOpsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "bodyguard_weapon_ops"; }
	const char* Title() const override { return "Weapons"; }
	void Draw() override;
};

}