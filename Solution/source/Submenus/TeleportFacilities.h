#pragma once

#include "../Menu/Submenu.h"

#include "../Util/GTAmath.h"

#include "Teleport/TeleLocation.h"

#include <array>
#include <string>
#include <vector>

namespace Menu {

class TeleportFacilitiesSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "teleport_facilities"; }
	const char* Title() const override { return "Facilities"; }
	void Draw() override;

	struct FacilityLocation
	{
		std::string name;
		Vector3 pos;
		std::vector<std::string> ipls;
	};
	struct FacilityInteriorOption
	{
		std::string name;
		std::string value;
		unsigned __int8 maxTints;
	};
	struct FacilityInteriorOptionIndex
	{
		unsigned __int8 index = 0;
		unsigned __int8 currTint = 1;
	};
	struct FacilityInfoStructure
	{
		const FacilityLocation* location = nullptr;
		FacilityInteriorOptionIndex mainShellOption;
		FacilityInteriorOptionIndex graphicsOption;
		FacilityInteriorOptionIndex trophyOption;
		FacilityInteriorOptionIndex orbitalCannonOption;
		FacilityInteriorOptionIndex securityRoomOption;
		FacilityInteriorOptionIndex loungeOption;
		FacilityInteriorOptionIndex sleepingQuartersOption;
		FacilityInteriorOptionIndex clutterOption;
		FacilityInteriorOptionIndex crewEmblemOption;
	};
	struct FacilityInteriorOptionArray
	{
		std::string name;
		FacilityInteriorOptionIndex FacilityInfoStructure::* memberPtr;
		const std::vector<FacilityInteriorOption>* arr;
	};

	static FacilityInfoStructure currentFacilityInfo;
	static const FacilityInteriorOptionArray* selectedOptionArray;

	static const std::vector<sub::TeleportLocations_catind::TeleLocation> vOtherFacilityRelatedTeleports;
	static const std::vector<FacilityLocation> vLocations;
	static const std::array<std::string, 10> vTintNames;

	static const std::vector<FacilityInteriorOption> vMainShellOptions;
	static const std::vector<FacilityInteriorOption> vGraphicsOptions;
	static const std::vector<FacilityInteriorOption> vTrophyOptions;
	static const std::vector<FacilityInteriorOption> vOrbitalCannonOptions;
	static const std::vector<FacilityInteriorOption> vSecurityRoomOptions;
	static const std::vector<FacilityInteriorOption> vLoungeOptions;
	static const std::vector<FacilityInteriorOption> vSleepingQuartersOptions;
	static const std::vector<FacilityInteriorOption> vClutterOptions;
	static const std::vector<FacilityInteriorOption> vCrewEmblemOptions;

	static const std::vector<FacilityInteriorOptionArray> vOptionArrays;

	static void CreateFacility(FacilityInfoStructure& info);
	static void UpdateFacilityProp(FacilityInfoStructure& info, const FacilityInteriorOptionArray& arr);
};

class TeleportFacilitiesInLocSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "teleport_facilities_in_loc"; }
	const char* Title() const override { return "Facility"; }
	void Draw() override;
};

class TeleportFacilitiesInOptionSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "teleport_facilities_in_option"; }
	const char* Title() const override { return "Option"; }
	void Draw() override;
};

}