#include "PedModelChanger.h"

#include "../Menu/Engine.h"
#include "../Menu/SubmenuRegistry.h"
#include "../Menu/Menu.h"
#include "../Menu/Routine.h"
#include "PlayerRuntime.h"

#include "../Natives/natives2.h"
#include "../Scripting/Game.h"
#include "../Scripting/Model.h"
#include "../Scripting/ModelNames.h"
#include "../Scripting/GTAped.h"
#include "../Util/ExePath.h"
#include "../Util/StringManip.h"
#include "../Util/keyboard.h"
#include "../Util/FileLogger.h"

#include "PedModelRuntime.h"

#include <pugixml\src\pugixml.hpp>

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <string>
#include <utility>
#include <vector>

namespace Menu {

namespace {

inline void toUpperInPlace(std::string& s)
{
	std::transform(s.begin(), s.end(), s.begin(),
		[](unsigned char c) { return static_cast<char>(std::toupper(c)); });
}
inline std::string toLowerCopy(const std::string& s)
{
	std::string out(s);
	std::transform(out.begin(), out.end(), out.begin(),
		[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	return out;
}
inline std::string toUpperCopy(const std::string& s)
{
	std::string out(s); toUpperInPlace(out); return out;
}

bool RowIsActive()
{
	Engine* engine = Engine::Current();
	if (!engine) return false;
	return engine->ActiveSelection() == engine->printingOption;
}

void EmitModelOptionRow(const std::string& label, const Model& model,
	Checkbox tickIcon = Checkbox::TICK)
{
	if (!model.IsInCdImage())
		return;

	Engine* engine = Engine::Current();
	const GTAped& ped = Game::PlayerPed();
	const bool isCurrent = model.Equals(ped.Model());

	const bool pressed = engine
		? engine->AddCheckbox(label, isCurrent, tickIcon, Checkbox::NONE)
		: false;

	if (pressed)
	{
		sub::ChangeModel(model);
		addlog(ige::LogType::LOG_TRACE, "Changed model to: " + label);
	}

	if (engine && RowIsActive())
	{
		const bool isFav = sub::PedFavourites::IsPedAFavourite(model);
		const std::string hint = (isFav ? "Remove from" : "Add to") + std::string(" favourites");

		if (Menu::bitController)
		{
			engine->AddInstructionalButton(INPUT_SCRIPT_RLEFT, hint, /*isKey=*/false);
			if (IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_SCRIPT_RLEFT))
			{
				if (isFav) sub::PedFavourites::RemovePedFromFavourites(model);
				else sub::PedFavourites::AddPedToFavourites(model,
					Game::InputBox("", 28U, "Enter custom name:", GetPedModelLabel(model, true)));
			}
		}
		else
		{
			engine->AddInstructionalButton(static_cast<int>(VirtualKey::B), hint, /*isKey=*/true);
			if (IsKeyJustUp(VirtualKey::B))
			{
				if (isFav) sub::PedFavourites::RemovePedFromFavourites(model);
				else sub::PedFavourites::AddPedToFavourites(model,
					Game::InputBox("", 28U, "Enter custom name:", GetPedModelLabel(model, true)));
			}
		}
	}
}

std::pair<std::string, std::string> PickRandomPed(const std::vector<std::pair<std::string, std::string>>& list)
{
	if (list.empty())
		return { "", "" };
	const auto& sel = list[std::rand() % list.size()];
	addlog(ige::LogType::LOG_TRACE, "Got Random Ped Model: " + sel.first + ", " + sel.second);
	return sel;
}

} // namespace

void PedModelChangerSubmenu::Draw()
{
	DrawTitle();

	g_Ped1 = PLAYER_PED_ID();

	if (DrawOption("Randomize Ped Variation"))
	{
		addlog(ige::LogType::LOG_TRACE, "Random Ped Selected");
		SET_PED_RANDOM_COMPONENT_VARIATION(g_Ped1, 0);
		SET_PED_RANDOM_PROPS(g_Ped1);
		return;
	}

	if (DrawOption("Favourites"))               NavigateTo("ped_model_changer_favourites");
	if (DrawOption("Player"))                   NavigateTo("ped_model_changer_player");
	if (DrawOption("Animals"))                  NavigateTo("ped_model_changer_animal");
	if (DrawOption("Ambient Females"))          NavigateTo("ped_model_changer_amb_females");
	if (DrawOption("Ambient Males"))            NavigateTo("ped_model_changer_amb_males");
	if (DrawOption("Cutscene Models"))          NavigateTo("ped_model_changer_cs");
	if (DrawOption("Gang Female"))              NavigateTo("ped_model_changer_gang_females");
	if (DrawOption("Gang Males"))               NavigateTo("ped_model_changer_gang_males");
	if (DrawOption("Story Models"))             NavigateTo("ped_model_changer_story");
	if (DrawOption("Multiplayer Models"))       NavigateTo("ped_model_changer_mp");
	if (DrawOption("Scenario Females"))         NavigateTo("ped_model_changer_scenario_females");
	if (DrawOption("Scenario Males"))           NavigateTo("ped_model_changer_scenario_males");
	if (DrawOption("Story Scenario Females"))   NavigateTo("ped_model_changer_st_scenario_females");
	if (DrawOption("Story Scenario Males"))     NavigateTo("ped_model_changer_st_scenario_males");
	if (DrawOption("Others"))                   NavigateTo("ped_model_changer_others");

	if (DrawOption("~b~Input~s~ Model"))
	{
		std::string inputStr = Game::InputBox("", 64U, "Enter ped model name (e.g. IG_BENNY):");
		if (!inputStr.empty())
		{
			Model model = inputStr;
			if (model.IsInCdImage())
				sub::ChangeModel(model);
			else
				Game::Print::PrintErrorInvalidModel(inputStr);
		}
		return;
	}
}

void PedModelFavouritesSubmenu::Draw()
{
	DrawTitle();

	pugi::xml_document doc;
	const std::string path = GetPathffA(Pathff::Main, true) + sub::PedFavourites::xmlFavouritePeds;
	if (doc.load_file(path.c_str()).status != pugi::status_ok)
	{
		// First-run: create an empty XML and bail.
		doc.reset();
		auto nodeDeclaration = doc.append_child(pugi::node_declaration);
		nodeDeclaration.append_attribute("version") = "1.0";
		nodeDeclaration.append_attribute("encoding") = "ISO-8859-1";
		auto nodeRoot = doc.append_child("FavouritePeds");
		(void)nodeRoot;
		doc.save_file(path.c_str());
		return;
	}
	pugi::xml_node nodeRoot = doc.document_element();

	if (DrawOption("Add New Ped Model"))
	{
		std::string hashNameStr = Game::InputBox("", 40U, "Enter model name (e.g. IG_BENNY):");
		if (hashNameStr.length())
		{
			Model hashNameHash = GET_HASH_KEY(hashNameStr);
			if (hashNameHash.IsInCdImage())
			{
				WAIT(500);
				std::string customNameStr = Game::InputBox("", 28U, "Enter custom name:",
					GetPedModelLabel(hashNameHash, true));
				if (customNameStr.length())
				{
					if (sub::PedFavourites::AddPedToFavourites(hashNameHash, customNameStr))
						Game::Print::PrintBottomLeft("Ped model ~b~added~s~.");
					else
						Game::Print::PrintBottomLeft("~r~Error:~s~ Unable to add ped model.");
				}
				else
				{
					Game::Print::PrintErrorInvalidInput(customNameStr);
				}
			}
			else
			{
				Game::Print::PrintErrorInvalidModel(hashNameStr);
			}
		}
		else
		{
			Game::Print::PrintErrorInvalidInput(hashNameStr);
		}
	}

	if (!nodeRoot.first_child())
		return;

	DrawBreak("---Added Ped Models---");

	// Search row — accept opens an input box (legacy customInput).
	if (DrawOption(searchStr.empty() ? "SEARCH" : searchStr))
	{
		searchStr = Game::InputBox(searchStr, 126U, "SEARCH", toLowerCopy(searchStr));
		toUpperInPlace(searchStr);
	}

	for (auto nodeLocToLoad = nodeRoot.first_child(); nodeLocToLoad; nodeLocToLoad = nodeLocToLoad.next_sibling())
	{
		const std::string customName = nodeLocToLoad.attribute("customName").as_string();
		Model model = nodeLocToLoad.attribute("hash").as_uint();

		if (!searchStr.empty())
		{
			if (toUpperCopy(customName).find(searchStr) == std::string::npos)
				continue;
		}
		EmitModelOptionRow(customName, model);
	}
}

void PedModelCategorySubmenu::Draw()
{
	DrawTitle();

	const auto& models = Models();

	if (rngped.first.empty() || rngped.first == Game::PlayerPed().Model())
	{
		rngped = PickRandomPed(models);
	}

	EmitModelOptionRow("Random", rngped.first, Checkbox::NONE);

	for (const auto& pmn : models)
	{
		EmitModelOptionRow(pmn.second, pmn.first);
	}
}

#define MENU_PED_MODEL_CATEGORY_IMPL(ClassName, ListVar) \
	const std::vector<std::pair<std::string, std::string>>& ClassName::Models() const { return ListVar; }

MENU_PED_MODEL_CATEGORY_IMPL(PedModelChangerPlayerSubmenu,             g_pedModels_Player)
MENU_PED_MODEL_CATEGORY_IMPL(PedModelChangerAnimalSubmenu,             g_pedModels_Animal)
MENU_PED_MODEL_CATEGORY_IMPL(PedModelChangerAmbFemaleSubmenu,          g_pedModels_AmbientFemale)
MENU_PED_MODEL_CATEGORY_IMPL(PedModelChangerAmbMaleSubmenu,            g_pedModels_AmbientMale)
MENU_PED_MODEL_CATEGORY_IMPL(PedModelChangerCutsceneSubmenu,           g_pedModels_Cutscene)
MENU_PED_MODEL_CATEGORY_IMPL(PedModelChangerGangFemaleSubmenu,         g_pedModels_GangFemale)
MENU_PED_MODEL_CATEGORY_IMPL(PedModelChangerGangMaleSubmenu,           g_pedModels_GangMale)
MENU_PED_MODEL_CATEGORY_IMPL(PedModelChangerStorySubmenu,              g_pedModels_Story)
MENU_PED_MODEL_CATEGORY_IMPL(PedModelChangerMultiplayerSubmenu,        g_pedModels_Multiplayer)
MENU_PED_MODEL_CATEGORY_IMPL(PedModelChangerScenarioFemaleSubmenu,     g_pedModels_ScenarioFemale)
MENU_PED_MODEL_CATEGORY_IMPL(PedModelChangerScenarioMaleSubmenu,       g_pedModels_ScenarioMale)
MENU_PED_MODEL_CATEGORY_IMPL(PedModelChangerStScenarioFemaleSubmenu,   g_pedModels_StoryScenarioFemale)
MENU_PED_MODEL_CATEGORY_IMPL(PedModelChangerStScenarioMaleSubmenu,     g_pedModels_StoryScenarioMale)
MENU_PED_MODEL_CATEGORY_IMPL(PedModelChangerOthersSubmenu,             g_pedModels_Others)

#undef MENU_PED_MODEL_CATEGORY_IMPL

}
REGISTER_SUBMENU(::Menu::PedModelChangerSubmenu)
REGISTER_SUBMENU(::Menu::PedModelFavouritesSubmenu)
REGISTER_SUBMENU(::Menu::PedModelChangerPlayerSubmenu)
REGISTER_SUBMENU(::Menu::PedModelChangerAnimalSubmenu)
REGISTER_SUBMENU(::Menu::PedModelChangerAmbFemaleSubmenu)
REGISTER_SUBMENU(::Menu::PedModelChangerAmbMaleSubmenu)
REGISTER_SUBMENU(::Menu::PedModelChangerCutsceneSubmenu)
REGISTER_SUBMENU(::Menu::PedModelChangerGangFemaleSubmenu)
REGISTER_SUBMENU(::Menu::PedModelChangerGangMaleSubmenu)
REGISTER_SUBMENU(::Menu::PedModelChangerStorySubmenu)
REGISTER_SUBMENU(::Menu::PedModelChangerMultiplayerSubmenu)
REGISTER_SUBMENU(::Menu::PedModelChangerScenarioFemaleSubmenu)
REGISTER_SUBMENU(::Menu::PedModelChangerScenarioMaleSubmenu)
REGISTER_SUBMENU(::Menu::PedModelChangerStScenarioFemaleSubmenu)
REGISTER_SUBMENU(::Menu::PedModelChangerStScenarioMaleSubmenu)
REGISTER_SUBMENU(::Menu::PedModelChangerOthersSubmenu)
