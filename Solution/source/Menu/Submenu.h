#pragma once

#include <string>
#include <string_view>
#include <vector>

enum class Checkbox : unsigned char;

namespace Menu
{
	// Optional per-submenu title-bar sprite override. A submenu can return one
	// from GetTitleSpriteOverride() to render a custom sprite (e.g. the legacy
	// "shopui_title_carmod2" / "shopui_title_tennis" banners) instead of the
	// flat title-box tint. Leave both pointers null to use the default tint.
	struct TitleSpriteOverride
	{
		const char* dict = nullptr;
		const char* name = nullptr;

		bool IsActive() const { return dict != nullptr && name != nullptr; }
	};

	class Submenu
	{
	public:
		virtual ~Submenu() = default;

		// Stable, unique identifier used by the registry. Lowercase + snake_case
		// recommended. Must remain stable across builds (saved configs may
		// reference it).
		virtual const char* Id() const = 0;

		// Display title shown at the top of the submenu.
		virtual const char* Title() const = 0;

		// Called once per frame while this submenu is the active one. Use the
		// protected Draw* helpers here.
		virtual void Draw() = 0;

		// Lifecycle hooks. Override as needed.
		virtual void OnEnter() {}
		virtual void OnExit() {}
		virtual void Reset() {}

		// Optional: return a non-default title-bar sprite for this submenu.
		// Default returns an inactive override (the engine renders the flat
		// title-box tint).
		virtual TitleSpriteOverride GetTitleSpriteOverride() const { return {}; }

	protected:
		// Title banner using Title().
		void DrawTitle();

		// Section separator with a label (can be empty).
		void DrawBreak(const std::string& label);

		// Toggle that mutates `value` directly. Returns true on the frame the
		// toggle flipped (useful for one-shot side effects).
		bool DrawToggle(const std::string& label, bool& value);

		// Plain selectable option. Returns true on the frame the user pressed
		// Accept on this row — caller can then navigate or invoke an action.
		bool DrawOption(const std::string& label);

		// Number field. Right/Left adjust `value` by `step`, clamped to
		// [minValue, maxValue]. Returns true on the frame value changed.
		// Default min/max are effectively unclamped (+/-1e9 for float,
		// +/-2e9 for int); pass explicit bounds when you need clamping.
		bool DrawNumber(const std::string& label, float& value, float step, int decimals,
			float minValue = -1e9f, float maxValue = 1e9f);
		bool DrawNumber(const std::string& label, int& value, int step,
			int minValue = -2000000000, int maxValue = 2000000000);

		// Checkmark-style toggle. Mirrors `value` and flips it on press.
		// Returns true on the frame value changed.
		bool DrawCheckbox(const std::string& label, bool& value);

		// Cycle through a list of string options. Right/Left advance with
		// wrap-around. Returns true on the frame the selection changed.
		bool DrawTextList(const std::string& label, int& selectedIndex, const std::vector<std::string>& options);

		// "Show tick if isCurrent, fire on press" — for selection-from-list
		// rows (weather picker, model picker, etc.). Does NOT mutate any
		// state; caller does whatever should happen on press.
		bool DrawSelectionItem(const std::string& label, bool isCurrent);

		// Same, with explicit Checkbox sprite for the true/false states. Pass
		// Checkbox::NONE to suppress that side.
		bool DrawSelectionItem(const std::string& label, bool isCurrent,
			Checkbox onTick, Checkbox offTick);

		// External-state toggle row: shows the ON/OFF status badge based on
		// `currentValue`, returns true on accept. Caller is responsible for
		// flipping whatever underlying state backs it (e.g. calling
		// `SetThing(!IsThing())` on press).
		bool DrawToggleExternal(const std::string& label, bool currentValue);

		// True if the row most recently drawn by an Add* call is the engine's
		// active (keyboard or mouse) selection. Useful for contextual hints
		// or per-row sprite previews:
		//
		//     DrawNumber("Radius", radius, 0.1f, 2);
		//     if (IsCurrentRowSelected()) DrawRadiusMarkerInWorld(...);
		bool IsCurrentRowSelected() const;

		// Queue a navigation to another submenu. The Engine performs the
		// switch after the current Draw() returns. Use from inside a
		// `if (DrawOption(...))` block.
		void NavigateTo(std::string_view id);
	};
}
