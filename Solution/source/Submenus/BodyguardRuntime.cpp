#include "BodyguardRuntime.h"

#include "../Menu/Menu.h"
#include "../Menu/Routine.h"
#include "PlayerRuntime.h"

#include "../Scripting/Game.h"
#include "../Scripting/GTAped.h"
#include "../Scripting/GTAentity.h"
#include "../Scripting/GTAblip.h"
#include "../Scripting/GTAvehicle.h"
#include "../Scripting/Model.h"
#include "../Scripting/ModelNames.h"
#include "../Scripting/World.h"

#include "../Natives/natives.h"
#include "../Natives/natives2.h"

#include "../Util/StringManip.h"
#include "../Util/ExePath.h"

#include "../Misc/MeteorShower.h"

#include "Spooner/SpoonerShared.h"
#include "Spooner/EntityManagement.h"
#include "Spooner/SpoonerEntity.h"
#include "PedModelChanger.h"
#include "PedAnimationRuntime.h"
#include "PedSpeech.h"

#include <algorithm>
#include <functional>

using namespace sub::BodyguardMenu;

namespace sub::BodyguardMenu
{
	bool operator==(const BodyguardEntity& left, const BodyguardEntity& right)
	{
		return left.Handle == right.Handle;
	}
	bool operator!=(const BodyguardEntity& left, const BodyguardEntity& right)
	{
		return !(left == right);
	}

	std::vector<BodyguardEntity> BodyguardDb;

	// Currently selected bodyguard in the menu
	BodyguardEntity* SelectedBodyguard = nullptr;

	namespace BodyguardManagement
	{
		std::vector<int> s_bodyguards;
	}
}

namespace sub::BodyguardMenu
{
	namespace BodyguardManagement
	{
		unsigned int GetNumberOfBodyguardsSpawned(const EntityType& type)
		{
			switch (type)
			{
			case EntityType::ALL:
				return static_cast<unsigned int>(BodyguardDb.size());
			default:
				return static_cast<unsigned int>(std::count_if(
					BodyguardDb.begin(),
					BodyguardDb.end(),
					[type](const BodyguardEntity& item)
					{
						return item.Type == type;
					}));
			}
		}

		int GetBodyguardIndexInDb(const GTAentity& entity)
		{
			for (int i = 0; i < static_cast<int>(BodyguardDb.size()); ++i)
			{
				if (BodyguardDb[i].Handle == entity)
					return i;
			}
			return -1;
		}

		int GetBodyguardIndexInDb(const BodyguardEntity& ent)
		{
			return GetBodyguardIndexInDb(ent.Handle);
		}

		void AddBodyguardToDb(BodyguardEntity ent)
		{
			if (!ent.Handle.Exists())
				return;

			if (ent.HashName.empty())
				ent.HashName = IntToHexString(ent.Handle.Model().hash, true);

			BodyguardDb.push_back(std::move(ent));
		}

		void RemoveBodyguardFromDb(const BodyguardEntity& ent)
		{
			auto it = std::remove_if(
				BodyguardDb.begin(),
				BodyguardDb.end(),
				[&](const BodyguardEntity& e)
				{
					return e.Handle.GetHandle() == ent.Handle.GetHandle();
				}
			);

			if (it != BodyguardDb.end())
				BodyguardDb.erase(it, BodyguardDb.end());
		}

		void DeleteBodyguard(BodyguardEntity& ent)
		{
			if (!ent.Handle.Exists())
				return;

			Ped ped = ent.Handle.GetHandle();

			ent.Handle.RequestControl();

			GTAblip blip = ent.Handle.CurrentBlip();
			if (blip.Exists())
				blip.Remove();

			ent.Handle.Detach();

			ent.Handle.SetMissionEntity(false);

			if (ped && ENTITY::DOES_ENTITY_EXIST(ped))
			{
				PED::DELETE_PED(&ped);
			}

			ent.Handle = GTAped();

			RemoveBodyguardFromDb(ent);
		}

		void ShowArrowAboveEntity(const GTAentity& ent, RGBA colour)
		{
			if (ent.Exists())
			{
				const auto& soe_pos = ent.GetPosition();
				const auto& soe_md = ent.ModelDimensions();
				const auto& markerPos = soe_pos + Vector3(0, 0, (std::max)(soe_md.Dim1.z, soe_md.Dim2.z) + 0.20f); // May not be at the right position if the entity is tilted
				World::DrawMarker(MarkerType::UpsideDownCone, markerPos, Vector3(), Vector3(), Vector3(0.45f, 0.45f, 0.50f), RGBA(190, 0, 0, 190));
			}


		}
	}
}

namespace sub::BodyguardMenu
{
    int armor = 200;
    int health = 200;
    bool godmode = true;
    int blipIcon = 1; // 1 = Standard, 280 = Friend, 480 = VIP
}

static constexpr int BLIP_COLOUR_BLUELIGHT = 3;

void sub::BodyguardMenu::RemoveBodyguardBlip(Ped ped)
{
    if (!ped || !ENTITY::DOES_ENTITY_EXIST(ped))
        return;

    Blip blip = GET_BLIP_FROM_ENTITY(ped);
    if (!blip)
        return;

    GTAblip gtaBlip(blip);
    if (gtaBlip.Exists())
        gtaBlip.Remove();
}

void sub::BodyguardMenu::ApplyBodyguardBlip(Ped ped, int icon)
{
    if (!ped || !ENTITY::DOES_ENTITY_EXIST(ped))
        return;

    RemoveBodyguardBlip(ped);

    Blip blip = ADD_BLIP_FOR_ENTITY(ped);
    if (!blip)
        return;

    SET_BLIP_SPRITE(blip, icon);
    SET_BLIP_SCALE(blip, 0.80f);
    SET_BLIP_COLOUR(blip, BLIP_COLOUR_BLUELIGHT);
    SET_BLIP_AS_FRIENDLY(blip, true);
}

void sub::BodyguardMenu::RefreshAllBodyguardBlips()
{
    for (unsigned int i = 0; i < sub::BodyguardMenu::BodyguardDb.size(); ++i)
    {
        auto& bg = sub::BodyguardMenu::BodyguardDb[i];
        if (!bg.Handle.Exists())
            continue;

        Ped ped = bg.Handle.GetHandle();
        if (!ped || !ENTITY::DOES_ENTITY_EXIST(ped))
            continue;

        int hp = ENTITY::GET_ENTITY_HEALTH(ped);
        if (hp <= 0)
            ApplyBodyguardBlip(ped, 274); // dead blip
        else
            ApplyBodyguardBlip(ped, sub::BodyguardMenu::blipIcon);
    }
}

void sub::BodyguardMenu::UpdateBodyguardBlipsOnDeath()
{
    for (unsigned int i = 0; i < sub::BodyguardMenu::BodyguardDb.size(); ++i)
    {
        auto& bg = sub::BodyguardMenu::BodyguardDb[i];
        if (!bg.Handle.Exists())
            continue;

        Ped ped = bg.Handle.GetHandle();
        if (!ped || !ENTITY::DOES_ENTITY_EXIST(ped))
            continue;

        int hp = ENTITY::GET_ENTITY_HEALTH(ped);
        if (hp <= 0)
        {
            Blip blip = GET_BLIP_FROM_ENTITY(ped);
            if (!blip || GET_BLIP_SPRITE(blip) != 274)
                ApplyBodyguardBlip(ped, 274);
            RefreshAllBodyguardBlips();
        }
    }
    void sub::BodyguardMenu::RefreshAllBodyguardBlips();
    {
        for (auto& bg : BodyguardDb)
        {
            if (!bg.Handle.Exists())
                continue;

            Ped ped = bg.Handle.GetHandle();
            if (!ped || !ENTITY::DOES_ENTITY_EXIST(ped))
                continue;

            Blip blip = GET_BLIP_FROM_ENTITY(ped);
            if (blip)
            {
                SET_BLIP_SCALE(blip, 0.8f);
                SET_BLIP_AS_SHORT_RANGE(blip, true);
            }
        }
    }
}
