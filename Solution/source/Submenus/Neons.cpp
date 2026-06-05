#include "Neons.h"

#include "..\Natives\types.h" // RGBA/RgbS & types
#include "..\Util\FileLogger.h"

#include <Windows.h>
#include <array>
#include <cmath>
#include <cstdlib>

static DWORD g_NeonFaderTick = 0UL;
static DWORD g_NeonSliderTick = 0UL;
static DWORD g_NeonShifterTick = 0UL;
static DWORD g_NeonHeartBeatTick = 0UL;
static DWORD g_FlashTick = 0UL;
static DWORD g_SpinTick = 0UL;
static DWORD g_FwkTick = 0UL;
static DWORD g_FaderTick = 0UL;

// Global state variables

RgbS g_fadedRGB(255, 0, 0);
RgbS g_neonFade(0, 0, 0);
RgbS g_neonSlide(0, 0, 0);
RgbS g_neonHeart(0, 0, 0);
RgbS g_neonShift(0, 0, 0);
bool g_neonFlash = false;
int g_neonSpin = 0;
int g_neonSpinBack = 0;
bool g_neonFwk[4] = { false, false, false, false };

bool rainbowBoxes = false;

bool s_neonDirty = false;

int loop_neon_fade = 0;
int loop_neon_flash = 0;
int loop_neon_delay = 1000;

bool loop_neon_rgb = false;
bool neonstate[4] = { false };

RgbS g_setNeonColour = { 0, 255, 0 };

void TickRainbowFader()
{
	const DWORD now = GetTickCount();
	static bool firstTick = true;
	if (firstTick)
		addlog(ige::LogType::LOG_TRACE, "First Tick - Run TickRainbowFader");
	if (now > g_FaderTick + 20U) {
		auto& colour = g_fadedRGB;
		if (colour.R > 0 && colour.B == 0)
		{
			colour.R--;
			colour.G++;
		}
		if (colour.G > 0 && colour.R == 0)
		{
			colour.G--;
			colour.B++;
		}
		if (colour.B > 0 && colour.G == 0)
		{
			colour.R++;
			colour.B--;
		}
		firstTick = false;

		g_FaderTick = now;
		s_neonDirty = true;
	}
}

void TickNeonFlashAnim()
{
	const DWORD now = GetTickCount();
	if (now >= g_FlashTick + loop_neon_delay)
	{
		auto& neonpower = g_neonFlash;
		neonpower = !neonpower;

		g_FlashTick = now;
		s_neonDirty = true;
	}
}
void TickNeonFadeAnim()
{
	const DWORD now = GetTickCount();
	if (now > g_NeonFaderTick + 20U)
	{
		auto& fade = g_neonFade;
		float loop_fade_multiplier = 1.0f;
		int time = now % (2 * loop_neon_delay);
		if (time > 0)
			loop_fade_multiplier = 0.5 * (cos((3.142 * time) / loop_neon_delay) + 1);
		else
			loop_fade_multiplier = 1;
		fade.R = g_setNeonColour.R * loop_fade_multiplier;
		fade.G = g_setNeonColour.G * loop_fade_multiplier;
		fade.B = g_setNeonColour.B * loop_fade_multiplier;
		g_NeonFaderTick = now;
		s_neonDirty = true;
	}

}
void TickNeonSlideAnim()
{
	const DWORD now = GetTickCount();
	if (now > g_NeonSliderTick + 20U)
	{
		auto& slide = g_neonSlide;
		float loop_slide_multiplier = 1.0f;
		int time = now % (2 * loop_neon_delay);
		loop_slide_multiplier = ((abs(1.2*tanh(1.2 * sin(((0.5 * 3.142 * time / loop_neon_delay))))) - 1) * floor(cos(3.142 * ((time / loop_neon_delay) + 0.5)))) + (1.2*(-(abs(tanh(1.2 * cos(((0.5 * 3.142 * time) / loop_neon_delay)))))*floor(sin(((3.142 * time) / loop_neon_delay)))));
		slide.R = g_setNeonColour.R * loop_slide_multiplier;
		slide.G = g_setNeonColour.G * loop_slide_multiplier;
		slide.B = g_setNeonColour.B * loop_slide_multiplier;
		g_NeonSliderTick = now;
		s_neonDirty = true;
	}

}
void TickNeonShiftAnim()
{
	const DWORD now = GetTickCount();
	if (now > g_NeonShifterTick + 20U)
	{
		auto& shift = g_neonShift;
		int time = now % (2*loop_neon_delay);
		shift.R = (96 - (0.75 * g_setNeonColour.R)) * (sin((3.142 * time) / loop_neon_delay)) + 96 + (0.25 * g_setNeonColour.R);
		shift.G = (96 - (0.75 * g_setNeonColour.G)) * (sin((3.142 * time) / loop_neon_delay)) + 96 + (0.25 * g_setNeonColour.G);
		shift.B = (96 - 0.75 * (g_setNeonColour.B)) * (sin((3.142 * time) / loop_neon_delay)) + 96 + (0.25 * g_setNeonColour.B);
		g_NeonShifterTick = now;
		s_neonDirty = true;
	}

}
void TickNeonHeartbeatAnim() {
	const DWORD now = GetTickCount();
	if (now > g_NeonHeartBeatTick + 20U)
	{
		float loop_heart_multiplier = 1.0f;
		auto& fade = g_neonHeart;
		int time = now % (loop_neon_delay);
		if (time < loop_neon_delay / 2)
			loop_heart_multiplier = 1 - abs(((cos((3.142 * (time)) / loop_neon_delay * 4))));
		else
			loop_heart_multiplier = 0;
		fade.R = g_setNeonColour.R * loop_heart_multiplier;
		fade.G = g_setNeonColour.G * loop_heart_multiplier;
		fade.B = g_setNeonColour.B * loop_heart_multiplier;
		g_NeonHeartBeatTick = now;
		s_neonDirty = true;
	}
}
void TickNeonSpinAnim()
{
	const DWORD now = GetTickCount();
	if (now > g_SpinTick + (loop_neon_delay / 4))
	{
		auto& spindex = g_neonSpin;
		auto& spindex2 = g_neonSpinBack;
		static constexpr int kSpinNext[] = { 2, 3, 1, 0 }; // 0→2→1→3→0
		static constexpr int kSpinBackNext[] = { 3, 2, 0, 1 }; // 0→3→1→2→0
		spindex = kSpinNext[spindex];
		spindex2 = kSpinBackNext[spindex2];
		g_SpinTick = now;
		s_neonDirty = true;
	}
}
void TickNeonFwkAnim()
{
	const DWORD now = GetTickCount();
	if (now > g_FwkTick + 20U)
	{
		auto& fwindex = g_neonFwk;
		int time = now % loop_neon_delay;
		int step = time/(loop_neon_delay/20);
		switch (step)
		{
		case 1:	case 2:
		{
			fwindex[0] = 0;
			fwindex[1] = 0;
			fwindex[2] = 1;
			fwindex[3] = 0;
			break;
		}
		case 3: case 4:
		{
			fwindex[0] = 1;
			fwindex[1] = 1;
			fwindex[2] = 0;
			fwindex[3] = 0;
			break;
		}
		case 5: case 6:
		{
			fwindex[0] = 0;
			fwindex[1] = 0;
			fwindex[2] = 0;
			fwindex[3] = 1;
			break;
		}
		case 7: case 8: case 10:case 11: case 13: default:
		{
			fwindex[0] = 0;
			fwindex[1] = 0;
			fwindex[2] = 0;
			fwindex[3] = 0;
			break;
		}
		case 9:case 12:case 15:
		{
			fwindex[0] = 1;
			fwindex[1] = 1;
			fwindex[2] = 1;
			fwindex[3] = 1;
			break;
		}
		}
		g_FwkTick = now;
		s_neonDirty = true;
	}
}

std::array<int, 3> GetHSVFromRGB(int r, int g, int b)
{
	std::array<int, 3> hsv;
	//setup
	float R = (r / 255.0f);
	float G = (g / 255.0f);
	float B = (b / 255.0f);

	float M = max(R, max(G, B));
	float m = min(R, min(G, B));
	float C = M - m;

	int H = 0;
	int	S = 0;
	//Hue
	if (C != 0)
	{
		if (M == R)
		{
			H = 60 * ((G - B) / C);
		}
		else if (M == G)
		{
			H = 60 * (((B - R) / C) + 2);
		}
		else if (M == B)
		{
			H = 60 * (((R - G) / C) + 4);
		}
		H %= 360;
		if (H < 0)
		{
			H += 360;
		}
	}
	//Saturation
	if (M != 0)
	{
		S = static_cast<int>((C / M) * 100);
	}

	hsv[0] = H;
	hsv[1] = S;
	hsv[2] = static_cast<int>(M * 100);

	return hsv;
}

void GetHSVFromRGB(RgbS colour)
{
	int r, g, b;
	r = colour.R;
	g = colour.G;
	b = colour.B;;
	GetHSVFromRGB(r, g, b);
}

float NormalizeHSV(int h, int s, int v)
{
	float normalOut = sqrt(pow(h, 2) + pow(s, 2) + pow(v / 2, 2));
	return normalOut;
}
