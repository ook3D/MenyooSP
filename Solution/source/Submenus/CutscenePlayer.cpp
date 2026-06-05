#include "CutscenePlayer.h"

#include "../Menu/SubmenuRegistry.h"

namespace Menu {

void CutscenePlayerSubmenu::Draw()
{
	DrawTitle();

	if (DrawOption("STOP CUTSCENE(S)"))
	{
		sub::CutscenePlayer::EndCutscene();
	}

	for (const std::string& label : sub::CutscenePlayer::cutsceneLabels)
	{
		if (DrawOption(label))
		{
			sub::CutscenePlayer::PlayCutscene(label);
		}
	}
}

}
REGISTER_SUBMENU(::Menu::CutscenePlayerSubmenu)

// ---- Migrated from CutscenePlayer.cpp ----
namespace sub
{
	namespace CutscenePlayer
	{
		std::vector<std::string> cutsceneLabels;

		void PopulateCutsceneLabels()
		{
			const std::string& filePath = GetPathffA(Pathff::Main, true) + "CutsceneLabels.txt";
			std::ifstream fin(filePath);

			if (fin.is_open())
			{
				cutsceneLabels.clear();

				for (std::string line; std::getline(fin, line);)
				{
					if (line.length() > 2)
					{
						cutsceneLabels.push_back(line);
					}
				}
				addlog(ige::LogType::LOG_INFO,  "Loaded cutscene names from " + filePath);
				fin.close();
			}
		}

		void EndCutscene()
		{
			GTAplayer player = Game::Player();

			STOP_CUTSCENE_IMMEDIATELY();
			player.SetControl(true, 32);
			player.SetControl(true, 16);
			player.SetControl(true, 0);
			REMOVE_CUTSCENE();

		}
		void PlayCutscene(const std::string& label)
		{
			if (IS_CUTSCENE_PLAYING())
			{
				EndCutscene();
				WAIT(1500);
			}

			GTAplayer player = Game::Player();

			if (player.IsAlive())
			{
				player.SetControl(false, 32);
				player.SetControl(false, 16);
				player.SetControl(false, 0);
			}

			REQUEST_CUTSCENE(label.c_str(), 8);

			DWORD timeOut = GetTickCount() + 1500;
			while (GetTickCount() < timeOut)
			{
				WAIT(0);
				if (!HAS_CUTSCENE_LOADED())
				{
					continue;
				}
				SET_CUTSCENE_FADE_VALUES(0, 0, 1, 0);
				START_CUTSCENE(0);
				SET_WIDESCREEN_BORDERS(0, 0);
				SET_RADIO_TO_STATION_NAME("OFF");
				break;
			}
		}
	}
}
