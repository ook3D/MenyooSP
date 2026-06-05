#include "Engine.h"
#include "Submenu.h"
#include "SubmenuRegistry.h"

#include "Menu.h"
#include "Language.h"

#include "../macros.h"
#include "../Natives/natives2.h"
#include "../Scripting/Game.h"
#include "../Scripting/Scaleform.h"
#include "../Scripting/ModelNames.h"
#include "../Util/keyboard.h"

namespace Menu
{
	Engine* Engine::s_current = nullptr;
	Engine* Engine::Current() { return s_current; }

	void Engine::BeginFrame()
	{
		printingOption = 0;
		breakCount = 0;
		optionY = 0.0f;
		acceptScratch = false;
		numberSelected = false;
		pendingNavId.clear();
		mouseHitList.clear();
		ibList.clear();
	}

	void Engine::DrawActiveSubmenu(Submenu& sub)
	{
		Engine* prev = s_current;
		s_current = this;
		sub.Draw();
		s_current = prev;
	}

	void Engine::EndFrame()
	{
		totalOptions = printingOption;
		if (currentOption < 1) currentOption = totalOptions > 0 ? totalOptions : 1;
		if (currentOption > totalOptions && totalOptions > 0) currentOption = 1;
	}

	void Engine::RequestNavigate(std::string_view id)
	{
		pendingNavId.assign(id);
	}

	static float XCoordAtMenuRightEdge(const Theme& theme, float widthOfElement,
		float extraWidth, bool centered)
	{
		if (centered)
		{
			return (0.16f + theme.position.x + 0.1f) - 0.002f - extraWidth - (widthOfElement / 2);
		}
		return (0.16f + theme.position.x + 0.1f) - 0.002f - extraWidth - (widthOfElement);
	}

	static inline void EngineUp(Engine& e)
	{
		if (e.currentOption <= 1) e.currentOption = e.totalOptions > 0 ? e.totalOptions : 1;
		else --e.currentOption;
	}
	static inline void EngineDown(Engine& e)
	{
		if (e.currentOption >= e.totalOptions) e.currentOption = 1;
		else ++e.currentOption;
	}
	static inline void EngineTop(Engine& e)    { e.currentOption = 1; }
	static inline void EngineBottom(Engine& e) { e.currentOption = e.totalOptions > 0 ? e.totalOptions : 1; }

	int Engine::ComputeRowSlot() const
	{
		int slot = 0;
		if ((currentOption < GTA_SCROLLOP && printingOption <= GTA_MAXOP) || totalOptions <= GTA_MAXOP)
		{
			slot = printingOption;
		}
		else if (currentOption >= GTA_SCROLLOP)
		{
			if (currentOption > (totalOptions - GTA_BETOP))
			{
				slot = GTA_SCROLLOP + (GTA_BETOP - (totalOptions - printingOption));
			}
			else
			{
				slot = GTA_SCROLLOP + (printingOption - currentOption);
			}
		}
		return slot;
	}

	void Engine::AddTitle(const std::string& text)
	{
		std::string translated = Language::TranslateToSelected(text);

		if (theme.titleAlphaDis)
		{
			Game::Print::SetupDraw(theme.fontTitle, Vector2(0.26, 0.26), true, false, false, theme.colorTitleText);
			Game::Print::drawstringGXT(translated, 0.16f + theme.position.x, 0.1406f + theme.position.y);
			return;
		}

		Game::Print::setupdraw();
		const RGBA& titleCol = theme.colorTitleText;
		if (titleCol.A > 0 && titleCol.R > 240 && titleCol.G > 240 && titleCol.B > 240)
		{
			SET_TEXT_OUTLINE();
		}
		SET_TEXT_FONT(theme.fontTitle);
		SET_TEXT_COLOUR(titleCol.R, titleCol.G, titleCol.B, titleCol.A);

		float titleX;
		if (theme.centreTitle)
		{
			SET_TEXT_CENTRE(1);
			titleX = 0.16f;
		}
		else
		{
			titleX = 0.066f;
		}

		const auto length = translated.length();
		float offset = 0.0f;

		if (length < 15)
		{
			SET_TEXT_SCALE(0.75f, 0.75f);
		}
		else if (length < 19)
		{
			SET_TEXT_SCALE(0.62f, 0.62f);
			offset = 0.006f;
		}
		else if (length < 23)
		{
			SET_TEXT_SCALE(0.51f, 0.51f);
			offset = 0.011f;
		}
		else
		{
			offset = 0.015f;
		}

		Game::Print::drawstringGXT(translated, titleX + theme.position.x, 0.1f + offset + theme.position.y);
	}

	bool Engine::AddOption(const std::string& text, bool showArrow, bool gxt)
	{
		++printingOption;

		const int slot = ComputeRowSlot();
		if (slot > GTA_MAXOP || slot <= 0)
		{
			optionY = 0.0f;
			acceptScratch = false;
			return false;
		}

		mouseHitList.push_back({ printingOption, slot });

		optionY = slot * 0.035f + 0.125f;

		Game::Print::setupdraw();
		if (theme.fontOptions == 0)
		{
			SET_TEXT_SCALE(0, 0.33f);
		}
		SET_TEXT_FONT(theme.fontOptions);
		SET_TEXT_COLOUR(theme.colorOption.R, theme.colorOption.G, theme.colorOption.B, theme.colorOption.A);

		const bool isSelected = (printingOption == ActiveSelection());
		bool wasPressed = false;
		std::string tempArrow;

		if (isSelected)
		{
			if (theme.fontSelection == 2 || theme.fontSelection == 7)
			{
				tempArrow = "  ~b~==";
			}
			else
			{
				tempArrow = "  ~b~>";
			}

			SET_TEXT_FONT(theme.fontSelection);
			SET_TEXT_COLOUR(theme.colorSelected.R, theme.colorSelected.G, theme.colorSelected.B, theme.colorSelected.A);

			if (IsOptionPressed())
			{
				wasPressed = true;
				Game::Sound::PlayFrontend_default("SELECT");
			}
		}
		else
		{
			if (theme.fontOptions == 2 || theme.fontOptions == 7)
			{
				tempArrow = "  ~b~==";
			}
			else
			{
				tempArrow = "  ~b~>";
			}
		}

		acceptScratch = wasPressed;

		std::string drawText = Language::TranslateToSelected(text);
		if (showArrow && !gxt)
		{
			drawText += tempArrow;
		}

		if (gxt)
		{
			if (theme.centreOptions)
			{
				SET_TEXT_CENTRE(1);
				Game::Print::drawstringGXT(drawText, 0.16f + theme.position.x, optionY + theme.position.y);
			}
			else
			{
				Game::Print::drawstringGXT(drawText, 0.066f + theme.position.x, optionY + theme.position.y);
			}
		}
		else
		{
			if (theme.centreOptions)
			{
				SET_TEXT_CENTRE(1);
				Game::Print::drawstring(drawText, 0.16f + theme.position.x, optionY + theme.position.y);
			}
			else
			{
				Game::Print::drawstring(drawText, 0.066f + theme.position.x, optionY + theme.position.y);
			}
		}

		return wasPressed;
	}

	void Engine::DrawStatusBadge(bool status)
	{
		if (optionY < 0.6325f && optionY > 0.1425f)
		{
			if (!HAS_STREAMED_TEXTURE_DICT_LOADED("mprankbadge"))
			{
				REQUEST_STREAMED_TEXTURE_DICT("mprankbadge", 0);
			}

			const Vector2 res { 0.022f, 0.03f };
			const float x = XCoordAtMenuRightEdge(theme, res.x, 0.0f, true);
			const float y = optionY + 0.0166f + theme.position.y;

			if (!status)
			{
				DRAW_SPRITE("mprankbadge", "rankglobe_21x21_colour", x, y, res.x, res.y, 0.0f, 255, 102, 102, 250, false, 0);
			}
			else
			{
				DRAW_SPRITE("mprankbadge", "rankglobe_21x21_colour", x, y, res.x, res.y, 0.0f, 102, 255, 102, 250, false, 0);
			}
		}
	}

	bool Engine::AddToggle(const std::string& text, bool& value, bool gxt)
	{
		AddOption(text, false, gxt);

		bool flipped = false;
		if (acceptScratch)
		{
			value = !value;
			flipped = true;
		}

		DrawStatusBadge(value);
		return flipped;
	}

	void Engine::AddBreak(const std::string& text)
	{
		++printingOption;
		++breakCount;

		const int slot = ComputeRowSlot();
		if (slot > GTA_MAXOP || slot <= 0)
		{
			optionY = 0.0f;
			return;
		}
		optionY = slot * 0.035f + 0.125f;

		Game::Print::setupdraw();
		SET_TEXT_FONT(theme.fontBreaks);
		SET_TEXT_COLOUR(theme.colorBreak.R, theme.colorBreak.G, theme.colorBreak.B, theme.colorBreak.A);

		if (printingOption == currentOption)
		{
			switch (breakScrollDirection)
			{
			case 1:
				if (currentOption <= 1) 
				{
					EngineBottom(*this);
				}
				else 
				{
					EngineUp(*this);
				}
				break;
			case 2:
			default:
				if (currentOption >= totalOptions) 
				{
					EngineTop(*this);
				}
				else 
				{
					EngineDown(*this);
				}
				break;
			}
		}

		std::string drawText = Language::TranslateToSelected(text);

		if (theme.centreBreaks)
		{
			SET_TEXT_CENTRE(1);
			Game::Print::drawstringGXT(drawText, 0.16f + theme.position.x, optionY + theme.position.y);
		}
		else
		{
			Game::Print::drawstringGXT(drawText, 0.066f + theme.position.x, optionY + theme.position.y);
		}
	}

	InputResult Engine::AddNumber(const std::string& text, double value, int decimalPlaces, bool gxt)
	{
		AddOption(text, false, gxt);
		InputResult result;

		if (optionY < 0.6325f && optionY > 0.1425f)
		{
			float newXpos;
			Game::Print::SetupDraw(0, Vector2(0.26, 0.26), true, true, false, theme.colorOption);

			const __int8 decimals = static_cast<__int8>(decimalPlaces);
			const int activeSel = ActiveSelection();

			if (printingOption == activeSel)
			{
				Vector3 textureRes = GET_TEXTURE_RESOLUTION("CommonMenu", "arrowright");
				textureRes.x /= (Game::defaultScreenRes.first * 2);
				textureRes.y /= (Game::defaultScreenRes.second * 2);
				newXpos = XCoordAtMenuRightEdge(theme, textureRes.x - 0.005f, 0.0f, true);
				DRAW_SPRITE("CommonMenu", "arrowright", newXpos, optionY + 0.016f + theme.position.y, textureRes.x, textureRes.y, 0.0f, theme.colorSelected.R, theme.colorSelected.G, theme.colorSelected.B, theme.colorSelected.A, false, 0);

				newXpos = XCoordAtMenuRightEdge(theme, textureRes.x - 0.005f, textureRes.x - 0.005f + Game::Print::GetTextWidth(value, decimals), true);
				DRAW_SPRITE("CommonMenu", "arrowleft", newXpos, optionY + 0.016f + theme.position.y, textureRes.x, textureRes.y, 0.0f, theme.colorSelected.R, theme.colorSelected.G, theme.colorSelected.B, theme.colorSelected.A, false, 0);

				newXpos = XCoordAtMenuRightEdge(theme, Game::Print::GetTextWidth(value, decimals), textureRes.x - 0.005f, true);
				Game::Print::SetupDraw(0, Vector2(0.26, 0.26), true, true, false, theme.colorSelected);
			}
			else
			{
				newXpos = XCoordAtMenuRightEdge(theme, Game::Print::GetTextWidth(value, decimals), 0.0024f, true);
				Game::Print::SetupDraw(0, Vector2(0.26, 0.26), true, true, false, theme.colorOption);
			}

			Game::Print::drawfloat(value, decimals, newXpos, optionY + 0.0056 + theme.position.y);
		}

		if (printingOption == ActiveSelection())
		{
			numberSelected = true;
			AddInstructionalButton(INPUT_CELLPHONE_SELECT, "Input");

			if (acceptScratch) 
			{
				result.accepted = true;
			}
			else if (IsOptionRPressed()) 
			{
				result.rightPressed = true;
			}
			else if (IsOptionLPressed()) 
			{
				result.leftPressed = true;
			}
		}
		return result;
	}

	struct CheckboxSpriteInfo
	{
		const char* dict;
		const char* name;
		const char* selectedName;
	};
	static CheckboxSpriteInfo LookupCheckboxSprite(Checkbox kind)
	{
		switch (kind)
		{
		case Checkbox::TICK:               return { "CommonMenu",   "shop_tick_icon",          nullptr };
		case Checkbox::TICK2:              return { "crosstheline", "timer_largetick_32",      nullptr };
		case Checkbox::CROSS:              return { "crosstheline", "timer_largecross_32",     nullptr };
		case Checkbox::ARROWRIGHT:         return { "CommonMenu",   "arrowright",              nullptr };
		case Checkbox::ARROWLEFT:          return { "CommonMenu",   "arrowleft",               nullptr };
		case Checkbox::MANWON:             return { "CommonMenuTU", "last_team_standing",      nullptr };
		case Checkbox::SKULL_DM:           return { "CommonMenuTU", "deathmatch",              nullptr };
		case Checkbox::SKULL_TDM:          return { "CommonMenuTU", "team_deathmatch",         nullptr };
		case Checkbox::CARBANG_DM:         return { "CommonMenuTU", "vehicle_deathmatch",      nullptr };
		case Checkbox::SMALLNEWSTAR:       return { "CommonMenu",   "shop_new_star",           nullptr };
		case Checkbox::PERCENTAGESTICKER:  return { "mpshopsale",   "saleicon",                nullptr };
		case Checkbox::BOXTICK:            return { "CommonMenu",   "shop_box_tick",           "shop_box_tickb"        };
		case Checkbox::BOXCROSS:           return { "CommonMenu",   "shop_box_cross",          "shop_box_crossb"       };
		case Checkbox::BOXBLANK:           return { "CommonMenu",   "shop_box_blank",          "shop_box_blankb"       };
		case Checkbox::CARTHING:           return { "CommonMenu",   "shop_garage_icon_a",      "shop_garage_icon_b"    };
		case Checkbox::BIKETHING:          return { "CommonMenu",   "shop_garage_bike_icon_a", "shop_garage_bike_icon_b" };
		case Checkbox::WEAPONTHING:        return { "CommonMenu",   "shop_gunclub_icon_a",     "shop_gunclub_icon_b"   };
		case Checkbox::TATTOOTHING:        return { "CommonMenu",   "shop_tattoos_icon_a",     "shop_tattoos_icon_b"   };
		case Checkbox::MAKEUPTHING:        return { "CommonMenu",   "shop_makeup_icon_a",      "shop_makeup_icon_b"    };
		case Checkbox::MASKTHING:          return { "CommonMenu",   "shop_mask_icon_a",        "shop_mask_icon_b"      };
		case Checkbox::NONE:
		default:                         return { nullptr, nullptr, nullptr };
		}
	}

	static void DrawCheckboxSprite(const Engine& e, Checkbox kind)
	{
		const CheckboxSpriteInfo info = LookupCheckboxSprite(kind);
		if (!info.dict) return;

		if (!HAS_STREAMED_TEXTURE_DICT_LOADED(info.dict))
		{
			REQUEST_STREAMED_TEXTURE_DICT(info.dict, 0);
		}

		const bool isSelected = (e.printingOption == e.ActiveSelection());
		const char* name = info.name;
		RGBA col;

		if (info.selectedName)
		{
			if (isSelected) 
			{
				name = info.selectedName;
			}
			col = isSelected ? e.theme.colorSelected : e.theme.colorOption;
			col.R = col.G = col.B = 255; // BNW sprites are pre-coloured; only alpha is used.
		}
		else
		{
			col = isSelected ? e.theme.colorSelected : e.theme.colorOption;
		}

		Vector3 texRes = GET_TEXTURE_RESOLUTION(info.dict, name);
		texRes.x /= (Game::defaultScreenRes.first * 2);
		texRes.y /= (Game::defaultScreenRes.second * 2);

		const float x = XCoordAtMenuRightEdge(e.theme, texRes.x, 0.0f, true);
		const float y = e.optionY + 0.016f + e.theme.position.y;
		DRAW_SPRITE(info.dict, name, x, y, texRes.x, texRes.y, 0.0f, col.R, col.G, col.B, col.A, false, 0);
	}

	bool Engine::AddCheckbox(const std::string& text, bool condition, bool gxt)
	{
		return AddCheckbox(text, condition, Checkbox::TICK, Checkbox::NONE, gxt);
	}

	bool Engine::AddCheckbox(const std::string& text, bool condition, Checkbox tickTrue, Checkbox tickFalse, bool gxt)
	{
		AddOption(text, false, gxt);

		if (optionY < 0.6325f && optionY > 0.1425f)
		{
			if (condition && tickTrue != Checkbox::NONE)
			{
				DrawCheckboxSprite(*this, tickTrue);
			}

			else if (!condition && tickFalse != Checkbox::NONE)
			{
				DrawCheckboxSprite(*this, tickFalse);
			}
		}

		return acceptScratch;
	}

	bool Engine::AddToggleStatus(const std::string& text, bool currentValue, bool gxt)
	{
		const bool pressed = AddOption(text, false, gxt);
		DrawStatusBadge(currentValue);
		return pressed;
	}

	InputResult Engine::AddTextList(const std::string& text, int selectedIndex, const std::vector<std::string>& options, bool gxt)
	{
		AddOption(text, false, gxt);
		InputResult result;

		if (optionY < 0.6325f && optionY > 0.1425f)
		{
			std::string display;
			if (selectedIndex < 0 || selectedIndex >= static_cast<int>(options.size()))
			{
				display = std::to_string(selectedIndex);
			}
			else
			{
				display = options.at(selectedIndex);
			}

			display = DOES_TEXT_LABEL_EXIST(display.c_str()) ? GET_FILENAME_FOR_AUDIO_CONVERSATION(display.c_str()) : Language::TranslateToSelected(display);

			float newXpos;
			Game::Print::SetupDraw(0, Vector2(0.26, 0.26), true, true, false, theme.colorOption);
			const int activeSel = ActiveSelection();

			if (printingOption == activeSel)
			{
				Vector3 textureRes = GET_TEXTURE_RESOLUTION("CommonMenu", "arrowright");
				textureRes.x /= (Game::defaultScreenRes.first * 2);
				textureRes.y /= (Game::defaultScreenRes.second * 2);
				newXpos = XCoordAtMenuRightEdge(theme, textureRes.x - 0.005f, 0.0f, true);
				DRAW_SPRITE("CommonMenu", "arrowright", newXpos, optionY + 0.016f + theme.position.y, textureRes.x, textureRes.y, 0.0f, theme.colorSelected.R, theme.colorSelected.G, theme.colorSelected.B, theme.colorSelected.A, false, 0);

				newXpos = XCoordAtMenuRightEdge(theme, textureRes.x - 0.005f, textureRes.x - 0.005f + Game::Print::GetTextWidth(display), true);
				DRAW_SPRITE("CommonMenu", "arrowleft", newXpos, optionY + 0.016f + theme.position.y, textureRes.x, textureRes.y, 0.0f, theme.colorSelected.R, theme.colorSelected.G, theme.colorSelected.B, theme.colorSelected.A, false, 0);

				newXpos = XCoordAtMenuRightEdge(theme, Game::Print::GetTextWidth(display), textureRes.x - 0.005f, true);
				Game::Print::SetupDraw(0, Vector2(0.26, 0.26), true, true, false, theme.colorSelected);
			}
			else
			{
				newXpos = XCoordAtMenuRightEdge(theme, Game::Print::GetTextWidth(display), 0.0024f, true);
				Game::Print::SetupDraw(0, Vector2(0.26, 0.26), true, true, false, theme.colorOption);
			}

			Game::Print::drawstring(display, newXpos, optionY + 0.0056 + theme.position.y);
		}

		if (printingOption == ActiveSelection())
		{
			numberSelected = true;
			AddInstructionalButton(INPUT_CELLPHONE_SELECT, "Input");

			if (acceptScratch) 
			{
				result.accepted = true;
			}
			else if (IsOptionRPressed()) 
			{
				result.rightPressed = true;
			}
			else if (IsOptionLPressed()) 
			{
				result.leftPressed = true;
			}
		}
		return result;
	}

	void Engine::FireExitOnActive()
	{
		if (activeSubmenuId.empty()) return;
		if (Submenu* sub = SubmenuRegistry::Find(activeSubmenuId))
			sub->OnExit();
	}

	void Engine::FireEnterOnActive()
	{
		if (activeSubmenuId.empty()) return;
		if (Submenu* sub = SubmenuRegistry::Find(activeSubmenuId))
			sub->OnEnter();
	}

	void Engine::Open()
	{
		if (isOpen) return;
		if (rootSubmenuId.empty()) return;

		activeSubmenuId = rootSubmenuId;
		navStack.clear();
		currentOption = 1;
		totalOptions = 0;
		isOpen = true;
		Game::Sound::PlayFrontend("FocusIn", "HintCamSounds");
		FireEnterOnActive();
	}

	void Engine::Close()
	{
		if (!isOpen) return;
		FireExitOnActive();
		isOpen = false;
		navStack.clear();
		ENABLE_ALL_CONTROL_ACTIONS(0);
		ENABLE_ALL_CONTROL_ACTIONS(2);
		Game::Sound::PlayFrontend_default("BACK");
	}

	void Engine::Toggle()
	{
		if (isOpen) Close();
		else Open();
	}

	void Engine::OpenAt(std::string_view id, int initialCursor)
	{
		if (rootSubmenuId.empty()) return;
		FireExitOnActive();
		navStack.clear();
		activeSubmenuId = rootSubmenuId;
		currentOption = 1;
		totalOptions = 0;
		isOpen = true;
		NavigateTo(id);
		currentOption = initialCursor;
	}

	bool Engine::IsInSubmenuFamily(std::string_view idOrPrefix) const
	{
		auto startsWith = [&](const std::string& s) 
		{
			return s.size() >= idOrPrefix.size() && std::string_view(s).substr(0, idOrPrefix.size()) == idOrPrefix;
		};
		if (startsWith(activeSubmenuId)) return true;
		for (const auto& [id, opt] : navStack)
		{
			if (startsWith(id)) return true;
		}
		return false;
	}

	void Engine::NavigateTo(std::string_view id)
	{
		FireExitOnActive();
		// Save the current location on the back stack and switch.
		navStack.emplace_back(activeSubmenuId, currentOption);
		activeSubmenuId.assign(id);
		currentOption = 1;
		totalOptions = 0;
		Game::Sound::PlayFrontend_default("NAV_UP_DOWN");
		FireEnterOnActive();
	}

	void Engine::GoBack()
	{
		if (navStack.empty())
		{
			Close();
			return;
		}
		FireExitOnActive();
		auto& [prevId, prevOption] = navStack.back();
		activeSubmenuId = std::move(prevId);
		currentOption = prevOption;
		navStack.pop_back();
		totalOptions = 0;
		Game::Sound::PlayFrontend_default("BACK");
		FireEnterOnActive();
	}

	void Engine::DrawTitleBarBackground()
	{
		const RGBA& tb = theme.colorTitleBox;
		const float x = 0.16f + theme.position.x;

		// If the active submenu provides a custom title sprite, render it across the upper title-bar block
		Submenu* sub = SubmenuRegistry::Find(activeSubmenuId);
		if (sub)
		{
			const TitleSpriteOverride override = sub->GetTitleSpriteOverride();
			if (override.IsActive())
			{
				if (!HAS_STREAMED_TEXTURE_DICT_LOADED(override.dict))
				{
					REQUEST_STREAMED_TEXTURE_DICT(override.dict, 0);
				}
				DRAW_SPRITE(override.dict, override.name, x, 0.0989f + theme.position.y, 0.20f, 0.083f, 0.0f, 255, 255, 255, tb.A, false, 0);
				return;
			}
		}

		const float y = 0.1175f + theme.position.y;
		if (theme.useGradients)
		{
			if (!HAS_STREAMED_TEXTURE_DICT_LOADED("CommonMenu"))
			{
				REQUEST_STREAMED_TEXTURE_DICT("CommonMenu", 0);
			}
			DRAW_SPRITE("CommonMenu", "Gradient_Nav", x, y, 0.20f, 0.083f, 0.0f, tb.R, tb.G, tb.B, tb.A, false, 0);
		}
		else
		{
			DRAW_RECT(x, y, 0.20f, 0.083f, tb.R, tb.G, tb.B, tb.A, false);
		}
	}

	void Engine::DrawBackgroundPanel()
	{
		const float visible = (totalOptions > GTA_MAXOP) ? float(GTA_MAXOP) : float(totalOptions);
		if (visible <= 0) return;

		const float bgY = ((visible * 0.035f) / 2.0f) + 0.159f + theme.position.y;
		const float bgLen = visible * 0.035f;
		const float x = 0.16f + theme.position.x;
		const RGBA& bg = theme.colorBackground;

		if (theme.useGradients && bg.R < 20 && bg.G < 20 && bg.B < 20)
		{
			if (!HAS_STREAMED_TEXTURE_DICT_LOADED("CommonMenu"))
			{
				REQUEST_STREAMED_TEXTURE_DICT("CommonMenu", 0);
			}
			DRAW_SPRITE("CommonMenu", "Gradient_Bgd", x, bgY, 0.20f, bgLen, 0.0f, 255, 255, 255, bg.A, false, 0);
		}
		else
		{
			DRAW_RECT(x, bgY, 0.20f, bgLen, bg.R, bg.G, bg.B, bg.A, false);
		}

		// Scroller indicator strip below the visible options.
		const float scrY = ((visible + 1.0f) * 0.035f) + 0.1415f + theme.position.y;
		const RGBA& tb = theme.colorTitleBox;
		if (theme.useGradients)
		{
			DRAW_SPRITE("CommonMenu", "Gradient_Nav", x, scrY, 0.20f, 0.0345f, 0.0f, tb.R, tb.G, tb.B, tb.A, false, 0);
		}
		else
		{
			DRAW_RECT(x, scrY, 0.20f, 0.0345f, tb.R, tb.G, tb.B, tb.A, false);
		}

		// Thin separator line drawn over the scroller indicator rect.
		if (theme.useThinScrollLine)
		{
			const float lineY = ((totalOptions < GTA_MAXOP ? totalOptions : 14) * 0.035f + 0.1589f) + theme.position.y;
			DRAW_RECT(x, lineY, 0.20f, 0.0011f, 255, 255, 255, 255, false);
		}

		// Up/down scroll arrows when the list overflows.
		if (totalOptions > GTA_MAXOP)
		{
			Vector3 texRes = GET_TEXTURE_RESOLUTION("CommonMenu", "shop_arrows_upANDdown");
			texRes.x /= (Game::defaultScreenRes.first * 2);
			texRes.y /= (Game::defaultScreenRes.second * 2);
			const float arrY = ((GTA_MAXOP + 1.0f) * 0.035f) + 0.1413f + theme.position.y;
			const RGBA& oc = theme.colorOptionCount;
			DRAW_SPRITE("CommonMenu", "shop_arrows_upANDdown", x, arrY, texRes.x, texRes.y, 0.0f, oc.R, oc.G, oc.B, 255, false, 0);
		}

		// Option count "n / N" at the right edge of the scroller strip.
		const std::string count = std::to_string(ActiveSelection()) + " / " + std::to_string(totalOptions);
		Game::Print::SetupDraw(GTAfont::Arial, Vector2(0.0f, 0.26f), false, false, false, theme.colorOptionCount);
		const float countWidth = Game::Print::GetTextWidth(count);
		Game::Print::SetupDraw(GTAfont::Arial, Vector2(0.0f, 0.26f), false, false, false, theme.colorOptionCount);
		const float countY = scrY - 0.0124f - theme.position.y;
		Game::Print::drawstring(count, XCoordAtMenuRightEdge(theme, countWidth, 0.0f, false), countY + theme.position.y);
	}

	void Engine::DrawSelectionHighlight()
	{
		if (totalOptions < 1) return;

		const int active = ActiveSelection();
		float row;
		if (active > GTA_SCROLLOP && totalOptions > GTA_MAXOP)
		{
			row = GTA_SCROLLOP;
			if (active > totalOptions - GTA_BETOP)
			{
				row = GTA_SCROLLOP + active - totalOptions + GTA_BETOP;
			}
		}
		else
		{
			row = static_cast<float>(active);
		}

		const float y = (row * 0.035f) + 0.1415f + theme.position.y;
		const float x = 0.16f + theme.position.x;
		const RGBA& hi = theme.colorSelectionHighlight;

		if (theme.useGradients)
		{
			DRAW_SPRITE("CommonMenu", "Gradient_Nav", x, y, 0.20f, 0.035f, 0.0f, hi.R, hi.G, hi.B, hi.A, false, 0);
		}
		else
		{
			DRAW_RECT(x, y, 0.20f, 0.035f, hi.R, hi.G, hi.B, hi.A, false);
		}
	}

	void Engine::DisableGameControls()
	{
		HIDE_HELP_TEXT_THIS_FRAME();
		SET_CINEMATIC_BUTTON_ACTIVE(1);
		SET_INPUT_EXCLUSIVE(2, INPUT_FRONTEND_ACCEPT);
		SET_INPUT_EXCLUSIVE(2, INPUT_FRONTEND_CANCEL);
		SET_INPUT_EXCLUSIVE(2, INPUT_FRONTEND_DOWN);
		SET_INPUT_EXCLUSIVE(2, INPUT_FRONTEND_UP);
		SET_INPUT_EXCLUSIVE(2, INPUT_FRONTEND_LEFT);
		SET_INPUT_EXCLUSIVE(2, INPUT_FRONTEND_RIGHT);
		DISABLE_CONTROL_ACTION(2, INPUT_FRONTEND_ACCEPT, TRUE);
		DISABLE_CONTROL_ACTION(2, INPUT_FRONTEND_CANCEL, TRUE);
		DISABLE_CONTROL_ACTION(2, INPUT_FRONTEND_UP, TRUE);
		DISABLE_CONTROL_ACTION(2, INPUT_FRONTEND_DOWN, TRUE);
		DISABLE_CONTROL_ACTION(2, INPUT_FRONTEND_LEFT, TRUE);
		DISABLE_CONTROL_ACTION(2, INPUT_FRONTEND_RIGHT, TRUE);
		DISABLE_CONTROL_ACTION(2, INPUT_CELLPHONE_CANCEL, TRUE);
		DISABLE_CONTROL_ACTION(2, INPUT_CELLPHONE_SELECT, TRUE);
	}

	void Engine::HandleNavigationInput()
	{
		if (totalOptions > 0)
		{
			if (MenuPressTimer::IsButtonHeldOrTapped(MenuPressTimer::Button::Up))
			{
				if (currentOption <= 1) 
				{
					currentOption = totalOptions;
				}
				else
				{
					--currentOption;
				}
				Game::Sound::PlayFrontend_default("NAV_UP_DOWN");
				breakScrollDirection = 1;
			}
			else if (MenuPressTimer::IsButtonHeldOrTapped(MenuPressTimer::Button::Down))
			{
				if (currentOption >= totalOptions) 
				{
					currentOption = 1;
				}
				else
				{
					++currentOption;
				}
				Game::Sound::PlayFrontend_default("NAV_UP_DOWN");
				breakScrollDirection = 2;
			}
		}

		if (MenuPressTimer::IsButtonTapped(MenuPressTimer::Button::Back))
		{
			GoBack();
		}
	}

	void Engine::ApplyPendingNavigation()
	{
		if (pendingNavId.empty()) return;
		const std::string id = std::move(pendingNavId);
		pendingNavId.clear();
		NavigateTo(id);
	}

	Engine::Engine() = default;
	Engine::~Engine() = default;

	int Engine::ActiveSelection() const
	{
		if (theme.useMouse && currentOptionMouse > 0) 
		{
			return currentOptionMouse;
		}
		return currentOption;
	}

	void Engine::AddInstructionalButton(int button, const std::string& label, bool isKey)
	{
		ibList.push_back({ button, label, isKey });
	}

	bool Engine::AddPresetColourOptions(int& r, int& g, int& b)
	{
		bool pressed = false;
		for (const auto& entry : _vNeonColours)
		{
			const bool matches = (r == entry.rgb.R && g == entry.rgb.G && b == entry.rgb.B);
			if (AddCheckbox(entry.name, matches))
			{
				r = entry.rgb.R;
				g = entry.rgb.G;
				b = entry.rgb.B;
				pressed = true;
			}

			if (printingOption == ActiveSelection() && optionY < 0.6325f && optionY > 0.1425f)
			{
				AddPresetColourOptionsPreview(entry.rgb.R, entry.rgb.G, entry.rgb.B);
			}
		}
		return pressed;
	}

	void Engine::AddPresetColourOptionsPreview(unsigned char r, unsigned char g, unsigned char b)
	{
		const Vector2 res { 0.1f, 0.0889f };
		float xCoord = 0.324f + theme.position.x;
		if (theme.position.x > 0.45f) 
		{
			xCoord = theme.position.x - 0.003f;
		}
		DRAW_RECT(xCoord, optionY + 0.044f + theme.position.y, res.x + 0.003f, res.y + 0.003f, 0, 0, 0, 212, false);
		DRAW_RECT(xCoord, optionY + 0.044f + theme.position.y, res.x, res.y, r, g, b, 255, false);
	}

	void Engine::AddPresetColourOptionsPreview(const RgbS& rgb)
	{
		AddPresetColourOptionsPreview(rgb.R, rgb.G, rgb.B);
	}

	void Engine::DrawGlare()
	{
		if (!glareScaleform) 
		{
			glareScaleform = std::make_unique<Scaleform>();
		}
		if (!glareScaleform->Load("MP_MENU_GLARE")) return;

		glareScaleform->PushFunction("SET_DATA_SLOT");
		glareScaleform->PushFloat(0.0f);
		glareScaleform->PopFunction();

		const Vector2 pos { 0.4800f + theme.position.x, 0.4850f + theme.position.y };
		const Vector2 size { 0.9800f, 0.9100f };

		glareScaleform->Render2DScreenSpace(pos, size, { theme.colorTitleText.R, theme.colorTitleText.G, theme.colorTitleText.A, theme.colorTitleBox.A });
	}

	std::string Engine::IbKeyString(const IBEntry& entry) const
	{
		if (entry.button == -3) return "";
		if (!entry.isKey)
		{
			return GET_CONTROL_INSTRUCTIONAL_BUTTONS_STRING(2, entry.button, 1);
		}
		return "t_" + VkCodeToStr(static_cast<UINT8>(entry.button));
	}

	void Engine::PushDefaultInstructionalButtons()
	{
		if (!numberSelected)
		{
			AddInstructionalButton(INPUT_CELLPHONE_SELECT, "ITEM_SELECT");
		}
		if (!navStack.empty())
		{
			AddInstructionalButton(INPUT_FRONTEND_RRIGHT, "ITEM_BACK");
		}
		else
		{
			AddInstructionalButton(INPUT_FRONTEND_RRIGHT, "ITEM_EXIT");
		}
	}

	void Engine::DrawInstructionalButtons()
	{
		if (ibList.empty()) return;
		if (UPDATE_ONSCREEN_KEYBOARD() == 0) return;

		if (!instructionalButtonsScaleform)
		{
			instructionalButtonsScaleform = std::make_unique<Scaleform>();
		}
		if (!instructionalButtonsScaleform->Load("instructional_buttons")) return;

		Scaleform& sf = *instructionalButtonsScaleform;

		sf.PushFunction("CLEAR_ALL");
		sf.PopFunction();

		sf.PushFunction("SET_MAX_WIDTH");
		sf.PushFloat(100.0f);
		sf.PopFunction();

		sf.PushFunction("TOGGLE_MOUSE_BUTTONS");
		sf.PushBoolean(true);
		sf.PopFunction();

		for (size_t i = 0; i < ibList.size(); ++i)
		{
			const IBEntry& entry = ibList[i];
			sf.PushFunction("SET_DATA_SLOT");
			sf.PushInteger(static_cast<int>(i));

			if (entry.button >= 1000)
			{
				sf.PushInteger(entry.button - 1000);
				sf.PushTextComponent(entry.text);
			}
			else
			{
				sf.PushString2(IbKeyString(entry));
				sf.PushTextComponent(entry.text);
				sf.PushBoolean(true);
				sf.PushInteger(entry.button);
			}
			sf.PopFunction();
		}

		sf.PushFunction("SET_BACKGROUND_COLOUR");
		sf.PushRGBA(RGBA(0, 0, 0, 80));
		sf.PopFunction();

		sf.PushFunction("DRAW_INSTRUCTIONAL_BUTTONS");
		sf.PushInteger(0);
		sf.PopFunction();

		SET_SCRIPT_GFX_ALIGN(76, 66);
		SET_SCRIPT_GFX_ALIGN_PARAMS(0.0f, 0.0f, 0.0f, 0.0f);
		RESET_SCRIPT_GFX_ALIGN();

		sf.Render2D();
		ibList.clear();
	}

	Vector2 Engine::MouseItemNumberToCoords(int itemNumber) const
	{
		for (const MouseHitItem& it : mouseHitList)
		{
			if (it.real == itemNumber)
			{
				return Vector2(0.16f + theme.position.x, (it.onScreen * 0.035f) + 0.1415f + theme.position.y);
			}
		}
		return Vector2(-1.0f, -1.0f);
	}

	static Vector2 ReadMousePosition()
	{
		Vector2 p;
		p.x = GET_DISABLED_CONTROL_NORMAL(2, INPUT_CURSOR_X);
		p.y = GET_DISABLED_CONTROL_NORMAL(2, INPUT_CURSOR_Y);
		return p;
	}

	static bool IsMouseInBounds(const Vector2& centre, const Vector2& size)
	{
		const Vector2 p = ReadMousePosition();
		return (p.x >= centre.x - size.x / 2 && p.x <= centre.x + size.x / 2)
			&& (p.y >  centre.y - size.y / 2 && p.y <  centre.y + size.y / 2);
	}

	void Engine::HandleMouseInput()
	{
		if (!theme.useMouse) return;

		// Seed/clamp the mouse cursor into the valid range.
		if (currentOptionMouse < 1) 
		{
			currentOptionMouse = currentOption;
		}
		if (totalOptions > 0 && currentOptionMouse > totalOptions)
		{
			currentOptionMouse = currentOption;
		}

		SET_MOUSE_CURSOR_THIS_FRAME();
		SET_MOUSE_CURSOR_STYLE(1);

		// Scroll wheel.
		if (totalOptions > 0)
		{
			if (IS_DISABLED_CONTROL_PRESSED(0, INPUT_CURSOR_SCROLL_UP))
			{
				if (currentOptionMouse > 1) 
				{
					--currentOptionMouse;
				}
			}
			else if (IS_DISABLED_CONTROL_PRESSED(0, INPUT_CURSOR_SCROLL_DOWN))
			{
				if (currentOptionMouse < totalOptions) 
				{
					++currentOptionMouse;
				}
			}
		}

		// Hover + click.
		pressedSelectAfterSelect = false;
		const Vector2 rowSize { 0.20f, 0.035f };
		for (const MouseHitItem& it : mouseHitList)
		{
			const Vector2 rowPos = MouseItemNumberToCoords(it.real);
			if (rowPos.x < 0.0f) continue;
			if (!IsMouseInBounds(rowPos, rowSize)) continue;

			// Hover dim-highlight (1/3 alpha of the regular selection colour).
			const RGBA& hi = theme.colorSelectionHighlight;
			DRAW_RECT(rowPos.x, rowPos.y, rowSize.x, rowSize.y, hi.R, hi.G, hi.B, hi.A / 3, false);

			if (IS_DISABLED_CONTROL_JUST_PRESSED(0, INPUT_ATTACK))
			{
				if (currentOptionMouse != it.real)
					currentOptionMouse = it.real;
				else
					pressedSelectAfterSelect = true;
			}
		}

		currentOption = currentOptionMouse;
	}

	void Engine::DrawMouseHoverHighlight()
	{
		if (totalOptions < 1) return;

		const Vector2 pos = MouseItemNumberToCoords(currentOptionMouse);
		if (pos.x < 0.0f) return;
		const Vector2 size { 0.20f, 0.035f };
		const RGBA& hi = theme.colorSelectionHighlight;

		if (theme.useGradients)
		{
			DRAW_SPRITE("CommonMenu", "Gradient_Nav", pos.x, pos.y, size.x, size.y, 0.0f, hi.R, hi.G, hi.B, hi.A, false, 0);
		}
		else
		{
			DRAW_RECT(pos.x, pos.y, size.x, size.y, hi.R, hi.G, hi.B, hi.A, false);
		}
	}

	void Engine::Tick()
	{
		if (!isOpen) return;
		if (activeSubmenuId.empty()) return;

		Submenu* sub = SubmenuRegistry::Find(activeSubmenuId);
		if (!sub) return;

		MenuPressTimer::Update();
		DisableGameControls();

		BeginFrame();
		DrawTitleBarBackground();
		DrawActiveSubmenu(*sub);
		EndFrame();
		DrawBackgroundPanel();

		if (theme.useMouse)
		{
			DrawMouseHoverHighlight();
		}
		else
		{
			DrawSelectionHighlight();
		}

		if (theme.useGlare)
		{
			DrawGlare();
		}

		if (theme.useInstructionalButtons)
		{
			PushDefaultInstructionalButtons();
			DrawInstructionalButtons();
		}

		HandleNavigationInput();
		HandleMouseInput();
		ApplyPendingNavigation();
	}
}
