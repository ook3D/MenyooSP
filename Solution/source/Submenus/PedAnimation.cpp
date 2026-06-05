#include "PedAnimation.h"

#include "../Menu/Engine.h"
#include "../Menu/SubmenuRegistry.h"
#include "../Menu/Menu.h"      // Checkbox, Menu::bitController, VirtualKey
#include "../Menu/Routine.h"   // shared globals: g_Ped1, g_pedListMovGroup, GetPedFacialMood, etc.
#include "PlayerRuntime.h"     // g_Ped1
#include "Misc.h"              // dict

#include "../Natives/natives2.h"
#include "../Scripting/enums.h"
#include "../Scripting/GTAentity.h"
#include "../Scripting/GTAped.h"
#include "../Scripting/GTAvehicle.h"
#include "../Scripting/Game.h"
#include "../Util/keyboard.h"
#include "../Util/ExePath.h"
#include "../Util/StringManip.h"

#include "PedAnimationRuntime.h"                   // ped animation helpers (favourites XML, AllPedAnims data, etc.)
#include "Spooner/Databases.h"
#include "Spooner/EntityManagement.h"
#include "Spooner/SpoonerEntity.h"

#include <pugixml/src/pugixml.hpp>

#include <algorithm>
#include <cctype>
#include <string>
#include <vector>

namespace Menu {

static std::string g_customAnimDict   = "Enter Dictionary";
static std::string g_customAnimName   = "Enter Name";
static float       g_customAnimSpeed      = 4.0f;
static float       g_customAnimSpeedMult  = -4.0f;
static float       g_CustomAnimRate       = 0.0f;
static int         g_customAnimDuration   = -1;
static int         g_customAnimFlag       = AnimFlag::Loop;
static bool        g_customAnimLockPos    = false;

static std::string ToLowerCopy(const std::string& s)
{
	std::string out = s;
	std::transform(out.begin(), out.end(), out.begin(),
		[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	return out;
}

static std::string ToUpperCopy(const std::string& s)
{
	std::string out = s;
	std::transform(out.begin(), out.end(), out.begin(),
		[](unsigned char c) { return static_cast<char>(std::toupper(c)); });
	return out;
}

static void PlayAnimationOnPlayerPed(const std::string& animDict, const std::string& animName)
{
	GTAped ped = g_Ped1;
	GTAentity att;
	auto spi = sub::Spooner::EntityManagement::GetEntityIndexInDb(ped);
	if (spi >= 0)
	{
		auto& spe = sub::Spooner::Databases::EntityDb[spi];
		sub::Spooner::EntityManagement::GetEntityThisEntityIsAttachedTo(spe.handle, att);
	}

	ped.RequestControl();
	ped.Task().PlayAnimation(animDict, animName, g_customAnimSpeed, g_customAnimSpeedMult,
		g_customAnimDuration, g_customAnimFlag, g_CustomAnimRate, g_customAnimLockPos);

	if (spi >= 0)
	{
		auto& spe = sub::Spooner::Databases::EntityDb[spi];
		spe.lastAnimation.dict = animDict;
		spe.lastAnimation.name = animName;
		if (att.Exists() && spe.attachmentArgs.isAttached)
		{
			spe.handle.AttachTo(att, spe.attachmentArgs.boneIndex,
				spe.handle.GetIsCollisionEnabled(),
				spe.attachmentArgs.offset, spe.attachmentArgs.rotation);
		}
		spe.taskSequence.Reset();
		if (sub::Spooner::selectedEntity.handle.Equals(spe.handle))
		{
			sub::Spooner::selectedEntity.lastAnimation.dict = spe.lastAnimation.dict;
			sub::Spooner::selectedEntity.lastAnimation.name = spe.lastAnimation.name;
			sub::Spooner::selectedEntity.taskSequence = spe.taskSequence;
		}
	}
}

static void DrawAnimRow(const std::string& text,
	const std::string& animDict, const std::string& animNameIn)
{
	const std::string animName = animNameIn.empty() ? text : animNameIn;

	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	const bool isPlaying = IS_ENTITY_PLAYING_ANIM(g_Ped1, animDict.c_str(), animName.c_str(), 3) != 0;
	if (engine->AddCheckbox(text, isPlaying, Checkbox::MANWON, Checkbox::NONE))
	{
		PlayAnimationOnPlayerPed(animDict, animName);
	}

	const bool rowSelected = (engine->printingOption == engine->ActiveSelection());
	if (rowSelected)
	{
		const bool isFav = sub::IsAnimationAFavourite(animDict, animName);
		const std::string hint = (!isFav ? "Add to" : "Remove from") + std::string(" favourites");
		if (Menu::bitController)
		{
			engine->AddInstructionalButton(INPUT_SCRIPT_RLEFT, hint, /*isKey=*/false);
			if (IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_SCRIPT_RLEFT))
			{
				if (!isFav) sub::AddAnimationToFavourites(animDict, animName);
				else        sub::RemoveAnimationFromFavourites(animDict, animName);
			}
		}
		else
		{
			engine->AddInstructionalButton(static_cast<int>(VirtualKey::B), hint, /*isKey=*/true);
			if (IsKeyJustUp(VirtualKey::B))
			{
				if (!isFav) sub::AddAnimationToFavourites(animDict, animName);
				else        sub::RemoveAnimationFromFavourites(animDict, animName);
			}
		}
	}
}

static void DrawStopAnimationRow()
{
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;
	if (engine->AddCheckbox("Stop Animation", true, Checkbox::CROSS, Checkbox::NONE))
	{
		sub::AnimationStopAnimationCallback();
	}
}

static void StepAnimFlag(bool forward)
{
	if (forward)
	{
		for (auto it = AnimFlag::vFlagNames.begin(); it != AnimFlag::vFlagNames.end(); ++it)
		{
			if (it->first == g_customAnimFlag)
			{
				++it;
				if (it != AnimFlag::vFlagNames.end())
					g_customAnimFlag = it->first;
				break;
			}
		}
	}
	else
	{
		for (auto it = AnimFlag::vFlagNames.rbegin(); it != AnimFlag::vFlagNames.rend(); ++it)
		{
			if (it->first == g_customAnimFlag)
			{
				++it;
				if (it != AnimFlag::vFlagNames.rend())
					g_customAnimFlag = it->first;
				break;
			}
		}
	}
}

static void DrawCustomAnimSettingsBlock()
{
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	{
		::Menu::InputResult res = engine->AddNumber("Blend-In Speed", g_customAnimSpeed, 2);
		if (res.rightPressed && g_customAnimSpeed < FLT_MAX) g_customAnimSpeed += 0.1f;
		else if (res.leftPressed && g_customAnimSpeed > 0)   g_customAnimSpeed -= 0.1f;
	}
	{
		::Menu::InputResult res = engine->AddNumber("Blend-Out Speed", g_customAnimSpeedMult, 2);
		if (res.rightPressed && g_customAnimSpeedMult < FLT_MAX) g_customAnimSpeedMult += 0.1f;
		else if (res.leftPressed && g_customAnimSpeedMult > -FLT_MAX) g_customAnimSpeedMult -= 0.1f;
	}
	{
		::Menu::InputResult res = engine->AddNumber("Duration (ms)",
			static_cast<double>(g_customAnimDuration), 0);
		if (res.rightPressed && g_customAnimDuration < INT_MAX) g_customAnimDuration += 100;
		else if (res.leftPressed && g_customAnimDuration > -1)   g_customAnimDuration -= 100;
	}
	{
		const std::vector<std::string> flagDisplay{ AnimFlag::vFlagNames[g_customAnimFlag] };
		::Menu::InputResult res = engine->AddTextList("Flag", 0, flagDisplay);
		if (res.rightPressed) StepAnimFlag(true);
		else if (res.leftPressed) StepAnimFlag(false);
	}
	{
		::Menu::InputResult res = engine->AddNumber("Playback Rate", g_CustomAnimRate, 2);
		if (res.rightPressed && g_CustomAnimRate < FLT_MAX) g_CustomAnimRate += 0.1f;
		else if (res.leftPressed && g_CustomAnimRate > 0)    g_CustomAnimRate -= 0.1f;
	}
	if (engine->AddCheckbox("Lock Position", g_customAnimLockPos,
		Checkbox::BOXTICK, Checkbox::BOXBLANK))
	{
		g_customAnimLockPos = !g_customAnimLockPos;
	}
}

void PedAnimationSubmenu::Draw()
{
	SET_PED_CAN_PLAY_AMBIENT_ANIMS(g_Ped1, TRUE);
	SET_PED_CAN_PLAY_AMBIENT_BASE_ANIMS(g_Ped1, TRUE);
	SET_PED_CAN_PLAY_GESTURE_ANIMS(g_Ped1, TRUE);
	SET_PED_CAN_PLAY_VISEME_ANIMS(g_Ped1, TRUE, TRUE);
	SET_PED_IS_IGNORED_BY_AUTO_OPEN_DOORS(g_Ped1, TRUE);

	DrawTitle();
	DrawStopAnimationRow();

	DrawAnimRow("Pole Dance", "mini@strip_club@pole_dance@pole_dance3", "pd_dance_03");
	DrawAnimRow("Hood Dance", "missfbi3_sniping", "dance_m_default");
	DrawAnimRow("Burning", "ragdoll@human", "on_fire");
	DrawAnimRow("Getting Stunned", "ragdoll@human", "electrocute");
	DrawAnimRow("Private Dance", "mini@strip_club@private_dance@part1", "priv_dance_p1");
	DrawAnimRow("The Rear Abundance", "rcmpaparazzo_2", "shag_loop_poppy");
	DrawAnimRow("The Invisible Man", "rcmpaparazzo_2", "shag_loop_a");
	DrawAnimRow("Push ups", "amb@world_human_push_ups@male@base", "base");
	DrawAnimRow("Sit ups", "amb@world_human_sit_ups@male@base", "base");
	DrawAnimRow("Wave Yo' Arms", "random@car_thief@victimpoints_ig_3", "arms_waving");
	DrawAnimRow("Give BJ to Driver", "mini@prostitutes@sexnorm_veh", "bj_loop_prostitute");
	DrawAnimRow("Pleasure Driver", "mini@prostitutes@sexnorm_veh", "sex_loop_prostitute");
	DrawAnimRow("Mime", "special_ped@mime@monologue_8@monologue_8a", "08_ig_1_wall_ba_0");
	DrawAnimRow("Mime 2", "special_ped@mime@monologue_7@monologue_7a", "11_ig_1_run_aw_0");
	DrawAnimRow("Throw", "switch@franklin@throw_cup", "throw_cup_loop");
	DrawAnimRow("Smoke Coughing", "timetable@gardener@smoking_joint", "idle_cough");
	DrawAnimRow("Chilling with Friends", "friends@laf@ig_1@base", "base");
	DrawAnimRow("They Think We Dumb", "timetable@ron@they_think_were_stupid", "they_think_were_stupid");
	DrawAnimRow("Come Here", "gestures@m@standing@fat", "gesture_come_here_hard");
	DrawAnimRow("No Way", "gestures@m@standing@fat", "gesture_no_way");
	DrawAnimRow("They're Gonna Kill Me", "random@bicycle_thief@ask_help", "my_dads_going_to_kill_me");
	DrawAnimRow("You Gotta Help Me", "random@bicycle_thief@ask_help", "please_man_you_gotta_help_me");
	DrawAnimRow("Sleep", "savecouch@", "t_sleep_loop_couch");
	DrawAnimRow("Sleep 2", "savem_default@", "m_sleep_r_loop");
	DrawAnimRow("Sleep 3", "timetable@tracy@sleep@", "idle_c");
	DrawAnimRow("Meditate", "rcmcollect_paperleadinout@", "meditiate_idle");
	DrawAnimRow("Fap", "switch@trevor@jerking_off", "trev_jerking_off_loop");
	DrawAnimRow("Yeah Yeah Yeah", "special_ped@jessie@michael_1@michael_1b", "jessie_ig_2_yeahyeahyeah_1");
	DrawAnimRow("Idle On Laptop", "switch@franklin@on_laptop", "001927_01_fras_v2_4_on_laptop_idle");
	DrawAnimRow("Hands Up", "random@arrests", "idle_2_hands_up");
	DrawAnimRow("Stand Still, Arms Spread", "mp_sleep", "bind_pose_180");

	if (DrawOption("Sitting Animations"))  NavigateTo("ped_animation_gesture_sit");
	if (DrawOption("Rappel Movements"))    NavigateTo("ped_animation_miss_rappel");
	if (DrawOption("Arrest Movements"))    NavigateTo("ped_animation_random_arrest");
	if (DrawOption("Swat Movements"))      NavigateTo("ped_animation_swat");
	if (DrawOption("Guard Movements"))     NavigateTo("ped_animation_guard_react");
	if (DrawOption("Deer Movements"))      { dict = "creatures@deer@move"; NavigateTo("ped_animation_deer"); }
	if (DrawOption("Cow Movements"))       { dict = "creatures@cow@move";  NavigateTo("ped_animation_deer"); }
	if (DrawOption("Shark Movements"))     NavigateTo("ped_animation_shark");
	if (DrawOption("All Animations"))      NavigateTo("ped_animation_all");
	if (DrawOption("Custom Input"))        NavigateTo("ped_animation_custom");
	if (DrawOption("Favourites"))          NavigateTo("ped_animation_favourites");
	if (DrawOption("Settings"))            NavigateTo("ped_animation_settings");
}

void AnimationSettingsSubmenu::Draw()
{
	DrawTitle();
	DrawCustomAnimSettingsBlock();
}

void AnimationFavouritesSubmenu::OnExit()
{
	searchStr.clear();
}

void AnimationFavouritesSubmenu::Draw()
{
	pugi::xml_document doc;
	if (doc.load_file((const char*)(GetPathffA(Pathff::Main, true)
		+ "FavouriteAnims.xml").c_str()).status != pugi::status_ok)
	{
		Game::Print::PrintBottomCentre("~r~Error~s~: No favourites found. Go to ~b~Custom Input~s~ and add an animation to the favourites.");
		::Menu::Engine* engine = ::Menu::Engine::Current();
		if (engine) engine->GoBack();
		return;
	}
	auto nodeAnims = doc.child("PedAnims");

	DrawTitle();

	if (DrawOption(searchStr.empty() ? "SEARCH" : ToUpperCopy(searchStr)))
	{
		searchStr = std::string(Game::InputBox(searchStr, 126U, "SEARCH", searchStr));
		searchStr = ToLowerCopy(searchStr);
	}

	for (auto nodeAnim = nodeAnims.first_child(); nodeAnim; nodeAnim = nodeAnim.next_sibling())
	{
		std::string animDict = nodeAnim.attribute("dict").as_string();
		std::string animName = nodeAnim.attribute("name").as_string();

		if (!searchStr.empty())
		{
			if (animDict.find(searchStr) == std::string::npos
				&& animName.find(searchStr) == std::string::npos)
			{
				continue;
			}
		}

		DrawAnimRow(animDict + ", " + animName, animDict, animName);
	}
}

void AnimationCustomSubmenu::Draw()
{
	DrawTitle();

	if (DrawOption(g_customAnimDict))
	{
		std::string inputStr = std::string(Game::InputBox(g_customAnimDict,
			126U, "Enter dict:", g_customAnimDict));
		if (!inputStr.empty())
			g_customAnimDict = inputStr;
		else
			Game::Print::PrintErrorInvalidInput(inputStr);
		return;
	}
	if (DrawOption(g_customAnimName))
	{
		std::string inputStr = std::string(Game::InputBox(g_customAnimName,
			126U, "Enter name:", g_customAnimName));
		if (!inputStr.empty())
			g_customAnimName = inputStr;
		else
			Game::Print::PrintErrorInvalidInput(inputStr);
		return;
	}
	if (DrawOption("Apply"))
	{
		PlayAnimationOnPlayerPed(g_customAnimDict, g_customAnimName);
		return;
	}
	if (DrawOption("Stop"))
	{
		sub::AnimationStopAnimationCallback();
		return;
	}

	const bool isFav = sub::IsAnimationAFavourite(g_customAnimDict, g_customAnimName);
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (engine && engine->AddCheckbox("Favourite", isFav,
		Checkbox::BOXTICK, Checkbox::BOXBLANK))
	{
		if (!isFav) sub::AddAnimationToFavourites(g_customAnimDict, g_customAnimName);
		else        sub::RemoveAnimationFromFavourites(g_customAnimDict, g_customAnimName);
	}

	DrawBreak("---Settings---");
	DrawCustomAnimSettingsBlock();
}

void DeerAnimationSubmenu::Draw()
{
	const std::string& tempDict = dict;

	DrawTitle();
	DrawAnimRow("Idle Turn Left",  tempDict, "idle_turn_l");
	DrawAnimRow("Idle Turn Right", tempDict, "idle_turn_r");
	DrawAnimRow("gallop_turn_l",   tempDict, "");
	DrawAnimRow("gallop_turn_r",   tempDict, "");
	DrawAnimRow("walk_turn_l",     tempDict, "");
	DrawAnimRow("walk_turn_r",     tempDict, "");
	DrawAnimRow("walk_start_0_l",  tempDict, "");
	DrawAnimRow("walk_start_0_r",  tempDict, "");
	DrawAnimRow("walk_start_90_l", tempDict, "");
	DrawAnimRow("walk_start_90_r", tempDict, "");
	DrawAnimRow("walk_backwards",  tempDict, "");
	DrawAnimRow("gallop",          tempDict, "");
	DrawAnimRow("canter",          tempDict, "");
	DrawAnimRow("trot",            tempDict, "");
	DrawAnimRow("dead_left",       tempDict, "");
	DrawAnimRow("dying",           tempDict, "");
}

void SharkAnimationSubmenu::Draw()
{
	const std::string tempDict = "creatures@shark@move";

	DrawTitle();
	DrawAnimRow("attack_player", tempDict, "attack_player");
	DrawAnimRow("swim_turn_r",   tempDict, "swim_turn_r");
	DrawAnimRow("swim_turn_l",   tempDict, "swim_turn_l");
	DrawAnimRow("dead_left",     tempDict, "dead_left");
	DrawAnimRow("dead_right",    tempDict, "dead_right");
	DrawAnimRow("dead_up",       tempDict, "dead_up");
	DrawAnimRow("dying",         tempDict, "dying");
	DrawAnimRow("attack_cam",    tempDict, "attack_cam");
	DrawAnimRow("attack",        tempDict, "attack");
	DrawAnimRow("attack_onspot", tempDict, "attack_onspot");
	DrawAnimRow("attack_jump",   tempDict, "attack_jump");
	DrawAnimRow("swim",          tempDict, "swim");
	DrawAnimRow("accelerate",    tempDict, "accelerate");
}

void MissionRappelAnimationSubmenu::Draw()
{
	const std::string tempDict = "missrappel";

	DrawTitle();
	DrawAnimRow("rappel_to_free_rope_prop", tempDict, "");
	DrawAnimRow("rope_slide",               tempDict, "");
	DrawAnimRow("rappel_jump_a_prop",       tempDict, "");
	DrawAnimRow("rappel_jump_c",            tempDict, "");
	DrawAnimRow("rappel_walk",              tempDict, "");
	DrawAnimRow("rappel_jump_c_prop",       tempDict, "");
	DrawAnimRow("rappel_loop",              tempDict, "");
	DrawAnimRow("rappel_to_free_rope",      tempDict, "");
	DrawAnimRow("rope_idle",                tempDict, "");
	DrawAnimRow("rappel_walk_prop",         tempDict, "");
	DrawAnimRow("rappel_idle",              tempDict, "");
	DrawAnimRow("rappel_idle_prop",         tempDict, "");
	DrawAnimRow("rappel_intro_player",      tempDict, "");
	DrawAnimRow("land_action",              tempDict, "");
	DrawAnimRow("land_crouched",            tempDict, "");
	DrawAnimRow("rappel_jump_a",            tempDict, "");
	DrawAnimRow("land",                     tempDict, "");
}

void GestureSitAnimationSubmenu::Draw()
{
	const std::string tempDict = "gestures@m@sitting@generic@casual";

	DrawTitle();
	DrawAnimRow("Sit n Shit", "timetable@trevor@on_the_toilet", "trevonlav_struggleloop");
	DrawAnimRow("Smoke Meth", "timetable@trevor@smoking_meth@base", "base");
	DrawAnimRow("Michael Exit Chair", "switch@michael@sitting", "exit_forward_chair");
	DrawAnimRow("Michael Exit Chair 2", "switch@michael@sitting", "exit_forward");
	DrawAnimRow("Michael Idle", "switch@michael@sitting", "idle_chair");
	DrawAnimRow("Michael Idle 2", "switch@michael@sitting", "idle");
	DrawAnimRow("Sitting On Car Bonnet", "switch@michael@sitting_on_car_bonnet", "sitting_on_car_bonnet_loop");
	DrawAnimRow("Sitting On Car Premiere", "switch@michael@sitting_on_car_premiere", "sitting_on_car_premiere_loop_player");
	DrawAnimRow("sitting_stungun_idk", "stungun@sitting", "damage_vehicle");
	DrawAnimRow("gesture_come_here_hard", tempDict, "");
	DrawAnimRow("gesture_come_here_soft", tempDict, "");
	DrawAnimRow("gesture_me_hard",        tempDict, "");
	DrawAnimRow("gesture_you_hard",       tempDict, "");
	DrawAnimRow("gesture_no_way",         tempDict, "");
	DrawAnimRow("gesture_why_left",       tempDict, "");
	DrawAnimRow("gesture_nod_no_hard",    tempDict, "");
	DrawAnimRow("gesture_hello",          tempDict, "");
	DrawAnimRow("gesture_i_will",         tempDict, "");
	DrawAnimRow("getsure_its_mine",       tempDict, "");
	DrawAnimRow("gesture_me",             tempDict, "");
	DrawAnimRow("gesture_you_soft",       tempDict, "");
	DrawAnimRow("gesture_what_hard",      tempDict, "");
	DrawAnimRow("gesture_pleased",        tempDict, "");
	DrawAnimRow("gesture_shrug_soft",     tempDict, "");
	DrawAnimRow("gesture_point",          tempDict, "");
	DrawAnimRow("gesture_shrug_hard",     tempDict, "");
	DrawAnimRow("gesture_why",            tempDict, "");
	DrawAnimRow("gesture_nod_no_soft",    tempDict, "");
	DrawAnimRow("gesture_nod_yes_hard",   tempDict, "");
	DrawAnimRow("gesture_what_soft",      tempDict, "");
	DrawAnimRow("gesture_nod_yes_soft",   tempDict, "");
	DrawAnimRow("gesture_damn",           tempDict, "");
	DrawAnimRow("gesture_displeased",     tempDict, "");
	DrawAnimRow("gesture_easy_now",       tempDict, "");
	DrawAnimRow("gesture_hand_down",      tempDict, "");
	DrawAnimRow("gesture_hand_left",      tempDict, "");
	DrawAnimRow("gesture_bring_it_on",    tempDict, "");
	DrawAnimRow("gesture_bye_hard",       tempDict, "");
	DrawAnimRow("gesture_bye_soft",       tempDict, "");
	DrawAnimRow("gesture_head_no",        tempDict, "");
	DrawAnimRow("gesture_hand_right",     tempDict, "");
}

void SwatAnimationSubmenu::Draw()
{
	const std::string tempDict = "swat";

	DrawTitle();
	DrawAnimRow("understood",  tempDict, "");
	DrawAnimRow("you_back",    tempDict, "");
	DrawAnimRow("rally_point", tempDict, "");
	DrawAnimRow("you_fwd",     tempDict, "");
	DrawAnimRow("you_left",    tempDict, "");
	DrawAnimRow("you_right",   tempDict, "");
	DrawAnimRow("freeze",      tempDict, "");
	DrawAnimRow("go_fwd",      tempDict, "");
	DrawAnimRow("come",        tempDict, "");
}

void GuardReactAnimationSubmenu::Draw()
{
	const std::string tempDict = "guard_reactions";

	DrawTitle();
	DrawAnimRow("1hand_fwd_fire_additive", tempDict, "");
	DrawAnimRow("1hand_turn0r",            tempDict, "");
	DrawAnimRow("1hand_turn90r",           tempDict, "");
	DrawAnimRow("1hand_turn180r",          tempDict, "");
	DrawAnimRow("1hand_turn0l",            tempDict, "");
	DrawAnimRow("1hand_turn90l",           tempDict, "");
	DrawAnimRow("1hand_turn180l",          tempDict, "");
	DrawAnimRow("1hand_aim_med_sweep",     tempDict, "");
	DrawAnimRow("1hand_aiming_cycle",      tempDict, "");
	DrawAnimRow("1hand_aiming_to_idle",    tempDict, "");
	DrawAnimRow("med_down",                tempDict, "");
	DrawAnimRow("1hand_aim_additive",      tempDict, "");
	DrawAnimRow("1hand_aim_high_sweep",    tempDict, "");
	DrawAnimRow("1hand_aim_low_sweep",     tempDict, "");
	DrawAnimRow("1hand_right_trans",       tempDict, "");
	DrawAnimRow("1hand_left_trans",        tempDict, "");
}

void RandomArrestAnimationSubmenu::Draw()
{
	const std::string tempDict = "random@arrests";

	DrawTitle();
	DrawAnimRow("kneeling_arrest_get_up",       tempDict, "");
	DrawAnimRow("generic_radio_enter",          tempDict, "");
	DrawAnimRow("kneeling_arrest_escape",       tempDict, "");
	DrawAnimRow("generic_radio_chatter",        tempDict, "");
	DrawAnimRow("cop_gunaimed_door_open_idle",  tempDict, "");
	DrawAnimRow("generic_radio_exit",           tempDict, "");
	DrawAnimRow("thanks_male_05",               tempDict, "");
	DrawAnimRow("radio_exit",                   tempDict, "");
	DrawAnimRow("radio_enter",                  tempDict, "");
	DrawAnimRow("radio_chatter",                tempDict, "");
	DrawAnimRow("kneeling_arrest_idle",         tempDict, "");
	DrawAnimRow("idle_c",                       tempDict, "");
	DrawAnimRow("idle_2_hands_up",              tempDict, "");
	DrawAnimRow("arrest_walk",                  tempDict, "");
	DrawAnimRow("idle_a", "random@arrests@busted", "idle_a");
	DrawAnimRow("idle_b", "random@arrests@busted", "idle_b");
	DrawAnimRow("enter",  "random@arrests@busted", "enter");
	DrawAnimRow("idle_c", "random@arrests@busted", "idle_c");
	DrawAnimRow("exit",   "random@arrests@busted", "exit");
}

void AllPedAnimsSubmenu::OnEnter()
{
	if (!loaded)
	{
		sub::AnimationMenu::PopulateAllPedAnimsList();
		loaded = true;
	}
}

void AllPedAnimsSubmenu::Draw()
{
	sub::AnimationMenu::selectedAnimDictPtr = nullptr;

	DrawTitle();

	if (DrawOption(searchStr.empty() ? "SEARCH" : ToUpperCopy(searchStr)))
	{
		searchStr = std::string(Game::InputBox(searchStr, 126U, "SEARCH", searchStr));
		searchStr = ToLowerCopy(searchStr);
	}

	DrawStopAnimationRow();

	for (auto& current : sub::AnimationMenu::allPedAnims)
	{
		if (current.second.empty()) continue;

		if (!searchStr.empty())
		{
			if (current.first.find(searchStr) == std::string::npos)
			{
				bool foundInValues = false;
				for (auto& current2 : current.second)
				{
					if (current2.find(searchStr) != std::string::npos)
					{
						foundInValues = true;
						break;
					}
				}
				if (!foundInValues) continue;
			}
		}

		if (DrawOption(current.first))
		{
			sub::AnimationMenu::selectedAnimDictPtr = &current;
			NavigateTo("ped_animation_all_in_dict");
		}
	}
}

void AllPedAnimsInDictSubmenu::Draw()
{
	if (sub::AnimationMenu::selectedAnimDictPtr == nullptr)
	{
		::Menu::Engine* engine = ::Menu::Engine::Current();
		if (engine) engine->GoBack();
		return;
	}
	auto& selectedDict = *sub::AnimationMenu::selectedAnimDictPtr;

	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (engine) engine->AddTitle(selectedDict.first);

	for (auto& current : selectedDict.second)
	{
		DrawAnimRow(current, selectedDict.first, current);
	}

	if (DrawOption("Settings")) NavigateTo("ped_animation_settings");
}

static void StopScenarioPls()
{
	GTAped ped = g_Ped1;
	GTAentity att;
	auto spi = sub::Spooner::EntityManagement::GetEntityIndexInDb(ped);
	if (spi >= 0)
	{
		auto& spe = sub::Spooner::Databases::EntityDb[spi];
		sub::Spooner::EntityManagement::GetEntityThisEntityIsAttachedTo(spe.handle, att);
	}

	GTAvehicle veh = ped.CurrentVehicle();
	const bool isInVehicle = veh.Exists();
	VehicleSeat vehSeat;
	if (isInVehicle) vehSeat = ped.GetCurrentVehicleSeat();
	ped.Task().ClearAllImmediately();
	if (isInVehicle) ped.SetIntoVehicle(veh, vehSeat);

	if (spi >= 0)
	{
		auto& spe = sub::Spooner::Databases::EntityDb[spi];
		spe.lastAnimation.dict.clear();
		spe.lastAnimation.name.clear();
		if (att.Exists() && spe.attachmentArgs.isAttached)
		{
			spe.handle.AttachTo(att, spe.attachmentArgs.boneIndex,
				spe.handle.GetIsCollisionEnabled(),
				spe.attachmentArgs.offset, spe.attachmentArgs.rotation);
		}
		spe.taskSequence.Reset();
		if (sub::Spooner::selectedEntity.handle.Equals(spe.handle))
		{
			sub::Spooner::selectedEntity.lastAnimation.dict = spe.lastAnimation.dict;
			sub::Spooner::selectedEntity.lastAnimation.name = spe.lastAnimation.name;
			sub::Spooner::selectedEntity.taskSequence = spe.taskSequence;
		}
	}
}

static void DrawScenarioRow(const std::string& text, const std::string& scenarioLabel,
	int delay = -1, bool playEnterAnim = true)
{
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	const bool isUsing = IS_PED_USING_SCENARIO(g_Ped1, scenarioLabel.c_str()) != 0;
	if (engine->AddCheckbox(text, isUsing, Checkbox::MANWON, Checkbox::NONE))
	{
		GTAped ped = g_Ped1;
		GTAentity att;
		auto spi = sub::Spooner::EntityManagement::GetEntityIndexInDb(ped);
		if (spi >= 0)
		{
			auto& spe = sub::Spooner::Databases::EntityDb[spi];
			sub::Spooner::EntityManagement::GetEntityThisEntityIsAttachedTo(spe.handle, att);
		}

		ped.Task().ClearAllImmediately();
		if (!ped.Task().IsUsingScenario(scenarioLabel))
		{
			TASK_START_SCENARIO_IN_PLACE(g_Ped1, scenarioLabel.c_str(), delay, playEnterAnim ? TRUE : FALSE);
		}

		if (scenarioLabel.find("MUSICIAN") != std::string::npos)
		{
			ped.Task().ClearAllImmediately();
			TASK_START_SCENARIO_IN_PLACE(g_Ped1, "WORLD_HUMAN_MUSICIAN", delay, playEnterAnim ? TRUE : FALSE);
		}

		if (spi >= 0)
		{
			auto& spe = sub::Spooner::Databases::EntityDb[spi];
			spe.lastAnimation.dict.clear();
			spe.lastAnimation.name = scenarioLabel;
			if (att.Exists() && spe.attachmentArgs.isAttached)
			{
				spe.handle.AttachTo(att, spe.attachmentArgs.boneIndex,
					spe.handle.GetIsCollisionEnabled(),
					spe.attachmentArgs.offset, spe.attachmentArgs.rotation);
			}
			spe.taskSequence.Reset();
			if (sub::Spooner::selectedEntity.handle.Equals(spe.handle))
			{
				sub::Spooner::selectedEntity.lastAnimation.dict = spe.lastAnimation.dict;
				sub::Spooner::selectedEntity.lastAnimation.name = spe.lastAnimation.name;
				sub::Spooner::selectedEntity.taskSequence = spe.taskSequence;
			}
		}
	}
}

void AnimationTaskScenarios1Submenu::Draw()
{
	DrawTitle();

	if (DrawOption("ALL SCENARIOS")) NavigateTo("ped_animation_task_scenarios2");

	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (engine && engine->AddCheckbox("End Scenarios", true, Checkbox::CROSS, Checkbox::NONE))
	{
		StopScenarioPls();
	}

	for (auto& scen : sub::AnimationTaskScenarios::vNamedScenarios)
	{
		DrawScenarioRow(scen.name, scen.label);
	}
}

void AnimationTaskScenarios2Submenu::OnExit()
{
	searchStr.clear();
}

void AnimationTaskScenarios2Submenu::Draw()
{
	DrawTitle();

	if (DrawOption(searchStr.empty() ? "SEARCH" : ToUpperCopy(searchStr)))
	{
		searchStr = std::string(Game::InputBox(searchStr, 126U, "SEARCH", searchStr));
		searchStr = ToLowerCopy(searchStr);
	}

	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (engine && engine->AddCheckbox("End Scenarios", true, Checkbox::CROSS, Checkbox::NONE))
	{
		StopScenarioPls();
	}

	for (auto& current : sub::AnimationTaskScenarios::vValues_TaskScenarios)
	{
		if (!searchStr.empty())
		{
			if (current.find(searchStr) == std::string::npos) continue;
		}
		DrawScenarioRow(current, current);
	}
}

static void DrawMoveGroupRow(const std::string& text, const std::string& moveGroupIn)
{
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;
	const std::string moveGroup = moveGroupIn.empty() ? text : moveGroupIn;
	const bool isActive = sub::GetPedMovementClipSet(g_Ped1) == moveGroup;
	if (engine->AddCheckbox(text, isActive))
	{
		sub::SetPedMovementClipSet(g_Ped1, moveGroup);
	}
}

static void DrawWMoveGroupRow(const std::string& text, const std::string& moveGroupIn)
{
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;
	const std::string moveGroup = moveGroupIn.empty() ? text : moveGroupIn;
	const bool isActive = sub::GetPedWeaponMovementClipSet(g_Ped1) == moveGroup;
	if (engine->AddCheckbox(text, isActive))
	{
		sub::SetPedWeaponMovementClipSet(g_Ped1, moveGroup);
	}
}

void MovementGroupSubmenu::Draw()
{
	auto mgit = g_pedListMovGroup.find(g_Ped1);
	const bool mgitIsValid = mgit != g_pedListMovGroup.end();

	auto wmgit = g_pedListWMovGroup.find(g_Ped1);
	const bool wmgitIsValid = mgit != g_pedListWMovGroup.end();

	bool movementGroupReset = false;
	bool movementGroupResetW = false;

	DrawTitle();

	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (engine && engine->AddCheckbox("Default", !mgitIsValid))
	{
		movementGroupReset = true;
	}

	DrawMoveGroupRow("Generic Male",        "move_m@generic");
	DrawMoveGroupRow("Generic Female",      "move_f@generic");
	DrawMoveGroupRow("Policeman",           "move_cop@action");
	DrawMoveGroupRow("Drunk",               "move_m@drunk@a");
	DrawMoveGroupRow("Moderate Drunk",      "move_m@drunk@moderatedrunk");
	DrawMoveGroupRow("Moderate Drunk 2",    "move_m@drunk@moderatedrunk_head_up");
	DrawMoveGroupRow("Slightly Drunk",      "move_m@drunk@slightlydrunk");
	DrawMoveGroupRow("Very Drunk",          "move_m@drunk@verydrunk");
	DrawMoveGroupRow("Gangster",            "move_m@gangster@generic");
	DrawMoveGroupRow("Hipster",             "move_m@hipster@a");
	DrawMoveGroupRow("Hobo",                "move_m@hobo@a");
	DrawMoveGroupRow("Hobo2",               "move_m@hobo@b");
	DrawMoveGroupRow("Obese",               "move_m@fat@a");
	DrawMoveGroupRow("Obese2",              "move_f@fat@a");
	DrawMoveGroupRow("Lester",              "move_lester_CaneUp");
	DrawMoveGroupRow("Film Female",         "move_f@film_reel");
	DrawMoveGroupRow("Cool Jog",            "move_m@jog@");
	DrawMoveGroupRow("Leaf Blower",         "move_m@leaf_blower");
	DrawMoveGroupRow("Tool Belt Walk",      "move_m@tool_belt@a");
	DrawMoveGroupRow("Tool Belt Walk 2",    "move_f@tool_belt@a");
	DrawMoveGroupRow("Appealing",           "move_f@sexy@a");
	DrawMoveGroupRow("Amanda - Bag",        "move_characters@amanda@bag");
	DrawMoveGroupRow("Michael - Fire",      "move_characters@michael@fire");
	DrawMoveGroupRow("Franklin - Fire",     "move_characters@franklin@fire");
	DrawMoveGroupRow("Jimmy - Nervous",     "move_characters@jimmy@nervous@");
	DrawMoveGroupRow("Jimmy - Slow",        "move_characters@jimmy@slow@");
	DrawMoveGroupRow("Alien",               "move_m@alien");
	DrawMoveGroupRow("Brave",               "move_m@brave");
	DrawMoveGroupRow("Brave2",              "move_m@brave@a");
	DrawMoveGroupRow("Brave3",              "move_m@brave@b");
	DrawMoveGroupRow("Brave4",              "move_m@brave@fallback");
	DrawMoveGroupRow("BraveStill",          "move_m@brave@idle_a");
	DrawMoveGroupRow("BraveStill2",         "move_m@brave@idle_b");
	DrawMoveGroupRow("Business",            "move_m@business@a");
	DrawMoveGroupRow("Business2",           "move_m@business@b");
	DrawMoveGroupRow("Business3",           "move_m@business@c");
	DrawMoveGroupRow("Casual",              "move_m@casual@a");
	DrawMoveGroupRow("Casual2",             "move_m@casual@b");
	DrawMoveGroupRow("Casual3",             "move_m@casual@c");
	DrawMoveGroupRow("Casual4",             "move_m@casual@d");
	DrawMoveGroupRow("Casual5",             "move_m@casual@e");
	DrawMoveGroupRow("Casual6",             "move_m@casual@f");
	DrawMoveGroupRow("Clipboard",           "move_m@clipboard");
	DrawMoveGroupRow("Coward",              "move_m@coward");
	DrawMoveGroupRow("Burning",             "move_m@fire");
	DrawMoveGroupRow("Flee",                "move_m@flee@a");
	DrawMoveGroupRow("Flee2",               "move_m@flee@b");
	DrawMoveGroupRow("Flee3",               "move_m@flee@c");
	DrawMoveGroupRow("Flee4",               "move_f@flee@a");
	DrawMoveGroupRow("Flee5",               "move_f@flee@b");
	DrawMoveGroupRow("Flee6",               "move_f@flee@c");
	DrawMoveGroupRow("Hiking",              "move_m@hiking");
	DrawMoveGroupRow("Hiking2",             "move_f@hiking");
	DrawMoveGroupRow("Hurry",               "move_m@hurry@a");
	DrawMoveGroupRow("Hurry2",              "move_m@hurry@b");
	DrawMoveGroupRow("Hurry3",              "move_m@hurry@c");
	DrawMoveGroupRow("Hurry4",              "move_f@hurry@a");
	DrawMoveGroupRow("Hurry5",              "move_f@hurry@b");
	DrawMoveGroupRow("Injured",             "move_m@injured");
	DrawMoveGroupRow("Injured2",            "move_injured_generic");
	DrawMoveGroupRow("Injured3",            "move_f@injured");
	DrawMoveGroupRow("Intimidation",        "move_m@intimidation@1h");
	DrawMoveGroupRow("Intimidation2",       "move_m@intimidation@cop@unarmed");
	DrawMoveGroupRow("Intimidation3",       "move_m@intimidation@unarmed");
	DrawMoveGroupRow("Muscular",            "move_m@muscle@a");
	DrawMoveGroupRow("Quick",               "move_m@quick");
	DrawMoveGroupRow("Sad",                 "move_m@sad@a");
	DrawMoveGroupRow("Sad2",                "move_m@sad@b");
	DrawMoveGroupRow("Sad3",                "move_m@sad@c");
	DrawMoveGroupRow("Sad4",                "move_f@sad@a");
	DrawMoveGroupRow("Sad5",                "move_f@sad@b");
	DrawMoveGroupRow("Shady",               "move_m@shadyped@a");
	DrawMoveGroupRow("Shocked",             "move_m@shocked@a");
	DrawMoveGroupRow("Arrogant",            "move_f@arrogant@a");
	DrawMoveGroupRow("Chubby",              "move_f@chubby@a");
	DrawMoveGroupRow("Handbag Walk",        "move_f@handbag");
	DrawMoveGroupRow("Heels",               "move_f@heels@c");
	DrawMoveGroupRow("move_p_m_one",            "");
	DrawMoveGroupRow("move_p_m_one_briefcase",  "");
	DrawMoveGroupRow("move_p_m_two",            "");
	DrawMoveGroupRow("move_p_m_zero",           "");
	DrawMoveGroupRow("move_p_m_zero_slow",      "");
	DrawMoveGroupRow("Ballistic",           "anim_group_move_ballistic");

	DrawBreak("Weapon Handling");
	if (engine && engine->AddCheckbox("Default", !wmgitIsValid))
	{
		movementGroupResetW = true;
	}
	DrawWMoveGroupRow("Lester's Cane",                    "move_lester_CaneUp");
	DrawWMoveGroupRow("Crouched",                         "move_ped_crouched");
	DrawWMoveGroupRow("Bucket",                           "move_ped_wpn_bucket");
	DrawWMoveGroupRow("Mop",                              "move_ped_wpn_mop");
	DrawWMoveGroupRow("Assault Rifle (Crouched)",         "Wpn_AssaultRifle_WeaponHoldingCrouched");
	DrawWMoveGroupRow("Garbageman",                       "missfbi4prepp1_garbageman");
	DrawWMoveGroupRow("Prison Guard",                     "MOVE_M@PRISON_GAURD");
	DrawWMoveGroupRow("Jerrycan (Generic)",               "move_ped_wpn_jerrycan_generic");
	DrawWMoveGroupRow("Golfer",                           "move_m@golfer@");
	DrawWMoveGroupRow("Rucksack",                         "MOVE_P_M_ZERO_RUCKSACK");
	DrawWMoveGroupRow("Clipboard",                        "MOVE_M@CLIPBOARD");
	DrawWMoveGroupRow("Tennis (Male)",                    "weapons@tennis@male");
	DrawWMoveGroupRow("Tennis Locomotion (Female)",       "TENNIS_LOCOMOTION_FEMALE");
	DrawWMoveGroupRow("Paparazzi (Standing)",             "random@escape_paparazzi@standing@");
	DrawWMoveGroupRow("Paparazzi (In Car)",               "random@escape_paparazzi@incar@");
	DrawWMoveGroupRow("Leaf Blower",                      "MOVE_M@LEAF_BLOWER");

	DrawBreak("Weapon Animations (Doesn't Save)");
	const std::vector<std::pair<std::string, Hash>> vWeaponAnimsOrWhatever
	{
		{ "Default",              0xE4DF46D5 },
		{ "Female",               0x6D155A1B },
		{ "MP Freemode Female",   0xACB10C83 },
		{ "Ballistic",            0x5534A626 },
		{ "Unknown 1",            0xc531a409 },
		{ "Unknown 2",            0x529e5780 },
	};
	for (auto& wa : vWeaponAnimsOrWhatever)
	{
		if (DrawOption(wa.first))
		{
			WEAPON::SET_WEAPON_ANIMATION_OVERRIDE(g_Ped1, wa.second);
		}
	}

	if (movementGroupReset)
	{
		RESET_PED_MOVEMENT_CLIPSET(g_Ped1, 0x3E800000);
		WAIT(10);
		Vector3 coord = GET_ENTITY_COORDS(g_Ped1, 1);
		SET_ENTITY_COORDS_NO_OFFSET(g_Ped1, coord.x, coord.y, coord.z + 0.05f, 1, 1, 0);
		FREEZE_ENTITY_POSITION(g_Ped1, 0);
		if (mgitIsValid) g_pedListMovGroup.erase(mgit);
	}
	if (movementGroupResetW)
	{
		RESET_PED_WEAPON_MOVEMENT_CLIPSET(g_Ped1);
		WAIT(10);
		Vector3 coord = GET_ENTITY_COORDS(g_Ped1, 1);
		SET_ENTITY_COORDS_NO_OFFSET(g_Ped1, coord.x, coord.y, coord.z + 0.05f, 1, 1, 0);
		FREEZE_ENTITY_POSITION(g_Ped1, 0);
		if (wmgitIsValid) g_pedListWMovGroup.erase(wmgit);
	}
}

void FacialMoodSubmenu::Draw()
{
	GTAentity ped = g_Ped1;
	auto current = GetPedFacialMood(ped);

	DrawTitle();

	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (engine && engine->AddCheckbox("Default", current.empty()))
	{
		ped.RequestControl(400);
		ClearPedFacialMood(ped);
	}

	for (auto& fa : sub::FacialAnims::vFacialAnims)
	{
		if (engine && engine->AddCheckbox(fa.caption, current == fa.animName))
		{
			ped.RequestControl(400);
			SetPedFacialMood(ped, fa.animName);
			current = fa.animName;
		}
	}
}

}
REGISTER_SUBMENU(::Menu::PedAnimationSubmenu)
REGISTER_SUBMENU(::Menu::AnimationSettingsSubmenu)
REGISTER_SUBMENU(::Menu::AnimationFavouritesSubmenu)
REGISTER_SUBMENU(::Menu::AnimationCustomSubmenu)
REGISTER_SUBMENU(::Menu::DeerAnimationSubmenu)
REGISTER_SUBMENU(::Menu::SharkAnimationSubmenu)
REGISTER_SUBMENU(::Menu::MissionRappelAnimationSubmenu)
REGISTER_SUBMENU(::Menu::GestureSitAnimationSubmenu)
REGISTER_SUBMENU(::Menu::SwatAnimationSubmenu)
REGISTER_SUBMENU(::Menu::GuardReactAnimationSubmenu)
REGISTER_SUBMENU(::Menu::RandomArrestAnimationSubmenu)
REGISTER_SUBMENU(::Menu::AllPedAnimsSubmenu)
REGISTER_SUBMENU(::Menu::AllPedAnimsInDictSubmenu)
REGISTER_SUBMENU(::Menu::AnimationTaskScenarios1Submenu)
REGISTER_SUBMENU(::Menu::AnimationTaskScenarios2Submenu)
REGISTER_SUBMENU(::Menu::MovementGroupSubmenu)
REGISTER_SUBMENU(::Menu::FacialMoodSubmenu)
