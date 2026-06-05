#pragma once

#include "../Menu/Submenu.h"

#include <string>
#include <utility>
#include <vector>

namespace Menu {

class PedModelChangerSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_model_changer"; }
	const char* Title() const override { return "Model Changer"; }
	void Draw() override;
};

class PedModelFavouritesSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_model_changer_favourites"; }
	const char* Title() const override { return "Favourites"; }
	void Draw() override;
	void OnExit() override { searchStr.clear(); }

private:
	std::string searchStr;
};

class PedModelCategorySubmenu : public ::Menu::Submenu
{
public:
	void Draw() override;

protected:
	virtual const std::vector<std::pair<std::string, std::string>>& Models() const = 0;

private:
	std::pair<std::string, std::string> rngped;
};

#define MENU_PED_MODEL_CATEGORY(ClassName, IdStr, TitleStr, ListVar) \
	class ClassName final : public PedModelCategorySubmenu \
	{ \
	public: \
		const char* Id() const override    { return IdStr; } \
		const char* Title() const override { return TitleStr; } \
	protected: \
		const std::vector<std::pair<std::string, std::string>>& Models() const override; \
	}

MENU_PED_MODEL_CATEGORY(PedModelChangerPlayerSubmenu,         "ped_model_changer_player",                "Player",                  g_pedModels_Player);
MENU_PED_MODEL_CATEGORY(PedModelChangerAnimalSubmenu,         "ped_model_changer_animal",                "Animals",                 g_pedModels_Animal);
MENU_PED_MODEL_CATEGORY(PedModelChangerAmbFemaleSubmenu,      "ped_model_changer_amb_females",           "Ambient Females",         g_pedModels_AmbientFemale);
MENU_PED_MODEL_CATEGORY(PedModelChangerAmbMaleSubmenu,        "ped_model_changer_amb_males",             "Ambient Males",           g_pedModels_AmbientMale);
MENU_PED_MODEL_CATEGORY(PedModelChangerCutsceneSubmenu,       "ped_model_changer_cs",                    "Cutscene Models",         g_pedModels_Cutscene);
MENU_PED_MODEL_CATEGORY(PedModelChangerGangFemaleSubmenu,     "ped_model_changer_gang_females",          "Gang Females",            g_pedModels_GangFemale);
MENU_PED_MODEL_CATEGORY(PedModelChangerGangMaleSubmenu,       "ped_model_changer_gang_males",            "Gang Males",              g_pedModels_GangMale);
MENU_PED_MODEL_CATEGORY(PedModelChangerStorySubmenu,          "ped_model_changer_story",                 "Story Models",            g_pedModels_Story);
MENU_PED_MODEL_CATEGORY(PedModelChangerMultiplayerSubmenu,    "ped_model_changer_mp",                    "Multiplayer Models",      g_pedModels_Multiplayer);
MENU_PED_MODEL_CATEGORY(PedModelChangerScenarioFemaleSubmenu, "ped_model_changer_scenario_females",      "Scenario Females",        g_pedModels_ScenarioFemale);
MENU_PED_MODEL_CATEGORY(PedModelChangerScenarioMaleSubmenu,   "ped_model_changer_scenario_males",        "Scenario Males",          g_pedModels_ScenarioMale);
MENU_PED_MODEL_CATEGORY(PedModelChangerStScenarioFemaleSubmenu, "ped_model_changer_st_scenario_females", "Story Scenario Females",  g_pedModels_StoryScenarioFemale);
MENU_PED_MODEL_CATEGORY(PedModelChangerStScenarioMaleSubmenu, "ped_model_changer_st_scenario_males",     "Story Scenario Males",    g_pedModels_StoryScenarioMale);
MENU_PED_MODEL_CATEGORY(PedModelChangerOthersSubmenu,         "ped_model_changer_others",                "Others",                  g_pedModels_Others);

#undef MENU_PED_MODEL_CATEGORY

}