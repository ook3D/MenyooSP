/*
* Menyoo PC - Grand Theft Auto V single-player trainer mod
*
* Menu::Theme — visual config for an Engine instance: fonts, colours,
* on-screen position, alignment flags. Defaults mirror the old menu's
* baked-in look so a fresh Engine renders the same as the legacy ::Add*
* without further config.
*
* Each Engine instance owns one Theme, so two engines can render side by
* side with different palettes / fonts / positions.
*/
#pragma once

#include "../Util/GTAmath.h"
#include "../Natives/types.h"

namespace Menu
{
	class Theme
	{
	public:
		// Top-left position of the menu in normalised screen coords.
		Vector2 position { 0.0f, 0.0f };

		// Font IDs (match the values used by SET_TEXT_FONT).
		__int8 fontTitle = 7;
		__int8 fontOptions = 0;
		__int8 fontSelection = 0;
		__int8 fontBreaks = 0;

		// Colours.
		RGBA colorTitleText { 255, 255, 255, 255 };
		RGBA colorOption    { 255, 255, 255, 180 };
		RGBA colorSelected  {   0,   0,   0, 255 };
		RGBA colorBreak     { 255, 255, 255, 200 };

		// Chrome colours (background panel, title-bar tint, selection highlight,
		// option-count text).
		RGBA colorBackground         {   0,   0,   0, 140 };
		RGBA colorTitleBox           {   0,   0,   0, 200 };
		RGBA colorSelectionHighlight { 255, 255, 255, 211 };
		RGBA colorOptionCount        { 255, 255, 255, 255 };

		// If true, chrome uses the legacy "Gradient_Nav" / "Gradient_Bgd"
		// sprites instead of flat rects.
		bool useGradients = true;

		// Thin white separator line drawn over the scroller indicator rect.
		bool useThinScrollLine = true;

		// Enable mouse hover / click / scroll-wheel selection in addition to
		// keyboard navigation.
		bool useMouse = false;

		// Render the "MP_MENU_GLARE" scaleform overlay (decorative).
		bool useGlare = false;

		// Render the bottom-of-screen instructional-button bar listing
		// Select / Back / Input hints.
		bool useInstructionalButtons = true;

		// Alignment flags (mirror Menu::bit_centre_title / _options / _breaks).
		bool centreTitle = false;
		bool centreOptions = false;
		bool centreBreaks = false;

		// If true, AddTitle draws using the alt path that bypasses outline /
		// dynamic scaling (mirror of the legacy titletext_ALPHA_DIS_TEMP flag).
		bool titleAlphaDis = false;
	};
}
