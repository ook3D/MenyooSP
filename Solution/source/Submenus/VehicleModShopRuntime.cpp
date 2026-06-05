#include "VehicleModShopRuntime.h"
#include "VehicleRuntime.h"
#include "VehicleSpawnerRuntime.h"
#include "PlayerRuntime.h"

INT msCurrentPaintIndex = 0;
INT bitMSPaintsRGBMode;
bool g_unlockMaxIDs = false;
UINT8 max_shapeAndSkinIDs = 46;

namespace sub
{
	bool lowersuspension = false;
	int lastMod = -2;
	bool selectmod = false;
	int lastpaint = 0;
	int lastpearl = 0;
	int lastr = 0;
	int lastg = 0;
	int lastb = 0;
	bool getpaint = true;
	bool iscustompaint = false;

#pragma region paintvectors
	std::vector<NamedVehiclePaint> PAINTS_NORMAL
	{

	};

	std::vector<NamedVehiclePaint> PAINTS_METALLIC
	{

	};

	std::vector<NamedVehiclePaint> PAINTS_MATTE
	{

	};

	std::vector<NamedVehiclePaint> PAINTS_METAL
	{

	};

	std::vector<NamedVehiclePaint> PAINTS_PEARL
	{

	};

	const std::vector<NamedVehiclePaint> PAINTS_WHEELS
	{
		{ "Alloy", 156, -1 },
		{ "Black", 0, -1 },
		{ "Carbon Black (Graphite)", 1, -1 },
		{ "Anthracite Black", 11, -1 },
		{ "Black Steel", 2, -1 },
		{ "Stone Silver", 8, -1 },
		{ "Frost White (Util Off White)", 122, -1 },
		{ "Red", 27, -1 },
		{ "Blaze Red", 30, -1 },
		{ "Garnet Red (Util Garnet Red)", 45, -1 },
		{ "Candy Red", 35, -1 },
		{ "Sunset Red", 33, -1 },
		{ "Salmon Pink", 136, -1 },
		{ "Hot Pink", 135, -1 },
		{ "Sunrise Orange", 36, -1 },
		{ "Orange (Matte Orange)", 41, -1 },
		{ "Bright Orange", 138, -1 },
		{ "Gold", 37, -1 },
		{ "Straw Brown", 99, -1 },
		{ "Dark Copper (Bronze)", 90, -1 },
		{ "Dark Ivory (Creek Brown)", 95, -1 },
		{ "Dark Brown (Worn Dark Brown)", 115, -1 },
		{ "Bronze (Util Medium Brown)", 109, -1 },
		{ "Dark Earth (Matte Dark Earth)", 153, -1 },
		{ "Desert Tan (Matte Desert Tan)", 154, -1 },
		{ "Yellow", 88, -1 },
		{ "Race Yellow", 89, -1 },
		{ "Yellow Bird (Dew Yellow)", 91, -1 },
		{ "Lime Green (Matte Light Green)", 55, -1 },
		{ "Pea Green", 125, -1 },
		{ "Green (Bright Green)", 53, -1 },
		{ "Dark Green (Util Dark Green)", 56, -1 },
		{ "Olive Green (Matte Forest Green)", 151, -1 },
		{ "Midnight Blue (Matte Dark Blue)", 82, -1 },
		{ "Royal Blue (Blue)", 64, -1 },
		{ "Baby Blue (Worn Baby Blue)", 87, -1 },
		{ "Bright Blue (Ultra Blue)", 70, -1 },
		{ "Fluorescent Blue", 140, -1 },
		{ "Slate Blue (Util Bright Blue)", 81, -1 },
		{ "Schafter Purple (Bright Purple)", 145, -1 },
		{ "Midnight Purple", 142, -1 },
	};

	const std::vector<NamedVehiclePaint> PAINTS_INTERIOR
	{
		{ "Black", 0, -1 },
		{ "Graphite", 1, -1 },
		{ "Anthracite Black", 11, -1 },
		{ "Black Steel", 2, -1 },
		{ "Dark Steel", 3, -1 },
		{ "Bluish Silver", 5, -1 },
		{ "Rolled Steel", 6, -1 },
		{ "Shadow Silver", 7, -1 },
		{ "Stone Silver", 8, -1 },
		{ "Midnight Silver", 9, -1 },
		{ "Cast Iron Silver", 10, -1 },
		{ "Red", 27, -1 },
		{ "Torino Red", 28, -1 },
		{ "Lava Red", 150, -1 },
		{ "Blaze Red", 30, -1 },
		{ "Grace Red", 31, -1 },
		{ "Garnet Red", 32, -1 },
		{ "Sunset Red", 33, -1 },
		{ "Cabernet Red", 34, -1 },
		{ "Wine Red", 143, -1 },
		{ "Candy Red", 35, -1 },
		{ "Pfister Pink", 137, -1 },
		{ "Salmon Pink", 136, -1 },
		{ "Sunrise Orange", 36, -1 },
		{ "Orange", 38, -1 },
		{ "Bright Orange", 138, -1 },
		{ "Bronze", 90, -1 },
		{ "Yellow", 88, -1 },
		{ "Race Yellow", 89, -1 },
		{ "Dew Yellow", 91, -1 },
		{ "Dark Green", 49, -1 },
		{ "Racing Green", 50, -1 },
		{ "Sea Green", 51, -1 },
		{ "Olive Green", 52, -1 },
		{ "Bright Green", 53, -1 },
		{ "Gasoline Green", 54, -1 },
		{ "Lime Green", 92, -1 },
		{ "Midnight Blue", 141, -1 },
		{ "Galaxy Blue", 61, -1 },
		{ "Dark Blue", 62, -1 },
		{ "Saxon Blue", 63, -1 },
		{ "Mariner Blue", 65, -1 },
		{ "Harbor Blue", 66, -1 },
		{ "Diamond Blue", 67, -1 },
		{ "Surf Blue", 68, -1 },
		{ "Nautical Blue", 69, -1 },
		{ "Racing Blue", 73, -1 },
		{ "Ultra Blue", 70, -1 },
		{ "Light Blue", 74, -1 },
		{ "Chocolate Brown", 96, -1 },
		{ "Bison Brown", 101, -1 },
		{ "Creek Brown", 95, -1 },
		{ "Feltzer Brown", 94, -1 },
		{ "Maple Brown", 97, -1 },
		{ "Beechwood Brown", 103, -1 },
		{ "Sienna Brown", 104, -1 },
		{ "Saddle Brown", 98, -1 },
		{ "Moss Brown", 100, -1 },
		{ "Woodbeech Brown", 102, -1 },
		{ "Straw Brown", 99, -1 },
		{ "Sandy Brown", 105, -1 },
		{ "Bleached Brown", 106, -1 },
		{ "Spinnaker Purple", 72, -1 },
		{ "Midnight Purple", 146, -1 },
		{ "Bright Purple", 145, -1 },
		{ "Cream", 107, -1 },
		{ "Ice White", 111, -1 },
		{ "Frost White", 112, -1 },
	};

	const std::vector<NamedVehiclePaint> PAINTS_DASHBOARD
	{
		{ "Silver", 4, -1 },
		{ "Bluish Silver", 5, -1 },
		{ "Rolled Steel", 6, -1 },
		{ "Shadow Silver", 7, -1 },
		{ "Ice White", 111, -1 },
		{ "Frost White", 112, -1 },
		{ "Cream", 107, -1 },
		{ "Sienna Brown", 104, -1 },
		{ "Saddle Brown", 98, -1 },
		{ "Moss Brown", 100, -1 },
		{ "Woodbeech Brown", 102, -1 },
		{ "Straw Brown", 99, -1 },
		{ "Sandy Brown", 105, -1 },
		{ "Bleached Brown", 106, -1 },
		{ "Gold", 37, -1 },
		{ "Bronze", 90, -1 },
		{ "Yellow", 88, -1 },
		{ "Race Yellow", 89, -1 },
		{ "Dew Yellow", 91, -1 },
		{ "Orange", 38, -1 },
		{ "Bright Orange", 138, -1 },
		{ "Sunrise Orange", 36, -1 },
		{ "Red", 27, -1 },
		{ "Torino Red", 28, -1 },
		{ "Formula Red", 29, -1 },
		{ "Lava Red", 150, -1 },
		{ "Blaze Red", 30, -1 },
		{ "Grace Red", 31, -1 },
		{ "Garnet Red", 32, -1 },
		{ "Candy Red", 35, -1 },
		{ "Hot Pink", 135, -1 },
		{ "Pfister Pink", 137, -1 },
		{ "Salmon Pink", 136, -1 },
		{ "Schafter Purple", 71, -1 },
		{ "Bright Purple", 145, -1 },
		{ "Saxon Blue", 63, -1 },
		{ "Blue", 64, -1 },
		{ "Mariner Blue", 65, -1 },
		{ "Harbor Blue", 66, -1 },
		{ "Diamond Blue", 67, -1 },
		{ "Surf Blue", 68, -1 },
		{ "Nautical Blue", 69, -1 },
		{ "Racing Blue", 73, -1 },
		{ "Ultra Blue", 70, -1 },
		{ "Light Blue", 74, -1 },
		{ "Sea Green", 51, -1 },
		{ "Bright Green", 53, -1 },
		{ "Gasoline Green", 54, -1 },
		{ "Lime Green", 92, -1 },
	};

	const std::vector<NamedVehiclePaint> PAINTS_UTIL
	{
		{ "Black", COLOR_UTIL_BLACK, -1 },
		{ "Black Poly", COLOR_UTIL_BLACK_POLY, COLOR_UTIL_BLACK_POLY },
		{ "Dark Silver", COLOR_UTIL_DARK_SILVER, -1 },
		{ "Silver", 18, COLOR_CLASSIC_ICE_WHITE },
		{ "Alloy", COLOR_METALS_DEFAULT_ALLOY, COLOR_METALS_DEFAULT_ALLOY },
		{ "Gun Metal", COLOR_UTIL_GUN_METAL, COLOR_UTIL_GUN_METAL },
		{ "Shadow Silver", COLOR_UTIL_SHADOW_SILVER, COLOR_UTIL_SHADOW_SILVER },
		{ "Red", COLOR_UTIL_RED, COLOR_UTIL_RED },
		{ "Bright Red", COLOR_UTIL_BRIGHT_RED, COLOR_UTIL_BRIGHT_RED },
		{ "Garnet Red", COLOR_UTIL_GARNET_RED, COLOR_CLASSIC_BLACK },
		{ "Dark Green", 56, 56 },
		{ "Green", 57, 57 },
		{ "Dark Blue", 75, COLOR_WORN_BABY_BLUE },
		{ "Midnight Blue", 76, 77 },
		{ "Blue", 77, 0 },
		{ "Sea Foam Blue", 78, 80 },
		{ "Lightning Blue", 79, -1 },
		{ "Maui Blue Poly", 80, 68 },
		{ "Bright Blue", 81, -1 },
		{ "Brown", COLOR_UTIL_BROWN, -1 },
		{ "Medium Brown", COLOR_UTIL_MEDIUM_BROWN, COLOR_UTIL_MEDIUM_BROWN },
		{ "Light Brown", COLOR_UTIL_LIGHT_BROWN, COLOR_UTIL_LIGHT_BROWN },
		{ "Off White", COLOR_UTIL_OFF_WHITE, COLOR_UTIL_OFF_WHITE },
		{ "Pure White", COLOR_CLASSIC_PURE_WHITE, COLOR_CLASSIC_BLACK },
		{ "Police Blue", COLOR_CLASSIC_POLICE_BLUE, -1 },
		{ "Pearl Gold", COLOR_METALS_PEARLESCENT_GOLD, -1 },
	};

	const std::vector<NamedVehiclePaint> PAINTS_WORN
	{
		{ "Black", COLOR_WORN_BLACK, COLOR_CLASSIC_BLACK },
		{ "Graphite", COLOR_WORN_GRAPHITE, COLOR_CLASSIC_SILVER },
		{ "Silver Grey", COLOR_WORN_SILVER_GREY, COLOR_CLASSIC_ICE_WHITE },
		{ "Silver", COLOR_WORN_SILVER, COLOR_WORN_SILVER },
		{ "Bluish Silver", COLOR_WORN_BLUE_SILVER, COLOR_CLASSIC_ICE_WHITE },
		{ "Shadow Silver", COLOR_WORN_SHADOW_SILVER, COLOR_CLASSIC_DIAMOND_BLUE },
		{ "Red", COLOR_WORN_RED, COLOR_CLASSIC_BLACK },
		{ "Golden Red", COLOR_WORN_GOLDEN_RED, COLOR_WORN_GOLDEN_RED },
		{ "Dark Red", COLOR_WORN_DARK_RED, COLOR_CLASSIC_BLACK },
		{ "Green", 58, 58 },
		{ "Dark Green", 59, 59 },
		{ "Sea Wash", COLOR_WORN_SEA_WASH, COLOR_WORN_SEA_WASH },
		{ "Dark Blue", COLOR_WORN_DARK_BLUE, COLOR_WORN_DARK_BLUE },
		{ "Blue", COLOR_WORN_BLUE, COLOR_WORN_BLUE },
		{ "Baby Blue", COLOR_WORN_BABY_BLUE, COLOR_CLASSIC_BLACK },
		{ "Honey Beige", COLOR_WORN_HONEY_BEIGE, COLOR_WORN_HONEY_BEIGE },
		{ "Brown", COLOR_WORN_BROWN, COLOR_CLASSIC_BLACK },
		{ "Dark Brown", COLOR_WORN_DARK_BROWN, -1 },
		{ "Straw Beige", COLOR_WORN_STRAW_BEIGE, COLOR_WORN_STRAW_BEIGE },
		{ "White", COLOR_WORN_WHITE, COLOR_WORN_WHITE },
		{ "Off White", COLOR_WORN_OFF_WHITE, COLOR_CLASSIC_BLACK },
		{ "Orange", COLOR_WORN_ORANGE, COLOR_CLASSIC_BLACK_STEEL },
		{ "Light Orange", COLOR_WORN_LIGHT_ORANGE, COLOR_WORN_LIGHT_ORANGE },
		{ "Taxi Yellow", COLOR_WORN_TAXI_YELLOW, COLOR_WORN_TAXI_YELLOW },
		{ "Pale Orange", COLOR_WORN_PALE_ORANGE, COLOR_WORN_PALE_ORANGE },
		{ "Olive Green", COLOR_WORN_ARMY_OLIVE_GREEN, COLOR_WORN_ARMY_OLIVE_GREEN },

	};

	std::vector<NamedVehiclePaint> PAINTS_CHROME
	{

	};

	std::vector<NamedVehiclePaint> PAINTS_CHAMELEON
	{

	};

#pragma endregion

	INT paintIndex_maxValue = 0;

	INT8 lastwheeltype = 0;
	INT8 lastfwheel = 0;
	INT8 lastbwheel = 0;

	const std::vector<NamedVehiclePaint>& GetPaintsNormal()    { return PAINTS_NORMAL; }
	const std::vector<NamedVehiclePaint>& GetPaintsMetallic()  { return PAINTS_METALLIC; }
	const std::vector<NamedVehiclePaint>& GetPaintsPearl()     { return PAINTS_PEARL; }
	const std::vector<NamedVehiclePaint>& GetPaintsMatte()     { return PAINTS_MATTE; }
	const std::vector<NamedVehiclePaint>& GetPaintsMetal()     { return PAINTS_METAL; }
	const std::vector<NamedVehiclePaint>& GetPaintsChrome()    { return PAINTS_CHROME; }
	const std::vector<NamedVehiclePaint>& GetPaintsChameleon() { return PAINTS_CHAMELEON; }
	const std::vector<NamedVehiclePaint>& GetPaintsUtil()      { return PAINTS_UTIL; }
	const std::vector<NamedVehiclePaint>& GetPaintsWorn()      { return PAINTS_WORN; }
	const std::vector<NamedVehiclePaint>& GetPaintsWheels()    { return PAINTS_WHEELS; }
	const std::vector<NamedVehiclePaint>& GetPaintsInterior()  { return PAINTS_INTERIOR; }
	const std::vector<NamedVehiclePaint>& GetPaintsDashboard() { return PAINTS_DASHBOARD; }
	INT GetPaintIndexMaxValue() { return paintIndex_maxValue; }

	void PopulateAllPaintIDs()
	{
		Model model = VEHICLE_ADDER;
		model.Load(5000);

		//spawn dummy vehicle
		Vector3 coords = ENTITY::GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(PLAYER::PLAYER_PED_ID(), 0.0, 0.0, -100.0);
		float heading = ENTITY::GET_ENTITY_HEADING(PLAYER::PLAYER_PED_ID());
		Vehicle veh = CREATE_VEHICLE(model.hash, coords.x, coords.y, coords.z, heading, 1, 0, 0);
		VEHICLE::SET_VEHICLE_ON_GROUND_PROPERLY(veh, 5.0f);
		int painttype, colour, pearl, second;

		//Loop paint types (normal, metallic, matte etc...)
		for (painttype = 0; painttype < 7; painttype++)
		{
			int numcols = GET_NUM_MOD_COLORS(painttype, 0);
			const char* colourname;

			//loop colour options and assign to PAINTS_ vectors
			for (int i = 0; i < numcols; i++)
			{
				second = 0;
				//set and get colour ID's and names
				VEHICLE::SET_VEHICLE_MOD_KIT(veh, 0);
				VEHICLE::SET_VEHICLE_MOD_COLOR_1(veh, painttype, i, 0);
				VEHICLE::GET_VEHICLE_EXTRA_COLOURS(veh, &pearl, &second);
				VEHICLE::GET_VEHICLE_COLOURS(veh, &colour, &second);
				colourname = VEHICLE::GET_VEHICLE_MOD_COLOR_1_NAME(veh, 0);
				std::string colourid = std::to_string(i);
				if (colour > paintIndex_maxValue)
					paintIndex_maxValue = colour;
				if (pearl > paintIndex_maxValue)
					paintIndex_maxValue = pearl;

				// write to relevant vector, depending on painttype
				switch (painttype)
				{
				case 0:
					PAINTS_METALLIC.resize(numcols);
					PAINTS_METALLIC[i].name = "Extra Colour " + colourid;
					if (colourname != nullptr)
					{
						PAINTS_METALLIC[i].name = Game::GetGXTEntry(colourname, "Extra Colour " + colourid);
					}

					PAINTS_METALLIC[i].paint = colour;
					PAINTS_METALLIC[i].pearl = pearl;
					break;
				case 1:
					PAINTS_NORMAL.resize(numcols);
					PAINTS_NORMAL[i].name = "Extra Colour " + colourid;
					if (colourname != nullptr)
					{
						PAINTS_NORMAL[i].name = Game::GetGXTEntry(colourname, "Extra Colour " + colourid);
					}

					PAINTS_NORMAL[i].paint = colour;
					PAINTS_NORMAL[i].pearl = pearl;
					break;
				case 2:
					PAINTS_PEARL.resize(numcols);
					PAINTS_PEARL[i].name = "Extra Colour " + colourid;
					if (colourname != nullptr)
					{
						PAINTS_PEARL[i].name = Game::GetGXTEntry(colourname, "Extra Colour " + colourid);
					}

					PAINTS_PEARL[i].paint = -1;
					PAINTS_PEARL[i].pearl = colour;
					break;
				case 3:
					PAINTS_MATTE.resize(numcols);
					PAINTS_MATTE[i].name = "Extra Colour " + colourid;
					if (colourname != nullptr)
					{
						PAINTS_MATTE[i].name = Game::GetGXTEntry(colourname, "Extra Colour " + colourid);
					}

					PAINTS_MATTE[i].paint = colour;
					PAINTS_MATTE[i].pearl = pearl;
					break;
				case 4:
					PAINTS_METAL.resize(numcols);
					PAINTS_METAL[i].name = "Extra Colour " + colourid;
					if (colourname != nullptr)
					{
						PAINTS_METAL[i].name = Game::GetGXTEntry(colourname, "Extra Colour " + colourid);
					}

					PAINTS_METAL[i].paint = colour;
					PAINTS_METAL[i].pearl = pearl;
					break;
				case 5:
					PAINTS_CHROME.resize(numcols);
					PAINTS_CHROME[i].name = "Extra Colour " + colourid;
					if (colourname != nullptr)
					{
						PAINTS_CHROME[i].name = Game::GetGXTEntry(colourname, "Extra Colour " + colourid);
					}

					PAINTS_CHROME[i].paint = colour;
					PAINTS_CHROME[i].pearl = pearl;
					break;
				case 6:
					PAINTS_CHAMELEON.resize(numcols);
					PAINTS_CHAMELEON[i].name = "Extra Colour " + colourid;
					if (colourname != nullptr)
					{
						PAINTS_CHAMELEON[i].name = Game::GetGXTEntry(colourname, "Extra Colour " + colourid);
					}

					PAINTS_CHAMELEON[i].paint = colour;
					PAINTS_CHAMELEON[i].pearl = pearl;
					break;
				}

			}
		}
		//unloading test vehicle from memory
		ENTITY::SET_VEHICLE_AS_NO_LONGER_NEEDED(&veh);
		VEHICLE::DELETE_VEHICLE(&veh);
		model.Unload();
	}

	INT getpaintCarUsing_index(Vehicle veh, INT partIndex_CustomK)
	{
		GTAvehicle vehicle(veh);

		switch (partIndex_CustomK)
		{
		case 1:
			return vehicle.GetPrimaryColour();
			break;
		case 2:
			return vehicle.GetSecondaryColour();
			break;
		case 3:
			return vehicle.GetPearlescentColour();
			break;
		case 4:
			return vehicle.GetRimColour();
			break;
		case 5:
			return vehicle.GetInteriorColour();
			break;
		case 6:
			return vehicle.GetDashboardColour();
			break;
		case 10:
			return g_spawnVehiclePrimaryColor;
			break;
		case 11:
			return g_spawnVehicleSecondaryColor;
			break;
		}

		return 0;
	}

	void paintCarUsing_index(Vehicle veh, INT partIndex_CustomK, INT16 colour_index, INT16 pearl_index)
	{
		switch (partIndex_CustomK)
		{
		case 10:
			g_spawnVehiclePrimaryColor = colour_index;
			return;
		case 11:
			g_spawnVehicleSecondaryColor = colour_index;
			return;
		}

		GTAvehicle vehicle(veh);
		if (vehicle.Exists())
			vehicle.RequestControlOnce();

		switch (partIndex_CustomK)
		{
		case 1:
			vehicle.ClearCustomPrimaryColour();
			vehicle.SetPrimaryColour(colour_index);
			if (pearl_index != -1)
				vehicle.SetPearlescentColour(pearl_index);
			break;
		case 2:
			vehicle.ClearCustomSecondaryColour();
			vehicle.SetSecondaryColour(colour_index);
			break;
		case 3:
			vehicle.SetPearlescentColour(colour_index);
			break;
		case 4:
			vehicle.SetRimColour(colour_index);
			break;
		case 5:
			vehicle.SetInteriorColour(colour_index);
			break;
		case 6:
			vehicle.SetDashboardColour(colour_index);
			break;
		}

	}

	void rgb_mode_set_carcol(Vehicle veh, INT16 R, INT16 G, INT16 B, INT16 A)
	{
		GTAvehicle vehicle(veh);
		if (vehicle.IsVehicle())
		{
			vehicle.RequestControlOnce();
			if (GET_VEHICLE_MOD_KIT(vehicle.GetHandle()) != 0)
				SET_VEHICLE_MOD_KIT(vehicle.GetHandle(), 0);
		}

		switch (bitMSPaintsRGBMode)
		{
		case 0: vehicle.SetCustomPrimaryColour(R, G, B);
			break;
		case 1: vehicle.SetCustomSecondaryColour(R, G, B);
			break;
		case 2: vehicle.SetNeonLightsColour(R, G, B);
			break;
		case 3:
			g_multiPlatNeonsColor.R = R;
			g_multiPlatNeonsColor.G = G;
			g_multiPlatNeonsColor.B = B;
			break;
		case 4:
			vehicle.ToggleMod(VehicleMod::TireSmoke, true);
			vehicle.SetTyreSmokeColour(R, G, B);
			break;
		case 7:
			SET_PLAYER_PARACHUTE_SMOKE_TRAIL_COLOR(g_Ped2, R, G, B);
			SET_PLAYER_CAN_LEAVE_PARACHUTE_SMOKE_TRAIL(g_Ped2, TRUE);
			break;

		case 9:
			g_spawnVehicleNeonColor = RgbS(R, G, B);
			break;
		case 10:
			REPLACE_HUD_COLOUR_WITH_RGBA(g_Ped4, R, G, B, A);
			break;
		}

	}

	void SetVehicleMaxUpgrades(Vehicle vehicle, bool upgradeIt, bool invincible, INT8 plateType, std::string plateText,
		bool neonIt, UINT8 NeonR, UINT8 NeonG, UINT8 NeonB, INT16 prim_col_index, INT16 sec_col_index)
	{
		if (!DOES_ENTITY_EXIST(vehicle) || !IS_ENTITY_A_VEHICLE(vehicle))
			return;
		srand(time(0));
		int i;

		GTAvehicle(vehicle).RequestControl();

		if (GET_VEHICLE_MOD_KIT(vehicle) != 0)
			SET_VEHICLE_MOD_KIT(vehicle, 0);

		if (invincible)
			SetVehicleInvincibleOn(vehicle);
		else
			SetVehicleInvincibleOff(vehicle);

		if (plateText.length() > 0)
			SET_VEHICLE_NUMBER_PLATE_TEXT(vehicle, (PCHAR)plateText.c_str());

		SET_VEHICLE_NUMBER_PLATE_TEXT_INDEX(vehicle, plateType);

		{
			TOGGLE_VEHICLE_MOD(vehicle, 18, 1*upgradeIt);
			TOGGLE_VEHICLE_MOD(vehicle, 20, 1 * upgradeIt);
			TOGGLE_VEHICLE_MOD(vehicle, 22, 1 * upgradeIt);

			for (i = 0; i < vValues_ModSlotNames.size(); i++)
			{
				if (i >= 17 && i <= 22)
					continue;
				if (i == 24)
				{
					UINT8 modIndex = GET_VEHICLE_MOD(vehicle, 23);
					SET_VEHICLE_MOD(vehicle, i, upgradeIt?modIndex:-1, 0);
					continue;
				}
				UINT8 modIndex = GET_NUM_VEHICLE_MODS(vehicle, i) - 1;				
				if (modIndex > -1)
					modIndex = std::rand() % (modIndex + 2) - 1;
				if (11 <= i && 16 >= i)
					modIndex = GET_NUM_VEHICLE_MODS(vehicle, i) - 1;
				SET_VEHICLE_MOD(vehicle, i, upgradeIt ? modIndex : -1, 0);
			}
			SET_VEHICLE_WINDOW_TINT(vehicle, 1 * upgradeIt);

			SET_VEHICLE_TYRES_CAN_BURST(vehicle, false);
		}

		if (neonIt) // neons
		{
			for (i = 0; i <= 3; i++)
				SET_VEHICLE_NEON_ENABLED(vehicle, i, TRUE);
			SET_VEHICLE_NEON_COLOUR(vehicle, NeonR, NeonG, NeonB);
		}

		WAIT(50);

		int colour1, colour2;
		GET_VEHICLE_EXTRA_COLOURS(vehicle, &colour1, &colour2);
		int inull = 0;
		if (prim_col_index != -3) // basic paint primary
		{
			CLEAR_VEHICLE_CUSTOM_PRIMARY_COLOUR(vehicle);
			GET_VEHICLE_COLOURS(vehicle, &i, &inull);
			SET_VEHICLE_COLOURS(vehicle, prim_col_index, inull);
			SET_VEHICLE_EXTRA_COLOURS(vehicle, 0, colour2);
		}
		if (sec_col_index != -3) // basic paint secondary
		{
			CLEAR_VEHICLE_CUSTOM_SECONDARY_COLOUR(vehicle);
			GET_VEHICLE_COLOURS(vehicle, &inull, &i);
			SET_VEHICLE_COLOURS(vehicle, inull, sec_col_index);
			SET_VEHICLE_EXTRA_COLOURS(vehicle, 0, colour2);
		}

		WAIT(40);
	}
	// Wheels
	bool msWheelsBitBikeBack = false;
	int  msWheelsMaxWindices = 0;
	static const std::vector<std::string> g_msWheelTypeNames{
		"Sport", "Muscle", "Lowrider", "SUV", "Offroad", "Tuner",
		"Bike", "High-End", "Benny's Originals", "Benny's Bespoke",
		"Open Wheel", "Street", "Track" };
	const std::vector<std::string>& GetWheelTypeNames() { return g_msWheelTypeNames; }

	// Windows
	UINT8 msWindowsMode = 0;
	static const std::vector<std::string> g_msWindowsModeNames{
		"Open", "Close", "Break", "Fix", "Remove" };
	static const std::vector<std::string> g_msWindowsWindowNames{
		"Front Left", "Front Right", "Back Left", "Back Right" };
	static const std::vector<std::string> g_msWindowsWinTintNames{
		"None", "Black", "CMOD_WIN_2", "CMOD_WIN_1", "Stock", "CMOD_WIN_3", "Green" };
	const std::vector<std::string>& GetMsWindowsModeNames()    { return g_msWindowsModeNames; }
	const std::vector<std::string>& GetMsWindowsWindowNames()  { return g_msWindowsWindowNames; }
	const std::vector<std::string>& GetMsWindowsWinTintNames() { return g_msWindowsWinTintNames; }

	namespace MSWindows_catind
	{
		using MSWINDOWS_MODE = MSWindowsMode;

		void DoWindow(GTAvehicle vehicle, VehicleWindow window, UINT8 mode)
		{
			vehicle.RequestControl();

			switch (mode)
			{
			case MSWINDOWS_MODE_OPEN:
				vehicle.RollDownWindow(window);
				break;
			case MSWINDOWS_MODE_CLOSE:
				vehicle.RollUpWindow(window);
				break;
			case MSWINDOWS_MODE_BREAK:
				vehicle.SmashWindow(window);
				break;
			case MSWINDOWS_MODE_FIX:
				vehicle.FixWindow(window);
				break;
			case MSWINDOWS_MODE_REMOVE:
				vehicle.RemoveWindow(window);
				break;
			}
		}
	}

	void MsWindowsDoWindow(GTAvehicle& vehicle, VehicleWindow window, UINT8 mode)
	{
		MSWindows_catind::DoWindow(vehicle, window, mode);
	}
	UINT8 msDoorsActionIndex = 0;
	bool msLightsLeftInd = false;
	bool msLightsRightInd = false;
	bool msLightsHazard = false;
}
