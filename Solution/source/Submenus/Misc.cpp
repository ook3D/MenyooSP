#include "Misc.h"

#include "../Menu/Engine.h"
#include "../Menu/SubmenuRegistry.h"
#include "../Menu/Menu.h"      // Checkbox, Menu::bitController, VirtualKey
#include "../Menu/Routine.h"   // shared globals: noClip, blackoutMode, etc.
#include "../Menu/Ticks.h"     // hideHUD, showFullHUD
#include "PlayerRuntime.h"
#include "VehicleSpawnerRuntime.h"
#include "WeaponRuntime.h"     // shared weapon globals: weaponDamageIncrease, etc.
#include "VehicleRuntime.h"    // vehiclePopulation, g_frozenRadioStation, etc.
#include "VehicleModShopRuntime.h" // bitMSPaintsRGBMode
#include "Weather.h"           // currentTimecycleStrength

#include "../Natives/natives2.h"
#include "../Scripting/enums.h"
#include "../Util/keyboard.h"
#include "../Util/FileLogger.h"
#include "../Util/StringManip.h"
#include "../Util/GTAmath.h"
#include "../Scripting/Game.h"
#include "../Scripting/World.h"
#include "../Scripting/Camera.h"
#include "../Scripting/GameplayCamera.h"
#include "../Scripting/TimecycleModification.h"
#include "../Scripting/GTAped.h"
#include "../Scripting/GTAvehicle.h"

#include "../Misc/Gta2Cam.h"
#include "../Misc/ManualRespawn.h"
#include "../Misc/MeteorShower.h"
#include "../Misc/JumpAroundMode.h"
#include "../Misc/FpsCounter.h"

#include "Spooner/EntityManagement.h"          // DrawRadiusDisplayingMarker
#include "../Scripting/GTAentity.h"

#include <vector>

#include <algorithm>
#include <cctype>

namespace Menu {


void MiscOpsSubmenu::Draw()
{
	DrawTitle();

	g_Ped1 = PLAYER_PED_ID();

	if (DrawToggle("FreeCam (No-Clip)", noClip))
	{
		if (noClip)
		{
			noClipToggle = false;
			Game::Print::PrintBottomCentre("Press ~b~" + VkCodeToStr(BindNoClip)
				+ "~s~ OR ~b~X+LS~s~ OR ~b~Square+L3~s~ to toggle FreeCam.");
			return;
		}
		if (noClipToggle)
		{
			SetNoclipOff1();
			SetNoclipOff2();
			return;
		}
	}

	if (DrawToggleExternal("Top-Down View", GTA2Cam::g_gta2Cam.Enabled()))
		GTA2Cam::ToggleOnOff();
	if (DrawToggleExternal("Manual Respawn", ManualRespawn::g_manualRespawn.Enabled()))
		ManualRespawn::ToggleOnOff();

	{
		int autoKillIdx = autoKillEnemies;
		if (DrawTextList("Auto-kill Enemies", autoKillIdx,
			std::vector<std::string>{ "Off", "Weak", "Radical" }))
		{
			autoKillEnemies = static_cast<unsigned char>(autoKillIdx);
		}
	}

	if (DrawToggleExternal("Meteor Shower Mode", MeteorShower::g_meteorShower.Enabled()))
		MeteorShower::ToggleOnOff();

	const bool empJustChanged = DrawToggle("EMP Mode (For Night-time)", blackoutMode);
	const bool simpleJustChanged = DrawToggle("Simple Blackout Mode (For Night-time)", simpleBlackoutMode);
	if ((empJustChanged && !blackoutMode) || (simpleJustChanged && !simpleBlackoutMode))
	{
		SET_ARTIFICIAL_LIGHTS_STATE(FALSE);
		SET_ARTIFICIAL_VEHICLE_LIGHTS_STATE(TRUE);
	}

	if (DrawToggle("Jump-Around Mode", JumpAroundMode::bEnabled))
		JumpAroundMode::StartJumping(JumpAroundMode::bEnabled);

	DrawToggle("Fireworks Ahoy", fireworksDisplay);

	if (DrawToggle("Massacre Mode", massacreMode))
	{
		World::ClearWeatherOverride();
		World::SetWeatherOverTime(massacreMode ? WeatherType::Thunder : WeatherType::ExtraSunny, 4000);
		return;
	}

	DrawToggle("Restricted Area Access", restrictedAreasAccess);

	{
		const std::vector<std::string> names{ "Off", "Visible & Shaky", "Visible", "Invisible" };
		int explIdx = explostionWP;
		if (DrawTextList("Explosions At Waypoint", explIdx, names))
		{
			explostionWP = static_cast<unsigned char>(explIdx);
		}
	}

	if (DrawToggle("Decreased Ped Population", pedPopulation) && !pedPopulation)
	{
		SET_PED_POPULATION_BUDGET(1);
		return;
	}
	if (DrawToggle("Decreased Vehicle Population", vehiclePopulation) && !vehiclePopulation)
	{
		SET_VEHICLE_POPULATION_BUDGET(1);
		return;
	}

	DrawToggle("Decreased Weapon Pickups", clearWeaponPickups);

	if (DrawOption("Cutscene Player"))      NavigateTo("cutscene_player");
	if (DrawOption("TV Player"))            NavigateTo("misc_tv");
	if (DrawOption("Radio"))                NavigateTo("misc_radio");
	if (DrawOption("Animal Riding (SP)"))   NavigateTo("animal_riding");
	if (DrawOption("Clear Area"))           NavigateTo("misc_clear_area");
	if (DrawOption("Vision Hax"))           NavigateTo("misc_timecycles");
	if (DrawOption("Map Mods (Old)"))       NavigateTo("misc_map_mods");
	if (DrawOption("HUD Options"))          NavigateTo("misc_hud_options");
	if (DrawOption("Game Camera Options"))  NavigateTo("misc_game_cam");

	DrawBreak("Not So Fun");

	if (DrawOption("Delete All Cameras"))
	{
		World::SetRenderingCamera(0);
		World::DestroyAllCameras();
		World::SetRenderingCamera(0);
	}

	if (DrawOption("Rectangle Draw Tool (Mouse) (ALPHA) [DEV]"))
	{
		sub::DrawToolsMenu();
	}

	{
		Engine* engine = Engine::Current();
		const bool cellphoneActive = GET_NUMBER_OF_THREADS_RUNNING_THE_SCRIPT_WITH_THIS_HASH(0xF292D030) > 0;
		const bool pressed = engine
			? engine->AddCheckbox("In-Game Mobile Phone", cellphoneActive,
				Checkbox::BOXTICK, Checkbox::BOXBLANK)
			: false;
		if (pressed)
		{
			if (cellphoneActive)
				TERMINATE_ALL_SCRIPTS_WITH_THIS_NAME("cellphone_controller");
			else
				Game::RequestScript("cellphone_controller", 1424);
		}
	}

	{
		const std::vector<std::string> options{ "Load", "Unload" };
		int idx = yscScriptTexterIndex;
		const bool changed = DrawTextList("YSC Script [DEV]", idx, options);
		yscScriptTexterIndex = static_cast<unsigned char>(idx);
		Engine* engine = Engine::Current();
		if (engine && IsCurrentRowSelected())
		{
			(void)changed; // suppress unused-var warning when accept fires
			if (MenuPressTimer::IsButtonTapped(MenuPressTimer::Button::Accept))
			{
				std::string inputStr = std::string(Game::InputBox("", 65U, "Enter script name:"));
				if (!inputStr.empty())
				{
					static const std::map<std::string, unsigned short> stackSizes
					{
						{ "achievement_controller", 1424 }, { "ambient_sonar", 1424 },
						{ "ambient_tonya", 1424 }, { "ambient_tonyacall2", 1424 },
						{ "ambient_tonyacall5", 1424 }, { "am_mp_property_int", 11048 },
						{ "am_mp_yacht", 5000 }, { "am_pi_menu", 9800 },
						{ "appchecklist", 3800 }, { "appcontacts", 3800 },
						{ "appinternet", 3800 }, { "appmpjoblistnew", 3800 },
						{ "blip_controller", 1424 }, { "bootycallhandler", 1424 },
						{ "buddydeathresponse", 1424 }, { "building_controller", 1424 },
						{ "candidate_controller", 1424 }, { "carwash2", 1424 },
						{ "celebrations", 3650 }, { "cellphone_controller", 1424 },
						{ "cellphone_flashhand", 1424 }, { "cheat_controller", 1424 },
						{ "completionpercentage_controller", 1424 },
						{ "context_controller", 1424 }, { "controller_ambientarea", 1424 },
						{ "controller_races", 1424 }, { "controller_towing", 1424 },
						{ "country_race", 3650 }, { "dialogue_handler", 1424 },
						{ "drunk_controller", 1424 }, { "email_controller", 1424 },
						{ "emergencycall", 512 }, { "emergencycalllauncher", 1424 },
						{ "event_controller", 1424 }, { "fake_interiors", 1424 },
						{ "flow_controller", 1424 }, { "fmmc_launcher", 14000 },
						{ "fm_capture_creator", 18000 }, { "fm_deathmatch_creator", 18000 },
						{ "fm_lts_creator", 18000 }, { "fm_maintain_cloud_header_data", 1424 },
						{ "fm_main_menu", 3650 }, { "fm_mission_controller", 31000 },
						{ "fm_mission_creator", 18000 }, { "fm_race_creator", 18000 },
						{ "freemode", 21512 }, { "freemode_init", 3650 },
						{ "ingamehud", 3650 }, { "maintransition", 8032 },
						{ "maude_postbailbond", 1424 }, { "mission_stat_alerter", 1424 },
						{ "mission_stat_watcher", 1828 }, { "mpstatsinit", 1424 },
						{ "mrsphilips2", 18000 }, { "net_cloud_mission_loader", 2050 },
						{ "net_rank_tunable_loader", 1424 }, { "net_tunable_check", 1424 },
						{ "pickup_controller", 1424 }, { "player_controller", 1424 },
						{ "postrc_barry1and2", 1424 }, { "randomchar_controller", 1424 },
						{ "restrictedareas", 1424 }, { "selector", 1424 },
						{ "shop_controller", 1424 }, { "social_controller", 1828 },
						{ "stats_controller", 1424 }, { "stock_controller", 1424 },
						{ "taxilauncher", 1424 }, { "taxiservice", 1828 },
						{ "tennis_family", 3650 }, { "traffick_air", 18000 },
						{ "ugc_global_registration", 128 }, { "vehicle_gen_controller", 1828 }
					};
					std::transform(inputStr.begin(), inputStr.end(), inputStr.begin(),
						[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
					const auto it = stackSizes.find(inputStr);
					const unsigned short stack = (it != stackSizes.end()) ? it->second : 14000;
					if (yscScriptTexterIndex == YSC_LOAD)
						Game::RequestScript(inputStr.c_str(), stack);
					else
						TERMINATE_ALL_SCRIPTS_WITH_THIS_NAME(inputStr.c_str());
				}
			}
		}
	}
}

void TimecyclesSubmenu::Draw()
{
	DrawTitle();

	if (DrawToggleExternal("Heat Vision", GET_USINGSEETHROUGH()))
	{
		SET_SEETHROUGH(GET_USINGSEETHROUGH() ? FALSE : TRUE);
		return;
	}
	DrawToggle("Heat Vision On Aim", hvSnipers);

	if (DrawToggle("Night Vision (SP)", bitNightVision))
	{
		SET_NIGHTVISION(bitNightVision ? TRUE : FALSE);
		return;
	}

	DrawBreak("---Timecycle Hax---");
	if (DrawNumber("Timecycle Strength", currentTimecycleStrength, 0.02f, 2, 0.0f, 3.0f))
	{
		TimecycleModification::SetModStrength(currentTimecycleStrength);
		return;
	}

	if (DrawOption("Reset"))
	{
		TimecycleModification::ClearMod();
		currentTimecycleStrength = 0.9f;
		return;
	}

	for (const auto& tc : TimecycleModification::vTimecycles)
	{
		if (DrawOption(tc.second))
		{
			TimecycleModification::SetMod(tc.first);
			TimecycleModification::SetModStrength(currentTimecycleStrength);
		}
	}

	DrawBreak("---Custom---");
	if (DrawOption("Input Custom"))
	{
		std::string inputStr = Game::InputBox("DEFAULT", 28U);
		if (!inputStr.empty())
		{
			SET_TIMECYCLE_MODIFIER(inputStr.c_str());
			SET_TIMECYCLE_MODIFIER_STRENGTH(currentTimecycleStrength);
		}
	}
}

void ClearAreaSubmenu::Draw()
{
	DrawTitle();

	const bool radiusChanged = DrawNumber("Range To Clear", g_clearAreaRadius, 0.5f, 2, 0.0f, FLT_MAX);
	(void)radiusChanged;

	if (IsCurrentRowSelected())
	{
		sub::Spooner::EntityManagement::DrawRadiusDisplayingMarker(
			GET_ENTITY_COORDS(g_Ped1, 1), g_clearAreaRadius);
	}

	if (DrawOption("Vehicles"))
	{
		ClearAreaOfEntities(EntityType::VEHICLE, GET_ENTITY_COORDS(g_Ped1, 1),
			g_clearAreaRadius, { GET_VEHICLE_PED_IS_IN(g_Ped1, 0) });
		return;
	}
	if (DrawOption("Peds"))
	{
		ClearAreaOfEntities(EntityType::PED, GET_ENTITY_COORDS(g_Ped1, 1),
			g_clearAreaRadius, { g_Ped1 });
		return;
	}
	if (DrawOption("Objects"))
	{
		ClearAreaOfEntities(EntityType::PROP, GET_ENTITY_COORDS(g_Ped1, 1),
			g_clearAreaRadius, {});
		return;
	}
	if (DrawOption("All"))
	{
		ClearAreaOfEntities(EntityType::ALL, GET_ENTITY_COORDS(g_Ped1, 1),
			g_clearAreaRadius, { g_Ped1, GET_VEHICLE_PED_IS_IN(g_Ped1, 0) });
		return;
	}
}

void RadioSubmenu::Draw()
{
	DrawTitle();

	GTAped ped = g_Ped1;
	GTAvehicle vehicle = ped.CurrentVehicle();
	auto& frozenStation = g_frozenRadioStation;

	const unsigned char stationIds[] = {
		255U, 0U, 1U, 2U, 3U, 4U, 5U, 6U, 7U, 8U, 9U, 10U,
		11U, 12U, 13U, 14U, 15U, 16U, 17U, 18U, 19U, 20U
	};

	Engine* engine = Engine::Current();

	if (ped.Equals(Game::PlayerPed()))
	{
		// "Our ped" path: full radio control + per-row freeze hotkey.
		if (DrawToggleExternal("Mobile Radio", IS_MOBILE_PHONE_RADIO_ACTIVE()))
		{
			if (IS_MOBILE_PHONE_RADIO_ACTIVE())
			{
				SET_MOBILE_RADIO_ENABLED_DURING_GAMEPLAY(false);
				SET_MOBILE_PHONE_RADIO_STATE(0);
				PLAY_SOUND_FROM_ENTITY(-1, "Radio_Off", g_Ped1, "TAXI_SOUNDS", 0, 0);
			}
			else
			{
				SET_FRONTEND_RADIO_ACTIVE(true);
				SET_MOBILE_RADIO_ENABLED_DURING_GAMEPLAY(true);
				SET_MOBILE_PHONE_RADIO_STATE(1);
				PLAY_SOUND_FROM_ENTITY(-1, "Radio_On", g_Ped1, "TAXI_SOUNDS", 0, 0);
			}
		}

		if (DrawOption("Skip Track"))
		{
			SKIP_RADIO_FORWARD();
			Game::Print::PrintBottomCentre(oss_
				<< Game::GetGXTEntry(GET_PLAYER_RADIO_STATION_NAME()) << " - next track");
		}

		DrawBreak("---Stations---");

		for (unsigned char i : stationIds)
		{
			const bool isCurrent = (GET_PLAYER_RADIO_STATION_INDEX() == i);
			const bool isFrozen = (frozenStation == i);

			const bool pressed = engine
				? engine->AddCheckbox(GET_RADIO_STATION_NAME(i), isCurrent,
					isFrozen ? Checkbox::TICK2 : Checkbox::TICK, Checkbox::NONE, /*gxt=*/true)
				: false;

			if (pressed)
			{
				SET_RADIO_TO_STATION_INDEX(i);
				if (ped.IsInVehicle() && vehicle.Exists())
				{
					vehicle.RequestControl();
					SET_VEHICLE_RADIO_ENABLED(vehicle.Handle(), true);
					SET_VEH_RADIO_STATION(vehicle.Handle(), GET_RADIO_STATION_NAME(i));
				}
			}

			// Contextual freeze hotkey on the highlighted row.
			if (engine && IsCurrentRowSelected())
			{
				const std::string hint = (isFrozen ? "Unfreeze" : "Freeze") + std::string(" station");
				if (Menu::bitController)
				{
					engine->AddInstructionalButton(INPUT_SCRIPT_RLEFT, hint, /*isKey=*/false);
					if (IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_SCRIPT_RLEFT))
						frozenStation = isFrozen ? -1 : i;
				}
				else
				{
					engine->AddInstructionalButton(static_cast<int>(VirtualKey::B), hint, /*isKey=*/true);
					if (IsKeyJustUp(VirtualKey::B))
						frozenStation = isFrozen ? -1 : i;
				}
			}
		}
	}
	else
	{
		// "Other ped's vehicle" path: just set the station; no mobile or freeze.
		for (unsigned char i : stationIds)
		{
			const bool pressed = engine
				? engine->AddOption(GET_RADIO_STATION_NAME(i), /*showArrow=*/false, /*gxt=*/true)
				: false;
			if (pressed && vehicle.Exists())
			{
				vehicle.RequestControl();
				SET_VEHICLE_RADIO_ENABLED(vehicle.Handle(), true);
				SET_VEH_RADIO_STATION(vehicle.Handle(), GET_RADIO_STATION_NAME(i));
			}
		}
	}
}

void WaterHackSubmenu::Draw()
{
	DrawTitle();

	DrawToggle("Toggle", sub::WaterHack::Enabled());
	DrawNumber("Radius", sub::WaterHack::Radius(), 3.0f, 1, 0.0f, 650.0f);
	DrawNumber("Height", sub::WaterHack::Height(), 0.1f, 1, -800.0f, 800.0f);
}

const std::map<std::string, std::string> TvSubmenu::kPlaylists
{
	{ "CNT", "PL_STD_CNT" },
	{ "Weazel News", "PL_STD_WZL" },
	{ "Weazel News Transition", "PL_MP_WEAZEL" },
	{ "Lotto Adverts", "PL_LO_CNT" },
	{ "Piswasser Adverts", "PL_LO_WZL" },
	{ "Righteous Slaughter", "PL_LO_RS" },
	{ "Righteous Slaughter Cutscene", "PL_LO_RS_CUTSCENE" },
	{ "Workout", "PL_SP_WORKOUT" },
	{ "Life Invader", "PL_SP_INV" },
	{ "Life Invader Explosion", "PL_SP_INV_EXP" },
	{ "Weazel Adverts", "PL_SP_PLSH1_INTRO" },
	{ "Fame or Shame Episode 1", "PL_LES1_FAME_OR_SHAME" },
	{ "Fame or Shame Episode 2", "PL_STD_WZL_FOS_EP2" },
	{ "Capolavoro", "PL_CINEMA_ARTHOUSE" },
	{ "The Loneliest Robot", "PL_CINEMA_CARTOON" },
	{ "Meltdown", "PL_CINEMA_ACTION" },
	{ "Cinema (No Meltdown)", "PL_CINEMA_MULTIPLAYER_NO_MELTDOWN" },
	{ "Jack Howitzer", "PL_WEB_HOWITZER" },
	{ "Kung Fu Rainbow LazerForce", "PL_WEB_KFLF" },
	{ "Republican Space Rangers", "PL_WEB_RANGERS" },
	{ "CCTV", "PL_MP_CCTV" }
};

void TvSubmenu::Draw()
{
	DrawTitle();
	DrawToggle("Toggle Player", sub::TVChannelStuff::loopBasicTV);

	const float minVolume = -36.0f;
	const float maxVolume = 0.0f;
	float currentVolume = GRAPHICS::GET_TV_VOLUME();

	float display = currentVolume + (maxVolume - minVolume);
	if (DrawNumber("Volume", display, 1.0f, 0, 0.0f, 36.0f))
	{
		// Convert the adjusted display value back into native range.
		const float adjusted = display - (maxVolume - minVolume);
		const float clamped = adjusted > maxVolume ? maxVolume
			: adjusted < minVolume ? minVolume
			: adjusted;
		SET_TV_VOLUME(clamped);
	}

	DrawBreak("---Playlists---");
	for (const auto& pl : kPlaylists)
	{
		if (DrawOption(pl.first))
		{
			GRAPHICS::IS_TVSHOW_CURRENTLY_PLAYING(80996397);
			GRAPHICS::SET_TV_CHANNEL(-1);
			GRAPHICS::SET_TV_CHANNEL_PLAYLIST(0, pl.second.c_str(), 1);
			GRAPHICS::SET_TV_CHANNEL(0);
		}
	}
}

void HudOptionsSubmenu::Draw()
{
	DrawTitle();

	if (DrawToggle("Reveal Entire Minimap", revealMinimap))
		SET_MINIMAP_HIDE_FOW(revealMinimap);

	DrawToggle("Display XYZH Coords", bDisplayXyzhCoords);
	DrawToggle("Display FPS", FPSCounter::bDisplayFps);
	DrawToggle("Hide HUD", hideHUD);
	DrawToggle("Show Full HUD", showFullHUD);

	DrawBreak("Component Colours");
	const std::vector<std::pair<int, std::string>> namedComponents
	{
		{ HudColour::PURE_WHITE,    "Map Blips" },
		{ HudColour::WAYPOINT,      "Waypoint" },
		{ HudColour::WHITE,         "Pausemenu Text & Highlighting" },
		{ HudColour::PAUSE_BG,      "PauseMenu Background" },
		{ HudColour::PAUSEMAP_TINT, "PauseMenu Map Tint" },
		{ HudColour::BLACK,         "Selected Text And Notifications" },
		{ HudColour::FREEMODE,      "Freemode" },
		{ HudColour::MICHAEL,       "Michael" },
		{ HudColour::FRANKLIN,      "Franklin" },
		{ HudColour::TREVOR,        "Trevor" }
	};
	for (const auto& h : namedComponents)
	{
		if (DrawOption(h.second))
		{
			g_Ped4 = h.first;
			bitMSPaintsRGBMode = 10;
			NavigateTo("vehicle_modshop_paints_rgb");
		}
	}

	DrawBreak("All Colours");
	for (int i = 0; i < static_cast<int>(HudColour::vHudColours.size()); ++i)
	{
		if (DrawOption(HudColour::vHudColours[i]))
		{
			g_Ped4 = i;
			bitMSPaintsRGBMode = 10;
			NavigateTo("vehicle_modshop_paints_rgb");
		}
	}
}

void GameCamSubmenu::Draw()
{
	DrawTitle();

	const auto& shakeNames = cameraShakeNames;
	{
		Engine* engine = Engine::Current();
		const std::vector<std::string>& source = (shakeID < 0)
			? std::vector<std::string>{ "None" } : shakeNames;

		const ::Menu::InputResult res = engine
			? engine->AddTextList("Shake Type", shakeID < 0 ? 0 : shakeID, source)
			: ::Menu::InputResult{};

		if (res.rightPressed)
		{
			if (shakeID < static_cast<signed char>(shakeNames.size() - 1))
			{
				++shakeID;
				GameplayCamera::Shake(static_cast<CameraShake>(shakeID), shakeAmplitude);
			}
		}
		else if (res.leftPressed)
		{
			if (shakeID > -1)
			{
				--shakeID;
				if (shakeID < 0) GameplayCamera::StopShaking(true);
				else GameplayCamera::Shake(static_cast<CameraShake>(shakeID), shakeAmplitude);
			}
		}
	}

	if (DrawNumber("Shake Amplitude", shakeAmplitude, 0.05f, 2))
	{
		if (!GameplayCamera::IsShaking())
			GameplayCamera::Shake(static_cast<CameraShake>(shakeID < 0 ? 0 : shakeID), shakeAmplitude);
		else
			GameplayCamera::SetShakeAmplitude(shakeAmplitude);
	}
}

}
namespace sub
{
	void DrawToolsMenu()
	{
		Vector2 startPos;
		Vector2 sizePos;
		Vector2 Pos;

		for (;;)
		{
			WAIT(0);
			SET_MOUSE_CURSOR_THIS_FRAME();
			DISABLE_ALL_CONTROL_ACTIONS(1);

			Pos.x = GET_DISABLED_CONTROL_NORMAL(2, INPUT_CURSOR_X);
			Pos.y = GET_DISABLED_CONTROL_NORMAL(2, INPUT_CURSOR_Y);

			if (IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_CURSOR_ACCEPT))
			{
				startPos = Pos;
			}
			if (IS_DISABLED_CONTROL_PRESSED(2, INPUT_CURSOR_ACCEPT))
			{
				sizePos.x = (Pos.x - startPos.x);
				sizePos.y = (Pos.y - startPos.y);
			}

			SET_SCRIPT_GFX_ALIGN(76, 84);
			SET_SCRIPT_GFX_ALIGN_PARAMS(-0.05f, -0.05f, 0.0f, 0.0f);
			DRAW_RECT((startPos.x + sizePos.x) / 2, (startPos.y + sizePos.y) / 2, sizePos.x, sizePos.y, 107, 0, 107, 225, 0);


			DRAW_SPRITE("CommonMenu", "Gradient_Bgd", 0.90, 0.14, 0.15, 0.15, 0, 255, 255, 255, 210, false, 0);
			Game::Print::SetupDraw(7, Vector2(0.4, 0.4), true, false, false);
			Game::Print::drawstring("Details", 0.90, 0.0675);

			char str[30];

			Game::Print::SetupDraw(0, Vector2(0, 0.33), true, false, false);
			sprintf_s(str, "X - %f", startPos.x + sizePos.x);
			Game::Print::drawstring(str, 0.90, 0.0975);

			Game::Print::SetupDraw(0, Vector2(0, 0.33), true, false, false);
			sprintf_s(str, "Y - %f", startPos.y + sizePos.y);
			Game::Print::drawstring(str, 0.90, 0.1275);

			Game::Print::SetupDraw(0, Vector2(0, 0.33), true, false, false);
			sprintf_s(str, "SizeX - %f", sizePos.x);
			Game::Print::drawstring(str, 0.90, 0.1575);

			Game::Print::SetupDraw(0, Vector2(0, 0.33), true, false, false);
			sprintf_s(str, "SizeY - %f", sizePos.y);
			Game::Print::drawstring(str, 0.90, 0.1875);

			if (IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_FRONTEND_CANCEL))
			{
				break;
			}
		}
	}

	namespace WaterHack
	{
		class WaterHack final
		{
		private:
			bool bEnabled = false;
		public:
			WaterHack()
			{
			}
			bool& Enabled()
			{
				return bEnabled;
			}
			float radius = 0.0f;
			float height = 5.0f;

			void Tick()
			{
				if (bEnabled)
				{
					DoWaterHack();
				}
			}
			inline void DoWaterHack()
			{
				GTAped ped = PLAYER_PED_ID();
				const Vector3& centre = ped.GetPosition();

				std::vector<Vector3> points;
				centre.PointsOnCircle(points, this->radius, this->radius < 10.0f ? 60.0f : 13.0f, 3.5f, true);

				for (auto& current : points)
				{
					WATER::MODIFY_WATER(current.x, current.y, current.z, this->height);
				}
			}
		};

		WaterHack g_waterHack;

		void Tick()
		{
			g_waterHack.Tick();
		}

		bool&  Enabled() { return g_waterHack.Enabled(); }
		float& Radius()  { return g_waterHack.radius; }
		float& Height()  { return g_waterHack.height; }
	}

	namespace TVChannelStuff
	{
		bool loopBasicTV = false;

		std::string currentTvChannelLabel;
		std::map<std::string, std::string> tvPlaylists
		{
			{ "CNT", "PL_STD_CNT" },
			{ "Weazel News", "PL_STD_WZL" },
			{ "Weazel News Transition", "PL_MP_WEAZEL" },
			{ "Lotto Adverts", "PL_LO_CNT" },
			{ "Piswasser Adverts", "PL_LO_WZL" },
			{ "Righteous Slaughter", "PL_LO_RS" },
			{ "Righteous Slaughter Cutscene", "PL_LO_RS_CUTSCENE" },
			{ "Workout", "PL_SP_WORKOUT" },
			{ "Life Invader", "PL_SP_INV" },
			{ "Life Invader Explosion", "PL_SP_INV_EXP" },
			{ "Weazel Adverts", "PL_SP_PLSH1_INTRO" },
			{ "Fame or Shame Episode 1", "PL_LES1_FAME_OR_SHAME" },
			{ "Fame or Shame Episode 2", "PL_STD_WZL_FOS_EP2" },
			{ "Capolavoro", "PL_CINEMA_ARTHOUSE" },
			{ "The Loneliest Robot", "PL_CINEMA_CARTOON" },
			{ "Meltdown", "PL_CINEMA_ACTION" },
			{ "Cinema (No Meltdown)", "PL_CINEMA_MULTIPLAYER_NO_MELTDOWN" },
			{ "Jack Howitzer", "PL_WEB_HOWITZER" },
			{ "Kung Fu Rainbow LazerForce", "PL_WEB_KFLF" },
			{ "Republican Space Rangers", "PL_WEB_RANGERS" },
			{ "CCTV", "PL_MP_CCTV" }
		};

		void DrawTvWhereItsSupposedToBe()
		{
			Vector2 scale = { 0.2f, 0.2f };
			Vector2 pos = { menuPos.x > 0.45f ? 0.0f + scale.x / 2 : 1.0f - scale.x / 2, 0.1f + scale.y / 2 };

			GRAPHICS::SET_TV_AUDIO_FRONTEND(true);
			GRAPHICS::DRAW_TV_CHANNEL(pos.x, pos.y, scale.x, scale.y, 0.0f, 255, 255, 255, 250);
		}
	}

	namespace HudOptions
	{
		bool revealMinimap = false;
	}

	namespace GameCamOptions
	{
		float shakeAmplitude = 1.0f;
		signed char shakeID = -1;
	}
}

REGISTER_SUBMENU(::Menu::MiscOpsSubmenu)
REGISTER_SUBMENU(::Menu::TimecyclesSubmenu)
REGISTER_SUBMENU(::Menu::ClearAreaSubmenu)
REGISTER_SUBMENU(::Menu::RadioSubmenu)
REGISTER_SUBMENU(::Menu::WaterHackSubmenu)
REGISTER_SUBMENU(::Menu::TvSubmenu)
REGISTER_SUBMENU(::Menu::HudOptionsSubmenu)
REGISTER_SUBMENU(::Menu::GameCamSubmenu)

#include <random>
#include <ctime>

bool bDisplayXyzhCoords = false;
bool pedPopulation = false;
bool massacreMode = false;
bool blackoutMode = false;
bool simpleBlackoutMode = false;
bool restrictedAreasAccess = false;
bool fireworksDisplay = false;

// String variables used in various submenus for search, storage, etc.
std::string dict;
std::string dict2;
std::string dict3;

// Misc - massacre mode
void SetMassacreModeTick()
{
	float tempCoords1[3];
	Ped tempPed = PLAYER_PED_ID();

	for (GTAentity veh : nearbyVehicles)
	{
		if (veh.Equals(g_myVeh))
		{
			continue;
		}
		veh.RequestControlOnce();
		APPLY_FORCE_TO_ENTITY(veh.Handle(), 1, GET_RANDOM_FLOAT_IN_RANGE(1.0f, 9.0f), GET_RANDOM_FLOAT_IN_RANGE(1.0f, 9.0f), GET_RANDOM_FLOAT_IN_RANGE(1.0f, 6.0f), 5.0f, 13.0f, 6.5f, 1, 1, 1, 1, 0, 1);
	}

	for (GTAped ped : nearbyPeds)
	{
		ped.RequestControlOnce();
		ped.GiveNM(NMString::nm0286_handCuffsBehindBack);
		SET_PED_RAGDOLL_FORCE_FALL(ped.Handle());
		APPLY_FORCE_TO_ENTITY(ped.Handle(), 1, GET_RANDOM_FLOAT_IN_RANGE(1.0f, 9.0f), GET_RANDOM_FLOAT_IN_RANGE(1.0f, 9.0f), GET_RANDOM_FLOAT_IN_RANGE(1.0f, 6.0f), 5.0f, 13.0f, 6.5f, 1, 1, 1, 1, 0, 1);
	}

	if (rand() % 2)
	{
		Vector3(GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(tempPed, GET_RANDOM_FLOAT_IN_RANGE(25.0f, 50.0f), GET_RANDOM_FLOAT_IN_RANGE(25.0f, 50.0f), GET_RANDOM_FLOAT_IN_RANGE(0.4f, 20.0f))).ToArray(tempCoords1);
		ADD_EXPLOSION(tempCoords1[0], tempCoords1[1], tempCoords1[2], EXPLOSION::TRAIN, 0.2f, 0, 0, 0.05f, false);
		Vector3(GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(tempPed, GET_RANDOM_FLOAT_IN_RANGE(25.0f, 50.0f), GET_RANDOM_FLOAT_IN_RANGE(-25.0f, -50.0f), GET_RANDOM_FLOAT_IN_RANGE(0.4f, 20.0f))).ToArray(tempCoords1);
		ADD_EXPLOSION(tempCoords1[0], tempCoords1[1], tempCoords1[2], EXPLOSION::TRAIN, 0.2f, 0, 0, 0.05f, false);
	}
	else
	{
		Vector3(GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(tempPed, GET_RANDOM_FLOAT_IN_RANGE(-25.0f, -50.0f), GET_RANDOM_FLOAT_IN_RANGE(25.0f, 50.0f), GET_RANDOM_FLOAT_IN_RANGE(0.4f, 20.0f))).ToArray(tempCoords1);
		ADD_EXPLOSION(tempCoords1[0], tempCoords1[1], tempCoords1[2], EXPLOSION::TRAIN, 4.0f, 0, 0, 0.15f, false);
		Vector3(GET_OFFSET_FROM_ENTITY_IN_WORLD_COORDS(tempPed, GET_RANDOM_FLOAT_IN_RANGE(-25.0f, -50.0f), GET_RANDOM_FLOAT_IN_RANGE(-25.0f, -50.0f), GET_RANDOM_FLOAT_IN_RANGE(0.4f, 20.0f))).ToArray(tempCoords1);
		ADD_EXPLOSION(tempCoords1[0], tempCoords1[1], tempCoords1[2], EXPLOSION::TRAIN, 4.0f, 0, 0, 0.15f, false);
	}
}

int GetRandomSpriteId()
{
	static std::vector<int> values = { 396, 303, 304, 397, 394, 462, 206, 161, 42, 3 };
	static size_t index = 0;
	static std::mt19937 rng(static_cast<unsigned int>(std::time(nullptr)));

	// Shuffle the values once we've used them all
	if (index == 0)
	{
		std::shuffle(values.begin(), values.end(), rng);
	}

	int value = values[index++];
	if (index >= values.size())
	{
		index = 0; // Reset for the next shuffle cycle
	}

	return value;
}
// Misc
void SetBlackoutEMPMode()
{
	SET_ARTIFICIAL_LIGHTS_STATE(TRUE);

	for (auto& vehicle : nearbyVehicles)
	{
		if (vehicle == g_myVeh) continue;

		NETWORK_REQUEST_CONTROL_OF_ENTITY(vehicle);
		SET_VEHICLE_ENGINE_ON(vehicle, 0, 1, 0);
	}

	ScrHandle tempSeq;
	OPEN_SEQUENCE_TASK(&tempSeq);
	TASK_LEAVE_ANY_VEHICLE(0, 0, 0);
	TASK_CLEAR_LOOK_AT(0);
	TASK_SET_BLOCKING_OF_NON_TEMPORARY_EVENTS(0, 0);
	TASK_STAND_STILL(0, 300);
	TASK_START_SCENARIO_IN_PLACE(0, "WORLD_HUMAN_STAND_IMPATIENT", 800, 1);
	TASK_USE_MOBILE_PHONE_TIMED(0, 6000);
	TASK_WANDER_STANDARD(0, 0x471c4000, 0);
	CLOSE_SEQUENCE_TASK(tempSeq);

	for (auto& ped : nearbyPeds)
	{
		if (GET_SEQUENCE_PROGRESS(ped) < 0)
		{
			if (!IS_PED_IN_ANY_VEHICLE(ped, 0))
			{
				continue;
			}
			if (GET_ENTITY_SPEED(GET_VEHICLE_PED_IS_IN(ped, 0)) > 0.6f)
			{
				continue;
			}

			NETWORK_REQUEST_CONTROL_OF_ENTITY(ped);
			TASK_PERFORM_SEQUENCE(ped, tempSeq);
			SET_PED_KEEP_TASK(ped, 1);
			SET_PED_FLEE_ATTRIBUTES(ped, 0, 1);
			SET_PED_FLEE_ATTRIBUTES(ped, 1024, 1);
			SET_PED_FLEE_ATTRIBUTES(ped, 131072, 1);
		}
	}
	CLEAR_SEQUENCE_TASK(&tempSeq);
}
void SetBlackoutMode()
{
	SET_ARTIFICIAL_LIGHTS_STATE(TRUE);
	SET_ARTIFICIAL_VEHICLE_LIGHTS_STATE(FALSE);
}

void StartFireworksAtCoords(const Vector3& pos, const Vector3& rot, float scale)
{
	if (!HAS_NAMED_PTFX_ASSET_LOADED("scr_indep_fireworks"))
	{
		REQUEST_NAMED_PTFX_ASSET("scr_indep_fireworks");
	}
	{
		std::vector<std::string> fw{ "scr_indep_firework_starburst", "scr_indep_firework_fountain", "scr_indep_firework_shotburst", "scr_indep_firework_trailburst" };
		USE_PARTICLE_FX_ASSET("scr_indep_fireworks");
		SET_PARTICLE_FX_NON_LOOPED_COLOUR(GET_RANDOM_FLOAT_IN_RANGE(0.0f, 1.0f), GET_RANDOM_FLOAT_IN_RANGE(0.0f, 1.0f), GET_RANDOM_FLOAT_IN_RANGE(0.0f, 1.0f));
		START_NETWORKED_PARTICLE_FX_NON_LOOPED_AT_COORD(fw[rand() % 4].c_str(), pos.x, pos.y, pos.z, rot.x, rot.y, rot.z, scale, 0, 0, 0, false);
	}
}
