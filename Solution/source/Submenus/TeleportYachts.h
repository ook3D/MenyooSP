#pragma once

#include "../Menu/Submenu.h"

#include "../Natives/types.h"   // RGBA

#include <string>
#include <vector>

class GTAentity;

typedef int INT, Ped, Vehicle, Object, Entity;
typedef unsigned __int8 UINT8;
typedef float FLOAT;
typedef char *PCHAR;

namespace Menu {

class TeleportYachtsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "teleport_yachts"; }
	const char* Title() const override { return "Yachts"; }
	void Draw() override;
	::Menu::TitleSpriteOverride GetTitleSpriteOverride() const override
	{
		return { "dock_dlc_banner", "yacht_banner_0" };
	}
};

class TeleportYachtsInGrpSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "teleport_yachts_in_grp"; }
	const char* Title() const override { return "Yacht Group"; }
	void Draw() override;
	::Menu::TitleSpriteOverride GetTitleSpriteOverride() const override
	{
		return { "dock_dlc_banner", "yacht_banner_0" };
	}
};

}
namespace sub::TeleportLocations_catind
{
	class TeleLocation;

	namespace Yachts
	{

		void Tick();

		const std::vector<std::string>&  GroupLocationNames();
		const std::vector<std::string>&  OptionDisplayTitles();
		const std::vector<std::string>&  FlagSuffixes();

		std::size_t                      OtherYachtRelatedTeleportCount();
		const std::string&               OtherYachtRelatedTeleportName(std::size_t i);
		void                             InvokeOtherYachtRelatedTeleport(std::size_t i);

		void  SetOldYachtInfoFromCurrent();
		void  ClearOldYachtInfo();
		bool  HasCurrentYachtLocation();

		bool  SetCurrentLocationByIndex(std::size_t i);
		bool  SetCurrentOptionByIndex(std::size_t i);

		void  RestoreFromOldYachtInfo();

		std::size_t  CurrentLocationIndex();
		std::size_t  CurrentOptionIndex();

		const std::string& CurrentLocationName();
		UINT8 CurrentOptionId();

		UINT8 GetInternalLocationIndex();
		void  SetInternalLocationIndex(UINT8 idx);
		UINT8 GetYachtPropTextureVariation();
		void  SetYachtPropTextureVariation(UINT8 v);
		char  GetRailingColour();
		void  SetRailingColour(char c);
		UINT8 GetLightingType();
		void  SetLightingType(UINT8 v);
		char  GetLightingColour();
		void  SetLightingColour(char c);
		char  GetDoorColour();
		void  SetDoorColour(char c);
		UINT8 GetFlagIndex();
		void  SetFlagIndex(UINT8 v);
		RGBA& MarkerColour();              // Settings colour-picker target.

		// Build-yacht side effects.
		void  ResetCurrentYachtProp();
		void  UnloadCurrentYachtScaleforms();
		void  CreateYachtCurrent();
		void  DeleteOldYacht();
		void  TeleportPedToCurrentYacht(GTAentity ped);

		void  DrawYachtBmpPreview(UINT8 yachtId, float menuPosX, float optionY, float menuPosY);

	}
}
