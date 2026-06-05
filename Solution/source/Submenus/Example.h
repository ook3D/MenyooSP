#pragma once

#include "../Menu/Submenu.h"

namespace Menu { namespace Example {

class ExampleSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "example_v2"; }
	const char* Title() const override { return "Example"; }

	void Draw() override;
	void Reset() override;

private:
	bool isFeatureEnabled = false;
	bool useFancyMode = false;
	float intensity = 0.5f;
	int mode = 0;
	int actionCount = 0;
};

}}
