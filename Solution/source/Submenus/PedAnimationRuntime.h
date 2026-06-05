#pragma once

#include <vector>
#include <string>
#include <map>

class GTAentity;

namespace sub
{
	namespace AnimationMenu
	{
		struct NamedAnimation { std::string caption; std::string animDict, animName; };
		extern const std::vector<AnimationMenu::NamedAnimation> presetPedAnims;
		extern std::map<std::string, std::vector<std::string>> allPedAnims;
		extern std::pair<const std::string, std::vector<std::string>>* selectedAnimDictPtr;

		void PopulateAllPedAnimsList();
	}

	void GetFavouriteAnimations(std::vector<std::pair<std::string, std::string>>& result);
	bool IsAnimationAFavourite(const std::string animDict, const std::string& animName);
	void AddAnimationToFavourites(const std::string animDict, const std::string& animName);
	void RemoveAnimationFromFavourites(const std::string animDict, const std::string& animName);
	void AnimationStopAnimationCallback();

	namespace AnimationTaskScenarios
	{
		extern std::vector<std::string> vValues_TaskScenarios;
		struct NamedScenario { std::string name; std::string label; };
		extern std::vector<NamedScenario> vNamedScenarios;
	}

	std::string GetPedMovementClipSet(const GTAentity& ped);
	void SetPedMovementClipSet(GTAentity ped, const std::string& setName);
	std::string GetPedWeaponMovementClipSet(const GTAentity& ped);
	void SetPedWeaponMovementClipSet(GTAentity ped, const std::string& setName);

	namespace FacialAnims
	{
		struct NamedFacialAnim { std::string caption; std::string animName; };
		extern const std::vector<NamedFacialAnim> vFacialAnims;
	}

}

typedef int Ped;

extern std::map<Ped, std::string> g_pedListMovGroup;
extern std::map<Ped, std::string> g_pedListWMovGroup;
extern std::map<Ped, std::string> g_pedListFacialMood;

std::string GetPedFacialMood(GTAentity ped);
void SetPedFacialMood(GTAentity ped, const std::string& animName);
void ClearPedFacialMood(GTAentity ped);
