#include "Example.h"

#include "../Menu/SubmenuRegistry.h"

namespace Menu { namespace Example {

void ExampleSubmenu::Draw()
{
	DrawTitle();

	DrawToggle("Enable Feature", isFeatureEnabled);
	DrawCheckbox("Use Fancy Mode", useFancyMode);

	DrawBreak("---Tuning---");

	DrawNumber("Intensity", intensity, 0.05f, 2, 0.0f, 1.0f);
	DrawTextList("Mode", mode, std::vector<std::string>{ "Off", "Low", "Medium", "High" });

	DrawBreak("---Actions---");

	if (DrawOption("Do Action"))
	{
		++actionCount;
	}

	DrawNumber("Action Count", actionCount, 0, 0, 0);

	if (DrawOption("Reset to Defaults"))
	{
		Reset();
	}
}

void ExampleSubmenu::Reset()
{
	isFeatureEnabled = false;
	useFancyMode = false;
	intensity = 0.5f;
	mode = 0;
	actionCount = 0;
}

}}

REGISTER_SUBMENU(::Menu::Example::ExampleSubmenu)
