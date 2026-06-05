#pragma once

#include "../Menu/Submenu.h"

#include "../macros.h"
#include "../Menu/Menu.h"
#include "../Natives/natives2.h"
#include "../Scripting/GTAplayer.h"
#include "../Scripting/Game.h"
#include "../Util/ExePath.h"
#include "../Util/FileLogger.h"

#include <Windows.h>
#include <string>
#include <vector>

typedef unsigned long DWORD;
typedef char *PCHAR;

namespace sub
{
	namespace CutscenePlayer
	{
		extern std::vector<std::string> cutsceneLabels;

		void PopulateCutsceneLabels();
		void EndCutscene();
		void PlayCutscene(const std::string& label);
	}
}

namespace Menu {

class CutscenePlayerSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "cutscene_player"; }
	const char* Title() const override { return "Cutscene Player"; }
	void Draw() override;
};

}