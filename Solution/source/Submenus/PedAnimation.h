#pragma once

#include "../Menu/Submenu.h"

#include <string>
#include <utility>

namespace Menu {

class PedAnimationSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_animation"; }
	const char* Title() const override { return "Animations"; }
	void Draw() override;
};

class AnimationSettingsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_animation_settings"; }
	const char* Title() const override { return "Settings"; }
	void Draw() override;
};

class AnimationFavouritesSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_animation_favourites"; }
	const char* Title() const override { return "Favourites"; }
	void Draw() override;
	void OnExit() override;

private:
	std::string searchStr;
};

class AnimationCustomSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_animation_custom"; }
	const char* Title() const override { return "Custom Animation"; }
	void Draw() override;
};

class DeerAnimationSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_animation_deer"; }
	const char* Title() const override { return "Animalations"; }
	void Draw() override;
};

class SharkAnimationSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_animation_shark"; }
	const char* Title() const override { return "Animalations"; }
	void Draw() override;
};

class MissionRappelAnimationSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_animation_miss_rappel"; }
	const char* Title() const override { return "Swat Animations"; }
	void Draw() override;
};

class GestureSitAnimationSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_animation_gesture_sit"; }
	const char* Title() const override { return "Sitting Animations"; }
	void Draw() override;
};

class SwatAnimationSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_animation_swat"; }
	const char* Title() const override { return "Swat Animations"; }
	void Draw() override;
};

class GuardReactAnimationSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_animation_guard_react"; }
	const char* Title() const override { return "Guard Animations"; }
	void Draw() override;
};

class RandomArrestAnimationSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_animation_random_arrest"; }
	const char* Title() const override { return "Arrest Animations"; }
	void Draw() override;
};

class AllPedAnimsSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_animation_all"; }
	const char* Title() const override { return "All Animations"; }
	void Draw() override;
	void OnEnter() override;

private:
	std::string searchStr;
	bool loaded = false;
};

class AllPedAnimsInDictSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_animation_all_in_dict"; }
	const char* Title() const override { return "Dictionary"; }
	void Draw() override;
};

class AnimationTaskScenarios1Submenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_animation_task_scenarios"; }
	const char* Title() const override { return "Scenarios"; }
	void Draw() override;
};

class AnimationTaskScenarios2Submenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_animation_task_scenarios2"; }
	const char* Title() const override { return "All Scenarios"; }
	void Draw() override;
	void OnExit() override;

private:
	std::string searchStr;
};

class MovementGroupSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_animation_movement_group"; }
	const char* Title() const override { return "Movement Styles"; }
	void Draw() override;
};

class FacialMoodSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_animation_facial_mood"; }
	const char* Title() const override { return "Mood"; }
	void Draw() override;
};

}