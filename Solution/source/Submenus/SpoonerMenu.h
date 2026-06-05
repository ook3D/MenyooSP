#pragma once

#include "../Menu/Submenu.h"

namespace Menu {

class SpoonerSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_main"; }
	const char* Title() const override { return "Object Spooner"; }
	void Draw() override;
};

class SpoonerSettingsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_settings"; }
	const char* Title() const override { return "Settings"; }
	void Draw() override;
};

class SpoonerSpawnCategoriesSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_spawn_categories"; }
	const char* Title() const override { return "Spawn Into World"; }
	void Draw() override;
};

class SpoonerSpawnPropSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_spawn_prop"; }
	const char* Title() const override { return "Spawn Object"; }
	void Draw() override;
};

class SpoonerSpawnPropFavouritesSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_spawn_prop_favourites"; }
	const char* Title() const override { return "Favourites"; }
	void Draw() override;
};

class SpoonerSpawnPedSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_spawn_ped"; }
	const char* Title() const override { return "Spawn Ped"; }
	void Draw() override;
};

class SpoonerSpawnVehicleSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_spawn_vehicle"; }
	const char* Title() const override { return "Spawn Vehicle"; }
	void Draw() override;
};

class SpoonerManageMarkersSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_manage_markers"; }
	const char* Title() const override { return "Markers"; }
	void Draw() override;
};

class SpoonerManageMarkersRemovalSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_manage_markers_removal"; }
	const char* Title() const override { return "Removal"; }
	void Draw() override;
};

class SpoonerManageMarkersInMarkerSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_manage_markers_in_marker"; }
	const char* Title() const override { return "Marker"; }
	void Draw() override;
};

class SpoonerManageMarkersInMarkerDest2MarkerSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_manage_markers_in_marker_dest2marker"; }
	const char* Title() const override { return "Select Marker"; }
	void Draw() override;
};

class SpoonerManageMarkersInMarkerAttachSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_manage_markers_in_marker_attach"; }
	const char* Title() const override { return "Attach To Something"; }
	void Draw() override;
};

class SpoonerManageDbSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_manage_db"; }
	const char* Title() const override { return "Manage Entities"; }
	void Draw() override;
};

class SpoonerManageDbRemovalSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_manage_db_removal"; }
	const char* Title() const override { return "Removal"; }
	void Draw() override;
};

class SpoonerSelectedEntityOpsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_selected_entity_ops"; }
	const char* Title() const override { return "Entity Options"; }
	void Draw() override;
};

class SpoonerPedOpsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_ped_ops"; }
	const char* Title() const override { return "Ped Options"; }
	void Draw() override;
};

class SpoonerPedOpsWeaponSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_ped_ops_weapon"; }
	const char* Title() const override { return "Weapon"; }
	void Draw() override;
};

class SpoonerPedOpsWeaponInCategorySubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_ped_ops_weapon_in_category"; }
	const char* Title() const override { return "Weapons"; }
	void Draw() override;
};

class SpoonerAttachmentOpsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_attachment_ops"; }
	const char* Title() const override { return "Attachment"; }
	void Draw() override;
};

class SpoonerAttachmentOpsAttachToSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_attachment_ops_attach_to"; }
	const char* Title() const override { return "Attach To Something"; }
	void Draw() override;
};

class SpoonerAttachmentOpsSelectBoneSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_attachment_ops_select_bone"; }
	const char* Title() const override { return "Bone"; }
	void Draw() override;
};

class SpoonerManualPlacementSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_manual_placement"; }
	const char* Title() const override { return "Manual Placement"; }
	void Draw() override;
};

class SpoonerSizeManipulationSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_size_manipulation"; }
	const char* Title() const override { return "Size Manipulation"; }
	void Draw() override;
};

class SpoonerQuickManualPlacementSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_quick_manual_placement"; }
	const char* Title() const override { return "Manual Placement"; }
	void Draw() override;
};

class SpoonerVector3ManualPlacementSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_vector3_manual_placement"; }
	const char* Title() const override { return "Manual Placement"; }
	void Draw() override;
};

class SpoonerGroupSpoonSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_group_spoon"; }
	const char* Title() const override { return "Multiple Entities"; }
	void Draw() override;
};

class SpoonerGroupSpoonSelectEntitiesSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_group_spoon_select_entities"; }
	const char* Title() const override { return "Select Entities"; }
	void Draw() override;
};

class SpoonerGroupSpoonAttachToSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_group_spoon_attach_to"; }
	const char* Title() const override { return "Attach To Something"; }
	void Draw() override;
};

}