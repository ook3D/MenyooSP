#pragma once

#include <array>

#include "../Menu/Routine.h"

class RgbS;

extern RgbS g_fadedRGB;
extern RgbS g_neonFade;
extern RgbS g_neonSlide;
extern RgbS g_neonHeart;
extern RgbS g_neonShift;

extern bool g_neonFlash;
extern int g_neonSpin;
extern int g_neonSpinBack;
extern bool g_neonFwk[4];

extern bool rainbowBoxes;
extern bool s_neonDirty;

extern int loop_neon_fade;
extern int loop_neon_flash;
extern int loop_neon_delay;

extern bool loop_neon_rgb;
extern bool neonstate[4];

extern RgbS g_setNeonColour;

void TickRainbowFader();
void TickNeonFlashAnim();
void TickNeonFadeAnim();
void TickNeonSlideAnim();
void TickNeonShiftAnim();
void TickNeonSpinAnim();
void TickNeonFwkAnim();
void TickNeonHeartbeatAnim();

std::array<int, 3> GetHSVFromRGB(int r, int g, int b);
void GetHSVFromRGB(RgbS colour);
float NormalizeHSV(int h, int s, int v);
