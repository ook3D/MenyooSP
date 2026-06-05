#pragma once

#include "..\macros.h"

#include "..\Menu\Menu.h"
#include "..\Menu\Routine.h"
#include "Neons.h"
#include "..\Menu\MenuConfig.h"
#include "..\Menu\Language.h"

#include "..\Natives\natives2.h"
#include "..\Util\GTAmath.h"
#include "..\Natives\types.h" // RGBA
#include "..\Scripting\enums.h"
#include "..\Scripting\Game.h"

#include <string>
#include <math.h>

typedef signed char INT8;
typedef unsigned char UINT8;

class RGBA;

namespace sub
{
	extern UINT8 settingsHUDColor;

	extern RGBA* g_settingsRGBA;
	extern int* g_settingsRGBA2;
	extern INT8* g_settingsFont;

	int SettingsThemeCount();
	const std::string& SettingsThemeName(int index);
	void SettingsThemeApply(int index);
	bool SettingsThemeIsActive(int index);
}




