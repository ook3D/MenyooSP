#include "VehicleModShop.h"

#include "../Menu/Engine.h"
#include "../Menu/SubmenuRegistry.h"
#include "../Menu/Menu.h"
#include "../Menu/Routine.h"
#include "Neons.h"

#include "../Natives/natives2.h"
#include "../Scripting/enums.h"
#include "../Scripting/Game.h"
#include "../Scripting/Model.h"
#include "../Scripting/GTAvehicle.h"
#include "../Scripting/GTAvehicleData.h"
#include "../Util/GTAmath.h"
#include "../Util/StringManip.h"
#include "../Memory/GTAmemory.h"

#include "VehicleModShopRuntime.h"
#include "VehicleRuntime.h"
#include "VehicleSpawnerRuntime.h"
#include "PlayerRuntime.h"
#include "SettingsRuntime.h"

#include <array>
#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace Menu {

namespace {

constexpr Checkbox CHECKBOX_NONE     = (Checkbox)0;
constexpr Checkbox CHECKBOX_CARTHING  = (Checkbox)2;
constexpr Checkbox CHECKBOX_BIKETHING = (Checkbox)4;

void RememberLastPaintForVehicle(int vehicleHandle)
{
	sub::lastpaint  = sub::getpaintCarUsing_index(vehicleHandle, msCurrentPaintIndex);
	sub::lastpearl  = sub::getpaintCarUsing_index(vehicleHandle, 3);
	sub::iscustompaint = false;
}

void AddMsPaintsPointOption(Submenu& ownerForNav, const std::string& text, INT8 index, bool& outChanged)
{
	(void)ownerForNav;
	Engine* e = Engine::Current();
	if (!e) return;
	if (e->AddOption(text, /*showArrow=*/true))
	{
		msCurrentPaintIndex = index;
		sub::getpaint = true;
		outChanged = true;
	}
}

void AddCarColOption(const std::string& text, int vehicleHandle, INT16 colour_index, INT16 pearl_index_ifPrimary)
{
	Engine* e = Engine::Current();
	if (!e) return;

	const Checkbox tickIcon = IS_THIS_MODEL_A_BIKE(GET_ENTITY_MODEL(vehicleHandle))
		? CHECKBOX_BIKETHING : CHECKBOX_CARTHING;

	if (g_LSCCustoms)
	{
		if (sub::getpaint)
		{
			sub::lastpaint = sub::getpaintCarUsing_index(vehicleHandle, msCurrentPaintIndex);
			sub::lastpearl = sub::getpaintCarUsing_index(vehicleHandle, 3);
			if (msCurrentPaintIndex == 1 && GET_IS_VEHICLE_PRIMARY_COLOUR_CUSTOM(vehicleHandle))
			{
				sub::iscustompaint = true;
				GET_VEHICLE_CUSTOM_PRIMARY_COLOUR(vehicleHandle, &sub::lastr, &sub::lastg, &sub::lastb);
			}
			if (msCurrentPaintIndex == 2 && GET_IS_VEHICLE_SECONDARY_COLOUR_CUSTOM(vehicleHandle))
			{
				sub::iscustompaint = true;
				GET_VEHICLE_CUSTOM_SECONDARY_COLOUR(vehicleHandle, &sub::lastr, &sub::lastg, &sub::lastb);
			}
			sub::getpaint = false;
		}

		const bool isActive = (sub::lastpaint == colour_index);
		const bool pressed = e->AddCheckbox(text, isActive, tickIcon, CHECKBOX_NONE);

		// Hover-preview: while this row is the active selection, mirror the colour onto the vehicle.
		if (e->ActiveSelection() == e->printingOption
			&& sub::getpaintCarUsing_index(vehicleHandle, msCurrentPaintIndex) != colour_index)
		{
			sub::paintCarUsing_index(vehicleHandle, msCurrentPaintIndex, colour_index, pearl_index_ifPrimary);
		}

		if (pressed)
		{
			sub::lastpaint = sub::getpaintCarUsing_index(vehicleHandle, msCurrentPaintIndex);
			sub::lastpearl = sub::getpaintCarUsing_index(vehicleHandle, 3);
			sub::iscustompaint = false;
		}
	}
	else
	{
		const bool isActive = (colour_index == sub::getpaintCarUsing_index(vehicleHandle, msCurrentPaintIndex));
		if (e->AddCheckbox(text, isActive, tickIcon, CHECKBOX_NONE))
		{
			sub::paintCarUsing_index(vehicleHandle, msCurrentPaintIndex, colour_index, pearl_index_ifPrimary);
		}
	}
}

void DrawPaintTable(const std::vector<sub::NamedVehiclePaint>& paints, int vehicleHandle, bool pearlAsBoth = false)
{
	for (const sub::NamedVehiclePaint& p : paints)
	{
		if (pearlAsBoth)
			AddCarColOption(p.name, vehicleHandle, p.pearl, p.pearl);
		else
			AddCarColOption(p.name, vehicleHandle, p.paint, p.pearl);
	}
}

void AddPaintIndexSlider(int vehicleHandle)
{
	Engine* e = Engine::Current();
	if (!e) return;

	int paintIndex = sub::getpaintCarUsing_index(vehicleHandle, msCurrentPaintIndex);

	int totalpaints = 0;
	for (int i = 0; i < 5; i++)
		totalpaints += GET_NUM_MOD_COLORS(i, 1);
	const int extrapaints = totalpaints - 232;
	const int paintIndex_maxValue = 160 + extrapaints;

	const InputResult res = e->AddNumber("Paint Index", (double)paintIndex, 0);

	if (res.rightPressed)
	{
		if (paintIndex < paintIndex_maxValue) paintIndex++;
		else paintIndex = 0;
		sub::paintCarUsing_index(vehicleHandle, msCurrentPaintIndex, paintIndex, -1);
		if (g_LSCCustoms) RememberLastPaintForVehicle(vehicleHandle);
	}
	else if (res.leftPressed)
	{
		if (paintIndex > 0) paintIndex--;
		else paintIndex = paintIndex_maxValue;
		sub::paintCarUsing_index(vehicleHandle, msCurrentPaintIndex, paintIndex, -1);
		if (g_LSCCustoms) RememberLastPaintForVehicle(vehicleHandle);
	}
	else if (res.accepted)
	{
		std::string inputStr = Game::InputBox("", 4U, "Enter a paint index:", std::to_string(paintIndex));
		if (!inputStr.empty())
		{
			try
			{
				paintIndex = std::stoi(inputStr);
				sub::paintCarUsing_index(vehicleHandle, msCurrentPaintIndex, paintIndex, -1);
				if (g_LSCCustoms) RememberLastPaintForVehicle(vehicleHandle);
			}
			catch (...)
			{
				Game::Print::PrintErrorInvalidInput(inputStr);
			}
		}
	}
}

std::string PaintIndexToTitle(INT8 idx, const char* fallback)
{
	switch (idx)
	{
	case 1: case 10: return Game::GetGXTEntry("CMOD_COL0_0", "Primary");
	case 2: case 11: return Game::GetGXTEntry("CMOD_COL0_1", "Secondary");
	case 3:          return Game::GetGXTEntry("CMOD_COL1_6", "Pearlescent");
	case 4:          return Game::GetGXTEntry("CMOD_MOD_WHEM", "Wheels");
	case 5:          return "Interior";
	case 6:          return "Dashboard";
	default:         return fallback;
	}
}

void AddMsDoorsOption(const std::string& text, GTAvehicle vehicle, VehicleDoor door, UINT8 supposedAction, bool instantly = false, bool loose = false)
{
	Engine* e = Engine::Current();
	if (!e) return;

	UINT8 action = supposedAction;
	bool conditionForTick = false;

	switch (supposedAction)
	{
	case 0: // Open
		conditionForTick = vehicle.IsDoorOpen(door);
		action = conditionForTick ? 2 : 0;
		break;
	case 1: // Open All
		conditionForTick = true;
		for (auto& d : vehicle.Doors_get())
			conditionForTick = conditionForTick && vehicle.IsDoorOpen(d);
		action = conditionForTick ? 3 : 1;
		break;
	}

	if (!e->AddCheckbox(text, conditionForTick))
		return;

	vehicle.RequestControl();
	switch (action)
	{
	case 0: vehicle.OpenDoor(door, loose, instantly, true); break;
	case 1:
		vehicle.OpenDoor(VehicleDoor::FrontLeftDoor,  loose, instantly, true);
		vehicle.OpenDoor(VehicleDoor::FrontRightDoor, loose, instantly, true);
		vehicle.OpenDoor(VehicleDoor::BackLeftDoor,   loose, instantly, true);
		vehicle.OpenDoor(VehicleDoor::BackRightDoor,  loose, instantly, true);
		vehicle.OpenDoor(VehicleDoor::Hood,           loose, instantly, true);
		vehicle.OpenDoor(VehicleDoor::Trunk,          loose, instantly, true);
		vehicle.OpenDoor(VehicleDoor::Trunk2,         loose, instantly, true);
		break;
	case 2: vehicle.CloseDoor(door, instantly, true); break;
	case 3:
		vehicle.CloseDoor(VehicleDoor::FrontLeftDoor,  instantly, true);
		vehicle.CloseDoor(VehicleDoor::FrontRightDoor, instantly, true);
		vehicle.CloseDoor(VehicleDoor::BackLeftDoor,   instantly, true);
		vehicle.CloseDoor(VehicleDoor::BackRightDoor,  instantly, true);
		vehicle.CloseDoor(VehicleDoor::Hood,           instantly, true);
		vehicle.CloseDoor(VehicleDoor::Trunk,          instantly, true);
		vehicle.CloseDoor(VehicleDoor::Trunk2,         instantly, true);
		break;
	case 4: vehicle.BreakDoor(door, instantly); break;
	case 5: vehicle.BreakAllDoors(instantly);   break;
	case 6: vehicle.FixDoor(door); break;
	case 7: vehicle.FixAllDoors();  break;
	}
}

void AddWheelOption(const std::string& text, int vehicleHandle, INT8 wheelType, INT16 wheelIndex, bool isBikeBack)
{
	Engine* e = Engine::Current();
	if (!e) return;

	const Checkbox tickIcon = IS_THIS_MODEL_A_BIKE(GET_ENTITY_MODEL(vehicleHandle))
		? CHECKBOX_BIKETHING : CHECKBOX_CARTHING;

	if (g_LSCCustoms)
	{
		INT8& lastwheelRelevant = isBikeBack ? sub::lastbwheel : sub::lastfwheel;
		const bool isActive = (lastwheelRelevant == wheelIndex && sub::lastwheeltype == wheelType);
		const bool pressed = e->AddCheckbox(text, isActive, tickIcon, CHECKBOX_NONE);

		const bool allowSettingWheelPreview = GET_VEHICLE_WHEEL_TYPE(vehicleHandle) != wheelType ||
			(wheelType == WheelType::BikeWheels && isBikeBack
				? GET_VEHICLE_MOD(vehicleHandle, VehicleMod::BackWheels) != wheelIndex
				: GET_VEHICLE_MOD(vehicleHandle, VehicleMod::FrontWheels) != wheelIndex);

		if (e->ActiveSelection() == e->printingOption && allowSettingWheelPreview)
		{
			GTAvehicle(vehicleHandle).RequestControlOnce();
			SET_VEHICLE_WHEEL_TYPE(vehicleHandle, wheelType);
			if (wheelType == WheelType::BikeWheels)
			{
				if (isBikeBack)
					SET_VEHICLE_MOD(vehicleHandle, VehicleMod::BackWheels, wheelIndex,
						GET_VEHICLE_MOD_VARIATION(vehicleHandle, VehicleMod::BackWheels));
				else
					SET_VEHICLE_MOD(vehicleHandle, VehicleMod::FrontWheels, wheelIndex,
						GET_VEHICLE_MOD_VARIATION(vehicleHandle, VehicleMod::FrontWheels));
			}
			else
			{
				SET_VEHICLE_MOD(vehicleHandle, VehicleMod::FrontWheels, wheelIndex,
					GET_VEHICLE_MOD_VARIATION(vehicleHandle, VehicleMod::FrontWheels));
			}
		}

		if (pressed)
		{
			sub::lastwheeltype = wheelType;
			sub::lastfwheel = GET_VEHICLE_MOD(vehicleHandle, VehicleMod::FrontWheels);
			sub::lastbwheel = GET_VEHICLE_MOD(vehicleHandle, VehicleMod::BackWheels);
		}
	}
	else
	{
		const INT currWheelType  = GET_VEHICLE_WHEEL_TYPE(vehicleHandle);
		const INT currWheelIndex = GET_VEHICLE_MOD(vehicleHandle,
			isBikeBack ? (int)VehicleMod::BackWheels : (int)VehicleMod::FrontWheels);
		const bool isActive = (currWheelIndex == wheelIndex && currWheelType == wheelType);
		if (e->AddCheckbox(text, isActive, tickIcon, CHECKBOX_NONE))
		{
			GTAvehicle(vehicleHandle).RequestControl();
			SET_VEHICLE_WHEEL_TYPE(vehicleHandle, wheelType);
			if (wheelType == WheelType::BikeWheels)
			{
				if (isBikeBack)
					SET_VEHICLE_MOD(vehicleHandle, VehicleMod::BackWheels, wheelIndex,
						GET_VEHICLE_MOD_VARIATION(vehicleHandle, VehicleMod::BackWheels));
				else
					SET_VEHICLE_MOD(vehicleHandle, VehicleMod::FrontWheels, wheelIndex,
						GET_VEHICLE_MOD_VARIATION(vehicleHandle, VehicleMod::FrontWheels));
			}
			else
			{
				SET_VEHICLE_MOD(vehicleHandle, VehicleMod::FrontWheels, wheelIndex,
					GET_VEHICLE_MOD_VARIATION(vehicleHandle, VehicleMod::FrontWheels));
				SET_VEHICLE_MOD(vehicleHandle, VehicleMod::BackWheels, wheelIndex,
					GET_VEHICLE_MOD_VARIATION(vehicleHandle, VehicleMod::BackWheels));
			}
		}
	}
}

template <typename SetterFn>
void AddVehicleMultRow(const std::string& label, double& currentVal, std::map<Vehicle, float>& mult, GTAvehicle& vehicle,
	double step, int decimals, SetterFn applySetter)
{
	Engine* e = Engine::Current();
	if (!e) return;

	const InputResult res = e->AddNumber(label, currentVal, decimals);
	auto writeBack = [&](double newVal)
	{
		currentVal = newVal;
		vehicle.RequestControl(400);
		applySetter(vehicle, (float)currentVal);
		mult[vehicle.Handle()] = (float)currentVal;
	};

	if (res.rightPressed && currentVal < FLT_MAX - step) writeBack(currentVal + step);
	else if (res.leftPressed && currentVal > -FLT_MAX + step) writeBack(currentVal - step);
	else if (res.accepted)
	{
		std::stringstream ss;
		ss << std::fixed << std::setprecision(decimals) << currentVal;
		std::string inputStr = Game::InputBox("", 10U, "", ss.str());
		if (!inputStr.empty())
		{
			try { writeBack(std::stof(inputStr)); }
			catch (...) { Game::Print::PrintErrorInvalidInput(inputStr); }
		}
	}
}

void BumpRgbChannel(int& channel, int direction)
{
	channel += direction;
	if (channel > 255) channel = 0;
	else if (channel < 0) channel = 255;
}

} // namespace (anonymous)

void ModShopSubmenu::Draw()
{
	if (!(DOES_ENTITY_EXIST(g_Ped4) && IS_ENTITY_A_VEHICLE(g_Ped4)))
	{
		Engine* eng = Engine::Current();
		if (eng) eng->GoBack();
		return;
	}

	Engine* e = Engine::Current();
	if (!e) return;
	DrawTitle();

	const Model vehicleModel = GET_ENTITY_MODEL(g_Ped4);
	GTAvehicle vehicle = g_Ped4;
	const bool isBike = (vehicleModel.IsBike() || vehicleModel.IsBicycle() || vehicleModel.IsQuadbike());

	auto rpmMultIt = g_multListRPM.find(vehicle.Handle());
	double rpmMultVal = (rpmMultIt != g_multListRPM.end()) ? rpmMultIt->second : 1.0;

	auto torqueMultIt = g_multListTorque.find(vehicle.Handle());
	double torqueMultVal = (torqueMultIt != g_multListTorque.end()) ? torqueMultIt->second : 1.0;

	auto maxSpeedMultIt = g_multListMaxSpeed.find(vehicle.Handle());
	double maxSpeedMultVal = (maxSpeedMultIt != g_multListMaxSpeed.end())
		? maxSpeedMultIt->second
		: GET_VEHICLE_MODEL_ESTIMATED_MAX_SPEED(vehicleModel.hash);

	auto headLightsMultIt = g_multListHeadLights.find(vehicle.Handle());
	double headLightsMultVal = (headLightsMultIt != g_multListHeadLights.end()) ? headLightsMultIt->second : 1.0;

	std::string ms_plateText = GET_VEHICLE_NUMBER_PLATE_TEXT(g_Ped4);
	const std::vector<std::string> ms_vPlateTypeNames{
		"CMOD_PLA_0", "CMOD_PLA_4", "CMOD_PLA_3", "CMOD_PLA_1", "CMOD_PLA_2",
		"Yankton", "CMOD_PLA_6", "CMOD_PLA_7", "CMOD_PLA_8", "CMOD_PLA_9",
		"CMOD_PLA_10", "CMOD_PLA_11", "CMOD_PLA_12" };
	int veh_plate_current = GET_VEHICLE_NUMBER_PLATE_TEXT_INDEX(g_Ped4);
	if (veh_plate_current < 0) veh_plate_current = 0;

	if (DrawOption("Random Upgrades"))
	{
		vehicle.RequestControl(400);
		sub::SetVehicleMaxUpgrades(g_Ped4, true, false, GET_VEHICLE_NUMBER_PLATE_TEXT_INDEX(g_Ped4));
		SET_VEHICLE_TYRES_CAN_BURST(g_Ped4, false);
		return;
	}
	if (DrawOption("Return to Stock"))
	{
		vehicle.RequestControl(400);
		sub::SetVehicleMaxUpgrades(g_Ped4, false, false, GET_VEHICLE_NUMBER_PLATE_TEXT_INDEX(g_Ped4));
		SET_VEHICLE_TYRES_CAN_BURST(g_Ped4, true);
		return;
	}

	if (DrawOption(Game::GetGXTEntry("S_MO_09", "Benny's Lowrider Mods")))
		NavigateTo("vehicle_modshop_bennys");

	if (vehicleModel.IsWheelChangingSupportedVehicle() && 65535 != GET_VEHICLE_MOD_KIT(g_Ped4))
	{
		if (DrawOption(Game::GetGXTEntry("CMOD_MOD_WHEM", "Wheels")))
			NavigateTo("vehicle_modshop_wheels");
	}

	sub::selectmod = true;

	for (int i = 0; i <= 24; i++)
	{
		if (i >= 17 && i <= 24) continue; // Toggleables & wheel slots not here
		if (GET_NUM_VEHICLE_MODS(g_Ped4, i) > 0)
		{
			if (DrawOption(GetModSlotName(g_Ped4, i, true)))
			{
				sub::lastMod = -2;
				msCurrentPaintIndex = i;
				NavigateTo("vehicle_modshop_cat_all");
				return;
			}
		}
	}

	if (DrawOption("Paints")) NavigateTo("vehicle_modshop_paints");

	if (!isBike)
	{
		if (DrawOption(Game::GetGXTEntry("PIM_PVEO_004", "Neons Lights"))) NavigateTo("vehicle_modshop_neons");
		if (DrawOption(Game::GetGXTEntry("CMM_MOD_S6", "Doors")))          NavigateTo("vehicle_modshop_doors");
		if (DrawOption(Game::GetGXTEntry("CMOD_MOD_WIN", "Windows")))      NavigateTo("vehicle_modshop_windows");
	}
	if (DrawOption(Game::GetGXTEntry("CMOD_COL0_3", "Emblem")))            NavigateTo("vehicle_modshop_emblem");

	if (DrawOption(Game::GetGXTEntry("CMOD_MOD_GLD2", "Extras")))
	{
		bool anyExtra = false;
		for (int i = 0; i <= 12; i++)
			if (DOES_EXTRA_EXIST(g_Ped4, i)) { anyExtra = true; break; }
		if (anyExtra) NavigateTo("vehicle_modshop_extra");
		else Game::Print::PrintBottomCentre("~r~Error:~s~ Vehicle has no extras.");
		return;
	}

	if (GET_VEHICLE_LIVERY_COUNT(g_Ped4) > 0)
	{
		int ms_livery = GET_VEHICLE_LIVERY(g_Ped4) + 1;
		const InputResult res = e->AddNumber(Game::GetGXTEntry("CMOD_COL0_4", "Livery"), (double)ms_livery, 0);
		if (res.rightPressed)
		{
			if (ms_livery < GET_VEHICLE_LIVERY_COUNT(g_Ped4)) ms_livery++;
			SET_VEHICLE_LIVERY(g_Ped4, ms_livery - 1);
			return;
		}
		if (res.leftPressed)
		{
			if (ms_livery > 1) ms_livery--;
			SET_VEHICLE_LIVERY(g_Ped4, ms_livery - 1);
			return;
		}
	}
	if (GET_VEHICLE_LIVERY2_COUNT(g_Ped4) > 0)
	{
		int ms_livery2 = GET_VEHICLE_LIVERY2(g_Ped4) + 1;
		const InputResult res = e->AddNumber("Roof Livery", (double)ms_livery2, 0);
		if (res.rightPressed)
		{
			if (ms_livery2 < GET_VEHICLE_LIVERY2_COUNT(g_Ped4)) ms_livery2++;
			SET_VEHICLE_LIVERY2(g_Ped4, ms_livery2 - 1);
			return;
		}
		if (res.leftPressed)
		{
			if (ms_livery2 > 1) ms_livery2--;
			SET_VEHICLE_LIVERY2(g_Ped4, ms_livery2 - 1);
			return;
		}
	}

	if (DrawToggleExternal(Game::GetGXTEntry("CMOD_MOD_TUR", "Turbo"), IS_TOGGLE_MOD_ON(g_Ped4, VehicleMod::Turbo)))
	{
		TOGGLE_VEHICLE_MOD(g_Ped4, 18, IS_TOGGLE_MOD_ON(g_Ped4, 18) ? 0 : 1);
		return;
	}
	if (DrawToggleExternal(Game::GetGXTEntry("CMOD_LGT_1", "Xenon Lights"), IS_TOGGLE_MOD_ON(g_Ped4, VehicleMod::XenonHeadlights)))
	{
		TOGGLE_VEHICLE_MOD(g_Ped4, 22, IS_TOGGLE_MOD_ON(g_Ped4, 22) ? 0 : 1);
		return;
	}
	if (DrawToggleExternal("Lower Suspension", sub::lowersuspension))
	{
		vehicle.RequestControlOnce();
		sub::lowersuspension = !sub::lowersuspension;
		SET_REDUCED_SUSPENSION_FORCE(g_Ped4, sub::lowersuspension);
		return;
	}

	if (GTAmemory::GetGameVersion() >= eGameVersion::VER_1_0_1604_0_STEAM
		&& vehicle.IsToggleModOn(VehicleMod::XenonHeadlights))
	{
		const std::vector<std::string> vHlColours{
			"White","Blue","Light Blue","Green","Light Green","Light Yellow","Yellow",
			"Orange","Red","Light Pink","Pink","Purple","Light Purple" };
		int hlColour = vehicle.GetHeadlightColour();
		int idx = (hlColour == 255 ? 0 : hlColour);
		const InputResult res = e->AddTextList(std::string(Game::GetGXTEntry("CMOD_LGT_1", "Xenon Lights")) + " Colour",
			idx, hlColour == 255 ? std::vector<std::string>{"Stock"} : vHlColours);
		if (res.rightPressed)
		{
			if (hlColour >= (int)vHlColours.size()) hlColour = 0;
			else if (hlColour < (int)vHlColours.size() - 1) hlColour++;
			vehicle.SetHeadlightColour(hlColour);
		}
		else if (res.leftPressed)
		{
			if (hlColour == 0) hlColour = 255;
			else if (hlColour != 255 && hlColour > 0) hlColour--;
			vehicle.SetHeadlightColour(hlColour);
		}
	}

	AddVehicleMultRow(std::string(Game::GetGXTEntry("CMOD_MOD_LGT_H", "Headlights")) + " Intensity",
		headLightsMultVal, g_multListHeadLights, vehicle, 0.1, 2,
		[](GTAvehicle& v, float val) { v.SetLightsMultiplier(val); });

	// Plate type
	{
		const InputResult res = e->AddTextList(
			std::string(Game::GetGXTEntry("CMOD_MOD_PLA", "Plate")) + " " + Game::GetGXTEntry("FMMC_MTYPE", "Type"),
			veh_plate_current, ms_vPlateTypeNames);
		if (res.rightPressed && veh_plate_current < GET_NUMBER_OF_VEHICLE_NUMBER_PLATES() - 1)
		{
			veh_plate_current++;
			vehicle.RequestControl();
			SET_VEHICLE_NUMBER_PLATE_TEXT_INDEX(g_Ped4, veh_plate_current);
			return;
		}
		if (res.leftPressed && veh_plate_current > 0)
		{
			veh_plate_current--;
			vehicle.RequestControl();
			SET_VEHICLE_NUMBER_PLATE_TEXT_INDEX(g_Ped4, veh_plate_current);
			return;
		}
	}

	// Plate text
	{
		int dummyIdx = 0;
		const InputResult res = e->AddTextList(Game::GetGXTEntry("CMOD_MOD_18_D", "Plate Text"),
			dummyIdx, std::vector<std::string>{ms_plateText});
		if (res.accepted)
		{
			std::string inputStr = Game::InputBox("", 8U, "CMOD_MOD_18_D", ms_plateText);
			if (!inputStr.empty() && inputStr.length() <= 8)
			{
				vehicle.RequestControl(400);
				vehicle.SetNumberPlateText(inputStr);
				Game::Print::PrintBottomLeft("CMOD_PLATEFIT", true);
			}
			else Game::Print::PrintErrorInvalidInput(inputStr);
			return;
		}
	}

	if (DrawOption(std::string(Game::GetGXTEntry("CMM_MOD_G3", "Engine")) + " " + Game::GetGXTEntry("VEUI_AUD_TIT", "Sound")))
		NavigateTo("vehicle_modshop_engine_sound");

	AddVehicleMultRow("Horse Power Multiplier", rpmMultVal, g_multListRPM, vehicle, 0.1, 2,
		[](GTAvehicle& v, float val) { v.SetEnginePowerMultiplier(val); });
	AddVehicleMultRow("Torque Multiplier", torqueMultVal, g_multListTorque, vehicle, 0.1, 2,
		[](GTAvehicle& v, float val) { v.SetEngineTorqueMulitplier(val); });

	{
		double speedKmph = maxSpeedMultVal * 3.6;
		const InputResult res = e->AddNumber(
			std::string(Game::GetGXTEntry("FMMC_VEHST_0", "Top Speed")) + " (Kmph)", speedKmph, 0);
		auto writeBack = [&](double kmph)
		{
			maxSpeedMultVal = kmph / 3.6;
			vehicle.RequestControl(400);
			vehicle.SetMaxSpeed((float)maxSpeedMultVal);
			g_multListMaxSpeed[vehicle.Handle()] = (float)maxSpeedMultVal;
		};
		if (res.rightPressed && maxSpeedMultVal < FLT_MAX / 3.6 - 3.6) writeBack(speedKmph + 1.0);
		else if (res.leftPressed && maxSpeedMultVal > -FLT_MAX / 3.6 + 3.6) writeBack(speedKmph - 1.0);
		else if (res.accepted)
		{
			std::stringstream ss; ss << std::fixed << std::setprecision(0) << speedKmph;
			std::string inputStr = Game::InputBox("", 9U, "", ss.str());
			if (!inputStr.empty())
			{
				try { writeBack(std::stof(inputStr)); }
				catch (...) { Game::Print::PrintErrorInvalidInput(inputStr); }
			}
		}
	}

	if (DrawOption(Game::GetGXTEntry("CMOD_MOD_LGT", "Lights")))
		NavigateTo("vehicle_modshop_lights");

	if (e->AddCheckbox(Game::GetGXTEntry("CMM_MOD_G3", "Engine"),
		vehicle.GetEngineRunning(), Checkbox::BOXTICK, Checkbox::BOXBLANK))
	{
		vehicle.SetEngineRunning(!vehicle.GetEngineRunning());
	}

	if (vehicleModel.IsPlane())
	{
		if (DrawOption("Plane Aileron On"))  DISABLE_PLANE_AILERON(g_Ped4, 0, 0);
		if (DrawOption("Plane Aileron Off")) DISABLE_PLANE_AILERON(g_Ped4, 0, 1);
	}

	if (vehicle.GetHasSiren())
	{
		if (e->AddCheckbox("Sirens", vehicle.GetSirenActive(), Checkbox::BOXTICK, Checkbox::BOXBLANK))
			vehicle.SetSirenActive(!vehicle.GetSirenActive());
	}

	if (DrawOption("AUTO UPGRADE"))
	{
		vehicle.RequestControl(400);
		sub::SetVehicleMaxUpgrades(g_Ped4, true, false, GET_VEHICLE_NUMBER_PLATE_TEXT_INDEX(g_Ped4));
		SET_VEHICLE_TYRES_CAN_BURST(g_Ped4, false);
		return;
	}
	DrawToggle("LSC Style Part Selection", g_LSCCustoms);

	if (GET_VEHICLE_MOD_KIT(g_Ped4) != 0)
	{
		vehicle.RequestControlOnce();
		SET_VEHICLE_MOD_KIT(vehicle.GetHandle(), 0);
	}
}

void ModShopBennysSubmenu::Draw()
{
	GTAvehicle vehicle = g_Ped4;
	if (!vehicle.Exists())
	{
		Engine* eng = Engine::Current();
		if (eng) eng->GoBack();
		return;
	}

	DrawTitle();

	if (DrawOption("Interior Colour"))
	{
		msCurrentPaintIndex = 5;
		NavigateTo("vehicle_modshop_paints_shared");
		return;
	}
	if (DrawOption("Dashboard Colour"))
	{
		msCurrentPaintIndex = 6;
		NavigateTo("vehicle_modshop_paints_shared");
		return;
	}

	sub::selectmod = true;

	for (int i = 25; i <= 48; i++)
	{
		if (GET_NUM_VEHICLE_MODS(vehicle.Handle(), i) > 0)
		{
			if (DrawOption(GetModSlotName(vehicle.Handle(), i, true)))
			{
				sub::lastMod = -2;
				msCurrentPaintIndex = i;
				NavigateTo("vehicle_modshop_cat_all");
				return;
			}
		}
	}
}

void ModShopCatAllSubmenu::Draw()
{
	Vehicle& vehicle = g_Ped4;
	if (!DOES_ENTITY_EXIST(vehicle))
	{
		Engine* eng = Engine::Current();
		if (eng) eng->GoBack();
		return;
	}

	Engine* e = Engine::Current();
	if (!e) return;

	INT& modType = msCurrentPaintIndex;
	const INT maxMod  = GET_NUM_VEHICLE_MODS(vehicle, modType) - 1;
	const INT currMod = GET_VEHICLE_MOD(vehicle, modType);

	if (sub::selectmod)
	{
		sub::lastMod = currMod;
		sub::selectmod = false;
	}

	e->AddTitle(GetModSlotName(vehicle, modType, true));

	const Checkbox tickIcon = IS_THIS_MODEL_A_BIKE(GET_ENTITY_MODEL(vehicle))
		? CHECKBOX_BIKETHING : CHECKBOX_CARTHING;

	if (g_LSCCustoms)
	{
		for (INT i = -1; i <= maxMod; i++)
		{
			const bool isActive = (sub::lastMod == i);
			const bool pressed = e->AddCheckbox(GetModTextLabel(vehicle, modType, i, true), isActive, tickIcon, CHECKBOX_NONE);

			if (e->ActiveSelection() == e->printingOption && currMod != i)
				SET_VEHICLE_MOD(vehicle, modType, i, GET_VEHICLE_MOD_VARIATION(vehicle, modType));
			if (pressed)
				sub::lastMod = i;
		}
	}
	else
	{
		for (INT i = -1; i <= maxMod; i++)
		{
			const bool isActive = (currMod == i);
			if (e->AddCheckbox(GetModTextLabel(vehicle, modType, i, true), isActive, tickIcon, CHECKBOX_NONE))
			{
				SET_VEHICLE_MOD(vehicle, modType, i, GET_VEHICLE_MOD_VARIATION(vehicle, modType));
			}
		}
	}
}

void ModShopEmblemSubmenu::Draw()
{
	GTAvehicle vehicle = g_Ped4;
	if (!vehicle.Exists())
	{
		Engine* eng = Engine::Current();
		if (eng) eng->GoBack();
		return;
	}

	Engine* e = Engine::Current();
	if (!e) return;
	DrawTitle();

	if (e->AddCheckbox(Game::GetGXTEntry("FMMC_REM", "CLEAR"), true, Checkbox::CROSS, CHECKBOX_NONE))
	{
		vehicle.RequestControlOnce();
		REMOVE_DECALS_FROM_VEHICLE(vehicle.Handle());
	}

	for (int i = 0; i < GAME_PLAYERCOUNT; i++)
	{
		if (NETWORK_IS_PLAYER_ACTIVE(i))
		{
			if (DrawOption(GET_PLAYER_NAME(i)))
				add_emblem_to_vehicle(vehicle, GET_PLAYER_PED(i));
		}
	}
}

void ModShopWheelsSubmenu::Draw()
{
	if (!DOES_ENTITY_EXIST(g_Ped4))
	{
		Engine* eng = Engine::Current();
		if (eng) eng->GoBack();
		return;
	}

	Engine* e = Engine::Current();
	if (!e) return;
	DrawTitle();

	GTAvehicle vehicle = g_Ped4;
	Model vehModel = GET_ENTITY_MODEL(g_Ped4);
	const bool isBike = vehModel.IsBike();
	const INT wheel_no = GET_VEHICLE_MOD(g_Ped4, 23);
	const INT ms_custom_tyres = GET_VEHICLE_MOD_VARIATION(g_Ped4, 23);

	sub::lastwheeltype = GET_VEHICLE_WHEEL_TYPE(g_Ped4);
	sub::lastfwheel = GET_VEHICLE_MOD(g_Ped4, VehicleMod::FrontWheels);
	sub::lastbwheel = GET_VEHICLE_MOD(g_Ped4, VehicleMod::BackWheels);

	if (DrawOption(Game::GetGXTEntry("CMOD_MOD_WCL", "Rim Colour")))
	{
		msCurrentPaintIndex = 4;
		if (GET_VEHICLE_MOD(g_Ped4, VehicleMod::FrontWheels) < 0)
			Game::Print::PrintBottomCentre("~b~Note:~s~ Colours cannot always be applied to stock wheels.");
		NavigateTo("vehicle_modshop_paints_shared");
		return;
	}

	if (e->AddCheckbox(Game::GetGXTEntry("CMOD_TYR_0", "Stock Wheels"),
		GET_VEHICLE_MOD(g_Ped4, VehicleMod::FrontWheels) < 0))
	{
		vehicle.RequestControlOnce();
		vehicle.RemoveMod(VehicleMod::FrontWheels);
		vehicle.RemoveMod(VehicleMod::BackWheels);
		return;
	}

	const std::vector<std::string>& wheelNames = sub::GetWheelTypeNames();
	for (size_t i = 0; i < wheelNames.size(); i++)
	{
		const bool ibw = ((int)i == WheelType::BikeWheels);
		if (!((isBike && ibw) || (!isBike && !ibw))) continue;
		if (DrawOption(wheelNames[i]))
		{
			msCurrentPaintIndex = (int)i;
			if ((int)i == WheelType::BikeWheels)
			{
				sub::msWheelsBitBikeBack = true;
				NavigateTo("vehicle_modshop_wheels_2");
			}
			else
			{
				sub::msWheelsBitBikeBack = false;
				bitMSPaintsRGBMode = 0;
				SET_VEHICLE_WHEEL_TYPE(g_Ped4, (int)i);
				sub::msWheelsMaxWindices = GET_NUM_VEHICLE_MODS(g_Ped4, VehicleMod::FrontWheels);
				NavigateTo("vehicle_modshop_wheels_3");
			}
			return;
		}
	}

	if (DrawToggleExternal("CMOD_TYR_1", ms_custom_tyres != 0))
	{
		vehicle.RequestControlOnce();
		vehicle.SetMod(VehicleMod::FrontWheels, wheel_no, !ms_custom_tyres);
		vehicle.SetMod(VehicleMod::BackWheels,  wheel_no, !ms_custom_tyres);
		return;
	}
	if (DrawToggleExternal("CMOD_TYR_2", GET_VEHICLE_TYRES_CAN_BURST(g_Ped4) == FALSE))
	{
		vehicle.RequestControlOnce();
		vehicle.SetCanTyresBurst(!vehicle.GetCanTyresBurst());
		return;
	}
	if (DrawToggleExternal("Drift Tyres", GET_DRIFT_TYRES_SET(g_Ped4) != 0))
	{
		vehicle.RequestControlOnce();
		vehicle.SetCanTyresDrift(!vehicle.GetCanTyresDrift());
		return;
	}
	if (DrawOption("Remove Tires")) NavigateTo("vehicle_modshop_tyres_burst");

	if (DrawOption(Game::GetGXTEntry("CMOD_MOD_TYR3", "Tire Smoke Colour")))
	{
		bitMSPaintsRGBMode = 4;
		NavigateTo("vehicle_modshop_paints_rgb");
		return;
	}
	if (IsCurrentRowSelected())
		e->AddPresetColourOptionsPreview(vehicle.GetTyreSmokeColour());
}

void ModShopWheels2Submenu::Draw()
{
	if (!DOES_ENTITY_EXIST(g_Ped4))
	{
		Engine* eng = Engine::Current();
		if (eng) eng->GoBack();
		return;
	}
	Engine* e = Engine::Current();
	if (!e) return;

	const INT wtype = msCurrentPaintIndex;
	const std::vector<std::string>& wheelNames = sub::GetWheelTypeNames();
	if (wtype >= 0 && wtype < (int)wheelNames.size())
		e->AddTitle(wheelNames[wtype]);
	else
		DrawTitle();

	sub::lastwheeltype = 6;
	sub::lastfwheel = GET_VEHICLE_MOD(g_Ped4, VehicleMod::FrontWheels);
	sub::lastbwheel = GET_VEHICLE_MOD(g_Ped4, VehicleMod::BackWheels);

	if (DrawOption("Front"))
	{
		bitMSPaintsRGBMode = 0;
		SET_VEHICLE_WHEEL_TYPE(g_Ped4, wtype);
		sub::msWheelsMaxWindices = GET_NUM_VEHICLE_MODS(g_Ped4, VehicleMod::FrontWheels);
		NavigateTo("vehicle_modshop_wheels_3");
		return;
	}
	if (DrawOption("Rear"))
	{
		bitMSPaintsRGBMode = 2;
		SET_VEHICLE_WHEEL_TYPE(g_Ped4, wtype);
		sub::msWheelsMaxWindices = GET_NUM_VEHICLE_MODS(g_Ped4, VehicleMod::BackWheels);
		NavigateTo("vehicle_modshop_wheels_3");
		return;
	}
}

void ModShopWheels3Submenu::Draw()
{
	if (!DOES_ENTITY_EXIST(g_Ped4))
	{
		Engine* eng = Engine::Current();
		if (eng) eng->GoBack();
		return;
	}
	Engine* e = Engine::Current();
	if (!e) return;

	const INT wtype = msCurrentPaintIndex;
	const INT chrtype = bitMSPaintsRGBMode;

	if (wtype == WheelType::BikeWheels)
	{
		const bool bIsChromeSelected = (chrtype == 1 || chrtype == 3);
		e->AddTitle(bIsChromeSelected ? "Chrome Wheels" : "Bike Wheels");

		std::array<int, 5> ids{ 0, 13, 26, 48, sub::msWheelsMaxWindices };
		for (size_t j = 0; j < ids.size() - 1; j++)
		{
			for (int i = ids[j]; i < ids[j + 1]; i++)
			{
				AddWheelOption(GetModTextLabel(g_Ped4, VehicleMod::FrontWheels, i, false),
					g_Ped4, (INT8)wtype, (INT16)i, chrtype == 2);
			}
		}
		return;
	}

	const bool bIsChromeSelected = (chrtype == 1 || chrtype == 3);
	e->AddTitle(bIsChromeSelected ? "Chrome Wheels" : "Normal Wheels");

	const int windices2 = sub::msWheelsMaxWindices;
	for (int i = 0; i < windices2; i++)
	{
		AddWheelOption(GetModTextLabel(g_Ped4, VehicleMod::FrontWheels, i, false),
			g_Ped4, (INT8)wtype, (INT16)i, chrtype == 2);
	}
}

void ModShopTyresBurstSubmenu::Draw()
{
	GTAvehicle vehicle = g_Ped4;
	Engine* e = Engine::Current();
	if (!e) return;
	DrawTitle();

	const std::vector<std::string> vCaptions_tyres{ "FrontLeft", "FrontRight", "2", "3", "BackLeft", "BackRight", "6", "7", "8" };
	for (size_t i = 0; i < vCaptions_tyres.size(); i++)
	{
		const bool isBurst = vehicle.IsTyreBursted((int)i);
		if (e->AddCheckbox(vCaptions_tyres[i], isBurst, Checkbox::CROSS, CHECKBOX_NONE))
		{
			if (!isBurst)
			{
				vehicle.RequestControl(800);
				vehicle.SetCanTyresBurst(true);
				vehicle.BurstTyre((int)i);
			}
			else
			{
				// Unburst — preserve other bursted tyres.
				std::vector<int> vTyresBurstedAlready;
				for (size_t j = 0; j < vCaptions_tyres.size(); j++)
				{
					if (j != i && vehicle.IsTyreBursted((int)j))
						vTyresBurstedAlready.push_back((int)j);
				}
				vehicle.RequestControl(600);
				for (size_t j = 0; j < vCaptions_tyres.size(); j++)
					vehicle.FixTyre((int)j);
				vehicle.Repair(false);
				for (int ttb : vTyresBurstedAlready)
					vehicle.BurstTyre(ttb);
			}
		}
	}

	if (DrawToggleExternal("Make All Wheels Invisible", AreVehicleWheelsInvisible(vehicle)))
	{
		vehicle.RequestControl(600);
		if (!AreVehicleWheelsInvisible(vehicle))
		{
			SetVehicleWheelsInvisible(vehicle, true);
		}
		else
		{
			SetVehicleWheelsInvisible(vehicle, false);
			Game::Print::PrintBottomCentre("~b~Note:~s~ It may take a while for the wheels to come back.");
		}
	}
}

void ModShopPaintsSubmenu::Draw()
{
	if (!DOES_ENTITY_EXIST(g_Ped4))
	{
		Engine* eng = Engine::Current();
		if (eng) eng->GoBack();
		return;
	}

	Engine* e = Engine::Current();
	if (!e) return;
	DrawTitle();

	double paintFade = GET_VEHICLE_ENVEFF_SCALE(g_Ped4);
	double dirtLevel = GET_VEHICLE_DIRT_LEVEL(g_Ped4);
	double carvarcol = GET_VEHICLE_COLOUR_COMBINATION(g_Ped4) + 1;
	sub::getpaint = true;

	bool changed = false;
	AddMsPaintsPointOption(*this, Game::GetGXTEntry("CMOD_COL0_0", "Primary"), 1, changed);
	if (changed) { NavigateTo("vehicle_modshop_paints_2"); return; }
	AddMsPaintsPointOption(*this, Game::GetGXTEntry("CMOD_COL0_1", "Secondary"), 2, changed);
	if (changed) { NavigateTo("vehicle_modshop_paints_2"); return; }

	if (DrawOption(Game::GetGXTEntry("CMOD_COL1_6", "Pearlescent")))
	{
		msCurrentPaintIndex = 3;
		NavigateTo("vehicle_modshop_paints_pearl");
		return;
	}
	if (DrawOption(Game::GetGXTEntry("CMOD_MOD_WHEM", "Wheels")))
	{
		msCurrentPaintIndex = 4;
		if (GET_VEHICLE_MOD(g_Ped4, VehicleMod::FrontWheels) < 0)
			Game::Print::PrintBottomCentre("~b~Note:~s~ Colours cannot always be applied to stock wheels.");
		NavigateTo("vehicle_modshop_paints_shared");
		return;
	}
	if (DrawOption("Interior Colour"))
	{
		msCurrentPaintIndex = 5;
		NavigateTo("vehicle_modshop_paints_shared");
		return;
	}
	if (DrawOption("Dashboard Colour"))
	{
		msCurrentPaintIndex = 6;
		NavigateTo("vehicle_modshop_paints_shared");
		return;
	}

	DrawBreak("---Collateral---");

	// Paint Fade
	{
		const InputResult res = e->AddNumber("Paint Fade", paintFade, 2);
		if (res.rightPressed) { paintFade += 0.02; if (paintFade > 1.0) paintFade = 1.0; SET_VEHICLE_ENVEFF_SCALE(g_Ped4, (float)paintFade); }
		else if (res.leftPressed) { paintFade -= 0.02; if (paintFade < 0.0) paintFade = 0.0; SET_VEHICLE_ENVEFF_SCALE(g_Ped4, (float)paintFade); }
	}
	// Dirt Level
	{
		const InputResult res = e->AddNumber("Dirt Level", dirtLevel, 2);
		if (res.rightPressed) { dirtLevel += 0.1; if (dirtLevel > 15.0) dirtLevel = 15.0; SET_VEHICLE_DIRT_LEVEL(g_Ped4, (float)dirtLevel); }
		else if (res.leftPressed) { dirtLevel -= 0.1; if (dirtLevel < 0.0) dirtLevel = 0.0; SET_VEHICLE_DIRT_LEVEL(g_Ped4, (float)dirtLevel); }
	}
	// CarVariation Colours
	{
		const InputResult res = e->AddNumber("CarVariation Colours", carvarcol, 0);
		auto apply = [&](double v)
		{
			carvarcol = v;
			CLEAR_VEHICLE_CUSTOM_PRIMARY_COLOUR(g_Ped4);
			CLEAR_VEHICLE_CUSTOM_SECONDARY_COLOUR(g_Ped4);
			SET_VEHICLE_COLOUR_COMBINATION(g_Ped4, (int)carvarcol - 1);
			if (g_LSCCustoms) RememberLastPaintForVehicle(g_Ped4);
		};
		if (res.rightPressed)
		{
			if (carvarcol < GET_NUMBER_OF_VEHICLE_COLOURS(g_Ped4)) apply(carvarcol + 1);
			else apply(1);
		}
		else if (res.leftPressed)
		{
			if (carvarcol > 1) apply(carvarcol - 1);
			else apply(GET_NUMBER_OF_VEHICLE_COLOURS(g_Ped4));
		}
		else if (res.accepted)
		{
			std::string inputStr = Game::InputBox("", 4, "Enter a CarVariation index:", std::to_string((int)carvarcol));
			if (!inputStr.empty())
			{
				try { apply(std::stoi(inputStr)); }
				catch (...) { Game::Print::PrintErrorInvalidInput(inputStr); }
			}
		}
	}
}

void ModShopPaints2Submenu::Draw()
{
	Engine* e = Engine::Current();
	if (!e) return;

	GTAvehicle vehicle = g_Ped4;
	int paintIndex = sub::getpaintCarUsing_index(g_Ped4, msCurrentPaintIndex);

	e->AddTitle(PaintIndexToTitle(msCurrentPaintIndex, "Paint"));

	if (DrawOption("Chrome"))   NavigateTo("vehicle_modshop_paints_chrome");
	if (DrawOption("Classic"))  NavigateTo("vehicle_modshop_paints_normal");
	if (DrawOption("Matte"))    NavigateTo("vehicle_modshop_paints_matte");
	if (DrawOption("Metallic")) NavigateTo("vehicle_modshop_paints_metallic");
	if (DrawOption("Metal"))    NavigateTo("vehicle_modshop_paints_metal");
	if (g_isEnhanced || IS_DLC_PRESENT(GET_HASH_KEY("spchameleon")))
	{
		if (DrawOption("Chameleon")) NavigateTo("vehicle_modshop_paints_chameleon");
	}
	if (DrawOption("Utility")) NavigateTo("vehicle_modshop_paints_util");
	if (DrawOption("Worn"))    NavigateTo("vehicle_modshop_paints_worn");

	if (msCurrentPaintIndex < 10)
	{
		const InputResult res = e->AddNumber("Paint Index", (double)paintIndex, 0);
		const INT maxVal = sub::GetPaintIndexMaxValue();
		auto applyIdx = [&](int newIdx)
		{
			paintIndex = newIdx;
			sub::paintCarUsing_index(g_Ped4, msCurrentPaintIndex, paintIndex, -1);
			if (g_LSCCustoms) RememberLastPaintForVehicle(g_Ped4);
		};
		if (res.rightPressed) applyIdx(paintIndex < maxVal ? paintIndex + 1 : 0);
		else if (res.leftPressed) applyIdx(paintIndex > 0 ? paintIndex - 1 : maxVal);
		else if (res.accepted)
		{
			std::string inputStr = Game::InputBox("", 4U, "Enter a paint index:", std::to_string(paintIndex));
			if (!inputStr.empty())
			{
				try { applyIdx(std::stoi(inputStr)); }
				catch (...) { Game::Print::PrintErrorInvalidInput(inputStr); }
			}
		}
	}

	if (DrawOption("Random Index"))
	{
		if (vehicle.IsVehicle())
		{
			int randindex = std::rand() % sub::GetPaintIndexMaxValue();
			sub::paintCarUsing_index(g_Ped4, msCurrentPaintIndex, randindex, -1);
			sub::getpaint = true;
		}
		return;
	}

	if (msCurrentPaintIndex == 1 || msCurrentPaintIndex == 2)
	{
		if (DrawOption("Random RGB"))
		{
			if (vehicle.IsVehicle())
			{
				int randr = std::rand() % 255;
				int randg = std::rand() % 255;
				int randb = std::rand() % 255;
				sub::paintCarUsing_index(g_Ped4, msCurrentPaintIndex, 0, -1);
				if (msCurrentPaintIndex == 1) vehicle.SetCustomPrimaryColour(randr, randg, randb);
				else if (msCurrentPaintIndex == 2) vehicle.SetCustomSecondaryColour(randr, randg, randb);
				sub::getpaint = true;
			}
			return;
		}
		if (DrawOption("Set RGB"))
		{
			bitMSPaintsRGBMode = (msCurrentPaintIndex == 1) ? 0 : 1;
			NavigateTo("vehicle_modshop_paints_rgb");
			return;
		}
		if (IsCurrentRowSelected())
		{
			e->AddPresetColourOptionsPreview(msCurrentPaintIndex == 1
				? vehicle.GetCustomPrimaryColour()
				: vehicle.GetCustomSecondaryColour());
		}

		const std::string painttypeswitch = (msCurrentPaintIndex == 1) ? "Secondary" : "Primary";
		if (DrawOption("Copy to " + painttypeswitch))
		{
			sub::paintCarUsing_index(g_Ped4, 3 - msCurrentPaintIndex,
				sub::getpaintCarUsing_index(g_Ped4, msCurrentPaintIndex), -1);
			switch (msCurrentPaintIndex)
			{
			case 1:
				if (GET_IS_VEHICLE_PRIMARY_COLOUR_CUSTOM(g_Ped4))
				{
					RgbS copy = vehicle.GetCustomPrimaryColour();
					vehicle.SetCustomSecondaryColour(copy.R, copy.G, copy.B);
				}
				break;
			case 2:
				if (GET_IS_VEHICLE_SECONDARY_COLOUR_CUSTOM(g_Ped4))
				{
					RgbS copy = vehicle.GetCustomSecondaryColour();
					vehicle.SetCustomPrimaryColour(copy.R, copy.G, copy.B);
				}
				break;
			}
		}
	}
}

void ModShopPaintsSharedSubmenu::Draw()
{
	Engine* e = Engine::Current();
	if (!e) return;

	const std::vector<sub::NamedVehiclePaint>* pPaints = &sub::GetPaintsNormal();

	switch (msCurrentPaintIndex)
	{
	case 1: case 10: e->AddTitle(Game::GetGXTEntry("CMOD_COL0_0", "Primary")); break;
	case 2: case 11: e->AddTitle(Game::GetGXTEntry("CMOD_COL0_1", "Secondary")); break;
	case 3:
		e->AddTitle(Game::GetGXTEntry("CMOD_COL1_6", "Pearlescent"));
		pPaints = &sub::GetPaintsPearl();
		break;
	case 5:
		e->AddTitle("Interior");
		pPaints = &sub::GetPaintsInterior();
		break;
	case 6:
		e->AddTitle("Dashboard");
		pPaints = &sub::GetPaintsDashboard();
		break;
	case 4:
	default:
		e->AddTitle(Game::GetGXTEntry("CMOD_MOD_WHEM", "Wheels"));
		pPaints = &sub::GetPaintsWheels();
		break;
	}

	DrawPaintTable(*pPaints, g_Ped4);
	AddPaintIndexSlider(g_Ped4);
}

void ModShopPaintsChromeSubmenu::Draw()
{
	DrawTitle();
	DrawPaintTable(sub::GetPaintsChrome(), g_Ped4);
}

void ModShopPaintsNormalSubmenu::Draw()
{
	DrawTitle();
	DrawPaintTable(sub::GetPaintsNormal(), g_Ped4);
}

void ModShopPaintsMatteSubmenu::Draw()
{
	DrawTitle();
	DrawPaintTable(sub::GetPaintsMatte(), g_Ped4);
}

void ModShopPaintsMetallicSubmenu::Draw()
{
	DrawTitle();
	DrawPaintTable(sub::GetPaintsMetallic(), g_Ped4);
}

void ModShopPaintsMetalSubmenu::Draw()
{
	DrawTitle();
	DrawPaintTable(sub::GetPaintsMetal(), g_Ped4);
}

void ModShopPaintsChameleonSubmenu::Draw()
{
	DrawTitle();
	DrawPaintTable(sub::GetPaintsChameleon(), g_Ped4);
}

void ModShopPaintsPearlSubmenu::Draw()
{
	DrawTitle();
	DrawPaintTable(sub::GetPaintsPearl(), g_Ped4, /*pearlAsBoth=*/true);
	AddPaintIndexSlider(g_Ped4);
}

void ModShopPaintsUtilSubmenu::Draw()
{
	DrawTitle();
	DrawPaintTable(sub::GetPaintsUtil(), g_Ped4);
}

void ModShopPaintsWornSubmenu::Draw()
{
	DrawTitle();
	DrawPaintTable(sub::GetPaintsWorn(), g_Ped4);
}

void ModShopPaintsRgbSubmenu::Draw()
{
	Engine* e = Engine::Current();
	if (!e) return;
	DrawTitle();

	GTAvehicle vehicle = g_Ped4;

	const std::vector<sub::NamedVehiclePaint> PAINTS_FINISH{
		{ "Standard Metallic", 2, -1 },
		{ "Dark Metallic", 0, -1 },
		{ "Bright Metallic", 111, -1 },
		{ "Matte", 12, -1 },
		{ "Util", 15, -1 },
		{ "Worn", 21, -1 },
		{ "Brushed Metal", 117, -1 },
		{ "Pure Chrome", 120, -1 },
		{ "Coloured Chrome", 158, -1 },
		{ "Satin", 159, -1 },
	};
	const std::vector<std::string> PAINTS_FINISH_NAMES{
		"Standard Metallic","Dark Metallic","Bright Metallic","Matte","Util",
		"Worn","Brushed Metal","Pure Chrome","Coloured Chrome","Satin" };

	int ms_paints_rgb_r = 0;
	int ms_paints_rgb_g = 0;
	int ms_paints_rgb_b = 0;
	int ms_paints_rgb_a = -1;
	int ms_paints_finish = 0;

	if (bitMSPaintsRGBMode == 0 || bitMSPaintsRGBMode == 1)
	{
		switch (sub::getpaintCarUsing_index(g_Ped4, msCurrentPaintIndex))
		{
		case 0:   ms_paints_finish = 1; break;
		case 111: ms_paints_finish = 2; break;
		case 12:  ms_paints_finish = 3; break;
		case 15:  ms_paints_finish = 4; break;
		case 21:  ms_paints_finish = 5; break;
		case 117: ms_paints_finish = 6; break;
		case 120: ms_paints_finish = 7; break;
		case 158: ms_paints_finish = 8; break;
		case 159: ms_paints_finish = 9; break;
		case 2: default: ms_paints_finish = 0; break;
		}
	}

	switch (bitMSPaintsRGBMode)
	{
	case 0: GET_VEHICLE_CUSTOM_PRIMARY_COLOUR(g_Ped4, &ms_paints_rgb_r, &ms_paints_rgb_g, &ms_paints_rgb_b); break;
	case 1: GET_VEHICLE_CUSTOM_SECONDARY_COLOUR(g_Ped4, &ms_paints_rgb_r, &ms_paints_rgb_g, &ms_paints_rgb_b); break;
	case 2: GET_VEHICLE_NEON_COLOUR(g_Ped4, &ms_paints_rgb_r, &ms_paints_rgb_g, &ms_paints_rgb_b); break;
	case 3: ms_paints_rgb_r = g_multiPlatNeonsColor.R; ms_paints_rgb_g = g_multiPlatNeonsColor.G; ms_paints_rgb_b = g_multiPlatNeonsColor.B; break;
	case 4: GET_VEHICLE_TYRE_SMOKE_COLOR(g_Ped4, &ms_paints_rgb_r, &ms_paints_rgb_g, &ms_paints_rgb_b); break;
	case 7: GET_PLAYER_PARACHUTE_SMOKE_TRAIL_COLOR(g_Ped2, &ms_paints_rgb_r, &ms_paints_rgb_g, &ms_paints_rgb_b); break;
	case 9: ms_paints_rgb_r = g_spawnVehicleNeonColor.R; ms_paints_rgb_g = g_spawnVehicleNeonColor.G; ms_paints_rgb_b = g_spawnVehicleNeonColor.B; break;
	case 10:
	{
		int inull_local = 0;
		GET_HUD_COLOUR(g_Ped4, &ms_paints_rgb_r, &ms_paints_rgb_g, &ms_paints_rgb_b, &inull_local);
		ms_paints_rgb_a = inull_local;
		break;
	}
	}

	if (bitMSPaintsRGBMode == 0 || bitMSPaintsRGBMode == 1)
	{
		const InputResult res = e->AddTextList("Paint Finish", ms_paints_finish, PAINTS_FINISH_NAMES);
		auto applyFinish = [&]()
		{
			switch (msCurrentPaintIndex)
			{
			case 1:
			{
				RgbS copy = vehicle.GetCustomPrimaryColour();
				sub::paintCarUsing_index(g_Ped4, msCurrentPaintIndex, PAINTS_FINISH[ms_paints_finish].paint, -1);
				vehicle.SetCustomPrimaryColour(copy.R, copy.G, copy.B);
				break;
			}
			case 2:
			{
				RgbS copy = vehicle.GetCustomSecondaryColour();
				sub::paintCarUsing_index(g_Ped4, msCurrentPaintIndex, PAINTS_FINISH[ms_paints_finish].paint, -1);
				vehicle.SetCustomSecondaryColour(copy.R, copy.G, copy.B);
				break;
			}
			}
		};
		if (res.rightPressed)
		{
			ms_paints_finish = (ms_paints_finish < 9) ? ms_paints_finish + 1 : 0;
			applyFinish();
		}
		else if (res.leftPressed)
		{
			ms_paints_finish = (ms_paints_finish > 0) ? ms_paints_finish - 1 : 9;
			applyFinish();
		}
	}

	auto commit = [&]()
	{
		sub::rgb_mode_set_carcol(g_Ped4, (INT16)ms_paints_rgb_r, (INT16)ms_paints_rgb_g, (INT16)ms_paints_rgb_b, (INT16)ms_paints_rgb_a);
	};

	auto channelRow = [&](const char* label, int& channel)
	{
		const InputResult res = e->AddNumber(label, (double)channel, 0);
		if (res.rightPressed) { BumpRgbChannel(channel, +1); commit(); }
		else if (res.leftPressed) { BumpRgbChannel(channel, -1); commit(); }
		else if (res.accepted)
		{
			std::string inputStr = Game::InputBox("", 4U, "", std::to_string(channel));
			if (!inputStr.empty())
			{
				try
				{
					int newVal = std::abs(std::stoi(inputStr));
					if (newVal < 0 || newVal > 255)
						Game::Print::PrintErrorInvalidInput(inputStr);
					else { channel = newVal; commit(); }
				}
				catch (...) { Game::Print::PrintErrorInvalidInput(inputStr); }
			}
		}
	};

	channelRow("Red", ms_paints_rgb_r);
	if (IsCurrentRowSelected())
		e->AddPresetColourOptionsPreview((unsigned char)ms_paints_rgb_r, (unsigned char)ms_paints_rgb_g, (unsigned char)ms_paints_rgb_b);
	channelRow("Green", ms_paints_rgb_g);
	channelRow("Blue",  ms_paints_rgb_b);
	if (ms_paints_rgb_a != -1) channelRow("Opacity", ms_paints_rgb_a);

	{
		int hudIdx = sub::settingsHUDColor;
		const InputResult res = e->AddTextList("HUD Colour", hudIdx, HudColour::vHudColours);
		auto applyHud = [&]()
		{
			int inull_local = 0;
			GET_HUD_COLOUR(sub::settingsHUDColor, &ms_paints_rgb_r, &ms_paints_rgb_g, &ms_paints_rgb_b, &inull_local);
			commit();
		};
		if (res.rightPressed)
		{
			if (sub::settingsHUDColor < HudColour::vHudColours.size() - 1) sub::settingsHUDColor++;
			else sub::settingsHUDColor = 0;
		}
		else if (res.leftPressed)
		{
			if (sub::settingsHUDColor > 0) sub::settingsHUDColor--;
			else sub::settingsHUDColor = 180;
		}
		else if (res.accepted)
		{
			applyHud();
		}
	}

	// Hex input
	if (DrawOption("~b~Input~s~ Hex Code"))
	{
		std::string hexr = IntToHexString(ms_paints_rgb_r, false);
		std::string hexg = IntToHexString(ms_paints_rgb_g, false);
		std::string hexb = IntToHexString(ms_paints_rgb_b, false);
		if (hexr.length() == 1) hexr = "0" + hexr;
		if (hexg.length() == 1) hexg = "0" + hexg;
		if (hexb.length() == 1) hexb = "0" + hexb;
		std::string titlestring = hexr + hexg + hexb;
		std::string inputStr = Game::InputBox("", 6U, "", titlestring);
		if (inputStr.length() == 6)
		{
			try
			{
				if (inputStr.find_first_not_of("0123456789abcdef") == std::string::npos)
				{
					ms_paints_rgb_r = (int)std::stoul(inputStr.substr(0, 2), nullptr, 16);
					ms_paints_rgb_g = (int)std::stoul(inputStr.substr(2, 2), nullptr, 16);
					ms_paints_rgb_b = (int)std::stoul(inputStr.substr(4, 2), nullptr, 16);
					sub::rgb_mode_set_carcol(g_Ped4, (INT16)ms_paints_rgb_r, (INT16)ms_paints_rgb_g, (INT16)ms_paints_rgb_b, 255);
				}
				else
				{
					Game::Print::PrintErrorInvalidInput(inputStr);
				}
			}
			catch (...) { Game::Print::PrintErrorInvalidInput(inputStr); }
		}
		else
		{
			Game::Print::PrintErrorInvalidInput(inputStr);
		}
	}

	DrawBreak("---Presets---");
	if (e->AddPresetColourOptions(ms_paints_rgb_r, ms_paints_rgb_g, ms_paints_rgb_b))
		commit();
}

void ModShopWindowsSubmenu::Draw()
{
	GTAvehicle vehicle = g_Ped4;
	if (!vehicle.Exists())
	{
		Engine* eng = Engine::Current();
		if (eng) eng->GoBack();
		return;
	}

	Engine* e = Engine::Current();
	if (!e) return;
	DrawTitle();

	const std::vector<std::string>& tintNames = sub::GetMsWindowsWinTintNames();
	int ms_wintint = vehicle.GetWindowTint();
	if (ms_wintint < 0 || ms_wintint >= (int)tintNames.size()) ms_wintint = 0;

	const InputResult tintRes = e->AddTextList("CMOD_GLD2_2", ms_wintint, tintNames, /*gxt=*/true);
	if (tintRes.rightPressed) { if (ms_wintint < (int)tintNames.size() - 1) ms_wintint++; vehicle.SetWindowTint(ms_wintint); }
	if (tintRes.leftPressed)  { if (ms_wintint > 0) ms_wintint--; vehicle.SetWindowTint(ms_wintint); }

	DrawBreak("---Status---");

	const std::vector<std::string>& modeNames   = sub::GetMsWindowsModeNames();
	const std::vector<std::string>& windowNames = sub::GetMsWindowsWindowNames();

	int modeIdx = sub::msWindowsMode;
	const InputResult modeRes = e->AddTextList("Action", modeIdx, modeNames);
	if (modeRes.rightPressed) { if (sub::msWindowsMode < (UINT8)modeNames.size() - 1) sub::msWindowsMode++; }
	if (modeRes.leftPressed)  { if (sub::msWindowsMode > 0) sub::msWindowsMode--; }

	for (size_t i = 0; i < windowNames.size(); i++)
	{
		if (DrawOption(windowNames[i]))
			sub::MsWindowsDoWindow(vehicle, VehicleWindow((int)i), sub::msWindowsMode);
	}
	if (DrawOption("All Windows"))
	{
		for (size_t i = 0; i < windowNames.size(); i++)
			sub::MsWindowsDoWindow(vehicle, VehicleWindow((int)i), sub::msWindowsMode);
	}
}

void ModShopEngineSoundSubmenu::Draw()
{
	if (!DOES_ENTITY_EXIST(g_Ped4))
	{
		Engine* eng = Engine::Current();
		if (eng) eng->GoBack();
		return;
	}
	DrawTitle();

	if (DrawOption("Input Model"))
	{
		std::string inputStr = Game::InputBox("", 28U, "Enter vehicle model name:");
		if (inputStr.length())
		{
			if (!Model(inputStr).IsVehicle())
				Game::Print::PrintErrorInvalidInput(inputStr);
			else
			{
				GTAvehicle vehicle = g_Ped4;
				vehicle.RequestControl(300);
				SetVehicleEngineSoundName(vehicle, inputStr);
			}
		}
	}
}

void ModShopLightsSubmenu::Draw()
{
	GTAvehicle vehicle = g_Ped4;
	Engine* e = Engine::Current();
	if (!e) return;
	DrawTitle();

	if (e->AddCheckbox("Headlights", vehicle.GetLightsOn(), Checkbox::BOXTICK, Checkbox::BOXBLANK))
		vehicle.SetLightsOn(!vehicle.GetLightsOn());

	if (e->AddCheckbox("Left Indicator", sub::msLightsLeftInd, Checkbox::BOXTICK, Checkbox::BOXBLANK))
	{
		sub::msLightsRightInd = false;
		sub::msLightsHazard = false;
		vehicle.SetLeftIndicatorLightOn(!sub::msLightsLeftInd);
		vehicle.SetRightIndicatorLightOn(false);
		sub::msLightsLeftInd = !sub::msLightsLeftInd;
	}

	if (e->AddCheckbox("Right Indicator", sub::msLightsRightInd, Checkbox::BOXTICK, Checkbox::BOXBLANK))
	{
		sub::msLightsLeftInd = false;
		sub::msLightsHazard = false;
		vehicle.SetRightIndicatorLightOn(!sub::msLightsRightInd);
		vehicle.SetLeftIndicatorLightOn(false);
		sub::msLightsRightInd = !sub::msLightsRightInd;
	}

	if (e->AddCheckbox("Hazard Lights", sub::msLightsHazard, Checkbox::BOXTICK, Checkbox::BOXBLANK))
	{
		if (sub::msLightsRightInd == sub::msLightsLeftInd)
		{
			sub::msLightsRightInd = false;
			sub::msLightsLeftInd = false;
			vehicle.SetRightIndicatorLightOn(!sub::msLightsHazard);
			vehicle.SetLeftIndicatorLightOn(!sub::msLightsHazard);
			sub::msLightsHazard = !sub::msLightsHazard;
		}
		else
		{
			sub::msLightsRightInd = false;
			sub::msLightsLeftInd = false;
			vehicle.SetRightIndicatorLightOn(true);
			vehicle.SetLeftIndicatorLightOn(true);
			sub::msLightsHazard = true;
		}
	}
}

void ModShopDoorsSubmenu::Draw()
{
	if (!DOES_ENTITY_EXIST(g_Ped4))
	{
		Engine* eng = Engine::Current();
		if (eng) eng->GoBack();
		return;
	}

	Engine* e = Engine::Current();
	if (!e) return;
	GTAvehicle vehicle = g_Ped4;
	DrawTitle();

	const std::vector<std::string> vDoorLockNames{
		"None","Unlocked","Locked","LockedForPlayer","ChildLock","Unknown 5",
		"Unknown 6","CanBeBrokenInto","CanBeBrokenIntoPersist","Unknown 9",
		"CannotBeTriedToBeEntered" };
	int lockStatus = (int)vehicle.GetLockStatus();
	{
		const InputResult res = e->AddTextList("Lock Status", lockStatus, vDoorLockNames);
		if (res.rightPressed && lockStatus < (int)vDoorLockNames.size() - 1)
		{
			lockStatus++;
			vehicle.SetLockStatus(static_cast<VehicleLockStatus>(lockStatus));
		}
		else if (res.leftPressed && lockStatus > 0)
		{
			lockStatus--;
			vehicle.SetLockStatus(static_cast<VehicleLockStatus>(lockStatus));
		}
	}

	auto& action = sub::msDoorsActionIndex;
	const std::vector<std::string> vActionNames{
		"Open/Close", "", "Close", "", "Remove", "", "Fix", "" };
	int actionIdx = action;
	{
		const InputResult res = e->AddTextList("Action", actionIdx, vActionNames);
		if (res.rightPressed)
		{
			if (action < (UINT8)vActionNames.size() - 1) action++;
			while (!vActionNames[action].length())
			{
				action++;
				if (action >= (UINT8)vActionNames.size()) { action = 0; }
			}
		}
		else if (res.leftPressed)
		{
			if (action > 0) action--;
			while (!vActionNames[action].length())
			{
				if (action == 0) { action = (UINT8)(vActionNames.size() - 1); break; }
				action--;
			}
		}
	}

	switch (action)
	{
	case 0: case 2: case 4: case 6:
		AddMsDoorsOption("Driver Door",    vehicle, VehicleDoor::FrontLeftDoor,  action, action == 4, false);
		AddMsDoorsOption("Passenger Door", vehicle, VehicleDoor::FrontRightDoor, action, action == 4, false);
		AddMsDoorsOption("Rear Left",      vehicle, VehicleDoor::BackLeftDoor,   action, action == 4, false);
		AddMsDoorsOption("Rear Right",     vehicle, VehicleDoor::BackRightDoor,  action, action == 4, false);
		AddMsDoorsOption("Hood",           vehicle, VehicleDoor::Hood,           action, action == 4, false);
		AddMsDoorsOption("Trunk",          vehicle, VehicleDoor::Trunk,          action, action == 4, false);
		AddMsDoorsOption("All",            vehicle, VehicleDoor::Hood,           action + 1, action == 4, false);
		break;
	}
}

void ModShopNeonsSubmenu::Draw()
{
	if (!DOES_ENTITY_EXIST(g_Ped4))
	{
		Engine* eng = Engine::Current();
		if (eng) eng->GoBack();
		return;
	}

	Engine* e = Engine::Current();
	if (!e) return;
	GTAvehicle vehicle = g_Ped4;
	DrawTitle();

	const std::vector<std::string> NEON_FLASH{ "None","Simple","Spin","SpinBack","Firework" };
	const std::vector<std::string> NEON_FADE { "None","Simple","Heartbeat","Shift","Slide" };

	struct NeonRow { VehicleNeonLight light; const char* label; };
	const NeonRow neonRows[] = {
		{ VehicleNeonLight::Left,  "Left"  },
		{ VehicleNeonLight::Right, "Right" },
		{ VehicleNeonLight::Front, "Front" },
		{ VehicleNeonLight::Back,  "Back"  },
	};
	for (const NeonRow& r : neonRows)
	{
		const int idx = static_cast<int>(r.light);
		if (e->AddCheckbox(r.label, neonstate[idx], CHECKBOX_CARTHING, CHECKBOX_NONE))
		{
			vehicle.RequestControl(300);
			const bool newState = !neonstate[idx];
			vehicle.SetNeonLightOn(r.light, newState);
			neonstate[idx] = newState;
		}
	}

	if (DrawOption("Set Colour"))
	{
		bitMSPaintsRGBMode = 2;
		NavigateTo("vehicle_modshop_paints_rgb");
		return;
	}
	if (IsCurrentRowSelected())
		e->AddPresetColourOptionsPreview(vehicle.GetNeonLightsColour());

	DrawToggle("Neon RGB", loop_neon_rgb);

	{
		int fadeIdx = loop_neon_fade;
		const InputResult res = e->AddTextList("Neon Fade", fadeIdx, NEON_FADE);
		if (res.rightPressed)
		{
			if (loop_neon_fade == (int)NEON_FADE.size() - 1) loop_neon_fade = 0;
			else loop_neon_fade++;
		}
		else if (res.leftPressed)
		{
			if (loop_neon_fade == 0) loop_neon_fade = (int)NEON_FADE.size() - 1;
			else loop_neon_fade--;
		}
	}
	{
		int flashIdx = loop_neon_flash;
		const InputResult res = e->AddTextList("Neon Flash", flashIdx, NEON_FLASH);
		if (res.rightPressed)
		{
			if (loop_neon_flash == (int)NEON_FLASH.size() - 1) loop_neon_flash = 0;
			else loop_neon_flash++;
		}
		else if (res.leftPressed)
		{
			if (loop_neon_flash == 0) loop_neon_flash = (int)NEON_FLASH.size() - 1;
			else loop_neon_flash--;
		}
	}

	if (loop_neon_flash > 0 || loop_neon_fade > 0)
	{
		const InputResult res = e->AddNumber("Animation Speed (ms)", (double)loop_neon_delay, 0);
		if (res.rightPressed && loop_neon_delay < 10000) loop_neon_delay += 50;
		else if (res.leftPressed && loop_neon_delay > 50) loop_neon_delay -= 50;
		else if (res.accepted)
		{
			std::string inputStr = Game::InputBox("", 4U, "", std::to_string(loop_neon_delay));
			if (!inputStr.empty())
			{
				try
				{
					int v = std::stoi(inputStr);
					if (v >= 50 && v <= 10000) loop_neon_delay = v;
				}
				catch (...) { Game::Print::PrintErrorInvalidInput(inputStr); }
			}
		}
	}
}

void ModShopExtraSubmenu::Draw()
{
	if (!DOES_ENTITY_EXIST(g_Ped4))
	{
		Engine* eng = Engine::Current();
		if (eng) eng->GoBack();
		return;
	}

	Engine* e = Engine::Current();
	if (!e) return;
	DrawTitle();

	GTAvehicle thisVehicle = g_Ped4;
	for (UINT8 i = 0; i <= 60; i++)
	{
		if (!thisVehicle.DoesExtraExist(i)) continue;
		if (e->AddCheckbox("Extra " + std::to_string(i), thisVehicle.GetExtraOn(i), CHECKBOX_CARTHING, CHECKBOX_NONE))
		{
			thisVehicle.SetExtraOn(i, !thisVehicle.GetExtraOn(i));
		}
	}
}

}
REGISTER_SUBMENU(::Menu::ModShopSubmenu)
REGISTER_SUBMENU(::Menu::ModShopBennysSubmenu)
REGISTER_SUBMENU(::Menu::ModShopEmblemSubmenu)
REGISTER_SUBMENU(::Menu::ModShopWheelsSubmenu)
REGISTER_SUBMENU(::Menu::ModShopWheels2Submenu)
REGISTER_SUBMENU(::Menu::ModShopWheels3Submenu)
REGISTER_SUBMENU(::Menu::ModShopTyresBurstSubmenu)
REGISTER_SUBMENU(::Menu::ModShopPaintsSubmenu)
REGISTER_SUBMENU(::Menu::ModShopPaints2Submenu)
REGISTER_SUBMENU(::Menu::ModShopPaintsSharedSubmenu)
REGISTER_SUBMENU(::Menu::ModShopPaintsChromeSubmenu)
REGISTER_SUBMENU(::Menu::ModShopPaintsMatteSubmenu)
REGISTER_SUBMENU(::Menu::ModShopPaintsNormalSubmenu)
REGISTER_SUBMENU(::Menu::ModShopPaintsMetallicSubmenu)
REGISTER_SUBMENU(::Menu::ModShopPaintsMetalSubmenu)
REGISTER_SUBMENU(::Menu::ModShopPaintsChameleonSubmenu)
REGISTER_SUBMENU(::Menu::ModShopPaintsPearlSubmenu)
REGISTER_SUBMENU(::Menu::ModShopPaintsUtilSubmenu)
REGISTER_SUBMENU(::Menu::ModShopPaintsWornSubmenu)
REGISTER_SUBMENU(::Menu::ModShopPaintsRgbSubmenu)
REGISTER_SUBMENU(::Menu::ModShopCatAllSubmenu)
REGISTER_SUBMENU(::Menu::ModShopWindowsSubmenu)
REGISTER_SUBMENU(::Menu::ModShopEngineSoundSubmenu)
REGISTER_SUBMENU(::Menu::ModShopLightsSubmenu)
REGISTER_SUBMENU(::Menu::ModShopDoorsSubmenu)
REGISTER_SUBMENU(::Menu::ModShopNeonsSubmenu)
REGISTER_SUBMENU(::Menu::ModShopExtraSubmenu)
