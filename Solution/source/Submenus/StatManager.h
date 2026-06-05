#pragma once

#include "../Menu/Submenu.h"

#include "../macros.h"
#include "../Menu/Menu.h"
#include "../Natives/natives2.h"
#include "../Scripting/Game.h"
#include "../Util/FileLogger.h"

#include <string>
#include <vector>
#include <utility>

#ifndef MENYOO_UINT8_DEFINED
#define MENYOO_UINT8_DEFINED
typedef unsigned __int8 UINT8;
#endif

namespace sub
{
	namespace SpStatManager
	{
		enum class StatDataType_t : UINT8
		{
			UNKNOWN,
			BOOL,
			INT,
			FLOAT
		};

		struct CharStat_t
		{
			std::string name;
			std::string caption;
			StatDataType_t type;
			float min;
			float max;
		};

		int StatGetInt(const std::string& name);
		bool StatGetBool(const std::string& name);
		float StatGetFloat(const std::string& name);
		std::string StatGetString(const std::string& name);
		void StatSetInt(const std::string& name, int value);
		void StatSetBool(const std::string& name, bool value);
		void StatSetFloat(const std::string& name, float value);
		void StatSetString(const std::string& name, const std::string& value);

		// Character roster: 0 = Michael, 1 = Franklin, 2 = Trevor.
		int CharCount();
		const std::string& CharStatPrefix(int charIndex);  // e.g. "SP0_"
		const std::string& CharDisplayName(int charIndex); // e.g. "Michael"

		// Top-level stat lists (Cash, Abilities, Special Ability, K/D, Properties).
		int StatListCount();
		const std::string& StatListTitle(int listIndex);
		int StatListSize(int listIndex);
		const CharStat_t& StatListEntry(int listIndex, int statIndex);

		// Cross-submenu selection state. These are set when the user enters a
		// child submenu and read by the deeper one.
		extern int g_selectedCharIndex;
		extern int g_selectedStatListIndex;
	}
}

namespace Menu {

class SpStatManagerSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "sp_stat_manager"; }
	const char* Title() const override { return "Stat Manager"; }
	void Draw() override;
};

class SpStatManagerInCharSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "sp_stat_manager_in_char"; }
	const char* Title() const override { return "Character"; }
	void Draw() override;
};

class SpStatManagerInCharInListSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "sp_stat_manager_in_char_in_list"; }
	const char* Title() const override { return "Stat List"; }
	void Draw() override;
};

}