#pragma once

#include "../Menu/Submenu.h"

namespace Menu {

class PedComponentsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_components"; }
	const char* Title() const override { return "Wardrobe"; }
	void Draw() override;
};

class PedComponentsSetSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_components_set"; }
	const char* Title() const override { return "Set Variation"; }
	void Draw() override;
};

class PedComponentsPropsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_components_props"; }
	const char* Title() const override { return "Accessories"; }
	void Draw() override;
};

class PedComponentsPropsSetSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_components_props_set"; }
	const char* Title() const override { return "Set Variation"; }
	void Draw() override;
};

class PedOutfitsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_components_outfits"; }
	const char* Title() const override { return "Outfits"; }
	void Draw() override;
};

class PedOutfitsItemSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_components_outfits_item"; }
	const char* Title() const override { return "Outfit"; }
	void Draw() override;
};

class PedDecalsTypesSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_decals_types"; }
	const char* Title() const override { return "Decal Overlays"; }
	void Draw() override;
};

class PedDecalsZonesSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_decals_zones"; }
	const char* Title() const override { return "Zones"; }
	void Draw() override;
};

class PedDecalsInZoneSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_decals_in_zone"; }
	const char* Title() const override { return "In Zone"; }
	void Draw() override;
	void OnExit() override;
};

class PedDamageCategoriesSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_damage_categories"; }
	const char* Title() const override { return "Damage Overlays"; }
	void Draw() override;
};

class PedDamageBoneSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_damage_bone"; }
	const char* Title() const override { return "Select Bone"; }
	void Draw() override;
};

class PedDamageBloodSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_damage_blood"; }
	const char* Title() const override { return "Blood Decals"; }
	void Draw() override;
};

class PedDamageDecalsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_damage_decals"; }
	const char* Title() const override { return "Damage Decals"; }
	void Draw() override;
};

class PedDamagePacksSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_damage_packs"; }
	const char* Title() const override { return "Damage Packs"; }
	void Draw() override;
};

class PedHeadFeaturesSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_head_features"; }
	const char* Title() const override { return "Head Features"; }
	void Draw() override;
};

class PedHeadOverlaysSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_head_overlays"; }
	const char* Title() const override { return "Overlays"; }
	void Draw() override;
};

class PedHeadOverlaysItemSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_head_overlays_item"; }
	const char* Title() const override { return "Overlay"; }
	void Draw() override;
};

class PedFaceFeaturesSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_face_features"; }
	const char* Title() const override { return "Facial Features"; }
	void Draw() override;
};

class PedSkinToneSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_skin_tone"; }
	const char* Title() const override { return "Shape & Skin Tone"; }
	void Draw() override;
};

}