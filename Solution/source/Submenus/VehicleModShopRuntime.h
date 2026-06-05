#pragma once

#include "..\macros.h"

#include "..\Menu\Menu.h"
#include "..\Menu\Routine.h"

#include "..\Natives\natives2.h"
#include "..\Util\GTAmath.h"
#include "..\Util\StringManip.h"
#include "..\Util\FileLogger.h"
#include "..\Scripting\enums.h"
#include "..\main.h"
#include "..\Scripting\GTAvehicle.h"
#include "..\Scripting\Model.h"
#include "..\Scripting\Game.h"
#include "..\Memory\GTAmemory.h"
#include "..\Scripting\World.h"

#include "SettingsRuntime.h"

#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <array>

typedef unsigned char UINT8;
typedef signed char INT8;
typedef signed short INT16;
typedef unsigned long DWORD, Hash;
typedef int INT, Vehicle, Ped, Object, Entity, ScrHandle;

extern INT msCurrentPaintIndex;
extern INT bitMSPaintsRGBMode;
extern bool g_unlockMaxIDs;
extern UINT8 max_shapeAndSkinIDs;

namespace sub
{
	struct NamedVehiclePaint
	{
		std::string name;
		INT16 paint;
		INT16 pearl;
	};

	const std::vector<NamedVehiclePaint>& GetPaintsNormal();
	const std::vector<NamedVehiclePaint>& GetPaintsMetallic();
	const std::vector<NamedVehiclePaint>& GetPaintsPearl();
	const std::vector<NamedVehiclePaint>& GetPaintsMatte();
	const std::vector<NamedVehiclePaint>& GetPaintsMetal();
	const std::vector<NamedVehiclePaint>& GetPaintsChrome();
	const std::vector<NamedVehiclePaint>& GetPaintsChameleon();
	const std::vector<NamedVehiclePaint>& GetPaintsUtil();
	const std::vector<NamedVehiclePaint>& GetPaintsWorn();
	const std::vector<NamedVehiclePaint>& GetPaintsWheels();
	const std::vector<NamedVehiclePaint>& GetPaintsInterior();
	const std::vector<NamedVehiclePaint>& GetPaintsDashboard();

	INT GetPaintIndexMaxValue();

	extern INT lastpaint;
	extern INT lastpearl;
	extern INT lastr;
	extern INT lastg;
	extern INT lastb;
	extern bool getpaint;
	extern bool iscustompaint;

	extern bool selectmod;
	extern INT lastMod;
	extern bool lowersuspension;

	extern INT8 lastwheeltype;
	extern INT8 lastfwheel;
	extern INT8 lastbwheel;

	const std::vector<std::string>& GetWheelTypeNames();
	extern bool msWheelsBitBikeBack;
	extern int  msWheelsMaxWindices;

	enum MSWindowsMode : UINT8
	{
		MSWINDOWS_MODE_OPEN = 0,
		MSWINDOWS_MODE_CLOSE,
		MSWINDOWS_MODE_BREAK,
		MSWINDOWS_MODE_FIX,
		MSWINDOWS_MODE_REMOVE,
	};
	extern UINT8 msWindowsMode;
	const std::vector<std::string>& GetMsWindowsModeNames();
	const std::vector<std::string>& GetMsWindowsWindowNames();
	const std::vector<std::string>& GetMsWindowsWinTintNames();
	void MsWindowsDoWindow(class GTAvehicle& vehicle, enum VehicleWindow window, UINT8 mode);

	extern UINT8 msDoorsActionIndex;

	// Lights submenu state.
	extern bool msLightsLeftInd;
	extern bool msLightsRightInd;
	extern bool msLightsHazard;

	void PopulateAllPaintIDs();

	// vehicle - upgrades
	void SetVehicleMaxUpgrades(Vehicle vehicle, bool upgradeIt = true, bool invincible = false, INT8 plateType = 5, std::string plateText = std::string(),
		bool neonIt = false, UINT8 NeonR = 0, UINT8 NeonG = 0, UINT8 NeonB = 0, INT16 prim_col_index = -3, INT16 sec_col_index = -3);


	// ModShop

	void ModShop_();
	void MSCatall_();

	// Emblem

	void MSEmblem_();

	// Wheels

	void MSWheels_();
	void MSWheels2_();
	void MSWheels3_();
	void MSTyresBurst_();

	// Windows

	namespace MSWindows_catind
	{
		void MSWindows_();
	}

	// Doors

	void MSDoors_();

	// Paints

	INT getpaintCarUsing_index(Vehicle veh, INT partIndex_CustomK);
	void paintCarUsing_index(Vehicle veh, INT partIndex_CustomK, INT16 colour_index, INT16 pearl_index);

	void MSPaints_();
	void MSPaints2_();

	namespace MSPaints_catind
	{
		void Sub_Shared();
		void Sub_Chameleon();
		void Sub_Pearl();
		void Sub_Util();
		void Sub_Worn();
	}

	void rgb_mode_set_carcol(Vehicle veh, INT16 R, INT16 G, INT16 B, INT16 A);
	void MSPaints_RGB();

	// Extras

	void MSExtra_();

	// Neons

	void MSNeons_();

	// Engine sound

	void MSEngineSound_();

	void MSLights_();

}



