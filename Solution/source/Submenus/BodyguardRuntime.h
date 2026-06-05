#pragma once

#include <vector>
#include <string>

#include "../Scripting/GTAentity.h"
#include "../Natives/types.h"
#include "../Menu/Routine.h"

using TargetPed = int;

namespace sub::BodyguardMenu
{
    class BodyguardEntity
    {
    public:
        EntityType Type{};
        std::string Name;
        std::string HashName;
        GTAentity Handle;
    };

    extern std::vector<BodyguardEntity> BodyguardDb;

    // Currently selected bodyguard in the menu
    extern BodyguardEntity* SelectedBodyguard;

    bool operator==(const BodyguardEntity& a, const BodyguardEntity& b);
    bool operator!=(const BodyguardEntity& a, const BodyguardEntity& b);

    namespace BodyguardManagement
    {
        constexpr size_t MAX_BODYGUARDS = 7;
        extern std::vector<int> s_bodyguards;

        unsigned int GetNumberOfBodyguardsSpawned(const EntityType&);
        int GetBodyguardIndexInDb(const GTAentity&);
        int GetBodyguardIndexInDb(const BodyguardEntity&);
        void RemoveBodyguardFromDb(const BodyguardEntity&);
        void DeleteBodyguard(BodyguardEntity&);
        void AddBodyguardToDb(BodyguardEntity ent);

        // Draws an arrow above the specified entity to highlight it in the world
        void ShowArrowAboveEntity(const GTAentity& ent, RGBA colour = {255, 0, 0, 190});
    }

    extern int armor;
    extern int health;
    extern bool godmode;
    extern int blipIcon;

    void ApplyBodyguardBlip(Ped ped, int icon);
    void RemoveBodyguardBlip(Ped ped);
    void RefreshAllBodyguardBlips();
    void UpdateBodyguardBlipsOnDeath();
}
