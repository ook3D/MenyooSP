#pragma once

#include "../Menu/Submenu.h"

#include <map>
#include <string>
#include <vector>

namespace sub
{
	void DrawToolsMenu();

	namespace WaterHack
	{
		void Tick();

		// Accessors so the WaterHackSubmenu can drive the same shared state
		// that the per-frame ticker reads (Menu::Ticks::WaterHack()).
		bool&  Enabled();
		float& Radius();
		float& Height();
	}

	namespace TVChannelStuff
	{
		extern bool loopBasicTV;
		extern std::string currentTvChannelLabel;
		extern std::map<std::string, std::string> tvPlaylists;

		void DrawTvWhereItsSupposedToBe();
	}

	namespace HudOptions
	{
		extern bool revealMinimap;
	}

	namespace GameCamOptions
	{
		extern float shakeAmplitude;
		extern signed char shakeID;
	}
}

namespace Menu {

class MiscOpsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "misc"; }
	const char* Title() const override { return "Misc Options"; }
	void Draw() override;

private:
	enum eYscScriptTexterIndex : unsigned char { YSC_LOAD = 0, YSC_UNLOAD = 1 };
	unsigned char yscScriptTexterIndex = YSC_LOAD;
};

class TimecyclesSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "misc_timecycles"; }
	const char* Title() const override { return "Vision Hax"; }
	void Draw() override;
};

class ClearAreaSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "misc_clear_area"; }
	const char* Title() const override { return "Clear Area"; }
	void Draw() override;
};

class RadioSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "misc_radio"; }
	const char* Title() const override { return "Radio"; }
	void Draw() override;
};

class WaterHackSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "misc_water_hack"; }
	const char* Title() const override { return "Water Hack"; }
	void Draw() override;
};

class TvSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "misc_tv"; }
	const char* Title() const override { return "TV"; }
	void Draw() override;

private:
	static const std::map<std::string, std::string> kPlaylists;
};

class HudOptionsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "misc_hud_options"; }
	const char* Title() const override { return "HUD Options"; }
	void Draw() override;

private:
	bool revealMinimap = false;
};

class GameCamSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "misc_game_cam"; }
	const char* Title() const override { return "Game Camera"; }
	void Draw() override;

private:
	float shakeAmplitude = 1.0f;
	signed char shakeID = -1;
};

}

class Vector3;

extern bool bDisplayXyzhCoords;
extern bool pedPopulation;
extern bool massacreMode;
extern bool blackoutMode;
extern bool simpleBlackoutMode;
extern bool restrictedAreasAccess;
extern bool fireworksDisplay;

extern std::string dict;
extern std::string dict2;
extern std::string dict3;

void SetMassacreModeTick();
void SetBlackoutEMPMode();
void SetBlackoutMode();
void StartFireworksAtCoords(const Vector3& pos, const Vector3& rot, float scale);
int GetRandomSpriteId();