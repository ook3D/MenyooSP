#include "MainMenu.h"

#include "../Menu/SubmenuRegistry.h"
#include "../Menu/Routine.h"
#include "PlayerRuntime.h"

#include "../Natives/natives2.h"
#include "../Util/FileLogger.h"

namespace Menu {

void MainMenuSubmenu::Draw()
{
	static bool firstLoad = true;
	if (firstLoad)
	{
		firstLoad = false;
		addlog(ige::LogType::LOG_TRACE, "First Load of MainMenu Submenu");
	}

	DrawTitle();

	if (DrawOption("Players"))           NavigateTo("players");
	if (DrawOption("Player Options"))    NavigateTo("player_ops");
	if (DrawOption("Vehicle Options"))   NavigateTo("vehicle");
	if (DrawOption("Teleport Options"))  NavigateTo("teleport");
	if (DrawOption("Weapon Options"))    NavigateTo("weapon");
	if (DrawOption("Weather Options"))   NavigateTo("weather");
	if (DrawOption("Time Options"))      NavigateTo("time");
	if (DrawOption("Bodyguard Options")) NavigateTo("bodyguard");
	if (DrawOption("Object Spooner"))    NavigateTo("spooner_main");
	if (DrawOption("Misc Options"))      NavigateTo("misc");
	if (DrawOption("Settings"))          NavigateTo("settings");
	if (DrawOption("The People Behind Menyoo")) NavigateTo("credits");

	g_Ped2 = PLAYER_ID();
	g_Ped1 = PLAYER_PED_ID();
	g_PlayerName = GET_PLAYER_NAME(g_Ped2);
	g_Ped3 = GET_PLAYER_GROUP(g_Ped2);
}

}
REGISTER_SUBMENU(::Menu::MainMenuSubmenu)
