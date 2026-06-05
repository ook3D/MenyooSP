#pragma once

#include "Theme.h"

#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

class Scaleform;

enum class Checkbox : unsigned char;

namespace Menu
{
	class Submenu;

	struct InputResult
	{
		bool accepted = false;
		bool rightPressed = false;
		bool leftPressed = false;
	};

	class Engine
	{
	public:
		Engine();
		~Engine();
		Engine(const Engine&) = delete;
		Engine& operator=(const Engine&) = delete;

		Theme theme;

		// Currently-selected option (1-based). Updated by the outer nav loop;
		// the Add* primitives read it to highlight the selected row.
		int currentOption = 1;

		// Total options counted on the previous frame. Used for scroll math.
		// Engine::EndFrame() snapshots printingOption into this.
		int totalOptions = 0;

		// Auto-skip direction when a Break separator becomes selected. 1 = up,
		// 2 = down. Matches the old Menu::breakscroll semantics.
		unsigned char breakScrollDirection = 2;

		// Pending navigation request. Add* methods that target a sub-menu
		// write here; the outer loop reads it after EndFrame().
		std::string pendingNavId;

		// Hint: set true on the frame a Number / TextList primitive owns the
		// selection. The outer loop uses it to swap input mode.
		bool numberSelected = false;

		// The submenu Id to enter when Open() is called with an empty nav
		// stack. If empty, Open() has no effect.
		std::string rootSubmenuId;

		// Open: enter the rootSubmenuId. No-op if already open or root unset.
		void Open();

		// Close: clear the nav stack and stop rendering. Re-enables game
		// controls that Tick() had disabled.
		void Close();

		// Toggle Open / Close. Convenient for a single keybind.
		void Toggle();

		// Is the menu currently displaying?
		bool IsOpen() const { return isOpen; }

		// Force-open at root and deep-navigate to `id` with the cursor on
		// `initialCursor`. Equivalent to Close() + Open() + NavigateTo(id) +
		// setting currentOption. Used by hotkey-driven entry points (e.g.
		// SpoonerMode) that want a clean nav stack regardless of prior state.
		void OpenAt(std::string_view id, int initialCursor = 1);

		// True if `activeSubmenuId` or any submenu on the nav stack starts
		// with `idOrPrefix`. Useful for "am I anywhere inside the spooner
		// family?" style checks that previously walked Menu::currentArray.
		bool IsInSubmenuFamily(std::string_view idOrPrefix) const;

		// Push the current submenu (and its selected option) onto the back
		// stack; switch to `id` and reset selection to 1. Queued: takes effect
		// at the end of Tick().
		void NavigateTo(std::string_view id);

		// Pop the back stack and restore the previous submenu + selection. If
		// the stack is empty, closes the menu.
		void GoBack();

		// Id of the submenu currently being drawn.
		const std::string& ActiveSubmenuId() const { return activeSubmenuId; }

		// All-in-one: input -> disable game controls -> chrome -> active
		// submenu draw -> nav. Call once per game frame. No-ops if closed.
		void Tick();

		// Reset per-frame counters. Call once before drawing the active submenu.
		void BeginFrame();

		// Set Current() to `this`, invoke sub.Draw(), restore Current().
		void DrawActiveSubmenu(Submenu& sub);

		// Snapshot printingOption into totalOptions for next frame's scroll math, etc.
		void EndFrame();

		// Request navigation to another submenu by Id (sets pendingNavId;
		// applied at the end of Tick()). Same as NavigateTo() but expresses
		// "I want to be there next frame" rather than "do it now".
		void RequestNavigate(std::string_view id);

		void AddTitle(const std::string& text);
		void AddBreak(const std::string& text);

		// Returns true on the frame the user accepted this option.
		// showArrow = true draws the "> "/"== " suffix used for sub-menu rows.
		bool AddOption(const std::string& text, bool showArrow = false, bool gxt = false);

		// Toggles `value` on accept. Returns true on the frame value flipped.
		bool AddToggle(const std::string& text, bool& value, bool gxt = false);

		// Number row. Caller owns `value`; this only displays it and reports
		// edge bits. Caller decides how to mutate (clamp, step size, etc).
		InputResult AddNumber(const std::string& text, double value, int decimalPlaces, bool gxt = false);

		// Checkmark row. Returns true on the frame the user
		// pressed accept while this row was selected; caller flips its bool.
		bool AddCheckbox(const std::string& text, bool condition, bool gxt = false);

		// Checkbox with explicit on/off sprite variants, Pass Checkbox::NONE to suppress one side.
		bool AddCheckbox(const std::string& text, bool condition, Checkbox tickTrue, Checkbox tickFalse, bool gxt = false);

		// Preset-colour picker that iterates the engine's neon-colour palette
		// and draws a swatch preview next to the selected row. Mutates r/g/b
		// and returns true on the frame the selection changed.
		bool AddPresetColourOptions(int& r, int& g, int& b);

		// Draws the swatch preview rect (used by AddPresetColourOptions and
		// also by any submenu that wants to show a colour preview next to the
		// currently-selected row). Anchored to the engine's last-drawn row Y.
		void AddPresetColourOptionsPreview(unsigned char r, unsigned char g, unsigned char b);
		void AddPresetColourOptionsPreview(const RgbS& rgb);

		// External-state toggle: row + ON/OFF status badge based on
		// `currentValue`. Returns true on accept. Caller flips the underlying state.
		bool AddToggleStatus(const std::string& text, bool currentValue, bool gxt = false);

		// Cycle-through list row. Returns per-frame edge bits;
		// caller updates the selected index.
		InputResult AddTextList(const std::string& text, int selectedIndex, const std::vector<std::string>& options, bool gxt = false);


		// Running option counter for this frame (1-based after increments).
		int printingOption = 0;

		// Running break counter — separator rows don't consume a selectable
		// index but still take a Y slot.
		int breakCount = 0;

		// Y coordinate of the last drawn row. Used by mouse hit-test in phase 2.
		float optionY = 0.0f;

		// Scratch flag for the "did this row capture accept?" path used by
		// composite primitives (Toggle / Checkbox / Number / TextList).
		bool acceptScratch = false;

		// When theme.useMouse is true, hover-over rows track a separate index
		// (currentOptionMouse). The "effective" selection used by Add* is
		// ActiveSelection() — currentOptionMouse if mouse-mode and valid, otherwise currentOption.

		int currentOptionMouse = -1;
		bool pressedSelectAfterSelect = false;

		// Returns the per-frame selection (keyboard or mouse, depending on useMouse).
		int ActiveSelection() const;

		// Negative `button` values are interpreted as ScaleformButton indexes
		// (offset of -1000 applied internally).
		// Positive values < 1000 are ControllerInput; >= 1000 are virtual keys flagged as `isKey`.
		void AddInstructionalButton(int button, const std::string& label, bool isKey = false);


		// Set by DrawActiveSubmenu for the duration of the Draw() call.
		// Submenu helpers read it to know which engine to render into.
		static Engine* Current();

	private:
		static Engine* s_current;

		// Open / nav state
		bool isOpen = false;
		std::string activeSubmenuId;
		std::vector<std::pair<std::string, int>> navStack;

		// Mouse hit-list populated by AddOption; one entry per visible
		// selectable row (skips Breaks). `real` is the 1-based printingOption;
		// `onScreen` is the slot used for Y-coord computation.
		struct MouseHitItem
		{
			int real;
			int onScreen;
		};
		std::vector<MouseHitItem> mouseHitList;

		// Instructional buttons accumulated this frame.
		struct IBEntry
		{
			int button;
			std::string text;
			bool isKey;
		};
		std::vector<IBEntry> ibList;

		// Lazily-constructed scaleforms.
		std::unique_ptr<Scaleform> glareScaleform;
		std::unique_ptr<Scaleform> instructionalButtonsScaleform;

		// Shared layout helper used by AddOption / AddBreak: maps a 1-based
		// printingOption into either an on-screen row index (1..GTA_MAXOP) or
		// 0 if scrolled out of view.
		int ComputeRowSlot() const;

		// Draws the right-edge ON/OFF status badge, Reads optionY and theme.
		void DrawStatusBadge(bool status);

		// Chrome rendering (called by Tick).
		void DrawTitleBarBackground();
		void DrawBackgroundPanel();
		void DrawSelectionHighlight();

		// Game-control suppression while open (avoid player triggering actions
		// when pressing up/down/accept). Called by Tick.
		void DisableGameControls();

		// Input -> currentOption / nav stack mutation. Called by Tick.
		void HandleNavigationInput();

		// After EndFrame, consume pendingNavId if set.
		void ApplyPendingNavigation();

		// Fire the active submenu's OnEnter / OnExit lifecycle hooks.
		// Called from Open/Close/NavigateTo/GoBack/OpenAt so submenus that
		// initialize lazily (e.g. CreditsSubmenu loading its XML on first
		// entry) get a hook point.
		void FireEnterOnActive();
		void FireExitOnActive();

		// Mouse: read cursor, update currentOptionMouse against mouseHitList,
		// scroll wheel, click. Called by Tick when theme.useMouse is true.
		void HandleMouseInput();

		// Mouse alt-highlight (used when theme.useMouse is true; replaces the
		// keyboard highlight). Called by Tick.
		void DrawMouseHoverHighlight();

		// Glare scaleform overlay. Called by Tick when theme.useGlare is true.
		void DrawGlare();

		// Push the default Select / Back / Exit / Input hints based on current
		// state (numberSelected, navStack depth). Called by Tick just before
		// DrawInstructionalButtons.
		void PushDefaultInstructionalButtons();

		// Render the IB bar via scaleform and clear ibList for next frame.
		void DrawInstructionalButtons();

		// Lookup-string conversion used when constructing IB rows. Mirrors
		// Menu::GetKeyIB; isolated so the IB code is self-contained.
		std::string IbKeyString(const IBEntry& entry) const;

		// Convenience: 1-based item number -> on-screen Y based on the mouse
		// hit list captured this frame.
		Vector2 MouseItemNumberToCoords(int itemNumber) const;
	};
}
