#include "Time.h"

#include "../Menu/SubmenuRegistry.h"
#include "../Menu/Menu.h"      // Checkbox
#include "../Menu/Routine.h"   // pauseClock, syncClock, currentTimescale, pauseClockH/M

#include "../Natives/natives2.h"
#include "../Scripting/DxHookIMG.h"
#include "../Scripting/GTAentity.h"   // GTAentity
#include "../Scripting/GTAped.h"      // GTAped
#include "../Util/keyboard.h"
#include "Teleport/TeleMethods.h"

#include <string>
#include <vector>
#include <time.h>

namespace Menu {

void TimeSubmenu::Draw()
{
	DrawTitle();

	if (DrawOption("Clock (System Time)"))
		NavigateTo("time_clock");

	const bool pauseToggled = DrawToggle("Pause Clock Time", pauseClock);
	DrawToggle("Sync With System Time", syncClock);

	int hour = GET_CLOCK_HOURS();
	int minute = GET_CLOCK_MINUTES();
	const int prevHour = hour;
	const int prevMinute = minute;

	if (DrawNumber("Hour", hour, 1))
	{
		const int diff = hour - prevHour;
		if (diff > 0)
		{
			if ((prevHour + 1) == 24)
			{
				if (GET_CLOCK_MONTH() + 1 == 12 && GET_CLOCK_DAY_OF_MONTH() + 1 == 30)
				{
					SET_CLOCK_DATE(1, 0, GET_CLOCK_YEAR() + 1);
				}
				else if (GET_CLOCK_DAY_OF_MONTH() + 1 == 30)
				{
					SET_CLOCK_DATE(1, GET_CLOCK_MONTH() + 1, GET_CLOCK_YEAR());
				}
				else
				{
					SET_CLOCK_DATE(GET_CLOCK_DAY_OF_MONTH() + 1, GET_CLOCK_MONTH(), GET_CLOCK_YEAR());
				}
				NETWORK_OVERRIDE_CLOCK_TIME(0, prevMinute, GET_CLOCK_SECONDS());
			}
			else
			{
				NETWORK_OVERRIDE_CLOCK_TIME((prevHour + 1), prevMinute, GET_CLOCK_SECONDS());
			}
			return;
		}
		else if (diff < 0)
		{
			if ((prevHour - 1) == -1)
			{
				if (GET_CLOCK_MONTH() - 1 == -1 && GET_CLOCK_DAY_OF_MONTH() - 1 == 0)
				{
					SET_CLOCK_DATE(28, 11, GET_CLOCK_YEAR() - 1);
				}
				else if (GET_CLOCK_DAY_OF_MONTH() - 1 == 0)
				{
					SET_CLOCK_DATE(28, GET_CLOCK_MONTH() - 1, GET_CLOCK_YEAR());
				}
				else
				{
					SET_CLOCK_DATE(GET_CLOCK_DAY_OF_MONTH() - 1, GET_CLOCK_MONTH(), GET_CLOCK_YEAR());
				}
				NETWORK_OVERRIDE_CLOCK_TIME(23, prevMinute, GET_CLOCK_SECONDS());
			}
			else
			{
				NETWORK_OVERRIDE_CLOCK_TIME((prevHour - 1), prevMinute, GET_CLOCK_SECONDS());
			}
			return;
		}
	}

	if (DrawNumber("Minute", minute, 1))
	{
		const int diff = minute - prevMinute;
		if (diff > 0)
		{
			if ((prevMinute + 1) == 61)
			{
				if (prevHour + 1 == 24)
				{
					if (GET_CLOCK_DAY_OF_MONTH() + 1 == 30)
					{
						SET_CLOCK_DATE(1, GET_CLOCK_MONTH() + 1, GET_CLOCK_YEAR());
					}
					else
					{
						SET_CLOCK_DATE(GET_CLOCK_DAY_OF_MONTH() + 1, GET_CLOCK_MONTH(), GET_CLOCK_YEAR());
					}
					NETWORK_OVERRIDE_CLOCK_TIME(0, 0, GET_CLOCK_SECONDS());
				}
				else
				{
					NETWORK_OVERRIDE_CLOCK_TIME(prevHour + 1, 0, GET_CLOCK_SECONDS());
				}
			}
			else
			{
				NETWORK_OVERRIDE_CLOCK_TIME(prevHour, prevMinute + 1, GET_CLOCK_SECONDS());
			}
			return;
		}
		else if (diff < 0)
		{
			if ((prevMinute - 1) == -1)
			{
				if (prevHour - 1 == -1)
				{
					if (GET_CLOCK_DAY_OF_MONTH() - 1 == 0)
					{
						SET_CLOCK_DATE(29, GET_CLOCK_MONTH() - 1, GET_CLOCK_YEAR());
					}
					else
					{
						SET_CLOCK_DATE(GET_CLOCK_DAY_OF_MONTH() - 1, GET_CLOCK_MONTH(), GET_CLOCK_YEAR());
					}
					NETWORK_OVERRIDE_CLOCK_TIME(23, 60, GET_CLOCK_SECONDS());
				}
				else
				{
					NETWORK_OVERRIDE_CLOCK_TIME(prevHour - 1, 60, GET_CLOCK_SECONDS());
				}
			}
			else
			{
				NETWORK_OVERRIDE_CLOCK_TIME(prevHour, prevMinute - 1, GET_CLOCK_SECONDS());
			}
			return;
		}
	}

	if (DrawNumber("World Speed", currentTimescale, 0.1f, 1, 0.0f, 1.1f))
	{
		// Legacy clamped to (0.0, 1.1) — DrawNumber min/max enforce that.
		SET_TIME_SCALE(currentTimescale);
		return;
	}

	if (pauseToggled)
	{
		pauseClockH = GET_CLOCK_HOURS();
		pauseClockM = GET_CLOCK_MINUTES();
		return;
	}
}

void ClockSubmenu::Draw()
{
	using sub::Clock::loopClock;
	using sub::Clock::analogueClockIndex;
	using sub::Clock::analogueClockPosition;
	using sub::Clock::clockImages;
	using sub::Clock::ClockImage;

	DrawTitle();

	const std::vector<std::string> modeNames{ "Off", "Digital", "Analogue" };
	int modeIdx = static_cast<int>(loopClock);
	if (DrawTextList("Clock", modeIdx, modeNames))
	{
		// Legacy clamped to [0, 2]; DrawTextList wraps, so clamp manually.
		if (modeIdx < 0) modeIdx = 0;
		if (modeIdx > 2) modeIdx = 2;
		loopClock = static_cast<UINT8>(modeIdx);
	}

	if (loopClock == 2) // Analogue
	{
		DrawBreak("---Themes---");
		for (UINT i = 0; i < clockImages.size(); ++i)
		{
			const bool isCurrent = (clockImages[analogueClockIndex].faceId == clockImages[i].faceId);
			if (DrawSelectionItem(clockImages[i].name, isCurrent))
			{
				analogueClockIndex = static_cast<UINT8>(i);
			}
		}

		DrawBreak("---Position---");
		DrawNumber("X", analogueClockPosition.x, 0.005f, 3);
		DrawNumber("Y", analogueClockPosition.y, 0.005f, 3);
	}
}

}
REGISTER_SUBMENU(::Menu::TimeSubmenu)
REGISTER_SUBMENU(::Menu::ClockSubmenu)

namespace sub
{
	namespace Clock
	{
		UINT8 loopClock = 0;
		UINT8 analogueClockIndex = 0;
		Vector2 analogueClockPosition = { 0.931f, 0.126f };

		const std::string weekDayNames[] = { "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday" };
		inline void DisplayClockDigital()
		{
			time_t now = time(0);
			tm t;
			localtime_s(&t, &now);

			char mintBuff[3];
			sprintf_s(mintBuff, "%02d", t.tm_min);

			Game::Print::SetupDraw(GTAfont::Arial, Vector2(0.3f, 0.3f), true, false, true, RGBA(255, 255, 255, 210));
			Game::Print::DrawString(oss_ << weekDayNames[t.tm_wday] << " - " << (t.tm_hour < 13 ? t.tm_hour : t.tm_hour - 12) << ':' << mintBuff << (t.tm_hour < 12 ? " am" : " pm"), 0.5f, 0.007f);//0.01f, 0.8f);
		}

		std::vector<ClockImage> clockImages;

		void LoadClockImages()
		{
			std::vector<std::string> results;
			get_all_filenames_with_extension(GetPathffA(Pathff::Graphics, true) + "Clock", ".png", results, false);
			for (auto sit = results.begin(); sit != results.end();)
			{
				if (sit->back() == 'e') // ..._face
				{
					std::string fullNameWithoutExtension = sit->substr(0, sit->rfind('_'));
					std::string faceName = *sit + ".png";//fullNameWithoutExtension + "_face.png";
					std::string handhName = fullNameWithoutExtension + "_handh.png";
					std::string handmName = fullNameWithoutExtension + "_handm.png";

					DxHookIMG::DxTexture faceId = GetPathffA(Pathff::Graphics, true) + "Clock\\" + faceName;
					DxHookIMG::DxTexture hourId = GetPathffA(Pathff::Graphics, true) + "Clock\\" + handhName;
					DxHookIMG::DxTexture minuteId = GetPathffA(Pathff::Graphics, true) + "Clock\\" + handmName;

					ClockImage cmg = { fullNameWithoutExtension, faceId, hourId, minuteId };
					clockImages.push_back(cmg);
				}
				++sit;
			}
		}

		inline void DisplayClockAnalogue()
		{
			time_t now = time(0);
			tm t;
			localtime_s(&t, &now);

			auto& cmg = clockImages[analogueClockIndex];

			const Vector2& size = Vector2(0.1540f, 0.164f) * 0.7f;
			const Vector2& pos = analogueClockPosition;

			cmg.faceId.Draw(0, pos, size, 0.0f, RGBA::AllWhite());
			cmg.hourId.Draw(0, pos, size, (30.0f * t.tm_hour) + (0.5f * t.tm_min), RGBA::AllWhite());
			cmg.minuteId.Draw(0, pos, size, (6.0f * t.tm_min), RGBA::AllWhite());
		}

		void DisplayClock()
		{
			switch (loopClock)
			{
				case 1: DisplayClockDigital();
					break; // Digital
				case 2: DisplayClockAnalogue();
					break; // Analogue
			}
		}
	}
}

UINT8 pauseClockH;
UINT8 pauseClockM;
FLOAT currentTimescale = 1.0f;
bool pauseClock = false;
bool syncClock = false;

// Time
void SetSyncClockTime()
{
	time_t now = time(0);
	tm t;
	localtime_s(&t, &now);
	SET_CLOCK_DATE(t.tm_mday, t.tm_mon, t.tm_year + 1900);
	SET_CLOCK_TIME(t.tm_hour, t.tm_min, t.tm_sec);
}

// Game - HUD (teleport to wp command)
void SetPauseMenuTeleToWpCommand()
{
	if ((PauseMenuState)GET_PAUSE_MENU_STATE() == PauseMenuState::ViewingMap)
	{
		GTAped myPed = PLAYER_PED_ID();
		if (IS_WAYPOINT_ACTIVE() && myPed.IsAlive())
		{
			(Menu::bitController ? DxHookIMG::teleToWpBoxIconGamepad : DxHookIMG::teleToWpBoxIconKeyboard).Draw(0, Vector2(0.5f, 0.04f), Vector2(0.0943f, 0.016f), 0.0f, RGBA::AllWhite());

			if (Menu::bitController ? IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_FRONTEND_RLEFT) : IsKeyJustUp(VirtualKey::T))
			{
				sub::TeleportLocations_catind::TeleMethods::ToWaypoint(myPed);
			}
		}
	}
}
