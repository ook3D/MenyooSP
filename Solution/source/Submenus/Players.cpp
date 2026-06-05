#include "Players.h"

#include "../macros.h"                         // GAME_PLAYERCOUNT
#include "../Menu/SubmenuRegistry.h"
#include "../Menu/Routine.h"
#include "PlayerRuntime.h"

#include "../Natives/natives2.h"
#include "../Scripting/Game.h"
#include "../Scripting/GTAplayer.h"
#include "../Scripting/GTAped.h"

#include <string>

namespace Menu {


void PlayersSubmenu::Draw()
{
	DrawTitle();

	for (int i = 0; i < GAME_PLAYERCOUNT; ++i)
	{
		if (!NETWORK_IS_PLAYER_ACTIVE(i)) continue;

		if (DrawOption(GET_PLAYER_NAME(i)))
		{
			g_Ped2 = i;
			g_Ped1 = GET_PLAYER_PED(g_Ped2);
			g_PlayerName = GET_PLAYER_NAME(g_Ped2);
			NavigateTo("players_sub");
		}
	}
}

void PlayersSubSubmenu::Draw()
{
	DrawTitle();

	std::string spectatePlayerStr = "CM_SPECTATE";
	if (Game::DoesGXTEntryExist(spectatePlayerStr))
	{
		spectatePlayerStr = Game::GetGXTEntry(spectatePlayerStr);
		spectatePlayerStr = spectatePlayerStr.substr(0, spectatePlayerStr.find('('));
	}
	else
	{
		spectatePlayerStr = "Spectate Player";
	}

	const bool setWaypoint = DrawOption("Set Waypoint To Player");
	const bool isSpectatingThis = (spectatePlayer == g_Ped2);
	const bool spectateTapped = DrawToggleExternal(spectatePlayerStr, isSpectatingThis);

	if (spectateTapped && !isSpectatingThis)
	{
		// Turn ON spectating this player.
		Ped ped = 0;
		spectatePlayer = g_Ped2;
		for (int i = 0; i < GAME_PLAYERCOUNT; ++i)
		{
			if (!NETWORK_IS_PLAYER_ACTIVE(i)) continue;
			ped = GET_PLAYER_PED(i);
			if (!DOES_ENTITY_EXIST(ped)) continue;
			NETWORK_SET_IN_SPECTATOR_MODE_EXTENDED(0, ped, 1);
			NETWORK_SET_IN_SPECTATOR_MODE(false, ped);
		}
		ped = GET_PLAYER_PED(spectatePlayer);
		if (DOES_ENTITY_EXIST(ped))
		{
			STAT_SET_BOOL(GET_HASH_KEY("MPPLY_CAN_SPECTATE"), true, true);
			NETWORK_SET_IN_SPECTATOR_MODE(true, ped);
		}
	}
	else if (spectateTapped && isSpectatingThis)
	{
		// Turn OFF spectating.
		Ped ped = 0;
		for (int i = 0; i < GAME_PLAYERCOUNT; ++i)
		{
			if (!NETWORK_IS_PLAYER_ACTIVE(i)) continue;
			ped = GET_PLAYER_PED(i);
			if (!DOES_ENTITY_EXIST(ped)) continue;
			NETWORK_SET_IN_SPECTATOR_MODE_EXTENDED(0, ped, 1);
			NETWORK_SET_IN_SPECTATOR_MODE(false, ped);
		}
		NETWORK_SET_ACTIVITY_SPECTATOR(false);
		spectatePlayer = -1;
	}

	if (setWaypoint)
	{
		GTAplayer player = g_Ped2;
		if (player.IsActive())
		{
			const GTAped& playerPed = player.GetPed();
			if (playerPed.IsAlive())
			{
				const Vector3& pos = playerPed.GetPosition();
				SET_NEW_WAYPOINT(pos.x, pos.y);
			}
			else
			{
				SET_WAYPOINT_OFF();
			}
		}
		else
		{
			SET_WAYPOINT_OFF();
		}
	}
}

}
REGISTER_SUBMENU(::Menu::PlayersSubmenu)
REGISTER_SUBMENU(::Menu::PlayersSubSubmenu)
