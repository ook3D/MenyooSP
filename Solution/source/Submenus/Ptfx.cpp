
#include "Ptfx.h"

#include "../Menu/Engine.h"
#include "../Menu/SubmenuRegistry.h"
#include "../Menu/Menu.h"
#include "../Menu/Routine.h"
#include "../Natives/natives2.h"
#include "../Util/keyboard.h"

#include "PtfxData.h"
#include "PlayerRuntime.h"
#include "WeaponRuntime.h"
#include "../Scripting/PTFX.h"

#include <string>
#include <vector>

namespace Menu {

namespace {
PtfxSource g_ptfxSource = PtfxSource::LoopOnEntity;
int        g_ptfxPage = 0;
bool       g_ptfxShowOnlyFaves = false;

constexpr int kItemsPerPageMin = 10;
constexpr int kItemsPerPageMax = 100;
}

void SetPtfxSource(PtfxSource source) { g_ptfxSource = source; }
PtfxSource GetPtfxSource()             { return g_ptfxSource; }

static void DrawRowForSource(::Menu::Submenu& self, const sub::PtfxSubs::PtfxS& effect)
{
	(void)self;
	switch (g_ptfxSource)
	{
	case PtfxSource::OneShotOnPed:
		if (Engine* e = Engine::Current())
		{
			if (e->AddOption(effect.name))
				sub::PtfxSubs::FireOneShotOnEntity(effect, g_Ped1);
		}
		break;

	case PtfxSource::LoopOnEntity:
	{
		bool tickOn = false;
		bool foundEntity = false;
		for (const auto& fxlop : sub::PtfxSubs::fxLoops)
		{
			if (fxlop.entity == g_Ped1)
			{
				foundEntity = true;
				if (fxlop.asset == effect.asset && fxlop.fx == effect.fx)
				{
					tickOn = true;
					break;
				}
			}
		}
		if (!foundEntity)
		{
			tickOn = effect.asset.empty() && effect.fx.empty();
		}

		if (Engine* e = Engine::Current())
		{
			if (e->AddCheckbox(effect.name, tickOn))
				sub::PtfxSubs::AddEntityToPtfxLops(effect, g_Ped1);
		}
		break;
	}

	case PtfxSource::TriggerFxGun:
	{
		bool tickOn = (triggerFXGunData.asset == effect.asset && triggerFXGunData.effect == effect.fx);
		if (Engine* e = Engine::Current())
		{
			if (e->AddCheckbox(effect.name, tickOn))
			{
				triggerFXGunData.asset = effect.asset;
				triggerFXGunData.effect = effect.fx;
			}
		}
		break;
	}
	}
}

void PtfxSubmenu::Draw()
{
	sub::PtfxSubs::LoadFavouritesFromFileOnce();

	// Build the displayed list: filter by favourites if asked.
	std::vector<sub::PtfxSubs::PtfxS> displayed;
	if (g_ptfxShowOnlyFaves)
	{
		for (const auto& fx : sub::PtfxSubs::PTFX)
		{
			if (sub::PtfxSubs::IsAlreadyFavorite(fx))
				displayed.push_back(fx);
		}
	}
	else
	{
		displayed = sub::PtfxSubs::PTFX;
	}

	const int totalItems = static_cast<int>(displayed.size());
	const int itemsPerPage = sub::PtfxSubs::ITEMS_PER_PAGE;
	const int totalPages = (totalItems + itemsPerPage - 1) / itemsPerPage;
	const int safePages = totalPages > 0 ? totalPages : 1;
	if (g_ptfxPage >= safePages) g_ptfxPage = 0;
	const int startIndex = g_ptfxPage * itemsPerPage;
	int endIndex = startIndex + itemsPerPage;
	if (endIndex > totalItems) endIndex = totalItems;

	const std::string pageLabel = "Page " + std::to_string(g_ptfxPage + 1)
		+ " / " + std::to_string(safePages);

	DrawTitle();
	DrawBreak(pageLabel);

	if (DrawNumber("Items Per Page", sub::PtfxSubs::ITEMS_PER_PAGE, 1, kItemsPerPageMin, kItemsPerPageMax))
	{
		// Clamp already applied by DrawNumber.
	}

	if (DrawOption("Favourites"))
	{
		NavigateTo("ptfx_favourites");
	}

	DrawToggle("Only Show Favorites", g_ptfxShowOnlyFaves);

	// Source-specific header rows.
	if (g_ptfxSource == PtfxSource::LoopOnEntity)
	{
		if (DrawOption("Clear On All Entities"))
		{
			sub::PtfxSubs::fxLoops.clear();
		}
		sub::PtfxSubs::PtfxS noneFx{ "None", "", "" };
		DrawRowForSource(*this, noneFx);
	}
	else if (g_ptfxSource == PtfxSource::OneShotOnPed)
	{
		if (DrawOption("Loop On Entity"))
		{
			SetPtfxSource(PtfxSource::LoopOnEntity);
			// re-enter same submenu with new source semantics.
		}
	}

	// Page rows + per-row favourite hotkey.
	Engine* engine = Engine::Current();
	for (int i = startIndex; i < endIndex; ++i)
	{
		const auto& current = displayed[i];
		DrawRowForSource(*this, current);

		if (engine && IsCurrentRowSelected())
		{
			const std::string label = sub::PtfxSubs::IsAlreadyFavorite(current)
				? "Remove From Favourites" : "Add To Favourites";
			bool pressed = false;
			if (Menu::bitController)
			{
				engine->AddInstructionalButton(INPUT_SCRIPT_RLEFT, label, /*isKey=*/false);
				pressed = IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_SCRIPT_RLEFT) != 0;
			}
			else
			{
				engine->AddInstructionalButton(static_cast<int>(VirtualKey::B), label, /*isKey=*/true);
				pressed = IsKeyJustUp(VirtualKey::B);
			}
			if (pressed)
				sub::PtfxSubs::ToggleFavorite(current);
		}
	}

	if (totalPages > 1)
	{
		DrawBreak(pageLabel);
		if (DrawOption("Previous Page"))
		{
			g_ptfxPage = (g_ptfxPage - 1 + totalPages) % totalPages;
		}
		if (DrawOption("Next Page"))
		{
			g_ptfxPage = (g_ptfxPage + 1) % totalPages;
		}
	}
}

void PtfxFavouritesSubmenu::Draw()
{
	DrawTitle();

	if (sub::PtfxSubs::favourites.empty())
	{
		DrawOption("Nothing yet has been saved here..");
		return;
	}

	for (const auto& fx : sub::PtfxSubs::favourites)
	{
		DrawRowForSource(*this, fx);
	}
}

}
REGISTER_SUBMENU(::Menu::PtfxSubmenu)
REGISTER_SUBMENU(::Menu::PtfxFavouritesSubmenu)
