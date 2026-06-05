/*
* Menyoo PC - Grand Theft Auto V single-player trainer mod
* Copyright (C) 2019  MAFINS
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*/
#include "Routine.h"

#include "..\macros.h"

#include "Menu.h"
#include "MenuConfig.h"
#include "Engine.h"
#include "ImGuiSpooner.h"
#include "Ticks.h"
#include "GlobalEngine.h"

#include "..\Util\FileLogger.h"
#include "..\Scripting\DxHookIMG.h"
#include "..\Scripting\TimecycleModification.h"
#include "..\Scripting\GTAplayer.h"
#include "..\Scripting\Game.h"
#include "..\Memory\GTAmemory.h"

#include "..\Submenus\PedAnimationRuntime.h"
#include "..\Submenus\PedComponentRuntime.h"
#include "..\Submenus\WeaponRuntime.h"
#include "..\Submenus\VehicleSpawnerRuntime.h"
#include "..\Submenus\AnimalRiding.h"
#include "..\Menu\FolderPreviewBmps.h"
#include "..\Submenus\PedSpeech.h"
#include "..\Submenus\Spooner\SpoonerMode.h"
#include "..\Submenus\CutscenePlayer.h"

#include <Windows.h>
#include <thread>

DWORD g_MenyooConfigTick = 0UL;
bool g_ConfigHasNotBeenRead = true;
bool defaultPedSet = false;

void MenyooJustOpened()
{
	Game::Print::PrintBottomLeft(oss_ << "Menyoo PC v" << MENYOO_CURRENT_VER_ << " by ItsJustCurtis and MAFINS");

	SET_AUDIO_FLAG("IsDirectorModeActive", true);

	SET_THIS_SCRIPT_CAN_BE_PAUSED(0);
	SET_THIS_SCRIPT_CAN_REMOVE_BLIPS_CREATED_BY_ANY_SCRIPT(0); // lol poopoo dummy me this isn't a ysc

	if (!GTAmemory::GetIsEnhanced()) {
		if (
			IS_DLC_PRESENT(GET_HASH_KEY("mp2023_01_g9ec")) or
			IS_DLC_PRESENT(GET_HASH_KEY("mp2023_02_g9ec")) or
			IS_DLC_PRESENT(GET_HASH_KEY("mpchristmas3_g9ec")) or
			IS_DLC_PRESENT(GET_HASH_KEY("mpg9ec")) or
			IS_DLC_PRESENT(GET_HASH_KEY("mpSum2_G9EC")) or
			IS_DLC_PRESENT(GET_HASH_KEY("patch2023_01_g9ec")) or
			IS_DLC_PRESENT(GET_HASH_KEY("patchday27g9ecng")) or
			IS_DLC_PRESENT(GET_HASH_KEY("patchday28g9ecng")) or
			IS_DLC_PRESENT(GET_HASH_KEY("patchdayg9ecng")) or
			IS_DLC_PRESENT(GET_HASH_KEY("patch2024_01_g9ec")) or
			IS_DLC_PRESENT(GET_HASH_KEY("mp2024_01_g9ec")) or
			IS_DLC_PRESENT(GET_HASH_KEY("mp2024_02_g9ec"))			//this hardcoding needs to get in the bin.
			)
		{
			//Game::Print::PrintBottomCentre("~r~Warning~s~: 9th Gen content detected, Game may crash. Read Menyoolog for fix instructions.");
				 //Rockstar seems to have fixed the invalid content crash in legacy, removing the on-screen warning, but will keep the log warning.
			ige::myLog << ige::LogType::LOG_WARNING << "Gen9 Content found in dlcpacks, this can cause instability when attempted to be loaded by Menyoo." << std::endl
				<< "			You can find these in your dlclist.xml by searching for \"g9\" and removing these lines or using a comment." << std::endl
				<< "				    		For example: <!--<Item>dlcpacks:/mpg9ec/</Item>-->" << std::endl << std::endl
				<< "				    Current known Gen9 Packs:" << std::endl
				<< "				    		patchdayg9ecng" << std::endl
				<< "				    		mpsum2_g9ec" << std::endl
				<< "				   	 	patchday27g9ecng" << std::endl
				<< "				   	 	mpchristmas3_g9ec" << std::endl
				<< "				   	 	patchday28g9ecng" << std::endl
				<< "				   	 	mp2023_01_g9ec" << std::endl
				<< "				   	 	mp2023_02_g9ec" << std::endl
				<< "				   	 	patch2024_01_g9ec" << std::endl
				<< "				   	 	mp2024_01_g9ec" << std::endl
				<< "				  	  	mp2024_02_g9ec" << std::endl << std::endl
				<< "				    Note: mp2024_02 also contains bugged content. If you continue to experience issues, removing this may help" << std::endl << std::endl;
		}
		else if (
			IS_DLC_PRESENT(GET_HASH_KEY("mp2024_02")) 		//f*cking rockstar cocked up some clothes, this warning is the only protection.
			)
		{
			//Game::Print::PrintBottomCentre("~r~Warning~s~: DLCPack mp2024_02 present, Game may crash. Read Menyoolog for fix instructions.");
			ige::myLog << ige::LogType::LOG_WARNING << "mp2024_02 found in dlcpacks, certain bugged clothing can cause instability when attempted to be loaded by Menyoo." << std::endl
				<< "				    You can find this in your dlclist.xml by searching for \"mp2024_02\" and removing these lines or using a comment." << std::endl
				<< "				    		For example: <!--<Item>dlcpacks:/mp2024_02/</Item>-->" << std::endl << std::endl
				<< "				    Note: this issue can be ignored if bugged content has been fixed by a mod" << std::endl;
		}
	}

	sub::PopulateAllPaintIDs();

	g_menuNotOpenedYet = false;
}
inline void MenyooMain()
{
	bool firstTick = true;
	addlog(ige::LogType::LOG_TRACE, "Loading Textures");
	DxHookIMG::LoadAllMenyooTexturesInit();
	addlog(ige::LogType::LOG_TRACE, "Populate Anims List");
	sub::AnimationMenu::PopulateAllPedAnimsList();
	addlog(ige::LogType::LOG_TRACE, "Populate Favourites");
	sub::WeaponFavourites_catind::PopulateFavouritesInfo();
	addlog(ige::LogType::LOG_TRACE, "Populate Decals");
	sub::PedDecals::PopulateDecalsDict();
	addlog(ige::LogType::LOG_TRACE, "Populate Animals");
	sub::AnimalRiding::PopulateAnimals();
	addlog(ige::LogType::LOG_TRACE, "Populate Vehicle Previews");
	sub::VehicleSpawner::PopulateVehicleBmps();
	addlog(ige::LogType::LOG_TRACE, "Populate Folder Previews");
	sub::FolderPreviewBmps_catind::PopulateFolderBmps();
	addlog(ige::LogType::LOG_TRACE, "Populate Voice Data");
	sub::Speech::PopulateVoiceData();
	addlog(ige::LogType::LOG_TRACE, "Populate Timecycle Names");
	TimecycleModification::PopulateTimecycleNames();
	addlog(ige::LogType::LOG_TRACE, "Populate Global Entity Arrays");
	PopulateGlobalEntityModelsArrays();
	addlog(ige::LogType::LOG_TRACE, "Populate Cutscene Labels");
	sub::CutscenePlayer::PopulateCutsceneLabels();

	DWORD tickNow = GetTickCount();
	srand(tickNow);
	SET_RANDOM_SEED(tickNow);
	g_MenyooConfigTick = tickNow;


	addlog(ige::LogType::LOG_TRACE, "Check Valid for Block Vehicles");
	if (!NETWORK_IS_SESSION_STARTED() && !IS_COMMANDLINE_END_USER_BENCHMARK() && !LANDING_SCREEN_STARTED_END_USER_BENCHMARK())
	{
		addlog(ige::LogType::LOG_TRACE, "Valid, Enabling Blocked Vehicles");
		if (GTAmemory::FindShopController())
			GeneralGlobalHax::EnableBlockedMpVehiclesInSp();
	}
	else
	{
		addlog(ige::LogType::LOG_ERROR, "Invalid, Unable to Unblock Vehicles");
	}

	addlog(ige::LogType::LOG_TRACE, "Creating Tick loop");
	for (;;)
	{
		if (firstTick)
			addlog(ige::LogType::LOG_TRACE, "First Tick - Textures");
		DxHookIMG::DxTexture::GlobalDrawOrderRef() = -9999;
		if (firstTick)
			addlog(ige::LogType::LOG_TRACE, "First Tick - Tick");
		Menu::bitController = MenuInput::IsUsingController();
		MenuPressTimer::Update();
		{
			Menu::Engine& eng = Menu::GlobalEngine();
			bool wasOpen = eng.IsOpen();
			Menu::TickGlobalEngine();
			if (eng.IsOpen() && !wasOpen && g_menuNotOpenedYet)
				MenyooJustOpened();
		}
		TickMenyooLoops();
		Menu::DrawIB();
		if (firstTick)
			addlog(ige::LogType::LOG_TRACE, "First Tick - Load MenyooConfig");
		TickMenyooConfig();
		if (firstTick)
			addlog(ige::LogType::LOG_TRACE, "First Tick - Neonanims");
		if (loop_neon_rgb)         TickRainbowFader();
		if (loop_neon_fade == 1)   TickNeonFadeAnim();
		if (loop_neon_fade == 2)   TickNeonHeartbeatAnim();
		if (loop_neon_fade == 3)   TickNeonShiftAnim();
		if (loop_neon_fade == 4)   TickNeonSlideAnim();
		if (loop_neon_flash == 2 || loop_neon_flash == 3) TickNeonSpinAnim();
		if (loop_neon_flash == 4)  TickNeonFwkAnim();
		if (loop_neon_flash == 1)  TickNeonFlashAnim();
		WAIT(0);
		if (firstTick)
			addlog(ige::LogType::LOG_TRACE, "First Tick - looping");
		firstTick = false;
	}

}
void ThreadMenyooMain()
{
	static bool s_imguiInitAttempted = false;
	if (!s_imguiInitAttempted && !g_isEnhanced)
	{
		s_imguiInitAttempted = true;
		addlog(ige::LogType::LOG_TRACE, "Spawning ImGui spooner init thread");
		CreateThread(NULL, 0, [](LPVOID) -> DWORD {
			sub::Spooner::ImGuiSpooner::Initialize();
			return 0;
		}, NULL, 0, NULL);
	}

	addlog(ige::LogType::LOG_TRACE, "Launching MenyooMain");
	MenyooMain();
}

void TickMenyooConfig()
{
	static bool firstTick = true;
	if (firstTick)
		addlog(ige::LogType::LOG_TRACE, "First Tick - Run TickMenyooConfig");
	//if (GetTickCount() > g_MenyooConfigOnceTick + 9000U)
	if (GetTickCount() > g_MenyooConfigTick + 30000U)
	{
		if (MenuConfig::bSaveAtIntervals)
		{
			MenuConfig::SaveConfig();
		}
		g_MenyooConfigTick = GetTickCount();
	}
	firstTick = false;
}
