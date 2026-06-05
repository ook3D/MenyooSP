#include "TeleportArenaWar.h"

#include "../Menu/Engine.h"
#include "../Menu/SubmenuRegistry.h"

#include "Teleport/TeleMethods.h"

namespace Menu {

using sub::TeleportLocations_catind::TeleLocation;
using sub::TeleportLocations_catind::TeleMethods::ToTeleLocation241;

const std::vector<TeleLocation>
TeleportArenaWarSubmenu::vOtherArenaWarRelatedTeleports
{
	TeleLocation("xs_arena_interior",       2800.0000f, -3800.0000f, 100.0000f, { "xs_arena_interior"_sv },       {}, {}, true, false, true),
	TeleLocation("xs_arena_interior_mod",    205.0000f,  5180.0000f, -90.0000f, { "xs_arena_interior_mod"_sv },   {}, {}, true, false, true),
	TeleLocation("xs_arena_interior_mod_2",  170.0000f,  5190.0000f,  10.0000f, { "xs_arena_interior_mod"_sv },   {}, {}, true, false, true),
	TeleLocation("xs_arena_interior_vip",   2799.5290f, -3930.5390f, 182.3500f, { "xs_arena_interior_vip"_sv },   {}, {}, true, false, true),
};

void TeleportArenaWarSubmenu::Draw()
{
	DrawTitle();

	for (auto& otherTele : vOtherArenaWarRelatedTeleports)
	{
		if (DrawOption(otherTele.name))
		{
			ToTeleLocation241(otherTele);
		}
	}
}

}
REGISTER_SUBMENU(::Menu::TeleportArenaWarSubmenu)
