#include "Weather.h"

#include "../Menu/SubmenuRegistry.h"
#include "../Menu/Menu.h"      // Checkbox
#include "../Menu/Routine.h"   // g_rainFXIntensity

#include "../Natives/natives2.h"
#include "../Memory/GTAmemory.h"
#include "../Scripting/World.h"
#include "../Util/FileLogger.h"

namespace Menu {

void WeatherSubmenu::Draw()
{
	DrawTitle();

	if (DrawSelectionItem("Reset Weather", true, Checkbox::CROSS, Checkbox::NONE))
	{
		World::ClearWeatherOverride();
	}

	for (const auto& weatherName : World::sWeatherNames)
	{
		const bool isCurrent = (GET_PREV_WEATHER_TYPE_HASH_NAME() == GET_HASH_KEY(weatherName.second));
		if (DrawSelectionItem(weatherName.first, isCurrent))
		{
			addlog(ige::LogType::LOG_DEBUG, "Setting weather to " + weatherName.first);
			World::SetWeatherOverride(weatherName.second);
		}
	}

	if (DrawToggleExternal("Snow On Terrain", g_spSnow.IsSnow()))
	{
		g_spSnow.ToggleSnow(!g_spSnow.IsSnow());
	}

	float windSpeed = GET_WIND_SPEED();
	if (DrawNumber("Wind Speed", windSpeed, 0.1f, 2))
	{
		SET_WIND_SPEED(windSpeed);
	}

	float wavesHeight = GET_DEEP_OCEAN_SCALER();
	if (DrawNumber("Ocean Wave Strength", wavesHeight, 0.1f, 2))
	{
		WATER::SET_DEEP_OCEAN_SCALER(wavesHeight);
	}

	if (DrawNumber("Rain Puddles Multiplier", g_rainFXIntensity, 0.1f, 2, 0.0f, 45.0f))
	{
		SET_RAIN(g_rainFXIntensity);
	}

	float gravityMultiplier = GTAmemory::GetWorldGravity();
	if (DrawNumber("Gravity", gravityMultiplier, 0.1f, 2))
	{
		GTAmemory::SetWorldGravity(gravityMultiplier);
	}

	if (DrawOption("Clouds"))                       NavigateTo("weather_clouds");
	if (DrawOption("Water Hack (For Waves At Beaches)")) NavigateTo("misc_water_hack");
}

const std::array<std::string, 20> CloudSubmenu::kCloudNames
{ {
	"Altostratus", "Cirrocumulus", "Cirrus", "Clear 01", "Cloudy 01",
	"Contrails", "Horizonband1", "Horizonband2", "Horizonband3", "Horsey",
	"Nimbus", "NoClouds", "Puffs", "Rain", "Shower",
	"Snowy 01", "Stormy 01", "Stratoscumulus", "Stripey", "Wispy"
} };

void CloudSubmenu::Draw()
{
	DrawTitle();

	if (DrawSelectionItem("Reset", true, Checkbox::CROSS, Checkbox::NONE))
	{
		UNLOAD_ALL_CLOUD_HATS();
	}

	for (const std::string& name : kCloudNames)
	{
		if (DrawOption(name))
		{
			addlog(ige::LogType::LOG_DEBUG, "Set Clouds: " + name);
			LOAD_CLOUD_HAT(name.c_str(), 0.5f);
		}
	}
}

}
REGISTER_SUBMENU(::Menu::WeatherSubmenu)
REGISTER_SUBMENU(::Menu::CloudSubmenu)

float currentTimecycleStrength = 1.0f;
float g_rainFXIntensity = 0.0f;
