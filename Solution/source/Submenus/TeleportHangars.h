#pragma once

#include "../Menu/Submenu.h"

#include "../Util/GTAmath.h"   // Vector3

#include <string>
#include <vector>

namespace Menu {

class TeleportHangarsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "teleport_hangars"; }
	const char* Title() const override { return "Hangars"; }
	void Draw() override;

	struct HangarLocation
	{
		std::string name;
		Vector3 pos;
		std::vector<std::string> ipls;
		std::string interior;
	};
	struct HangarInteriorOption
	{
		std::string name;
		std::string value;
		unsigned __int8 maxTints;
	};
	struct HangarInteriorOptionIndex
	{
		unsigned __int8 index = 0;
		unsigned __int8 currTint = 1;
	};
	struct HangarInfoStructure
	{
		const HangarLocation* location = nullptr;
		HangarInteriorOptionIndex mainShellOption;
		HangarInteriorOptionIndex bedroomOption;
		HangarInteriorOptionIndex bedroomStyleOption;
		HangarInteriorOptionIndex bedroomClutterOption;
		HangarInteriorOptionIndex bedroomBlindsOption;
		HangarInteriorOptionIndex modAreaOption;
		HangarInteriorOptionIndex craneOption;
		HangarInteriorOptionIndex officeOption;
		HangarInteriorOptionIndex floorOption;
		HangarInteriorOptionIndex floorDecalOption;
		HangarInteriorOptionIndex hangarLightingOption;
		HangarInteriorOptionIndex wallLightingOption;
	};
	struct HangarInteriorOptionArray
	{
		std::string name;
		HangarInteriorOptionIndex HangarInfoStructure::* memberPtr;
		const std::vector<HangarInteriorOption>* arr;
	};

	// Shared state (parent + sub-submenus read and write).
	static HangarInfoStructure currentHangarInfo;
	static const HangarInteriorOptionArray* selectedOptionArray;

	// Per-table data.
	static const std::vector<HangarLocation>          vLocations;
	static const std::vector<HangarInteriorOption>    vDefaultOptions;
	static const std::vector<HangarInteriorOption>    vMainShellOptions;
	static const std::vector<HangarInteriorOption>    vBedroomOptions;
	static const std::vector<HangarInteriorOption>    vBedroomStyleOptions;
	static const std::vector<HangarInteriorOption>    vBedroomClutterOptions;
	static const std::vector<HangarInteriorOption>    vBedroomBlindsOptions;
	static const std::vector<HangarInteriorOption>    vModAreaOptions;
	static const std::vector<HangarInteriorOption>    vCraneOptions;
	static const std::vector<HangarInteriorOption>    vOfficeOptions;
	static const std::vector<HangarInteriorOption>    vFloorOptions;
	static const std::vector<HangarInteriorOption>    vFloorDecalOptions;
	static const std::vector<HangarInteriorOption>    vHangarLightingOptions;
	static const std::vector<HangarInteriorOption>    vWallLightingOptions;
	static const std::vector<HangarInteriorOptionArray> vOptionArrays;

	// Shared helpers used by the InLoc / InOption sub-submenus.
	static void CreateHangar(HangarInfoStructure& info);
	static void UpdateHangarProp(HangarInfoStructure& info, const HangarInteriorOptionArray& arr);
};

class TeleportHangarsInLocSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "teleport_hangars_in_loc"; }
	const char* Title() const override { return "Hangar"; }
	void Draw() override;
};

class TeleportHangarsInOptionSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "teleport_hangars_in_option"; }
	const char* Title() const override { return "Option"; }
	void Draw() override;
};

}