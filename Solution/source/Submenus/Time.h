#pragma once

#include "../Menu/Submenu.h"

#include <vector>

#include "..\macros.h"

#include "..\Menu\Menu.h"
#include "..\Menu\Routine.h"

#include "..\Natives\natives2.h"
#include "..\Util\GTAmath.h"
#include "..\Scripting\DxHookIMG.h"
#include "..\Scripting\Game.h"
#include "..\Util\ExePath.h"

#include <string>
#include <time.h>
#include <stdio.h>

typedef unsigned __int8 UINT8;

class Vector2;

namespace sub
{
	namespace Clock
	{
		extern UINT8 loopClock;
		extern UINT8 analogueClockIndex;
		extern Vector2 analogueClockPosition;

		inline void DisplayClockDigital();

		struct ClockImage
		{
			std::string name;
			DxHookIMG::DxTexture faceId;
			DxHookIMG::DxTexture hourId;
			DxHookIMG::DxTexture minuteId;
		};

		extern std::vector<ClockImage> clockImages;

		void LoadClockImages();
		inline void DisplayClockAnalogue();

		void DisplayClock();
	}

}

namespace Menu {

class TimeSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "time"; }
	const char* Title() const override { return "Time"; }
	void Draw() override;
};

class ClockSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "time_clock"; }
	const char* Title() const override { return "Clock"; }
	void Draw() override;
};

}

extern UINT8 pauseClockH;
extern UINT8 pauseClockM;
extern FLOAT currentTimescale;
extern bool pauseClock;
extern bool syncClock;

void SetSyncClockTime();
void SetPauseMenuTeleToWpCommand();