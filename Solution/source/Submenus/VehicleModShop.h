#pragma once

#include "../Menu/Submenu.h"

namespace Menu {

class ModShopSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_modshop"; }
	const char* Title() const override { return "Menyoo Customs"; }
	void Draw() override;
};

class ModShopBennysSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_modshop_bennys"; }
	const char* Title() const override { return "Benny's Lowrider Mods"; }
	void Draw() override;
};

class ModShopEmblemSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_modshop_emblem"; }
	const char* Title() const override { return "Emblem"; }
	void Draw() override;
};

class ModShopWheelsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_modshop_wheels"; }
	const char* Title() const override { return "Wheels"; }
	void Draw() override;
};

class ModShopWheels2Submenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_modshop_wheels_2"; }
	const char* Title() const override { return "Wheel Position"; }
	void Draw() override;
};

class ModShopWheels3Submenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_modshop_wheels_3"; }
	const char* Title() const override { return "Wheels"; }
	void Draw() override;
};

class ModShopTyresBurstSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_modshop_tyres_burst"; }
	const char* Title() const override { return "Remove Tyres"; }
	void Draw() override;
};

class ModShopPaintsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_modshop_paints"; }
	const char* Title() const override { return "Paints"; }
	void Draw() override;
};

class ModShopPaints2Submenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_modshop_paints_2"; }
	const char* Title() const override { return "Paint Category"; }
	void Draw() override;
};

class ModShopPaintsSharedSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_modshop_paints_shared"; }
	const char* Title() const override { return "Paints"; }
	void Draw() override;
};

class ModShopPaintsChromeSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_modshop_paints_chrome"; }
	const char* Title() const override { return "Chrome"; }
	void Draw() override;
};

class ModShopPaintsMatteSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_modshop_paints_matte"; }
	const char* Title() const override { return "Matte"; }
	void Draw() override;
};

class ModShopPaintsNormalSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_modshop_paints_normal"; }
	const char* Title() const override { return "Classic"; }
	void Draw() override;
};

class ModShopPaintsMetallicSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_modshop_paints_metallic"; }
	const char* Title() const override { return "Metallic"; }
	void Draw() override;
};

class ModShopPaintsMetalSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_modshop_paints_metal"; }
	const char* Title() const override { return "Metal"; }
	void Draw() override;
};

class ModShopPaintsChameleonSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_modshop_paints_chameleon"; }
	const char* Title() const override { return "Chameleon"; }
	void Draw() override;
};

class ModShopPaintsPearlSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_modshop_paints_pearl"; }
	const char* Title() const override { return "Pearlescent"; }
	void Draw() override;
};

class ModShopPaintsUtilSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_modshop_paints_util"; }
	const char* Title() const override { return "Utility"; }
	void Draw() override;
};

class ModShopPaintsWornSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_modshop_paints_worn"; }
	const char* Title() const override { return "Worn"; }
	void Draw() override;
};

class ModShopPaintsRgbSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_modshop_paints_rgb"; }
	const char* Title() const override { return "Set Colour"; }
	void Draw() override;
};

class ModShopCatAllSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_modshop_cat_all"; }
	const char* Title() const override { return "Mods"; }
	void Draw() override;
};

class ModShopWindowsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_modshop_windows"; }
	const char* Title() const override { return "Windows"; }
	void Draw() override;
};

class ModShopEngineSoundSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_modshop_engine_sound"; }
	const char* Title() const override { return "Engine Sound"; }
	void Draw() override;
};

class ModShopLightsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_modshop_lights"; }
	const char* Title() const override { return "Lights"; }
	void Draw() override;
};

class ModShopDoorsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_modshop_doors"; }
	const char* Title() const override { return "Doors"; }
	void Draw() override;
};

class ModShopNeonsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_modshop_neons"; }
	const char* Title() const override { return "Neons Lights"; }
	void Draw() override;
};

class ModShopExtraSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_modshop_extra"; }
	const char* Title() const override { return "Extras"; }
	void Draw() override;
};

}