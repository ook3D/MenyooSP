/*
* Menyoo PC - Grand Theft Auto V single-player trainer mod
* Copyright (C) 2019  MAFINS
*
* Support-library state shared with the menu engine (theme colours, fonts,
* menuPos, settings keybinds) plus a thin auxiliary instructional-buttons
* facility used by Spooner when no menu is open.
*/
#pragma once

#include "..\Scripting\enums.h"
#include "..\Scripting\Scaleform.h"

#include <string>
#include <utility>
#include <vector>

typedef int INT;
typedef int BOOL;
typedef signed char INT8;
typedef short INT16;
typedef unsigned int UINT;
typedef unsigned __int8 UINT8;
typedef unsigned short UINT16;
typedef unsigned long DWORD;
typedef char* PCHAR;

namespace VirtualKey {
	enum VirtualKey : int;
}

class Vector2;
class RGBA;
class RgbS;

#define GTA_SCROLLOP 8
#define GTA_BETOP 6
#define GTA_MAXOP 14

float GetXCoordAtMenuRightEdge(float widthOfElement, float extraWidth, bool centered);
float GetXCoordAtMenuLeftEdge(float width, bool centered);

namespace MenuPressTimer
{
	enum class Button
	{
		None, Up, Down, Left, Right, Back, Accept
	};
	extern MenuPressTimer::Button currentButton;
	extern DWORD offsettedTime;

	void Update();
	bool IsButtonHeld(const MenuPressTimer::Button& button);
	bool IsButtonTapped(const MenuPressTimer::Button& button);
	bool IsButtonHeldOrTapped(const MenuPressTimer::Button& button);
}

extern bool g_menuNotOpenedYet;

extern Vector2 menuPos;
extern Vector2 g_deltaCursorNormal;
extern float OptionY;

extern INT8 font_title;
extern INT8 font_options;
extern INT8 font_selection;
extern INT8 font_breaks;
extern INT8 font_hud;
extern INT8 font_speedo;

extern RGBA titlebox;
extern RGBA BG;
extern RGBA titletext;
extern RGBA optiontext;
extern RGBA selectedtext;
extern RGBA optionbreaks;
extern RGBA optioncount;
extern RGBA selectionhi;
extern RGBA _globalPedTrackers_Col;

extern std::pair<UINT16, UINT16> menubindsGamepad;
extern UINT16 menubinds;
extern UINT16 respawnbinds;
extern UINT16 stopanimbinds;
extern INT8 g_loglevel;

class MenuInput final
{
public:
	static bool IsUsingController();
	static void UpdateDeltaCursorNormal();
};

namespace Menu
{
	// Settings flags. Read/written by Settings submenus and INI load/save.
	extern bool bitController;
	extern bool bit_mouse;
	extern bool bit_centre_title;
	extern bool bit_centre_options;
	extern bool bit_centre_breaks;
	extern bool gradients;
	extern bool thinLineOverScrect;
	extern bool bit_glare_test;

	// Used by Routine.cpp rainbow loops.
	extern int delayedTimer;

	// Instructional-button bar used by Spooner's closed-menu HUD hints.
	// The engine has its own IB facility when open; this one renders the
	// pending entries every frame from the main loop.
	extern Scaleform instructional_buttons;
	extern std::vector<Scaleform_IbT> vIB;

	void AddIB(ControllerInput buttonId, std::string label);
	void AddIB(VirtualKey::VirtualKey buttonId, std::string label);
	void AddIB(ScaleformButton buttonId, std::string label);
	std::string GetKeyIB(const Scaleform_IbT& ib);
	void DrawIB();
}

bool IsOptionPressed();
bool IsOptionRPressed();
bool IsOptionLPressed();

enum class Checkbox : UINT8
{
	NONE,
	TICK,
	TICK2,
	CROSS,
	BOXTICK,
	BOXCROSS,
	BOXBLANK,
	ARROWRIGHT,
	ARROWLEFT,
	CARTHING,
	BIKETHING,
	WEAPONTHING,
	TATTOOTHING,
	MAKEUPTHING,
	MASKTHING,
	MANWON,
	SKULL_DM,
	SKULL_TDM,
	CARBANG_DM,
	SMALLNEWSTAR,
	PERCENTAGESTICKER,
};
