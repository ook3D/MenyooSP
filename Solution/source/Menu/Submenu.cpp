/*
* Menyoo PC - Grand Theft Auto V single-player trainer mod
*
* Menu::Submenu — protected helpers delegate to the active Engine instance.
* The Engine is set by Engine::DrawActiveSubmenu before invoking Draw(); if
* a submenu's Draw() is invoked without an active Engine, helpers no-op
* harmlessly.
*/
#include "Submenu.h"

#include "Engine.h"

#include "Menu.h"   // Checkbox definition

namespace Menu
{
	void Submenu::DrawTitle()
	{
		Engine* engine = Engine::Current();
		if (!engine) return;
		engine->AddTitle(Title());
	}

	void Submenu::DrawBreak(const std::string& label)
	{
		Engine* engine = Engine::Current();
		if (!engine) return;
		engine->AddBreak(label);
	}

	bool Submenu::DrawToggle(const std::string& label, bool& value)
	{
		Engine* engine = Engine::Current();
		if (!engine) return false;
		return engine->AddToggle(label, value);
	}

	bool Submenu::DrawOption(const std::string& label)
	{
		Engine* engine = Engine::Current();
		if (!engine) return false;
		return engine->AddOption(label);
	}

	bool Submenu::DrawNumber(const std::string& label, float& value, float step,
		int decimals, float minValue, float maxValue)
	{
		Engine* engine = Engine::Current();
		if (!engine) return false;

		const InputResult input = engine->AddNumber(label, value, decimals);

		if (input.rightPressed)
		{
			const float next = value + step;
			value = next > maxValue ? maxValue : next;
			return true;
		}
		if (input.leftPressed)
		{
			const float next = value - step;
			value = next < minValue ? minValue : next;
			return true;
		}
		return false;
	}

	bool Submenu::DrawNumber(const std::string& label, int& value, int step,
		int minValue, int maxValue)
	{
		Engine* engine = Engine::Current();
		if (!engine) return false;

		const InputResult input = engine->AddNumber(label, static_cast<double>(value), 0);

		if (input.rightPressed)
		{
			const int next = value + step;
			value = next > maxValue ? maxValue : next;
			return true;
		}
		if (input.leftPressed)
		{
			const int next = value - step;
			value = next < minValue ? minValue : next;
			return true;
		}
		return false;
	}

	bool Submenu::DrawCheckbox(const std::string& label, bool& value)
	{
		Engine* engine = Engine::Current();
		if (!engine) return false;

		if (engine->AddCheckbox(label, value))
		{
			value = !value;
			return true;
		}
		return false;
	}

	bool Submenu::DrawTextList(const std::string& label, int& selectedIndex,
		const std::vector<std::string>& options)
	{
		Engine* engine = Engine::Current();
		if (!engine) return false;

		const InputResult input = engine->AddTextList(label, selectedIndex, options);

		const int size = static_cast<int>(options.size());
		if (size == 0) return false;

		if (input.rightPressed)
		{
			selectedIndex = (selectedIndex + 1) % size;
			return true;
		}
		if (input.leftPressed)
		{
			selectedIndex = (selectedIndex - 1 + size) % size;
			return true;
		}
		return false;
	}

	void Submenu::NavigateTo(std::string_view id)
	{
		Engine* engine = Engine::Current();
		if (!engine) return;
		engine->RequestNavigate(id);
	}

	bool Submenu::DrawSelectionItem(const std::string& label, bool isCurrent)
	{
		Engine* engine = Engine::Current();
		if (!engine) return false;
		return engine->AddCheckbox(label, isCurrent);
	}

	bool Submenu::DrawSelectionItem(const std::string& label, bool isCurrent,
		Checkbox onTick, Checkbox offTick)
	{
		Engine* engine = Engine::Current();
		if (!engine) return false;
		return engine->AddCheckbox(label, isCurrent, onTick, offTick);
	}

	bool Submenu::DrawToggleExternal(const std::string& label, bool currentValue)
	{
		Engine* engine = Engine::Current();
		if (!engine) return false;
		return engine->AddToggleStatus(label, currentValue);
	}

	bool Submenu::IsCurrentRowSelected() const
	{
		Engine* engine = Engine::Current();
		return engine != nullptr && engine->printingOption == engine->ActiveSelection();
	}
}
