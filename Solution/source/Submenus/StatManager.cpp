#include "StatManager.h"

#include "../Menu/Engine.h"
#include "../Menu/SubmenuRegistry.h"

#include <array>
#include <climits>

namespace Menu {

namespace
{
	using sub::SpStatManager::CharStat_t;
	using sub::SpStatManager::StatDataType_t;

	std::string FullStatName(const CharStat_t& stat)
	{
		const int charIdx = sub::SpStatManager::g_selectedCharIndex;
		return sub::SpStatManager::CharStatPrefix(charIdx) + stat.name;
	}
}

void SpStatManagerSubmenu::Draw()
{
	DrawTitle();

	for (int i = 0; i < sub::SpStatManager::CharCount(); ++i)
	{
		if (DrawOption(sub::SpStatManager::CharDisplayName(i)))
		{
			Game::Print::PrintBottomCentre(
				"~r~Note:~s~ Player Stats temporarily disabled while not working. Check future updates.");
			sub::SpStatManager::g_selectedCharIndex = i;
			NavigateTo("sp_stat_manager_in_char");
		}
	}

	DrawBreak("---Achievements---");

	if (DrawOption("Unlock All Achievements"))
	{
		const int numAchievements = 78;
		for (int i = 0; i < numAchievements; ++i)
		{
			if (!HAS_ACHIEVEMENT_BEEN_PASSED(i))
				GIVE_ACHIEVEMENT_TO_PLAYER(i);
		}
	}

	auto unlockAchievement = [this](int id, const char* description)
	{
		if (DrawOption(std::to_string(id) + ". " + description))
		{
			if (!HAS_ACHIEVEMENT_BEEN_PASSED(id))
				GIVE_ACHIEVEMENT_TO_PLAYER(id);
		}
	};

	unlockAchievement(1, "Unlock 'Welcome to Los Santos'");
	unlockAchievement(2, "Unlock 'A Friendship Resurrected'");
	unlockAchievement(3, "Unlock 'A Fair Day's Pay'");
	unlockAchievement(4, "Unlock 'The Moment of Truth'");
	unlockAchievement(5, "Unlock 'To Live or Die in Los Santos'");
	unlockAchievement(6, "Unlock 'Diamond Hard'");
	unlockAchievement(7, "Unlock 'Subversive'");
	unlockAchievement(8, "Unlock 'Blitzed'");
	unlockAchievement(9, "Unlock 'Small Town, Big Job'");
	unlockAchievement(10, "Unlock 'The Government Gimps'");
	unlockAchievement(11, "Unlock 'The Big One!'");
	unlockAchievement(12, "Unlock 'Solid Gold, Baby!'");
	unlockAchievement(13, "Unlock 'Career Criminal'");
	unlockAchievement(14, "Unlock 'San Andreas Sightseer'");
	unlockAchievement(15, "Unlock 'All's Fare in Love and War'");
	unlockAchievement(16, "Unlock 'TP Industries Arms Race'");
	unlockAchievement(17, "Unlock 'Multi-Disciplined'");
	unlockAchievement(18, "Unlock 'From Beyond the Stars'");
	unlockAchievement(19, "Unlock 'A Mystery, Solved'");
	unlockAchievement(20, "Unlock 'Waste Management'");
	unlockAchievement(21, "Unlock 'Red Mist'");
	unlockAchievement(22, "Unlock 'Show Off'");
	unlockAchievement(23, "Unlock 'Kifflom!'");
	unlockAchievement(24, "Unlock 'Three Man Army'");
	unlockAchievement(25, "Unlock 'Out of Your Depth'");
	unlockAchievement(26, "Unlock 'Altruist Acolyte'");
	unlockAchievement(27, "Unlock 'A Lot of Cheddar'");
	unlockAchievement(28, "Unlock 'Trading Pure Alpha'");
	unlockAchievement(29, "Unlock 'Pimp My Sidearm'");
	unlockAchievement(30, "Unlock 'Wanted: Alive Or Alive'");
	unlockAchievement(31, "Unlock 'Los Santos Customs'");
	unlockAchievement(32, "Unlock 'Close Shave'");
	unlockAchievement(33, "Unlock 'Off the Plane'");
	unlockAchievement(34, "Unlock 'Three-Bit Gangster'");
	unlockAchievement(35, "Unlock 'Making Moves'");
	unlockAchievement(36, "Unlock 'Above the Law'");
	unlockAchievement(37, "Unlock 'Numero Uno'");
	unlockAchievement(38, "Unlock 'The Midnight Club'");
	unlockAchievement(39, "Unlock 'Unnatural Selection'");
	unlockAchievement(40, "Unlock 'Backseat Driver'");
	unlockAchievement(41, "Unlock 'Run Like The Wind'");
	unlockAchievement(42, "Unlock 'Clean Sweep'");
	unlockAchievement(43, "Unlock 'Decorated'");
	unlockAchievement(44, "Unlock 'Stick Up Kid'");
	unlockAchievement(45, "Unlock 'Enjoy Your Stay'");
	unlockAchievement(46, "Unlock 'Crew Cut'");
	unlockAchievement(47, "Unlock 'Full Refund'");
	unlockAchievement(48, "Unlock 'Dialling Digits'");
	unlockAchievement(49, "Unlock 'American Dream'");
	unlockAchievement(50, "Unlock 'A New Perspective'");
	unlockAchievement(51, "Unlock 'Be Prepared'");
	unlockAchievement(52, "Unlock 'In the Name of Science'");
	unlockAchievement(53, "Unlock 'Dead Presidents'");
	unlockAchievement(54, "Unlock 'Parole Day'");
	unlockAchievement(55, "Unlock 'Shot Caller'");
	unlockAchievement(56, "Unlock 'Four Way'");
	unlockAchievement(57, "Unlock 'Live a Little'");
	unlockAchievement(58, "Unlock 'Can't Touch This'");
	unlockAchievement(59, "Unlock 'Mastermind'");
	unlockAchievement(60, "Unlock 'Vinewood Visionary'");
	unlockAchievement(61, "Unlock 'Majestic'");
	unlockAchievement(62, "Unlock 'Humans of Los Santos'");
	unlockAchievement(63, "Unlock 'First Time Director'");
	unlockAchievement(64, "Unlock 'Animal Lover'");
	unlockAchievement(65, "Unlock 'Ensemble Piece'");
	unlockAchievement(66, "Unlock 'Cult Movie'");
	unlockAchievement(67, "Unlock 'Location Scout'");
	unlockAchievement(68, "Unlock 'Method Actor'");
	unlockAchievement(69, "Unlock 'Cryptozoologist'");
	unlockAchievement(70, "Unlock 'Getting Started'");
	unlockAchievement(71, "Unlock 'The Data Breaches'");
	unlockAchievement(72, "Unlock 'The Bogdan Problem'");
	unlockAchievement(73, "Unlock 'The Doomsday Scenario'");
	unlockAchievement(74, "Unlock 'A World Worth Saving'");
	unlockAchievement(75, "Unlock 'Orbital Obliteration'");
	unlockAchievement(76, "Unlock 'Elitist'");
	unlockAchievement(77, "Unlock 'Masterminds'");
}

namespace
{
	void DrawStatRow(const CharStat_t& stat)
	{
		Engine* engine = Engine::Current();
		if (!engine) return;

		const std::string statName = FullStatName(stat);

		switch (stat.type)
		{
			case StatDataType_t::BOOL:
			{
				bool statValue = sub::SpStatManager::StatGetBool(statName);
				if (engine->AddCheckbox(stat.caption, statValue,
					Checkbox::BOXTICK, Checkbox::BOXBLANK))
				{
					addlog(ige::LogType::LOG_DEBUG,
						"Toggling Stat " + stat.caption
						+ " from " + std::string(statValue ? "true" : "false")
						+ " to " + std::string(!statValue ? "true" : "false"));
					statValue = !statValue;
					sub::SpStatManager::StatSetBool(statName, statValue);
				}
				break;
			}
			case StatDataType_t::INT:
			{
				int statValue = sub::SpStatManager::StatGetInt(statName);
				const ::Menu::InputResult res = engine->AddNumber(
					stat.caption, static_cast<double>(statValue), 0);

				if (res.rightPressed)
				{
					if (statValue < static_cast<int>(stat.max))
					{
						++statValue;
						sub::SpStatManager::StatSetInt(statName, statValue);
					}
				}
				else if (res.leftPressed)
				{
					if (statValue > static_cast<int>(stat.min))
					{
						--statValue;
						sub::SpStatManager::StatSetInt(statName, statValue);
					}
				}

				if (res.accepted)
				{
					const std::string inputStr = Game::InputBox(std::string(),
						static_cast<int>(std::to_string(static_cast<int>(stat.max)).length()) + 1,
						"Enter integer value:", std::to_string(statValue));
					if (!inputStr.empty())
					{
						try
						{
							int parsed = std::stoi(inputStr);
							sub::SpStatManager::StatSetInt(statName, parsed);
							addlog(ige::LogType::LOG_TRACE,
								"Stat " + stat.caption + " set to " + std::to_string(parsed) + " via input");
						}
						catch (...)
						{
							Game::Print::PrintErrorInvalidInput(inputStr);
							addlog(ige::LogType::LOG_ERROR,
								"Invalid Stat Integer for " + stat.caption + " Entered");
						}
					}
				}
				break;
			}
			case StatDataType_t::FLOAT:
			{
				float statValue = sub::SpStatManager::StatGetFloat(statName);
				const ::Menu::InputResult res = engine->AddNumber(
					stat.caption, static_cast<double>(statValue), 2);

				if (res.rightPressed)
				{
					if (statValue < stat.max)
					{
						statValue += 0.05f;
						sub::SpStatManager::StatSetFloat(statName, statValue);
					}
				}
				else if (res.leftPressed)
				{
					if (statValue > stat.min)
					{
						statValue -= 0.05f;
						sub::SpStatManager::StatSetFloat(statName, statValue);
					}
				}

				if (res.accepted)
				{
					const std::string inputStr = Game::InputBox(std::string(), 13U,
						"Enter floating point value:", std::to_string(statValue));
					if (!inputStr.empty())
					{
						try
						{
							float parsed = std::stof(inputStr);
							sub::SpStatManager::StatSetFloat(statName, parsed);
						}
						catch (...)
						{
							Game::Print::PrintErrorInvalidInput(inputStr);
							addlog(ige::LogType::LOG_ERROR,
								"Invalid Stat Float for " + stat.caption + " Entered");
						}
					}
				}
				break;
			}
			case StatDataType_t::UNKNOWN:
			default:
				break;
		}
	}
}

void SpStatManagerInCharSubmenu::Draw()
{
	const int charIdx = sub::SpStatManager::g_selectedCharIndex;
	if (charIdx < 0)
	{
		Engine* engine = Engine::Current();
		if (engine) engine->GoBack();
		return;
	}

	Engine* engine = Engine::Current();
	if (engine) engine->AddTitle(sub::SpStatManager::CharDisplayName(charIdx));

	const int listCount = sub::SpStatManager::StatListCount();
	for (int i = 0; i < listCount; ++i)
	{
		// Single-entry lists are rendered inline (matches legacy "Cash" row).
		if (sub::SpStatManager::StatListSize(i) == 1)
		{
			DrawStatRow(sub::SpStatManager::StatListEntry(i, 0));
		}
		else
		{
			if (DrawOption(sub::SpStatManager::StatListTitle(i)))
			{
				sub::SpStatManager::g_selectedStatListIndex = i;
				NavigateTo("sp_stat_manager_in_char_in_list");
			}
		}
	}
}

void SpStatManagerInCharInListSubmenu::Draw()
{
	const int listIdx = sub::SpStatManager::g_selectedStatListIndex;
	if (listIdx < 0)
	{
		Engine* engine = Engine::Current();
		if (engine) engine->GoBack();
		return;
	}

	Engine* engine = Engine::Current();
	if (engine) engine->AddTitle(sub::SpStatManager::StatListTitle(listIdx));

	const int statCount = sub::SpStatManager::StatListSize(listIdx);
	for (int i = 0; i < statCount; ++i)
	{
		DrawStatRow(sub::SpStatManager::StatListEntry(listIdx, i));
	}
}

}
REGISTER_SUBMENU(::Menu::SpStatManagerSubmenu)
REGISTER_SUBMENU(::Menu::SpStatManagerInCharSubmenu)
REGISTER_SUBMENU(::Menu::SpStatManagerInCharInListSubmenu)

namespace sub
{
	namespace SpStatManager
	{
		struct NamedCharStatList_t
		{
			std::string title;
			std::vector<CharStat_t> list;
		};

		const std::array<NamedCharStatList_t, 5> vCharStatLists
		{ {
			{ "Cash",{
				{ "TOTAL_CASH", "Total Cash", StatDataType_t::INT, 0, static_cast<float>(INT_MAX) }
			} },
			{ "Abilities (ALPHA)",{
				{ "STAMINA", "Stamina", StatDataType_t::INT, 0, 100 },
				{ "STRENGTH", "Strength", StatDataType_t::INT, 0, 100 },
				{ "LUNG_CAPACITY", "Lung Capacity", StatDataType_t::INT, 0, 100 },
				{ "WHEELIE_ABILITY", "Wheelieing", StatDataType_t::INT, 0, 100 },
				{ "FLYING_ABILITY", "Flying", StatDataType_t::INT, 0, 100 },
				{ "SHOOTING_ABILITY", "Shooting", StatDataType_t::INT, 0, 100 },
				{ "STEALTH_ABILITY", "Stealth", StatDataType_t::INT, 0, 100 }
			} },
			{ "Special Ability",{
				{ "SPECIAL_ABILITY", "Amount Not Unlocked (ALPHA)", StatDataType_t::INT, 0, 100 },
				{ "SPECIAL_ABILITY_UNLOCKED", "Special Capacity", StatDataType_t::INT, 0, 100 }
			} },
			{ "K/D Ratio",{
				{ "KILLS", "Kill Count", StatDataType_t::INT, 0, static_cast<float>(INT_MAX) },
				{ "DEATHS", "Death Count", StatDataType_t::INT, 0, static_cast<float>(INT_MAX) }
			} },
			{ "Properties",{
				{ "PROP_BOUGHT_TRAF", "Arms Trafficking", StatDataType_t::BOOL, 0, 1 },
				{ "PROP_BOUGHT_CSCR", "Car Scrap Yard", StatDataType_t::BOOL, 0, 1 },
				{ "PROP_BOUGHT_WEED", "Weed Shop", StatDataType_t::BOOL, 0, 1 },
				{ "PROP_BOUGHT_TAXI", "Taxi Lot", StatDataType_t::BOOL, 0, 1 },
				{ "PROP_BOUGHT_CMSH", "Car Mod Shop", StatDataType_t::BOOL, 0, 1 },
				{ "PROP_BOUGHT_SOCO", "Sonar Collections", StatDataType_t::BOOL, 0, 1 },
				{ "PROP_BOUGHT_TOWI", "Towing Impound", StatDataType_t::BOOL, 0, 1 },
				{ "PROP_BOUGHT_GOLF", "Golf Club", StatDataType_t::BOOL, 0, 1 },
				{ "PROP_BOUGHT_CINV", "Vinewood Cinema", StatDataType_t::BOOL, 0, 1 },
				{ "PROP_BOUGHT_CIND", "Downtown Cinema", StatDataType_t::BOOL, 0, 1 },
				{ "PROP_BOUGHT_CINM", "Morningwood Cinema", StatDataType_t::BOOL, 0, 1 },
				{ "PROP_BOUGHT_BARTE", "Tequilala Bar", StatDataType_t::BOOL, 0, 1 },
				{ "PROP_BOUGHT_BARPI", "Pitchers Bar", StatDataType_t::BOOL, 0, 1 },
				{ "PROP_BOUGHT_BARHE", "Hen House Bar", StatDataType_t::BOOL, 0, 1 },
				{ "PROP_BOUGHT_BARHO", "Hookies Bar", StatDataType_t::BOOL, 0, 1 },
				{ "PROP_BOUGHT_STRIP", "Strip Club", StatDataType_t::BOOL, 0, 1 }
			} }
			} };

		std::pair<std::string, std::string> charNames[3] = { { "SP0_", "Michael" },{ "SP1_", "Franklin" },{ "SP2_", "Trevor" } };

		// Cross-submenu selection indices used by the port.
		int g_selectedCharIndex = -1;
		int g_selectedStatListIndex = -1;

		int CharCount() { return 3; }
		const std::string& CharStatPrefix(int charIndex)  { return charNames[charIndex].first; }
		const std::string& CharDisplayName(int charIndex) { return charNames[charIndex].second; }

		int StatListCount() { return static_cast<int>(vCharStatLists.size()); }
		const std::string& StatListTitle(int listIndex) { return vCharStatLists[listIndex].title; }
		int StatListSize(int listIndex) { return static_cast<int>(vCharStatLists[listIndex].list.size()); }
		const CharStat_t& StatListEntry(int listIndex, int statIndex)
		{
			return vCharStatLists[listIndex].list[statIndex];
		}

		// Setters/Getters
		int StatGetInt(const std::string& name)
		{
			int tempp;
			STAT_GET_INT(GET_HASH_KEY(name), &tempp, -1);
			return tempp;
		}

		bool StatGetBool(const std::string& name)
		{
			int tempp;
			STAT_GET_BOOL(GET_HASH_KEY(name), &tempp, -1);
			return tempp != 0;
		}

		float StatGetFloat(const std::string& name)
		{
			float tempp;
			STAT_GET_FLOAT(GET_HASH_KEY(name), &tempp, -1);
			return tempp;
		}

		std::string StatGetString(const std::string& name)
		{
			return STAT_GET_STRING(GET_HASH_KEY(name), -1);
		}

		void StatSetInt(const std::string& name, int value)
		{
			addlog(ige::LogType::LOG_TRACE, "Setting Stat " + name + " to " + std::to_string(value));
			TERMINATE_ALL_SCRIPTS_WITH_THIS_NAME("stats_controller");
			STAT_SET_INT(GET_HASH_KEY(name), value, 1);
		}

		void StatSetBool(const std::string& name, bool value)
		{
			addlog(ige::LogType::LOG_TRACE, "Setting Stat " + name + " to " + std::string(value ? "true" : "false"));
			TERMINATE_ALL_SCRIPTS_WITH_THIS_NAME("stats_controller");
			STAT_SET_BOOL(GET_HASH_KEY(name), value, 1);
		}

		void StatSetFloat(const std::string& name, float value)
		{
			addlog(ige::LogType::LOG_TRACE, "Setting Stat " + name + " to " + std::to_string(value));
			TERMINATE_ALL_SCRIPTS_WITH_THIS_NAME("stats_controller");
			STAT_SET_FLOAT(GET_HASH_KEY(name), value, 1);
		}

		void StatSetString(const std::string& name, const std::string& value)
		{
			addlog(ige::LogType::LOG_TRACE, "Setting Stat " + name + " to " + value);
			TERMINATE_ALL_SCRIPTS_WITH_THIS_NAME("stats_controller");
			STAT_SET_STRING(GET_HASH_KEY(name), value.c_str(), 1);
		}
	}
}
