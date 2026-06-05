#include "Menu.h"

#include "..\macros.h"

#include "..\Util\GTAmath.h"
#include "..\Util\keyboard.h"
#include "..\Natives\natives2.h"
#include "..\Natives\types.h" // RGBA/RgbS
#include "..\Scripting\Scaleform.h"
#include "..\Scripting\Game.h"
#include "Routine.h"
#include "Ticks.h" // (hideHUD)

#include <Windows.h>

float GetXCoordAtMenuRightEdge(float widthOfElement, float extraWidth, bool centered)
{
	const float baseX = 0.16f + menuPos.x + 0.1f - 0.002f - extraWidth;
	if (centered)
		return baseX - widthOfElement / 2;
	return baseX - widthOfElement;
}

float GetXCoordAtMenuLeftEdge(float width, bool centered)
{
	const float baseX = 0.16f + menuPos.x - 0.1f + 0.002f;
	if (centered)
		return baseX + width / 2;
	return baseX;
}

//--------------------------------MenuPressTimer-------------------------------------------------

namespace MenuPressTimer
{
	MenuPressTimer::Button currentButton = MenuPressTimer::Button::None;
	DWORD offsettedTime = 0;

	void Update()
	{
		if (currentButton == Button::None)
			offsettedTime = GetTickCount() + 630;

		if (IS_DISABLED_CONTROL_PRESSED(2, INPUT_FRONTEND_RIGHT) || IsKeyDown(VirtualKey::Numpad6))
			currentButton = Button::Right;
		else if (IS_DISABLED_CONTROL_PRESSED(2, INPUT_FRONTEND_LEFT) || IsKeyDown(VirtualKey::Numpad4))
			currentButton = Button::Left;
		else if (IS_DISABLED_CONTROL_PRESSED(2, INPUT_FRONTEND_DOWN) || IsKeyDown(VirtualKey::Numpad2))
			currentButton = Button::Down;
		else if (IS_DISABLED_CONTROL_PRESSED(2, INPUT_FRONTEND_UP) || IsKeyDown(VirtualKey::Numpad8))
			currentButton = Button::Up;
		else
		{
			currentButton = Button::None;
			offsettedTime = 0;
		}
	}

	bool IsButtonHeld(const MenuPressTimer::Button& button)
	{
		return currentButton == button && offsettedTime < GetTickCount();
	}

	bool IsButtonTapped(const MenuPressTimer::Button& button)
	{
		switch (button)
		{
		case Button::Right:  return IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_FRONTEND_RIGHT)  || IsKeyJustUp(VirtualKey::Numpad6);
		case Button::Left:   return IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_FRONTEND_LEFT)   || IsKeyJustUp(VirtualKey::Numpad4);
		case Button::Down:   return IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_FRONTEND_DOWN)   || IsKeyJustUp(VirtualKey::Numpad2);
		case Button::Up:     return IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_FRONTEND_UP)     || IsKeyJustUp(VirtualKey::Numpad8);
		case Button::Back:   return IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_FRONTEND_RRIGHT) || IsKeyJustUp(VirtualKey::Numpad0);
		case Button::Accept: return IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_FRONTEND_ACCEPT) || IsKeyJustUp(VirtualKey::Numpad5);
		}
		return true;
	}

	bool IsButtonHeldOrTapped(const MenuPressTimer::Button& button)
	{
		return IsButtonHeld(button) || IsButtonTapped(button);
	}
}

//--------------------------------MenuInput------------------------------------------------------

bool MenuInput::IsUsingController()
{
	return !IS_USING_KEYBOARD_AND_MOUSE(2);
}

void MenuInput::UpdateDeltaCursorNormal()
{
	Vector2 cursor;
	cursor.x = GET_DISABLED_CONTROL_NORMAL(2, INPUT_CURSOR_X);
	cursor.y = GET_DISABLED_CONTROL_NORMAL(2, INPUT_CURSOR_Y);
	g_deltaCursorNormal = cursor - g_deltaCursorNormal;
}

//--------------------------------Shared state---------------------------------------------------

bool g_menuNotOpenedYet = true;

Vector2 menuPos;
Vector2 g_deltaCursorNormal;
float OptionY;

INT8 font_title = 7;
INT8 font_options = 4;
INT8 font_selection = 4;
INT8 font_breaks = 1;
INT8 font_hud = 0;
INT8 font_speedo = 0;

RGBA titlebox(0, 255, 255, 247);
RGBA BG(10, 10, 10, 200);
RGBA titletext(255, 255, 255, 255);
RGBA optiontext(255, 255, 255, 255);
RGBA selectedtext(0, 0, 0, 255);
RGBA optionbreaks(255, 255, 255, 240);
RGBA optioncount(255, 255, 255, 255);
RGBA selectionhi(255, 255, 255, 211);
RGBA _globalPedTrackers_Col(0, 255, 255, 205);

std::pair<UINT16, UINT16> menubindsGamepad = { INPUT_FRONTEND_RB, INPUT_FRONTEND_LEFT };
UINT16 menubinds = VirtualKey::F8;
UINT16 respawnbinds = INPUT_LOOK_BEHIND;
UINT16 stopanimbinds = VirtualKey::J;
INT8 g_loglevel = 2;

int Menu::delayedTimer = 0;
bool Menu::bitController = false;
bool Menu::bit_mouse = false;
bool Menu::bit_centre_title = true;
bool Menu::bit_centre_options = false;
bool Menu::bit_centre_breaks = true;
bool Menu::gradients = true;
bool Menu::thinLineOverScrect = true;
bool Menu::bit_glare_test = true;
Scaleform Menu::instructional_buttons;
std::vector<Scaleform_IbT> Menu::vIB;

//--------------------------------Edge-press helpers---------------------------------------------

bool IsOptionPressed()
{
	if (Menu::bitController)
		return IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_FRONTEND_ACCEPT);
	return IsKeyJustUp(VirtualKey::Numpad5) || IsKeyJustUp(VirtualKey::Return);
}

bool IsOptionRPressed()
{
	return MenuPressTimer::IsButtonHeldOrTapped(MenuPressTimer::Button::Right);
}

bool IsOptionLPressed()
{
	return MenuPressTimer::IsButtonHeldOrTapped(MenuPressTimer::Button::Left);
}

//--------------------------------Instructional buttons (auxiliary)------------------------------

void Menu::AddIB(ControllerInput buttonId, std::string label)
{
	vIB.push_back({ buttonId, std::move(label), false });
}

void Menu::AddIB(VirtualKey::VirtualKey buttonId, std::string label)
{
	vIB.push_back({ buttonId, std::move(label), true });
}

void Menu::AddIB(ScaleformButton buttonId, std::string label)
{
	vIB.push_back({ int(buttonId) + 1000, std::move(label), false });
}

std::string Menu::GetKeyIB(const Scaleform_IbT& ib)
{
	if (ib.button == -3)
		return "";
	if (!ib.isKey)
		return GET_CONTROL_INSTRUCTIONAL_BUTTONS_STRING(2, ib.button, 1);
	return "t_" + VkCodeToStr(ib.button);
}

void Menu::DrawIB()
{
	if (vIB.empty() || UPDATE_ONSCREEN_KEYBOARD() == 0 || hideHUD)
		return;

	if (!instructional_buttons.Load("instructional_buttons"))
		return;

	instructional_buttons.PushFunction("CLEAR_ALL");
	instructional_buttons.PopFunction();

	instructional_buttons.PushFunction("SET_MAX_WIDTH");
	instructional_buttons.PushFloat(100.0f);
	instructional_buttons.PopFunction();

	instructional_buttons.PushFunction("TOGGLE_MOUSE_BUTTONS");
	instructional_buttons.PushBoolean(true);
	instructional_buttons.PopFunction();

	for (size_t i = 0; i < vIB.size(); ++i)
	{
		const Scaleform_IbT& ib = vIB[i];

		instructional_buttons.PushFunction("SET_DATA_SLOT");
		instructional_buttons.PushInteger(static_cast<int>(i));

		if (ib.button >= 1000)
		{
			instructional_buttons.PushInteger(ib.button - 1000);
			instructional_buttons.PushTextComponent(ib.text);
		}
		else
		{
			instructional_buttons.PushString2(GetKeyIB(ib));
			instructional_buttons.PushTextComponent(ib.text);
			instructional_buttons.PushBoolean(true);
			instructional_buttons.PushInteger(ib.button);
		}
		instructional_buttons.PopFunction();
	}

	instructional_buttons.PushFunction("SET_BACKGROUND_COLOUR");
	instructional_buttons.PushRGBA(RGBA(0, 0, 0, 80));
	instructional_buttons.PopFunction();

	instructional_buttons.PushFunction("DRAW_INSTRUCTIONAL_BUTTONS");
	instructional_buttons.PushInteger(0);
	instructional_buttons.PopFunction();

	SET_SCRIPT_GFX_ALIGN(76, 66); // Safezone
	SET_SCRIPT_GFX_ALIGN_PARAMS(0.0f, 0.0f, 0.0f, 0.0f);
	RESET_SCRIPT_GFX_ALIGN();

	instructional_buttons.Render2D();

	vIB.clear();
}
