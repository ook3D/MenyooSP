#include "GlobalEngine.h"

#include "Engine.h"

#include "Menu.h"
#include "../Util/keyboard.h"

namespace Menu
{
	Engine& GlobalEngine()
	{
		static Engine s_engine;
		static bool s_init = []
		{
			s_engine.rootSubmenuId = "main_menu";
			return true;
		}();
		(void)s_init;
		return s_engine;
	}

	static void SyncThemeFromGlobals(Engine& eng)
	{
		eng.theme.position = menuPos;

		eng.theme.colorTitleBox           = titlebox;
		eng.theme.colorBackground         = BG;
		eng.theme.colorTitleText          = titletext;
		eng.theme.colorOption             = optiontext;
		eng.theme.colorSelected           = selectedtext;
		eng.theme.colorBreak              = optionbreaks;
		eng.theme.colorOptionCount        = optioncount;
		eng.theme.colorSelectionHighlight = selectionhi;

		eng.theme.fontTitle     = font_title;
		eng.theme.fontOptions   = font_options;
		eng.theme.fontSelection = font_selection;
		eng.theme.fontBreaks    = font_breaks;

		eng.theme.centreTitle      = Menu::bit_centre_title;
		eng.theme.centreOptions    = Menu::bit_centre_options;
		eng.theme.centreBreaks     = Menu::bit_centre_breaks;
		eng.theme.useGradients     = Menu::gradients;
		eng.theme.useThinScrollLine = Menu::thinLineOverScrect;
		eng.theme.useGlare         = Menu::bit_glare_test;
		eng.theme.useMouse         = Menu::bit_mouse;
	}

	void TickGlobalEngine()
	{
		Engine& eng = GlobalEngine();
		if (IsKeyJustUp(static_cast<VirtualKey::VirtualKey>(menubinds)))
		{
			eng.Toggle();
		}
		SyncThemeFromGlobals(eng);
		eng.Tick();
	}
}
