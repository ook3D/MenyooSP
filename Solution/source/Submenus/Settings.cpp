#include "Settings.h"

#include "../Menu/Engine.h"
#include "../Menu/SubmenuRegistry.h"
#include "../Menu/Menu.h"           // titlebox, BG, titletext, font_*, etc.
#include "../Menu/MenuConfig.h"
#include "../Menu/Language.h"
#include "../Menu/Routine.h"        // checkSelfDeathModel, menuPos
#include "PlayerRuntime.h"          // checkSelfDeathModel
#include "Neons.h"                 // rainbowBoxes

#include "../Natives/natives2.h"
#include "../Natives/types.h"
#include "../Scripting/enums.h"     // HudColour, GTAfont
#include "../Scripting/Game.h"

#include "SettingsRuntime.h"

#include <cstdlib>
#include <string>
#include <vector>

namespace Menu {

namespace
{
	void MirrorRgbaToActiveTheme(const RGBA* dst, const RGBA& value)
	{
		Engine* engine = Engine::Current();
		if (!engine) return;

		if (dst == &titlebox)               engine->theme.colorTitleBox = value;
		else if (dst == &BG)                engine->theme.colorBackground = value;
		else if (dst == &titletext)         engine->theme.colorTitleText = value;
		else if (dst == &optiontext)        engine->theme.colorOption = value;
		else if (dst == &selectedtext)      engine->theme.colorSelected = value;
		else if (dst == &optionbreaks)      engine->theme.colorBreak = value;
		else if (dst == &optioncount)       engine->theme.colorOptionCount = value;
		else if (dst == &selectionhi)       engine->theme.colorSelectionHighlight = value;
	}

	void MirrorFontToActiveTheme(const INT8* dst, INT8 value)
	{
		Engine* engine = Engine::Current();
		if (!engine) return;

		if (dst == &font_title)          engine->theme.fontTitle = value;
		else if (dst == &font_options)   engine->theme.fontOptions = value;
		else if (dst == &font_selection) engine->theme.fontSelection = value;
		else if (dst == &font_breaks)    engine->theme.fontBreaks = value;
		// font_hud / font_speedo are not part of Menu's Theme.
	}
}

void SettingsSubmenu::Draw()
{
	DrawTitle();

	Engine* engine = Engine::Current();
	if (engine)
	{
		const std::vector<std::string> langEntry{ Language::GetSelectedLangTitle() };
		const ::Menu::InputResult res = engine->AddTextList("Language", 0, langEntry);
		if (res.accepted)
			NavigateTo("settings_language");
	}

	if (DrawOption("Themes"))         NavigateTo("settings_themes");
	if (DrawOption("Menu Colours"))   NavigateTo("settings_colours");
	if (DrawOption("Menu Fonts"))     NavigateTo("settings_fonts");
	if (DrawOption("Menu Position"))  NavigateTo("settings_menupos");

	DrawToggle("Mouse Support", Menu::bit_mouse);
	DrawToggle("Gradients", Menu::gradients);
	DrawToggle("Titlebox Globe", Menu::bit_glare_test);
	DrawToggle("Centre Title", Menu::bit_centre_title);
	DrawToggle("Centre Options", Menu::bit_centre_options);
	DrawToggle("Centre Breaks", Menu::bit_centre_breaks);
	DrawToggle("Reset Player Model Upon Death (SP)", checkSelfDeathModel);

	// Sync flag — flipping it (either direction) saves the config.
	if (DrawToggle("Sync Menyoo With Config File", MenuConfig::bSaveAtIntervals))
	{
		MenuConfig::SaveConfig();
	}

	if (DrawOption("Reset Toggles (Most Of Them)"))
	{
		MenuConfig::ConfigResetHaxValues();
	}
}

void SettingsMenuPosSubmenu::Draw()
{
	DrawTitle();

	Engine* engine = Engine::Current();
	if (!engine) return;

	{
		const double displayX = static_cast<double>(menuPos.x) * 100.0 + 6.0;
		const ::Menu::InputResult res = engine->AddNumber("Offset X", displayX, 2);
		if (res.rightPressed)
		{
			if (menuPos.x < 0.7397f) menuPos.x += 0.002f;
		}
		else if (res.leftPressed)
		{
			if (menuPos.x > -0.0598f) menuPos.x -= 0.002f;
			if (menuPos.x < -0.0598f) menuPos.x = -0.0598f;
		}
	}

	{
		const double displayY = static_cast<double>(menuPos.y) * 100.0 + 7.4;
		const ::Menu::InputResult res = engine->AddNumber("Offset Y", displayY, 2);
		if (res.rightPressed)
		{
			if (menuPos.y < 0.85f) menuPos.y += 0.002f;
		}
		else if (res.leftPressed)
		{
			if (menuPos.y > -0.074f) menuPos.y -= 0.002f;
			if (menuPos.y < -0.074f) menuPos.y = -0.074f;
		}
	}

	engine->theme.position = menuPos;
}

void SettingsThemesSubmenu::Draw()
{
	DrawTitle();

	const int count = sub::SettingsThemeCount();
	for (int i = 0; i < count; ++i)
	{
		const bool isActive = sub::SettingsThemeIsActive(i);
		if (DrawSelectionItem(sub::SettingsThemeName(i), isActive,
			Checkbox::MAKEUPTHING, Checkbox::NONE))
		{
			sub::SettingsThemeApply(i);

			Engine* engine = Engine::Current();
			if (engine)
			{
				engine->theme.colorTitleBox = titlebox;
				engine->theme.colorBackground = BG;
				engine->theme.colorTitleText = titletext;
				engine->theme.colorOption = optiontext;
				engine->theme.colorSelected = selectedtext;
				engine->theme.colorBreak = optionbreaks;
				engine->theme.colorOptionCount = optioncount;
				engine->theme.colorSelectionHighlight = selectionhi;
				engine->theme.fontTitle = font_title;
				engine->theme.fontOptions = font_options;
				engine->theme.fontSelection = font_selection;
				engine->theme.fontBreaks = font_breaks;
				engine->theme.useGradients = Menu::gradients;
			}
		}
	}
}

void SettingsColoursSubmenu::Draw()
{
	DrawTitle();

	auto pickerRow = [this](const std::string& label, RGBA& feature)
	{
		const bool pressed = DrawOption(label);

		Engine* engine = Engine::Current();
		if (engine && IsCurrentRowSelected())
		{
			engine->AddPresetColourOptionsPreview(
				static_cast<unsigned char>(feature.R),
				static_cast<unsigned char>(feature.G),
				static_cast<unsigned char>(feature.B));
		}

		if (pressed)
		{
			sub::g_settingsRGBA = &feature;
			NavigateTo("settings_colours2");
		}
	};

	pickerRow("Title Box", titlebox);
	pickerRow("Background", BG);
	pickerRow("Title Text", titletext);
	pickerRow("Option Text", optiontext);
	pickerRow("Selected Text", selectedtext);
	pickerRow("Option Breaks", optionbreaks);
	pickerRow("Option Count", optioncount);
	pickerRow("Selection Box", selectionhi);
	pickerRow("Ped Trackers", _globalPedTrackers_Col);
	DrawToggle("Rainbow", rainbowBoxes);
}

void SettingsColours2Submenu::Draw()
{
	DrawTitle();

	RGBA* target = sub::g_settingsRGBA;
	if (!target)
	{
		Engine* engine = Engine::Current();
		if (engine) engine->GoBack();
		return;
	}

	auto channelRow = [this](const std::string& label, int& channel) -> bool
	{
		bool mutated = false;

		if (DrawNumber(label, channel, 1, 0, 255))
		{
			mutated = true;
		}

		if (IsCurrentRowSelected()
			&& MenuPressTimer::IsButtonTapped(MenuPressTimer::Button::Accept))
		{
			const std::string current = std::to_string(channel);
			const std::string inputStr = Game::InputBox(current, 4U, "", current);
			if (!inputStr.empty())
			{
				try
				{
					int parsed = std::abs(std::stoi(inputStr));
					if (parsed >= 0 && parsed <= 255)
					{
						channel = parsed;
						mutated = true;
					}
					else
					{
						Game::Print::PrintErrorInvalidInput(inputStr);
					}
				}
				catch (...)
				{
					Game::Print::PrintErrorInvalidInput(inputStr);
				}
			}
		}

		return mutated;
	};

	bool changed = false;
	if (channelRow("Red", target->R))     changed = true;
	if (channelRow("Green", target->G))   changed = true;
	if (channelRow("Blue", target->B))    changed = true;
	if (channelRow("Opacity", target->A)) changed = true;

	{
		Engine* engine = Engine::Current();
		if (engine)
		{
			int idx = sub::settingsHUDColor;
			const ::Menu::InputResult res = engine->AddTextList(
				"HUD Colour", idx, HudColour::vHudColours);
			if (res.rightPressed)
			{
				if (sub::settingsHUDColor < HudColour::vHudColours.size() - 1)
					sub::settingsHUDColor++;
				else
					sub::settingsHUDColor = 0;
			}
			else if (res.leftPressed)
			{
				if (sub::settingsHUDColor > 0)
					sub::settingsHUDColor--;
				else
					sub::settingsHUDColor = 180;
			}
			if (res.accepted)
			{
				int inull = 0;
				GET_HUD_COLOUR(sub::settingsHUDColor,
					&target->R, &target->G, &target->B, &inull);
				changed = true;
			}
		}
	}

	DrawBreak("---Presets---");

	{
		Engine* engine = Engine::Current();
		if (engine)
		{
			if (engine->AddPresetColourOptions(target->R, target->G, target->B))
			{
				changed = true;
			}
		}
	}

	if (changed)
		MirrorRgbaToActiveTheme(target, *target);
}

void SettingsFontsSubmenu::Draw()
{
	DrawTitle();

	auto pickerRow = [this](const std::string& label, INT8& feature)
	{
		if (DrawOption(label))
		{
			sub::g_settingsFont = &feature;
			NavigateTo("settings_fonts2");
		}
	};

	pickerRow("Title", font_title);
	pickerRow("Options", font_options);
	pickerRow("Selected Option", font_selection);
	pickerRow("Option Breaks", font_breaks);
	pickerRow("HUD Font", font_hud);
	pickerRow("Speedo Text", font_speedo);
}

void SettingsFonts2Submenu::Draw()
{
	DrawTitle();

	if (!sub::g_settingsFont)
	{
		Engine* engine = Engine::Current();
		if (engine) engine->GoBack();
		return;
	}

	auto applyRow = [this](const std::string& label, INT8 fontIndex)
	{
		if (DrawOption(label))
		{
			INT8* target = sub::g_settingsFont;
			if (target)
			{
				*target = fontIndex;
				MirrorFontToActiveTheme(target, fontIndex);
			}
		}
	};

	applyRow("Normalish",  static_cast<INT8>(GTAfont::Arial));
	applyRow("Impactish",  static_cast<INT8>(GTAfont::Impact));
	applyRow("Italic",     static_cast<INT8>(GTAfont::Italic));
	applyRow("Pricedown",  static_cast<INT8>(GTAfont::Pricedown));
	applyRow("Caps",       static_cast<INT8>(GTAfont::Caps));

	if (DrawOption("Input Index"))
	{
		INT8* target = sub::g_settingsFont;
		if (target)
		{
			const std::string current = std::to_string(static_cast<int>(*target));
			const std::string inputStr = Game::InputBox(current, 7U, "", current);
			if (!inputStr.empty())
			{
				try
				{
					int parsed = std::abs(std::stoi(inputStr));
					*target = static_cast<INT8>(parsed);
					MirrorFontToActiveTheme(target, *target);
				}
				catch (...)
				{
					Game::Print::PrintErrorInvalidInput(inputStr);
				}
			}
		}
	}
}

void SettingsLanguageSubmenu::Draw()
{
	DrawTitle();

	if (DrawSelectionItem("English", Language::selectedLang == nullptr))
	{
		Language::ResetSelectedLang();
	}

	for (auto& l : Language::allLangs)
	{
		if (DrawSelectionItem(l.GetName(), Language::selectedLang == &l))
		{
			Language::SetSelectedLang(&l);
		}
	}

	if (DrawSelectionItem("Reload Language Files", true, Checkbox::CROSS, Checkbox::NONE))
	{
		Language::Init();
	}
}

}
REGISTER_SUBMENU(::Menu::SettingsSubmenu)
REGISTER_SUBMENU(::Menu::SettingsMenuPosSubmenu)
REGISTER_SUBMENU(::Menu::SettingsThemesSubmenu)
REGISTER_SUBMENU(::Menu::SettingsColoursSubmenu)
REGISTER_SUBMENU(::Menu::SettingsColours2Submenu)
REGISTER_SUBMENU(::Menu::SettingsFontsSubmenu)
REGISTER_SUBMENU(::Menu::SettingsFonts2Submenu)
REGISTER_SUBMENU(::Menu::SettingsLanguageSubmenu)
