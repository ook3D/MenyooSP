/*
* Menyoo PC - Grand Theft Auto V single-player trainer mod
*
* Per-frame tick entry points owned by Menu::Ticks. These currently delegate
* to the legacy `sub::*::Tick()` implementations so Routine.cpp no longer
* references the legacy namespaces directly. Once a given tick's state is
* fully owned here (via accessors or a moved implementation), each wrapper
* can be promoted in-place to a native implementation without touching
* Routine.cpp again.
*/
#pragma once

// HUD-related globals migrated from Routine.h. These are consumed by the
// per-frame tick functions defined in Ticks.cpp as well as by the toggle UI
// in Misc.cpp and the persistence layer in MenuConfig.cpp.
extern bool hideHUD;
extern bool showFullHUD;

// HUD helpers migrated from Routine.cpp. Kept at file scope (not in the
// Menu::Ticks namespace) to match the call-site convention used by the
// legacy code.
void DisplayFullHUDThisFrame(bool enabled);
void UpdateNearbyStuffArraysTick();

// Main per-frame entry point invoked by `MenyooMain`. Migrated from
// Routine.cpp; kept at file scope for the same reason as the helpers above.
void TickMenyooLoops();

namespace Menu { namespace Ticks {

	// --- Unconditional per-frame ticks (called from TickSubsystems) ---
	// Each is a thin wrapper around a legacy `sub::*::Tick()` body that
	// reads/writes file-scope state in the legacy .cpp. Migrating that
	// state into Menu::Ticks-owned globals is a follow-up; for now these merely
	// re-route the call site so legacy headers stop appearing in
	// Routine.cpp's include list.
	void GhostRider();          // sub::GhostRiderMode::Tick
	void VehicleAutoDrive();    // sub::VehicleAutoDrive::Tick
	void GravityGun();          // sub::GravityGun_catind::Tick
	void WaterHack();           // sub::WaterHack::Tick
	void LaserSight();          // sub::LaserSight_catind::Tick
	void AnimalRiding();        // sub::AnimalRiding::Tick
	void SpoonerMode();         // sub::Spooner::SpoonerMode::Tick
	void TeleportYachts();      // sub::TeleportLocations_catind::Yachts::Tick

	// --- Conditional per-frame ticks (caller still gates on legacy state) ---
	// These wrappers internally check the same legacy enable flag the old
	// Routine.cpp branch checked, so the call site collapses to a single
	// `Menu::Ticks::Foo();` line.
	void Clock();               // if (sub::Clock::loopClock) sub::Clock::DisplayClock();
	void BreatheStuff();        // if (playerBreatheStuff != None) SetSelfBreathePTFX(...)
	void Speedometer();         // if (loopSpeedo != OFF) sub::Speedo::SpeedoTick();
	void Tv();                  // if (loopBasicTV) sub::TVChannelStuff::DrawTvWhereItsSupposedToBe();

	// Master entry point used by `TickMenyooLoops()`. Calls all per-frame
	// subsystem ticks in the legacy order — both the delegating wrappers
	// above and the non-audit-set subsystems (Spooner, MagnetGun, snow,
	// VehicleTow/Cruise/Fly, MeteorShower, SmashAbility, RopeGun, GTA2Cam,
	// SetPTFXLopTick) that were previously driven directly from
	// Routine.cpp::TickSubsystems().
	void TickSubsystems();

}}
