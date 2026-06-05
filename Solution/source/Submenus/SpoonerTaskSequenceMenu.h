#pragma once

#include "../Menu/Submenu.h"

#include <string>

namespace Menu {

class SpoonerTaskListSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_tasksequence_task_list"; }
	const char* Title() const override { return "Tasks"; }
	void Draw() override;
};

class SpoonerTaskAddTaskSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_tasksequence_add_task"; }
	const char* Title() const override { return "Add Task"; }
	void Draw() override;
};

class SpoonerTaskInTaskSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_tasksequence_in_task"; }
	const char* Title() const override { return "Task"; }
	void Draw() override;
};

class SpoonerTaskScenarioActionListSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_tasksequence_scenario_action_list"; }
	const char* Title() const override { return "All Actions"; }
	void Draw() override;
	void OnExit() override;

private:
	std::string searchStr;
};

class SpoonerTaskPlayAnimationSettingsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_tasksequence_play_animation_settings"; }
	const char* Title() const override { return "Settings"; }
	void Draw() override;
};

class SpoonerTaskPlayAnimationAllPedAnimsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_tasksequence_play_animation_all_ped_anims"; }
	const char* Title() const override { return "All Animations"; }
	void Draw() override;
	void OnEnter() override;
	void OnExit() override;

private:
	std::string searchStr;
	bool loaded = false;
};

class SpoonerTaskPlayAnimationAllPedAnimsInDictSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_tasksequence_play_animation_all_ped_anims_in_dict"; }
	const char* Title() const override { return "Dictionary"; }
	void Draw() override;
};

class SpoonerTaskPlaySpeechWithVoiceInVoiceSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "spooner_tasksequence_play_speech_with_voice_in_voice"; }
	const char* Title() const override { return "Voice"; }
	void Draw() override;
};

}