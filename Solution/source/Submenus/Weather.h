#pragma once

#include "../Menu/Submenu.h"

#include <array>
#include <string>

namespace Menu {

class WeatherSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "weather"; }
	const char* Title() const override { return "Weather"; }
	void Draw() override;
};

class CloudSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "weather_clouds"; }
	const char* Title() const override { return "Clouds"; }
	void Draw() override;

private:
	static const std::array<std::string, 20> kCloudNames;
};

}

extern float currentTimecycleStrength;
extern float g_rainFXIntensity;