#pragma once

#include "../Menu/Submenu.h"

#include "../Scripting/DxHookIMG.h"
#include "../Util/GTAmath.h"

#include <string>
#include <vector>

class GTAped;
class GTAvehicle;

namespace sub
{
	void TaskRappel(GTAped ped, GTAvehicle vehicle);

	namespace VehicleAutoDrive
	{
		void ToggleOnOff();
		void Tick();

		bool& Enabled();
		float& Speed();
		unsigned char& DrivingStyleIndex();
		bool& PushEnemiesAway();
		float& PushRadius();
		bool& RandomDestinationMode();
	}

	namespace VehicleSlam
	{
		void InitSub(GTAvehicle veh, float* val);

		// Exposed for ports.
		struct NamedSlamValueS { std::string name; float value; };
		extern std::vector<NamedSlamValueS> vValues_VehicleSlam;
		extern float* slamValue;
	}

	namespace Speedo
	{
		struct SpeedoImage
		{
			std::string fileName;
			DxHookIMG::DxTexture id;
		};

		extern SpeedoImage currentSpeedoBG;
		extern SpeedoImage currentSpeedoNeedle;
		extern unsigned __int8 speedoAlpha;
		extern Vector2 speedoPosition;

		enum eSpeedoMode : unsigned __int8 { SPEEDOMODE_OFF, SPEEDOMODE_DIGITAL, SPEEDOMODE_ANALOGUE };
		extern unsigned __int8 loopSpeedo;
		extern bool speedoMPH;

		void SetCurrentBgIdFromBgNameForConfig();
		void LoadSpeedoImages();

		void SpeedoTick();

		struct NamedSpeedoImage
		{
			std::string displayName;
			std::string fileName;
			DxHookIMG::DxTexture id;
		};
		extern std::vector<NamedSpeedoImage> speedoImagesNames[];
	}
}

namespace Menu {

class VehicleSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle"; }
	const char* Title() const override { return "Vehicle Options"; }
	void Draw() override;

private:
	int fixCarTexterValue = 0;
};

class PvSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_pv"; }
	const char* Title() const override { return "PV Options"; }
	void Draw() override;
};

class VehicleWeaponsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_weapons"; }
	const char* Title() const override { return "Vehicle Weapons"; }
	void Draw() override;
};

class VehicleMultipliersSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_multipliers"; }
	const char* Title() const override { return "Multipliers"; }
	void Draw() override;
};

class VehicleMultiPlatNeonsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_neons_multi"; }
	const char* Title() const override { return "Neons"; }
	void Draw() override;
};

class VehicleSlamSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_slam"; }
	const char* Title() const override { return "Slam It"; }
	void Draw() override;
};

class AutoDriveSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_auto_drive"; }
	const char* Title() const override { return "Auto Drive"; }
	void Draw() override;
};

class SpeedoMainSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_speedos"; }
	const char* Title() const override { return "Speedometers"; }
	void Draw() override;
};

class SpeedoThemesLightSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_speedos_light"; }
	const char* Title() const override { return "Light Themes"; }
	void Draw() override;
};

class SpeedoThemesDarkSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "vehicle_speedos_dark"; }
	const char* Title() const override { return "Dark Themes"; }
	void Draw() override;
};

}