# Creating Submenus

This guide explains how to build a submenu with the menu engine introduced in
the menu rewrite. It covers the lifecycle, the `Draw*` row helpers, navigation
between submenus, and the steps to register a new submenu so it shows up in a
build.

> Looking for a minimal, copy-pasteable starting point? See
> [`Solution/source/Submenus/Example.cpp`](../Solution/source/Submenus/Example.cpp)
> and [`Example.h`](../Solution/source/Submenus/Example.h). This guide expands
> on that example.

## Mental model

A submenu is a class that derives from `Menu::Submenu`. Each frame the engine
asks the active submenu to draw itself by calling `Draw()`. Inside `Draw()` you
call row helpers (`DrawOption`, `DrawToggle`, `DrawNumber`, ...) in top-to-bottom
order — the order you call them in is the order they appear on screen.

Key points:

- You never manage screen coordinates, selection indices, or input polling. The
  engine tracks which row is selected and tells each helper whether it was
  activated this frame.
- Each submenu has a **stable string `Id()`**. Navigation and saved configs
  reference submenus by this id, so once shipped it should not change.
- Submenus self-register at startup via the `REGISTER_SUBMENU` macro — there is
  no central list to edit.

## Minimal submenu

### Header (`MyFeature.h`)

```cpp
#pragma once

#include "../Menu/Submenu.h"

namespace Menu {

class MyFeatureSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "my_feature"; }
	const char* Title() const override { return "My Feature"; }

	void Draw() override;
	void Reset() override;

private:
	bool isEnabled = false;
	float intensity = 0.5f;
	int mode = 0;
};

}
```

### Source (`MyFeature.cpp`)

```cpp
#include "MyFeature.h"

#include "../Menu/SubmenuRegistry.h"

namespace Menu {

void MyFeatureSubmenu::Draw()
{
	DrawTitle();

	DrawToggle("Enabled", isEnabled);

	DrawBreak("---Tuning---");

	DrawNumber("Intensity", intensity, 0.05f, 2, 0.0f, 1.0f);
	DrawTextList("Mode", mode, std::vector<std::string>{ "Off", "Low", "High" });

	if (DrawOption("Reset to Defaults"))
	{
		Reset();
	}
}

void MyFeatureSubmenu::Reset()
{
	isEnabled = false;
	intensity = 0.5f;
	mode = 0;
}

}
REGISTER_SUBMENU(::Menu::MyFeatureSubmenu)
```

That's a complete, working submenu. The remaining sections explain each piece.

## The class contract

You must override:

| Method      | Purpose |
|-------------|---------|
| `Id()`      | Stable, unique, lowercase `snake_case` identifier. Used by the registry and by `NavigateTo`. Keep it stable across builds. |
| `Title()`   | Display text shown by `DrawTitle()`. |
| `Draw()`    | Called once per frame while this submenu is active. Build the rows here. |

You may optionally override:

| Method                     | Purpose |
|----------------------------|---------|
| `OnEnter()`                | Fires when the submenu becomes active (navigated to / opened). Good for lazy loading — see the Credits submenu loading its XML on first entry. |
| `OnExit()`                 | Fires when leaving the submenu. |
| `Reset()`                  | Restore default field values. You decide when to call it (e.g. from a "Reset" row). |
| `GetTitleSpriteOverride()` | Return a custom title-bar sprite (`dict` + `name`) instead of the flat tint. Leave default to use the tint. |

Store per-submenu state as member fields (like `isEnabled`, `intensity`). The
single registered instance persists across frames, so member values are
remembered while the menu is open and between visits.

## Row helpers

All helpers are `protected` members of `Submenu`, so you call them unqualified
from inside `Draw()`. They are defined in
[`Solution/source/Menu/Submenu.h`](../Solution/source/Menu/Submenu.h).

### Structure

- `DrawTitle()` — draws the title banner using `Title()`. Call it first.
- `DrawBreak(label)` — a section separator with an optional label. Breaks are
  not selectable; the cursor auto-skips over them.

### Selectable rows

```cpp
// Plain action row. Returns true on the frame the user presses Accept.
if (DrawOption("Do The Thing"))
{
	doTheThing();
}
```

```cpp
// Boolean toggle. Mutates `value` directly. Returns true on the frame it flipped.
bool wasFlipped = DrawToggle("Enabled", isEnabled);

// Checkmark-style variant (tick sprite instead of ON/OFF text).
DrawCheckbox("Use Fancy Mode", useFancyMode);
```

```cpp
// Number field. Left/Right adjust by `step`, clamped to [min, max].
// Signature: (label, value, step, decimals, min = unclamped, max = unclamped)
DrawNumber("Intensity", intensity, 0.05f, 2, 0.0f, 1.0f);

// Integer overload: (label, value, step, min, max)
DrawNumber("Count", count, 1, 0, 100);
```

```cpp
// Cycle through string options with Left/Right (wraps around).
// Mutates `selectedIndex`. Returns true on change.
DrawTextList("Mode", mode, std::vector<std::string>{ "Off", "Low", "High" });
```

### Selection-from-list rows

These are for "pick one of many" screens (weather picker, model picker) where
you draw one row per candidate and show a tick on the current one.

```cpp
// Shows a tick when isCurrent is true; returns true on press. Mutates nothing —
// you decide what happens on press.
for (int i = 0; i < weatherCount; ++i)
{
	if (DrawSelectionItem(weatherNames[i], i == currentWeather))
	{
		SetWeather(i);
	}
}
```

```cpp
// External-state toggle: shows an ON/OFF badge from a value you don't own
// directly (e.g. a game state). You flip the underlying state on press.
if (DrawToggleExternal("God Mode", IsGodModeOn()))
{
	SetGodMode(!IsGodModeOn());
}
```

### Per-row context

`IsCurrentRowSelected()` returns true if the row drawn by the most recent `Draw*`
call is the highlighted one. Use it for previews or contextual hints:

```cpp
DrawNumber("Radius", radius, 0.1f, 2);
if (IsCurrentRowSelected())
{
	DrawRadiusMarkerInWorld(radius);
}
```

## Navigating between submenus

Use `NavigateTo(id)` from inside an option block. The engine performs the switch
after `Draw()` returns and remembers a back-stack, so the Back button returns to
the previous submenu automatically.

```cpp
void MainMenuSubmenu::Draw()
{
	DrawTitle();

	if (DrawOption("Players"))          NavigateTo("players");
	if (DrawOption("Vehicle Options"))  NavigateTo("vehicle");
	if (DrawOption("Settings"))         NavigateTo("settings");
}
```

The `id` you pass must match the target submenu's `Id()`. If no submenu with
that id is registered, the navigation is a no-op — double-check the spelling.

To build a parent/child hierarchy, just have the parent `NavigateTo` the child's
id; the child needs no special "I have a parent" flag. The engine's back-stack
handles returning.

## Registration and the build

### Self-registration

The last line of your `.cpp` registers the submenu:

```cpp
REGISTER_SUBMENU(::Menu::MyFeatureSubmenu)
```

This constructs a single instance at startup and hands ownership to
`SubmenuRegistry`. From then on the engine can find it by `Id()`. There is **no**
central enum or array to update.

### Regenerating the project

The Visual Studio project is generated by premake from
[`premake5.lua`](../premake5.lua), which globs every `.h` and `.cpp` under
`Solution/source/`. New source files are picked up automatically — you do **not**
edit the `.vcxproj` by hand.

After creating `MyFeature.h` and `MyFeature.cpp` under
`Solution/source/Submenus/`, regenerate the project by running
[`generate.bat`](../generate.bat) from the repository root:

```bat
generate.bat
```

This runs `premake5 vs2022` and rewrites the solution so your new files are
included. Reload the solution in Visual Studio (or just build) and they will
compile. Re-run `generate.bat` any time you add, rename, or remove source files.

### Linking it into the menu

A registered submenu is reachable but not yet visible until something navigates
to it. Add a row to a parent submenu (usually
[`MainMenu.cpp`](../Solution/source/Submenus/MainMenu.cpp) or whichever submenu
should own it):

```cpp
if (DrawOption("My Feature"))  NavigateTo("my_feature");
```

## Lifecycle and lazy loading

If a submenu needs expensive setup (reading a file, building a list), do it in
`OnEnter()` and guard it with a `loaded` flag so it runs once:

```cpp
void MyFeatureSubmenu::OnEnter()
{
	if (!loaded)
		LoadData();
}
```

As a safety net you can also lazy-load on first `Draw()` (in case the submenu is
drawn without `OnEnter` firing), exactly as the Credits submenu does.

## Code style

Match the surrounding code (see the project's refactor conventions):

- `camelCase` for variables, `PascalCase` for functions/methods.
- One variable per line — no `float x = 0, y = 0;`.
- Declare variables at their point of use, not in a block at the top.
- Use real `bool` literals (`true` / `false`).

## Checklist

1. Create `YourFeature.h` / `.cpp` under `Solution/source/Submenus/`.
2. Derive from `Menu::Submenu`; implement `Id()`, `Title()`, `Draw()`.
3. Build rows with the `Draw*` helpers in display order.
4. End the `.cpp` with `REGISTER_SUBMENU(::Menu::YourFeatureSubmenu)`.
5. Run `generate.bat` to regenerate the project (premake picks up new files).
6. Add a `NavigateTo("your_id")` row in a parent submenu.
7. Build and test.
