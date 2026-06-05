#pragma once

#include "..\macros.h"

#include "..\Menu\Menu.h"
#include "..\Menu\Routine.h"

#include "..\Natives\natives2.h"
#include "..\Util\GTAmath.h"
#include "..\Scripting\Model.h"
#include "..\Scripting\PTFX.h"
#include "..\Scripting\GTAentity.h"
#include "..\Scripting\GTAvehicle.h"
#include "..\Scripting\GTAped.h"
#include "..\Scripting\Game.h"
#include "..\Scripting\DxHookIMG.h"
#include "..\Util\ExePath.h"
#include "..\Scripting\ModelNames.h"
#include "..\Menu\FolderPreviewBmps.h"
#include "..\Util\StringManip.h"
#include "..\Util\keyboard.h"
#include "..\Util\FileLogger.h"

#include "VehicleModShopRuntime.h"
#include "WeaponRuntime.h"
#include "Spooner\Databases.h"
#include "Spooner\EntityManagement.h"
#include "Spooner\FileManagement.h"
#include "Spooner\SpoonerEntity.h"

#include <Shlwapi.h> //PathIsDirectory
#pragma comment(lib, "Shlwapi.lib")
#include <string>
#include <vector>
#include <pugixml\src\pugixml.hpp>
#include <dirent\include\dirent.h>

typedef unsigned __int8 UINT8;
typedef signed char INT8;
typedef signed short INT16;
typedef int INT, Vehicle, Entity, Ped;
typedef unsigned long DWORD, Hash;
typedef float FLOAT;
typedef char *PCHAR;

class RgbS;
class GTAentity;
class GTAvehicle;
class GTAped;
namespace GTAmodel
{
	class Model;
}

extern std::string g_spawnVehiclePlateText;
extern INT8 g_spawnVehiclePlateType;
extern INT8 g_spawnVehiclePlateTexterValue;
extern RgbS g_spawnVehicleNeonColor;
extern bool g_spawnVehicleAutoSit;
extern bool g_warpNear;
extern bool g_addBlip;
extern bool g_spawnVehicleAutoUpgrade;
extern bool g_spawnVehicleInvincible;
extern bool g_spawnVehiclePersistent;
extern bool g_spawnVehicleDeleteOld;
extern bool g_spawnVehicleNeonToggle;
extern bool g_LSCCustoms;
extern bool g_vehiclePVOpsName;

extern INT16 g_spawnVehiclePrimaryColor;
extern INT16 g_spawnVehicleSecondaryColor;
extern bool g_spawnVehicleDrawBMPs;
extern FLOAT g_clearAreaRadius;

namespace sub
{
	int SpawnVehicle(GTAmodel::Model model, GTAped ped, bool deleteOld = false, bool warpIntoVehicle = true);

	namespace VehicleSpawner
	{
		extern UINT8 spawnVehicleIndex;

		enum Indices
		{
			COMPACT, SEDAN, SUV, COUPE, MUSCLE, SPORTSCLASSIC, SPORT, SUPER,
			MOTORCYCLE, OFFROAD, INDUSTRIAL, UTILITY, VAN, BICYCLE, BOAT, HELICOPTER,
			PLANE, SERVICE, EMERGENCY, MILITARY, COMMERCIAL, TRAIN, OPENWHEEL, OTHER, DRIFT
		};

		void PopulateVehicleBmps();
		void DrawVehicleBmp(const GTAmodel::Model& vehModel);
		void DrawVehicleModelName(const GTAmodel::Model& vehModel);

	}
	
	struct VehicleDlcCategory
	{
		std::string name;
		std::vector<std::string> captions;
		std::vector<std::string> values;
	};
	extern const std::vector<VehicleDlcCategory> VehicleDlcCategories;
	extern int vehDLCCategoryID;
	extern int vehDlcIdToSpawn;

	bool SpawnVehicleIsVehicleModelAFavourite(GTAmodel::Model vehModel);
	bool SpawnVehicleAddVehicleModelToFavourites(GTAmodel::Model vehModel, const std::string& customName);
	bool SpawnVehicleRemoveVehicleModelFromFavourites(GTAmodel::Model vehModel);

	void SpawnVehicleDLC();
	void SpawnVehicleDLCSelection();
	void SpawnVehicleAllCategoriesMenu();
	void SpawnVehicleFavouritesMenu();

	namespace VehicleSaver
	{
		extern UINT8 _persistentAttachmentsTexterIndex;
		extern UINT8 _driverVisibilityTexterIndex;

		void VehicleSaveToFile(std::string filePath, GTAvehicle ev);
		void VehicleReadFromFile(std::string filePath, GTAentity ped);

		void VehicleSaverMenu();
		void VehSaverInItemMenu();
		int saveCarVars(GTAvehicle vehicle);
		void saveColourVals();

	}

}
