#include "SpoonerTaskSequenceMenu.h"

#include "../Menu/Engine.h"
#include "../Menu/SubmenuRegistry.h"
#include "../Menu/Menu.h"      // Checkbox, Menu::bitController, VirtualKey, MenuPressTimer
#include "../Menu/Routine.h"
#include "Misc.h"              // shared search-string globals: dict, dict2, ...
#include "Spooner/SpoonerShared.h"

#include "../Natives/natives2.h"
#include "../Scripting/BlipEnums.h"
#include "../Scripting/Camera.h"
#include "../Scripting/Game.h"
#include "../Scripting/GTAblip.h"
#include "../Scripting/GTAentity.h"
#include "../Scripting/GTAped.h"
#include "../Scripting/GTAvehicle.h"
#include "../Scripting/World.h"
#include "../Scripting/WeaponIndivs.h"
#include "../Scripting/enums.h"
#include "../Util/FileLogger.h"
#include "../Util/GTAmath.h"
#include "../Util/keyboard.h"

#include "PedAnimationRuntime.h"
#include "PedSpeech.h"
#include "PtfxData.h"
#include "SettingsRuntime.h"                   // sub::g_settingsRGBA, AddPresetColourOptionsPreview
#include "Spooner/Databases.h"
#include "Spooner/EntityManagement.h"
#include "Spooner/SpoonerEntity.h"
#include "Spooner/SpoonerMode.h"
#include "Spooner/STSTask.h"
#include "Spooner/STSTasks.h"
#include "Spooner/SpoonerShared.h"               // SpoonerVector3ManualPlacementPtrs, _manualPlacementPrecision
#include "Spooner/SpoonerTaskSequenceStubs.h"  // _selectedSTST extern
#include "Spooner/SpoonerTaskSequence.h"

#include <algorithm>
#include <cctype>
#include <array>
#include <climits>
#include <cmath>
#include <string>
#include <utility>
#include <vector>

namespace Menu {

using sub::Spooner::Submenus::_manualPlacementPrecision;
using sub::Spooner::Submenus::SpoonerVector3ManualPlacementPtrs;
using sub::Spooner::Submenus::_selectedSTST;
using sub::Spooner::selectedEntity;

static std::string ToLowerCopy(const std::string& s)
{
	std::string out(s);
	std::transform(out.begin(), out.end(), out.begin(),
		[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	return out;
}

static std::string ToUpperCopy(const std::string& s)
{
	std::string out(s);
	std::transform(out.begin(), out.end(), out.begin(),
		[](unsigned char c) { return static_cast<char>(std::toupper(c)); });
	return out;
}

// Step an int by a delta, clamping to [lo, hi].
template <typename T>
static void StepClamped(T& value, long long delta, long long lo, long long hi)
{
	long long n = static_cast<long long>(value) + delta;
	if (n < lo) n = lo;
	else if (n > hi) n = hi;
	value = static_cast<T>(n);
}

// Replay of legacy DrawNumber-with-on-accept-input pattern for ints.
static bool PromptIntInputBox(int& target, int maxLen)
{
	std::string inputStr = Game::InputBox(std::string(), maxLen, std::string(),
		std::to_string(target));
	if (inputStr.empty()) return false;
	try { target = std::stoi(inputStr); return true; }
	catch (...) { Game::Print::PrintErrorInvalidInput(inputStr); return false; }
}

static bool PromptFloatInputBox(float& target, int maxLen, const std::string& title = std::string())
{
	std::string seed = std::to_string(target);
	if (seed.size() > 5) seed = seed.substr(0, 5);
	std::string inputStr = Game::InputBox(std::string(), maxLen, title, seed);
	if (inputStr.empty()) return false;
	try { target = std::stof(inputStr); return true; }
	catch (...) { Game::Print::PrintErrorInvalidInput(inputStr); return false; }
}

static void StepMapKey(const std::map<int, std::string>& m, int& key, bool forward)
{
	auto it = m.find(key);
	if (it == m.end()) return;
	if (forward)
	{
		auto next = std::next(it);
		if (next != m.end()) key = next->first;
	}
	else
	{
		if (it != m.begin())
		{
			--it;
			key = it->first;
		}
	}
}

static void DrawPAtCoord(Vector3& coord)
{
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	World::DrawMarker(MarkerType::ThickChevronUp, coord, Vector3(),
		Vector3(180.0f, 0.0f, 0.0f), Vector3(1, 1, 1), RGBA(177, 33, 193, 210));

	engine->AddOption("~italic~" + coord.ToString());

	auto& spoocam = sub::Spooner::SpoonerMode::spoonerModeCamera;
	if (!spoocam.IsActive())
	{
		if (engine->AddOption("Set Target To Player Position"))
		{
			coord = GTAentity(PLAYER_PED_ID()).GetPosition();
		}
	}
	else
	{
		if (engine->AddOption("Set Target To Camera Target"))
		{
			coord = spoocam.RaycastForCoord(Vector2(0.0f, 0.0f), 0, 160.0f, 3.0f);
		}
	}
	if (IS_WAYPOINT_ACTIVE())
	{
		if (engine->AddOption("Set Target To Waypoint"))
		{
			GTAblip wpBlip = GET_FIRST_BLIP_INFO_ID(BlipIcon::Waypoint);
			Vector3 wpCoords = wpBlip.GetPosition();
			wpCoords.z = World::GetGroundHeight(wpCoords);
			coord = wpCoords;
		}
	}

	if (engine->AddOption("Adjust Target Manually"))
	{
		SpoonerVector3ManualPlacementPtrs = std::make_tuple<GTAentity, Vector3*, Vector3*>(0, &coord, nullptr);
		engine->NavigateTo("spooner_vector3_manualplacement");
	}
}

static void DrawPAtEntity(GTAentity& targetEntity, EntityType eType = EntityType::ALL)
{
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	UINT8 eTypeInt = (UINT8)eType;
	std::array<std::string, 4> eTypeNames{ { "Entities", "Peds", "Vehicles", "Objects" } };
	engine->AddBreak("Available " +
		eTypeNames[eTypeInt >= 0 && eTypeInt < eTypeNames.size() ? eTypeInt : 0]);

	GTAentity myPed = PLAYER_PED_ID();
	if (myPed.Handle() != selectedEntity.handle.Handle() && myPed.Exists())
	{
		if (engine->AddCheckbox("Self", targetEntity == myPed))
		{
			targetEntity = myPed;
		}
	}

	for (auto& e : sub::Spooner::Databases::EntityDb)
	{
		if (e.handle.Handle() != selectedEntity.handle.Handle()
			&& e.handle.Exists()
			&& (eType == EntityType::ALL || e.type == eType))
		{
			const bool isCurrent = (targetEntity == e.handle);
			if (engine->AddCheckbox(e.hashName, isCurrent))
			{
				targetEntity = e.handle;
			}
			const bool rowSelected = (engine->printingOption == engine->ActiveSelection());
			if (rowSelected)
			{
				sub::Spooner::EntityManagement::ShowArrowAboveEntity(e.handle, RGBA(0, 255, 0, 200));
			}
		}
	}
}

// Vehicle-only picker (used by WarpIntoVehicle / EnterVehicle).
static void DrawVehiclePicker(GTAentity& targetVehicle)
{
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	engine->AddBreak("Available Vehicles");

	for (auto& e : sub::Spooner::Databases::EntityDb)
	{
		if (e.handle.Handle() != selectedEntity.handle.Handle() && e.handle.IsVehicle())
		{
			const bool isCurrent = (targetVehicle == e.handle);
			if (engine->AddCheckbox(e.hashName, isCurrent))
			{
				targetVehicle = e.handle;
			}
			const bool rowSelected = (engine->printingOption == engine->ActiveSelection());
			if (rowSelected)
			{
				sub::Spooner::EntityManagement::ShowArrowAboveEntity(e.handle, RGBA(0, 255, 0, 200));
			}
		}
	}
}

// Heading +/- row, wrapping at -180/180.
static void DrawHeadingRow(const std::string& label, float& heading)
{
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;
	::Menu::InputResult res = engine->AddNumber(label, heading, 0);
	if (res.rightPressed) { heading += 1.0f; if (heading > 180.0f) heading = -180.0f; }
	if (res.leftPressed)  { heading -= 1.0f; if (heading < -180.0f) heading = 180.0f; }
}

// Pitch +/- row, wrapping at -90/90.
static void DrawPitchRow(const std::string& label, float& pitch)
{
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;
	::Menu::InputResult res = engine->AddNumber(label, pitch, 2);
	if (res.rightPressed) { pitch += 1.0f; if (pitch > 90.0f) pitch = -90.0f; }
	if (res.leftPressed)  { pitch -= 1.0f; if (pitch < -90.0f) pitch = 90.0f; }
}

// Walk/Run texter row (mirrors AddTexter("Speed", ..., {"Walk","Run"})).
static void DrawWalkRunSpeed(float& speed)
{
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;
	const std::vector<std::string> options{ "Walk", "Run" };
	int selected = (speed > 2.5f ? 1 : 0);
	::Menu::InputResult res = engine->AddTextList("Speed", selected, options);
	if (res.rightPressed) { if (speed < 4.0f) speed = 4.0f; }
	if (res.leftPressed)  { if (speed > 1.0f) speed = 1.0f; }
}

// Scroll-sensitivity row for the manual-precision controls.
static void DrawScrollSensitivityRow()
{
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;
	::Menu::InputResult res = engine->AddNumber("Scroll Sensitivity",
		_manualPlacementPrecision, 4);
	// Legacy used inverted +/- here: right -> /10, left -> *10.
	if (res.rightPressed) { if (_manualPlacementPrecision > 0.0001f) _manualPlacementPrecision /= 10; }
	if (res.leftPressed)  { if (_manualPlacementPrecision < 10.0f)   _manualPlacementPrecision *= 10; }
}

static bool DrawVector3OffsetRows(Vector3& v, bool clamp180)
{
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return false;
	bool changed = false;
	auto axisRow = [&](const std::string& label, float& component) {
		::Menu::InputResult res = engine->AddNumber(label, component, 4);
		if (res.rightPressed)
		{
			if (!clamp180 || component < 180.0f) { component += _manualPlacementPrecision; changed = true; }
		}
		if (res.leftPressed)
		{
			if (!clamp180 || component > -180.0f) { component -= _manualPlacementPrecision; changed = true; }
		}
	};
	axisRow("X", v.x);
	axisRow("Y", v.y);
	axisRow("Z", v.z);
	return changed;
}

static void DrawDrivingStyleRow(int& drivingStyle)
{
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	int cdsi = 0;
	std::vector<std::string> cds;
	for (UINT i = 0; i < DrivingStyle::nameArray.size(); i++)
	{
		if (DrivingStyle::nameArray[i].style == drivingStyle)
		{
			cdsi = static_cast<int>(i);
			cds.push_back(DrivingStyle::nameArray[i].name);
			break;
		}
	}
	if (cds.empty()) return;
	::Menu::InputResult res = engine->AddTextList("Driving Style", 0, cds);
	if (res.rightPressed)
	{
		if (cdsi < static_cast<int>(DrivingStyle::nameArray.size()) - 1)
		{
			cdsi++;
			drivingStyle = DrivingStyle::nameArray[cdsi].style;
		}
	}
	if (res.leftPressed)
	{
		if (cdsi > 0)
		{
			cdsi--;
			drivingStyle = DrivingStyle::nameArray[cdsi].style;
		}
	}
}

// Speed (KMPH) row with on-accept InputBox parsing — used by drive tasks.
static void DrawSpeedKmphRow(const std::string& label, float& speedInKmph)
{
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	::Menu::InputResult res = engine->AddNumber(label, speedInKmph, 0);
	if (res.rightPressed) { if (speedInKmph < FLT_MAX)  speedInKmph += 1.0f; }
	if (res.leftPressed)  { if (speedInKmph > -FLT_MAX) speedInKmph -= 1.0f; }

	const bool rowSelected = (engine->printingOption == engine->ActiveSelection());
	if (rowSelected && MenuPressTimer::IsButtonTapped(MenuPressTimer::Button::Accept))
	{
		PromptFloatInputBox(speedInKmph, 6);
	}
}

// "Apply Only Once" toggle — mirrors the legacy bDuration-jiggle UX where
// flipping the bool also assigns duration/-2/looped fields.
static void DrawApplyOnlyOnceToggle(const char* label, int& duration, bool& isLoopedTask,
	int defaultLoopDuration)
{
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;
	if (engine->AddCheckbox(label, !isLoopedTask, Checkbox::BOXTICK, Checkbox::BOXBLANK))
	{
		if (isLoopedTask)
		{
			duration = -2;
			isLoopedTask = false;
		}
		else
		{
			duration = defaultLoopDuration;
			isLoopedTask = true;
		}
	}
}

namespace TaskDraw {

using namespace sub::Spooner;

static void Nothing() {}
static void Pause() {}
static void UsePhone() {}
static void Writhe() {}
static void WanderFreely() {}
static void RemoveBlip() {}

static void SetHealth()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::SetHealth>();
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	DrawApplyOnlyOnceToggle("Apply Only Once", tskPtr->duration, tskPtr->isLoopedTask, 5000);

	::Menu::InputResult res = engine->AddNumber("Health Value",
		static_cast<double>(tskPtr->healthValue), 0);
	if (res.rightPressed) { if (tskPtr->healthValue < INT_MAX) tskPtr->healthValue++; }
	if (res.leftPressed)  { if (tskPtr->healthValue > 0)       tskPtr->healthValue--; }

	const bool rowSelected = (engine->printingOption == engine->ActiveSelection());
	if (rowSelected && MenuPressTimer::IsButtonTapped(MenuPressTimer::Button::Accept))
	{
		PromptIntInputBox(tskPtr->healthValue, 5);
	}
}

static void AddBlip()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::AddBlip>();
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	if (engine->AddOption(tskPtr->label.empty() ? std::string("Label") : tskPtr->label))
	{
		tskPtr->label = Game::InputBox(tskPtr->label, 26U, "Enter custom label:", tskPtr->label);
	}

	{
		const std::vector<std::string> options{ BlipColour::vNames.at(tskPtr->colour) };
		::Menu::InputResult res = engine->AddTextList("Colour", 0, options);
		if (res.rightPressed) StepMapKey(BlipColour::vNames, tskPtr->colour, true);
		if (res.leftPressed)  StepMapKey(BlipColour::vNames, tskPtr->colour, false);
	}
	{
		const std::vector<std::string> options{ BlipIcon::vNames.at(tskPtr->icon) };
		::Menu::InputResult res = engine->AddTextList("Icon", 0, options);
		if (res.rightPressed) StepMapKey(BlipIcon::vNames, tskPtr->icon, true);
		if (res.leftPressed)  StepMapKey(BlipIcon::vNames, tskPtr->icon, false);
	}

	{
		double alphaDouble = static_cast<double>(tskPtr->alpha);
		::Menu::InputResult res = engine->AddNumber("Alpha", alphaDouble, 2);
		if (res.rightPressed) { if (tskPtr->alpha < UINT8_MAX) tskPtr->alpha = (uint8_t)(tskPtr->alpha + 1); }
		if (res.leftPressed)  { if (tskPtr->alpha > 0u)        tskPtr->alpha = (uint8_t)(tskPtr->alpha - 1); }
	}
	{
		::Menu::InputResult res = engine->AddNumber("Scale", tskPtr->scale, 2);
		if (res.rightPressed) { if (tskPtr->scale < FLT_MAX) tskPtr->scale += 0.01f; }
		if (res.leftPressed)  { if (tskPtr->scale > 0.0f)    tskPtr->scale -= 0.01f; }
	}

	if (engine->AddCheckbox("Flashing", tskPtr->isFlashing))    tskPtr->isFlashing = !tskPtr->isFlashing;
	if (engine->AddCheckbox("Friendly", tskPtr->isFriendly))    tskPtr->isFriendly = !tskPtr->isFriendly;
	if (engine->AddCheckbox("Short-Range", tskPtr->isShortRange)) tskPtr->isShortRange = !tskPtr->isShortRange;
	if (engine->AddCheckbox("Show Route", tskPtr->showRoute))   tskPtr->showRoute = !tskPtr->showRoute;

	{
		const std::vector<std::string> options{ std::string() };
		::Menu::InputResult res = engine->AddTextList("Display Number",
			tskPtr->showNumber, options);
		if (res.rightPressed) { if (tskPtr->showNumber < INT_MAX) tskPtr->showNumber++; }
		if (res.leftPressed)  { if (tskPtr->showNumber > INT_MIN) tskPtr->showNumber--; }
	}

	if (engine->AddCheckbox("Show Cone", tskPtr->showCone)) tskPtr->showCone = !tskPtr->showCone;

	{
		::Menu::InputResult res = engine->AddNumber("Display ID",
			static_cast<double>(tskPtr->displayId), 1);
		if (res.rightPressed) { if (tskPtr->displayId < INT_MAX) tskPtr->displayId++; }
		if (res.leftPressed)  { if (tskPtr->displayId > 0)       tskPtr->displayId--; }
	}
	{
		::Menu::InputResult res = engine->AddNumber("Priority",
			static_cast<double>(tskPtr->priority), 1);
		if (res.rightPressed) { if (tskPtr->priority < INT_MAX) tskPtr->priority++; }
		if (res.leftPressed)  { if (tskPtr->priority > 0)       tskPtr->priority--; }
	}

	if (engine->AddCheckbox("Sync Rotation With Entity", tskPtr->syncRotation))
		tskPtr->syncRotation = !tskPtr->syncRotation;

	bool nonSelectable = (tskPtr->displayMode == 8);
	if (engine->AddCheckbox("Non-Selectable", nonSelectable))
	{
		nonSelectable = !nonSelectable;
	}
	tskPtr->displayMode = nonSelectable ? 8 : 2;
}

static void ThrowProjectile()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::ThrowProjectile>();
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	World::DrawMarker(MarkerType::ThickChevronUp, tskPtr->targetPos, Vector3(),
		Vector3(180.0f, 0.0f, 0.0f), Vector3(1, 1, 1), RGBA(177, 33, 193, 210));

	engine->AddOption("~italic~" + tskPtr->targetPos.ToString());

	auto& spoocam = SpoonerMode::spoonerModeCamera;
	if (!spoocam.IsActive())
	{
		if (engine->AddOption("Set Target To Player Position"))
		{
			tskPtr->targetPos = GTAentity(PLAYER_PED_ID()).GetPosition();
		}
	}
	else
	{
		if (engine->AddOption("Set Target To Camera Target"))
		{
			tskPtr->targetPos = spoocam.RaycastForCoord(Vector2(0.0f, 0.0f), 0, 160.0f, 3.0f);
		}
	}
}

static void FaceDirection()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::FaceDirection>();

	World::DrawMarker(MarkerType::DebugSphere,
		selectedEntity.handle.GetPosition().PointOnCircle(1.0f, tskPtr->heading),
		Vector3(0, 0, tskPtr->heading), Vector3(), Vector3(0.3f, 0.3f, 0.3f),
		RGBA(177, 33, 193, 210));

	DrawHeadingRow("Direction To Face", tskPtr->heading);
}

static void FaceEntity()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::FaceEntity>();
	DrawPAtEntity(tskPtr->targetEntity);
}

static void LookAtCoord()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::LookAtCoord>();
	DrawPAtCoord(tskPtr->coord);
}

static void LookAtEntity()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::LookAtEntity>();
	DrawPAtEntity(tskPtr->targetEntity);
}

static void LookAtCoordEyesOnly()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::LookAtCoordEyesOnly>();
	DrawPAtCoord(tskPtr->coord);
}

static void LookAtEntityEyesOnly()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::LookAtEntityEyesOnly>();
	DrawPAtEntity(tskPtr->targetEntity);
}

static void TeleportToCoord()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::TeleportToCoord>();
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	if (selectedEntity.type == EntityType::PED)
	{
		if (engine->AddCheckbox("Take Vehicle Too (If In One)", tskPtr->takeVehicleToo,
			Checkbox::BOXTICK, Checkbox::BOXBLANK))
		{
			tskPtr->takeVehicleToo = !tskPtr->takeVehicleToo;
		}
	}
	DrawPAtCoord(tskPtr->destination);
}

static void SeekCoverAtCoord()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::SeekCoverAtCoord>();
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	if (engine->AddCheckbox("Allow Peeking", tskPtr->canPeekInCover,
		Checkbox::BOXTICK, Checkbox::BOXBLANK))
	{
		tskPtr->canPeekInCover = !tskPtr->canPeekInCover;
	}
	DrawPAtCoord(tskPtr->coverPos);
}

static void SlideToCoord()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::SlideToCoord>();
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	World::DrawMarker(MarkerType::DebugSphere,
		selectedEntity.handle.GetPosition().PointOnCircle(1.0f, tskPtr->heading),
		Vector3(), Vector3(0, 0, tskPtr->heading), Vector3(0.3f, 0.3f, 0.3f),
		RGBA(177, 33, 193, 210));

	{
		::Menu::InputResult res = engine->AddNumber("Speed", tskPtr->speed, 1);
		if (res.rightPressed) { if (tskPtr->speed < 250.0f) tskPtr->speed += 0.5f; }
		if (res.leftPressed)  { if (tskPtr->speed > 0.0f)   tskPtr->speed -= 0.5f; }
	}
	DrawHeadingRow("Direction To Face", tskPtr->heading);
	DrawPAtCoord(tskPtr->destination);
}

static void GoToCoord()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::GoToCoord>();
	DrawWalkRunSpeed(tskPtr->speed);
	DrawPAtCoord(tskPtr->destination);
}

static void FollowRoute()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::FollowRoute>();
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	const Vector3& entityPos = selectedEntity.handle.GetPosition();
	for (auto porit = tskPtr->route.begin(); porit != tskPtr->route.end(); ++porit)
	{
		World::DrawMarker(MarkerType::ThickChevronUp, *porit, Vector3(),
			Vector3(180.0f, 0.0f, 0.0f), Vector3(1, 1, 1), RGBA(177, 33, 193, 210));
		World::DrawLine(porit == tskPtr->route.begin() ? entityPos : *(porit - 1),
			*porit, RGBA(177, 33, 193, 210));
	}

	DrawWalkRunSpeed(tskPtr->speed);

	for (auto cit = tskPtr->route.begin(); cit != tskPtr->route.end();)
	{
		engine->AddOption("~italic~" + cit->ToString());

		const bool rowSelected = (engine->printingOption == engine->ActiveSelection());
		bool removeCoordPressed = false;
		if (rowSelected)
		{
			if (Menu::bitController)
			{
				engine->AddInstructionalButton(INPUT_SCRIPT_RLEFT, "Remove coord", false);
				removeCoordPressed = IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_SCRIPT_RLEFT) != 0;
			}
			else
			{
				engine->AddInstructionalButton(static_cast<int>(VirtualKey::B), "Remove coord", true);
				removeCoordPressed = IsKeyJustUp(VirtualKey::B);
			}
		}
		if (removeCoordPressed) cit = tskPtr->route.erase(cit);
		else ++cit;
	}

	auto& spoocam = SpoonerMode::spoonerModeCamera;
	if (!spoocam.IsActive())
	{
		if (engine->AddCheckbox("Add Player Position", true,
			Checkbox::SMALLNEWSTAR, Checkbox::NONE))
		{
			tskPtr->route.push_back(GTAentity(PLAYER_PED_ID()).GetPosition());
		}
	}
	else
	{
		if (engine->AddCheckbox("Add Camera Target Position", true,
			Checkbox::SMALLNEWSTAR, Checkbox::NONE))
		{
			tskPtr->route.push_back(spoocam.RaycastForCoord(Vector2(0.0f, 0.0f), 0, 160.0f, 3.0f));
		}
	}
}

static void FollowEntity()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::FollowEntity>();
	DrawWalkRunSpeed(tskPtr->speed);
	DrawPAtEntity(tskPtr->targetEntity);
}

static void PatrolInRange()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::PatrolInRange>();
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	::Menu::InputResult res = engine->AddNumber("Radius", tskPtr->radius, 0);
	const bool rowSelected = (engine->printingOption == engine->ActiveSelection());
	if (rowSelected)
	{
		sub::Spooner::EntityManagement::DrawRadiusDisplayingMarker(
			selectedEntity.handle.GetPosition(), tskPtr->radius);
	}
	if (res.rightPressed) { if (tskPtr->radius < FLT_MAX) tskPtr->radius += 1.0f; }
	if (res.leftPressed)  { if (tskPtr->radius > 0.0f)    tskPtr->radius -= 1.0f; }

	DrawPAtCoord(tskPtr->coord);
}

static void FleeFromCoord()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::FleeFromCoord>();
	DrawPAtCoord(tskPtr->originCoords);
}

static void NearestAppropriateAction()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::NearestAppropriateAction>();
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	::Menu::InputResult res = engine->AddNumber("Search Radius", tskPtr->searchRadius, 0);
	const bool rowSelected = (engine->printingOption == engine->ActiveSelection());
	if (rowSelected)
	{
		sub::Spooner::EntityManagement::DrawRadiusDisplayingMarker(
			selectedEntity.handle.GetPosition(), tskPtr->searchRadius);
	}
	if (res.rightPressed) { if (tskPtr->searchRadius < FLT_MAX) tskPtr->searchRadius += 1.0f; }
	if (res.leftPressed)  { if (tskPtr->searchRadius > 0.0f)    tskPtr->searchRadius -= 1.0f; }

	if (engine->AddCheckbox("Warp", tskPtr->warp, Checkbox::BOXTICK, Checkbox::BOXBLANK))
	{
		tskPtr->warp = !tskPtr->warp;
	}
}

static void ScenarioAction()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::ScenarioAction>();
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	engine->AddBreak("---Actions---");

	if (engine->AddOption("All Actions"))
	{
		engine->NavigateTo("spooner_tasksequence_scenario_action_list");
	}

	for (auto& sn : sub::AnimationTaskScenarios::vNamedScenarios)
	{
		if (engine->AddCheckbox(sn.name, tskPtr->scenarioName == sn.label))
		{
			tskPtr->scenarioName = sn.label;
		}
	}
}

static void PlayAnimation()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::PlayAnimation>();
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	dict.clear();

	if (engine->AddOption("Settings"))
		engine->NavigateTo("spooner_tasksequence_play_animation_settings");
	if (engine->AddOption("All Animations"))
		engine->NavigateTo("spooner_tasksequence_play_animation_all_ped_anims");

	if (!sub::AnimationMenu::presetPedAnims.empty())
	{
		engine->AddBreak("---Basic---");
		for (auto& npa : sub::AnimationMenu::presetPedAnims)
		{
			const bool isCurrent = (npa.animDict == tskPtr->animDict
				&& npa.animName == tskPtr->animName);
			if (engine->AddCheckbox(npa.caption, isCurrent))
			{
				tskPtr->animDict = npa.animDict;
				tskPtr->animName = npa.animName;
			}
		}
	}

	std::vector<std::pair<std::string, std::string>> vFavAnims;
	sub::GetFavouriteAnimations(vFavAnims);
	if (!vFavAnims.empty())
	{
		engine->AddBreak("---Favourites---");
		for (auto& animFav : vFavAnims)
		{
			const bool isCurrent = (animFav.first == tskPtr->animDict
				&& animFav.second == tskPtr->animName);
			if (engine->AddCheckbox(animFav.first + ", " + animFav.second, isCurrent))
			{
				tskPtr->animDict = animFav.first;
				tskPtr->animName = animFav.second;
			}
		}
	}

	engine->AddBreak("---Custom---");
	if (engine->AddOption(tskPtr->animDict))
	{
		tskPtr->animDict = Game::InputBox(tskPtr->animDict, 126U,
			"Enter Dictionary:", tskPtr->animDict);
	}
	if (engine->AddOption(tskPtr->animName))
	{
		tskPtr->animName = Game::InputBox(tskPtr->animName, 126U,
			"Enter Name:", tskPtr->animName);
	}
}

static void SetActiveWeapon()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::SetActiveWeapon>();
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	engine->AddBreak("---Weapon---");
	for (auto& w : WeaponIndivs::vWeaponLabels)
	{
		if (engine->AddCheckbox(w.second, tskPtr->weaponHash == w.first,
			Checkbox::TICK, Checkbox::NONE))
		{
			tskPtr->weaponHash = w.first;
		}
	}
}

static void AimAtCoord()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::AimAtCoord>();
	DrawPAtCoord(tskPtr->coord);
}

static void AimAtEntity()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::AimAtEntity>();
	DrawPAtEntity(tskPtr->targetEntity);
}

static void ShootAtCoord()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::ShootAtCoord>();
	DrawPAtCoord(tskPtr->coord);
}

static void ShootAtEntity()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::ShootAtEntity>();
	DrawPAtEntity(tskPtr->targetEntity);
}

static void FightHatedTargets()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::FightHatedTargets>();
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	::Menu::InputResult res = engine->AddNumber("Radius", tskPtr->radius, 0);
	const bool rowSelected = (engine->printingOption == engine->ActiveSelection());
	if (rowSelected)
	{
		sub::Spooner::EntityManagement::DrawRadiusDisplayingMarker(
			selectedEntity.handle.GetPosition(), tskPtr->radius);
	}
	if (res.rightPressed) { if (tskPtr->radius < FLT_MAX) tskPtr->radius += 1.0f; }
	if (res.leftPressed)  { if (tskPtr->radius > 0.0f)    tskPtr->radius -= 1.0f; }
}

static void FightPed()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::FightPed>();
	DrawPAtEntity(tskPtr->targetEntity, EntityType::PED);
}

static void SpeakToPed()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::SpeakToPed>();
	DrawPAtEntity(tskPtr->targetEntity, EntityType::PED);
}

static void PlaySpeechWithVoice()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::PlaySpeechWithVoice>();
	(void)tskPtr;
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	std::string& searchStr = dict;
	sub::Speech::_currVoiceInfo = nullptr;

	const std::string searchLabel = searchStr.empty() ? std::string("SEARCH") : searchStr;
	if (engine->AddOption(searchLabel))
	{
		searchStr = Game::InputBox(searchStr, 126U, "SEARCH", ToLowerCopy(searchStr));
		searchStr = ToUpperCopy(searchStr);
	}

	for (auto& v : sub::Speech::vVoiceData)
	{
		if (!searchStr.empty())
		{
			if (ToUpperCopy(v.voiceName).find(searchStr) == std::string::npos)
				continue;
		}
		if (engine->AddOption(v.voiceName))
		{
			sub::Speech::_currVoiceInfo = &v;
			engine->NavigateTo("spooner_tasksequence_play_speech_with_voice_in_voice");
		}
	}
}

static void WarpIntoVehicle()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::WarpIntoVehicle>();
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	const std::vector<std::string> seatOptions{
		"First Free Seat", "Driver", "Passenger", "Back Seat - Left", "Back Seat - Right" };
	int seatDisplay = tskPtr->seatIndex + 2;
	::Menu::InputResult res = engine->AddTextList("Seat", seatDisplay, seatOptions);
	if (res.rightPressed) { if (tskPtr->seatIndex < 20) tskPtr->seatIndex++; }
	if (res.leftPressed)  { if (tskPtr->seatIndex > -2) tskPtr->seatIndex--; }

	DrawVehiclePicker(tskPtr->vehicleToEnter);
}

static void EnterVehicle()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::EnterVehicle>();
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	const std::vector<std::string> seatOptions{
		"First Free Seat", "Driver", "Passenger", "Back Seat - Left", "Back Seat - Right" };
	int seatDisplay = tskPtr->seatIndex + 2;
	::Menu::InputResult res = engine->AddTextList("Seat", seatDisplay, seatOptions);
	if (res.rightPressed) { if (tskPtr->seatIndex < 20) tskPtr->seatIndex++; }
	if (res.leftPressed)  { if (tskPtr->seatIndex > -2) tskPtr->seatIndex--; }

	DrawVehiclePicker(tskPtr->vehicleToEnter);
}

static void DriveWander()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::DriveWander>();
	DrawSpeedKmphRow("Speed (KMPH)", tskPtr->speedInKmph);
	DrawDrivingStyleRow(tskPtr->drivingStyle);
}

static void DriveToCoord()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::DriveToCoord>();
	DrawSpeedKmphRow("Speed (KMPH)", tskPtr->speedInKmph);
	DrawDrivingStyleRow(tskPtr->drivingStyle);
	DrawPAtCoord(tskPtr->destination);
}

static void DriveFollowEntity()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::DriveFollowEntity>();
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	DrawSpeedKmphRow("Max Speed (KMPH)", tskPtr->speedInKmph);
	DrawDrivingStyleRow(tskPtr->drivingStyle);

	::Menu::InputResult res = engine->AddNumber("Minimum Distance", tskPtr->minDistance, 1);
	if (res.rightPressed) { if (tskPtr->minDistance < FLT_MAX) tskPtr->minDistance += 0.5f; }
	if (res.leftPressed)  { if (tskPtr->minDistance > 0.0f)    tskPtr->minDistance -= 0.5f; }

	DrawPAtEntity(tskPtr->targetEntity);
}

static void DriveLandPlane()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::DriveLandPlane>();
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	World::DrawMarker(MarkerType::ThickChevronUp, tskPtr->runwayStart, Vector3(),
		Vector3(180.0f, 0.0f, 0.0f), Vector3(1, 1, 1), RGBA(177, 33, 193, 210));
	World::DrawMarker(MarkerType::ThickChevronUp, tskPtr->runwayEnd, Vector3(),
		Vector3(180.0f, 0.0f, 0.0f), Vector3(1, 1, 1), RGBA(177, 33, 193, 210));
	World::DrawLine(tskPtr->runwayStart, tskPtr->runwayEnd, RGBA(177, 33, 193, 210));

	std::array<std::pair<std::string, Vector3*>, 2> nasArr{ {
		{ "Runway Start", &tskPtr->runwayStart },
		{ "Runway End",   &tskPtr->runwayEnd   }
	} };
	for (auto& nas : nasArr)
	{
		engine->AddBreak(nas.first);
		engine->AddOption("~italic~" + nas.second->ToString());

		auto& spoocam = SpoonerMode::spoonerModeCamera;
		if (!spoocam.IsActive())
		{
			if (engine->AddOption("Set To Player Position"))
				*nas.second = GTAentity(PLAYER_PED_ID()).GetPosition();
		}
		else
		{
			if (engine->AddOption("Set To Camera Target"))
				*nas.second = spoocam.RaycastForCoord(Vector2(0.0f, 0.0f), 0, 160.0f, 3.0f);
		}
		if (IS_WAYPOINT_ACTIVE())
		{
			if (engine->AddOption("Set To Waypoint"))
			{
				GTAblip wpBlip = GET_FIRST_BLIP_INFO_ID(BlipIcon::Waypoint);
				Vector3 wpCoords = wpBlip.GetPosition();
				wpCoords.z = World::GetGroundHeight(wpCoords);
				*nas.second = wpCoords;
			}
		}
		if (engine->AddOption("Adjust Manually"))
		{
			SpoonerVector3ManualPlacementPtrs = std::tuple<GTAentity, Vector3*, Vector3*>(GTAentity(0), nas.second, nullptr);
			engine->NavigateTo("spooner_vector3_manualplacement");
		}
	}
}

static void AchieveVehicleForwardSpeed()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::AchieveVehicleForwardSpeed>();
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	DrawSpeedKmphRow("Speed (KMPH)", tskPtr->speedInKmph);

	if (engine->AddCheckbox("On Ground Only", tskPtr->onGroundOnly,
		Checkbox::BOXTICK, Checkbox::BOXBLANK))
	{
		tskPtr->onGroundOnly = !tskPtr->onGroundOnly;
	}
}

static void ChangeTextureVariation()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::ChangeTextureVariation>();
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	::Menu::InputResult res = engine->AddNumber("New Value",
		static_cast<double>(tskPtr->newValue), 0);
	if (res.rightPressed) { if (tskPtr->newValue < UINT8_MAX) tskPtr->newValue = (UINT8)(tskPtr->newValue + 1); }
	if (res.leftPressed)  { if (tskPtr->newValue > 0)         tskPtr->newValue = (UINT8)(tskPtr->newValue - 1); }
}

static void AchieveVelocity()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::AchieveVelocity>();
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	const GTAentity& thisEntity = selectedEntity.handle;
	const Vector3& entityPos = thisEntity.GetPosition();
	const Vector3& entityRot = thisEntity.Rotation_get();
	const Vector3& rotForVel = (tskPtr->isRelative ? entityRot : Vector3::Zero())
		+ Vector3(tskPtr->pitch, 0.0f, tskPtr->heading);
	const Vector3& dirForVel = Vector3::RotationToDirection(rotForVel);

	const ModelDimensions& entityMd = thisEntity.ModelDimensions();
	const Vector3& markerPos = entityPos + (dirForVel * (entityMd.Dim1.y + 2.0f));
	World::DrawMarker(MarkerType::DebugSphere, markerPos, rotForVel, Vector3(),
		Vector3(0.3f, 0.3f, 0.3f), RGBA(177, 33, 193, 210));
	World::DrawLine(entityPos, markerPos, RGBA(177, 33, 193, 210));

	if (engine->AddCheckbox("Relative", tskPtr->isRelative,
		Checkbox::BOXTICK, Checkbox::BOXBLANK))
	{
		tskPtr->isRelative = !tskPtr->isRelative;
	}
	DrawHeadingRow("Direction", tskPtr->heading);
	DrawPitchRow("Pitch", tskPtr->pitch);

	::Menu::InputResult res = engine->AddNumber("Magnitude (m/s)", tskPtr->magnitude, 1);
	if (res.rightPressed) { if (tskPtr->magnitude < FLT_MAX)  tskPtr->magnitude += 0.5f; }
	if (res.leftPressed)  { if (tskPtr->magnitude > -FLT_MAX) tskPtr->magnitude -= 0.5f; }
}

static void AchievePushForce()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::AchievePushForce>();
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	const GTAentity& thisEntity = selectedEntity.handle;
	const Vector3& entityPos = thisEntity.GetPosition();
	const Vector3& entityRot = thisEntity.Rotation_get();
	const Vector3& rotForFrc = (tskPtr->isRelative ? entityRot : Vector3::Zero())
		+ Vector3(tskPtr->pitch, 0.0f, tskPtr->heading);
	const Vector3& dirForFrc = Vector3::RotationToDirection(rotForFrc);

	const ModelDimensions& entityMd = thisEntity.ModelDimensions();
	const Vector3& markerPos = entityPos + (dirForFrc * (entityMd.Dim1.y + 2.0f));
	World::DrawMarker(MarkerType::DebugSphere, markerPos, rotForFrc, Vector3(),
		Vector3(0.3f, 0.3f, 0.3f), RGBA(177, 33, 193, 210));
	World::DrawLine(entityPos, markerPos, RGBA(177, 33, 193, 210));

	World::DrawMarker(MarkerType::DebugSphere,
		thisEntity.GetOffsetInWorldCoords(tskPtr->offsetVector), rotForFrc, Vector3(),
		Vector3(0.07f, 0.07f, 0.07f), RGBA(177, 33, 193, 210));

	if (engine->AddCheckbox("Relative", tskPtr->isRelative,
		Checkbox::BOXTICK, Checkbox::BOXBLANK))
	{
		tskPtr->isRelative = !tskPtr->isRelative;
	}

	const std::vector<std::string> vForceTypeNames{ "0", "Standard", "2", "Standard Weak" };
	::Menu::InputResult resFt = engine->AddTextList("Force Type",
		tskPtr->forceType, vForceTypeNames);
	if (resFt.rightPressed) { if (tskPtr->forceType < (int)vForceTypeNames.size() - 1) tskPtr->forceType++; }
	if (resFt.leftPressed)  { if (tskPtr->forceType > 0) tskPtr->forceType--; }

	DrawHeadingRow("Direction", tskPtr->heading);
	DrawPitchRow("Pitch", tskPtr->pitch);

	::Menu::InputResult res = engine->AddNumber("Magnitude", tskPtr->magnitude, 1);
	if (res.rightPressed) { if (tskPtr->magnitude < FLT_MAX)  tskPtr->magnitude += 0.5f; }
	if (res.leftPressed)  { if (tskPtr->magnitude > -FLT_MAX) tskPtr->magnitude -= 0.5f; }

	engine->AddBreak("---Region To Push---");
	DrawScrollSensitivityRow();
	DrawVector3OffsetRows(tskPtr->offsetVector, /*clamp180*/false);
	if (engine->AddOption("RESET")) tskPtr->offsetVector.clear();
}

static void OscillateToPoint()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::OscillateToPoint>();
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	::Menu::InputResult resA = engine->AddNumber("Angular Frequency",
		tskPtr->angleFreq, 2);
	if (resA.rightPressed) { if (tskPtr->angleFreq < FLT_MAX - 2.0f) tskPtr->angleFreq += 0.01f; }
	if (resA.leftPressed)  { if (tskPtr->angleFreq > 0.0f)           tskPtr->angleFreq -= 0.01f; }

	::Menu::InputResult resD = engine->AddNumber("Damping Ratio",
		tskPtr->dampRatio, 2);
	if (resD.rightPressed) { if (tskPtr->dampRatio < FLT_MAX - 2.0f) tskPtr->dampRatio += 0.05f; }
	if (resD.leftPressed)  { if (tskPtr->dampRatio > 0.0f)           tskPtr->dampRatio -= 0.05f; }

	DrawPAtCoord(tskPtr->destination);
}

static void OscillateToEntity()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::OscillateToEntity>();
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	if (tskPtr->targetEntity.Exists())
	{
		World::DrawMarker(MarkerType::DebugSphere,
			tskPtr->targetEntity.GetOffsetInWorldCoords(tskPtr->offsetVector),
			Vector3(), Vector3(), Vector3(0.07f, 0.07f, 0.07f), RGBA(177, 33, 193, 210));
	}

	::Menu::InputResult resA = engine->AddNumber("Angular Frequency",
		tskPtr->angleFreq, 2);
	if (resA.rightPressed) { if (tskPtr->angleFreq < FLT_MAX - 2.0f) tskPtr->angleFreq += 0.01f; }
	if (resA.leftPressed)  { if (tskPtr->angleFreq > 0.0f)           tskPtr->angleFreq -= 0.01f; }

	::Menu::InputResult resD = engine->AddNumber("Damping Ratio",
		tskPtr->dampRatio, 2);
	if (resD.rightPressed) { if (tskPtr->dampRatio < FLT_MAX - 2.0f) tskPtr->dampRatio += 0.05f; }
	if (resD.leftPressed)  { if (tskPtr->dampRatio > 0.0f)           tskPtr->dampRatio -= 0.05f; }

	engine->AddBreak("---Destination Offset---");
	DrawScrollSensitivityRow();
	DrawVector3OffsetRows(tskPtr->offsetVector, /*clamp180*/false);
	if (engine->AddOption("RESET")) tskPtr->offsetVector.clear();

	DrawPAtEntity(tskPtr->targetEntity);
}

static void FreezeInPlace()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::FreezeInPlace>();
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	const std::array<std::pair<STSTasks::FreezeInPlace::eFreezeType, std::string>, 3> vFrzTypNames{ {
		{ STSTasks::FreezeInPlace::eFreezeType::FREEZETYPE_FREEZE,        "Freeze" },
		{ STSTasks::FreezeInPlace::eFreezeType::FREEZETYPE_UNFREEZE,      "Unfreeze" },
		{ STSTasks::FreezeInPlace::eFreezeType::FREEZETYPE_RESETVELOCITY, "Reset Velocity" }
	} };
	for (auto& ft : vFrzTypNames)
	{
		if (engine->AddCheckbox(ft.second, tskPtr->freezeType == ft.first))
		{
			tskPtr->freezeType = ft.first;
		}
	}
}

static void SetRotation()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::SetRotation>();
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	const GTAentity& thisEntity = selectedEntity.handle;
	const Vector3& entityPos = thisEntity.GetPosition();
	const Vector3& entityRot = thisEntity.Rotation_get();
	const Vector3& rotForRot = (tskPtr->isRelative ? entityRot : Vector3::Zero())
		+ tskPtr->rotationValue;
	const Vector3& dirForRot = Vector3::RotationToDirection(rotForRot);
	const Vector3& rotForRoll = Vector3(rotForRot.y + 90.0f, 0.0f,
		rotForRot.y > 0 ? rotForRot.z - 90.0f : rotForRot.z + 90.0f);
	const Vector3& dirForRoll = Vector3::RotationToDirection(rotForRoll);

	const ModelDimensions& entityMd = thisEntity.ModelDimensions();
	const Vector3& markerPos = entityPos + (dirForRot * (entityMd.Dim1.y + 2.0f));
	World::DrawMarker(MarkerType::DebugSphere, markerPos, Vector3(), Vector3(),
		Vector3(0.3f, 0.3f, 0.3f), RGBA(177, 33, 193, 210));
	const Vector3& rollMarkerPos = markerPos + (dirForRoll * 0.3f);
	World::DrawMarker(MarkerType::DebugSphere, rollMarkerPos, Vector3(), Vector3(),
		Vector3(0.1f, 0.1f, 0.1f), RGBA(0, 0, 0, 255));
	World::DrawLine(entityPos, markerPos, RGBA(177, 33, 193, 210));

	if (engine->AddCheckbox("Relative", tskPtr->isRelative,
		Checkbox::BOXTICK, Checkbox::BOXBLANK))
	{
		tskPtr->isRelative = !tskPtr->isRelative;
	}

	DrawScrollSensitivityRow();
	DrawVector3OffsetRows(tskPtr->rotationValue, /*clamp180*/true);
	if (engine->AddOption("RESET"))
	{
		if (tskPtr->isRelative) tskPtr->rotationValue.clear();
		else                    tskPtr->rotationValue = entityRot;
	}
}

static void ChangeOpacity()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::ChangeOpacity>();
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	if (engine->AddCheckbox("Reset To Default", tskPtr->opacityValue == 269))
	{
		tskPtr->opacityValue = 269;
	}

	const std::array<BYTE, 5> alphaLevels{ 0, 50, 89, 160, 255 };
	for (auto& lvl : alphaLevels)
	{
		const std::string label = std::to_string(
			(int)(((float)(lvl) / 255.0f) * 100)) + "%";
		if (engine->AddCheckbox(label, tskPtr->opacityValue == lvl))
		{
			tskPtr->opacityValue = lvl;
		}
	}

	if (tskPtr->opacityValue >= 0 && tskPtr->opacityValue <= 255)
	{
		engine->AddBreak("---Manual---");
		::Menu::InputResult res = engine->AddNumber("Level",
			static_cast<double>(tskPtr->opacityValue), 0);
		if (res.rightPressed) { if (tskPtr->opacityValue < 255) tskPtr->opacityValue++; }
		if (res.leftPressed)  { if (tskPtr->opacityValue > 0)   tskPtr->opacityValue--; }

		const bool rowSelected = (engine->printingOption == engine->ActiveSelection());
		if (rowSelected && MenuPressTimer::IsButtonTapped(MenuPressTimer::Button::Accept))
		{
			std::string inputStr = Game::InputBox(std::string(), 4U, std::string(),
				std::to_string(tskPtr->opacityValue));
			if (!inputStr.empty())
			{
				try
				{
					int inputVal = std::stoi(inputStr);
					if (inputVal < 0 || inputVal > 255) throw 0;
					tskPtr->opacityValue = inputVal;
				}
				catch (...) { Game::Print::PrintErrorInvalidInput(inputStr); }
			}
		}
	}
	else if (tskPtr->opacityValue != 269)
	{
		tskPtr->opacityValue = 255;
	}
}

static void TriggerFx()
{
	auto tskPtr = _selectedSTST->GetTypeTask<STSTasks::TriggerFx>();
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	const GTAentity& thisEntity = selectedEntity.handle;
	World::DrawMarker(MarkerType::DebugSphere,
		thisEntity.GetOffsetInWorldCoords(tskPtr->posOffset), Vector3(), Vector3(),
		Vector3(0.1f, 0.1f, 0.1f), RGBA(tskPtr->colour, 190));

	DrawApplyOnlyOnceToggle("Play Only Once", tskPtr->duration, tskPtr->isLoopedTask, 1000);

	if (tskPtr->isLoopedTask)
	{
		float delaySeconds = static_cast<float>(tskPtr->delay) / 1000.0f;
		::Menu::InputResult res = engine->AddNumber("Interval (In Seconds)",
			delaySeconds, 1);
		if (res.rightPressed) { if (tskPtr->delay < INT_MAX) tskPtr->delay += 100; }
		if (res.leftPressed)  { if (tskPtr->delay > 0u)      tskPtr->delay -= 100; }

		const bool rowSelected = (engine->printingOption == engine->ActiveSelection());
		if (rowSelected && MenuPressTimer::IsButtonTapped(MenuPressTimer::Button::Accept))
		{
			std::string seed = std::to_string(delaySeconds);
			std::string inputStr = Game::InputBox(std::string(), 8,
				"Enter delay/interval in seconds:", seed);
			if (!inputStr.empty())
			{
				UINT inputVal;
				try { inputVal = (UINT)(std::stof(inputStr) * 1000); }
				catch (std::out_of_range&) { inputVal = INT_MAX; }
				catch (...) { inputVal = tskPtr->delay; }
				inputVal -= (inputVal % 100);
				tskPtr->delay = inputVal;
			}
		}
	}

	::Menu::InputResult resSize = engine->AddNumber("Size", tskPtr->scale, 2);
	if (resSize.rightPressed) { if (tskPtr->scale < FLT_MAX - 1.0f) tskPtr->scale += 0.02f; }
	if (resSize.leftPressed)  { if (tskPtr->scale > 1.0f - FLT_MAX) tskPtr->scale -= 0.02f; }

	const bool colourPressed = engine->AddOption("Colour");
	const bool colourRowSelected = (engine->printingOption == engine->ActiveSelection());
	if (colourRowSelected)
	{
		engine->AddPresetColourOptionsPreview(tskPtr->colour.R,
			tskPtr->colour.G, tskPtr->colour.B);
	}
	if (colourPressed)
	{
		sub::g_settingsRGBA = &tskPtr->colour;
		engine->NavigateTo("settings_colours2");
	}

	if (engine->AddOption("Adjust Relative Position"))
	{
		SpoonerVector3ManualPlacementPtrs = std::tuple<GTAentity, Vector3*, Vector3*>(
			thisEntity, &tskPtr->posOffset, &tskPtr->rotOffset);
		engine->NavigateTo("spooner_vector3_manualplacement");
	}

	engine->AddBreak("---FX---");
	const auto& fxData = tskPtr->fx.GetFxData();
	for (auto& ef : sub::PtfxSubs::PTFX)
	{
		const bool isCurrent = (fxData.asset == ef.asset && fxData.effect == ef.fx);
		if (engine->AddCheckbox(ef.name, isCurrent))
		{
			tskPtr->fx = PTFX::NonLoopedPTFX(ef.asset, ef.fx);
			tskPtr->fx.EasyStart(thisEntity, tskPtr->scale, tskPtr->posOffset,
				tskPtr->rotOffset, tskPtr->colour);
		}
	}
}

// Dispatch by task type to the matching drawer above.
static void DispatchTaskSpecific(sub::Spooner::STSTask* tsk)
{
	using sub::Spooner::STSTaskType;
	switch (tsk->type)
	{
	case STSTaskType::SetHealth:                 SetHealth(); break;
	case STSTaskType::AddBlip:                   AddBlip(); break;
	case STSTaskType::RemoveBlip:                RemoveBlip(); break;
	case STSTaskType::Nothing:                   Nothing(); break;
	case STSTaskType::Pause:                     Pause(); break;
	case STSTaskType::UsePhone:                  UsePhone(); break;
	case STSTaskType::ThrowProjectile:           ThrowProjectile(); break;
	case STSTaskType::Writhe:                    Writhe(); break;
	case STSTaskType::FaceDirection:             FaceDirection(); break;
	case STSTaskType::FaceEntity:                FaceEntity(); break;
	case STSTaskType::LookAtCoord:               LookAtCoord(); break;
	case STSTaskType::LookAtEntity:              LookAtEntity(); break;
	case STSTaskType::LookAtCoordEyesOnly:       LookAtCoordEyesOnly(); break;
	case STSTaskType::LookAtEntityEyesOnly:      LookAtEntityEyesOnly(); break;
	case STSTaskType::TeleportToCoord:           TeleportToCoord(); break;
	case STSTaskType::SeekCoverAtCoord:          SeekCoverAtCoord(); break;
	case STSTaskType::SlideToCoord:              SlideToCoord(); break;
	case STSTaskType::GoToCoord:                 GoToCoord(); break;
	case STSTaskType::FollowRoute:               FollowRoute(); break;
	case STSTaskType::FollowEntity:              FollowEntity(); break;
	case STSTaskType::PatrolInRange:             PatrolInRange(); break;
	case STSTaskType::WanderFreely:              WanderFreely(); break;
	case STSTaskType::FleeFromCoord:             FleeFromCoord(); break;
	case STSTaskType::NearestAppropriateAction:  NearestAppropriateAction(); break;
	case STSTaskType::ScenarioAction:            ScenarioAction(); break;
	case STSTaskType::PlayAnimation:             PlayAnimation(); break;
	case STSTaskType::SetActiveWeapon:           SetActiveWeapon(); break;
	case STSTaskType::AimAtCoord:                AimAtCoord(); break;
	case STSTaskType::AimAtEntity:               AimAtEntity(); break;
	case STSTaskType::ShootAtCoord:              ShootAtCoord(); break;
	case STSTaskType::ShootAtEntity:             ShootAtEntity(); break;
	case STSTaskType::FightHatedTargets:         FightHatedTargets(); break;
	case STSTaskType::FightPed:                  FightPed(); break;
	case STSTaskType::SpeakToPed:                SpeakToPed(); break;
	case STSTaskType::PlaySpeechWithVoice:       PlaySpeechWithVoice(); break;
	case STSTaskType::WarpIntoVehicle:           WarpIntoVehicle(); break;
	case STSTaskType::EnterVehicle:              EnterVehicle(); break;
	case STSTaskType::DriveWander:               DriveWander(); break;
	case STSTaskType::DriveToCoord:              DriveToCoord(); break;
	case STSTaskType::DriveFollowEntity:         DriveFollowEntity(); break;
	case STSTaskType::DriveLandPlane:            DriveLandPlane(); break;
	case STSTaskType::AchieveVehicleForwardSpeed: AchieveVehicleForwardSpeed(); break;
	case STSTaskType::ChangeTextureVariation:    ChangeTextureVariation(); break;
	case STSTaskType::AchieveVelocity:           AchieveVelocity(); break;
	case STSTaskType::AchievePushForce:          AchievePushForce(); break;
	case STSTaskType::OscillateToPoint:          OscillateToPoint(); break;
	case STSTaskType::OscillateToEntity:         OscillateToEntity(); break;
	case STSTaskType::FreezeInPlace:             FreezeInPlace(); break;
	case STSTaskType::SetRotation:               SetRotation(); break;
	case STSTaskType::ChangeOpacity:             ChangeOpacity(); break;
	case STSTaskType::TriggerFx:                 TriggerFx(); break;
	default: break;
	}
}

} // namespace TaskDraw

void SpoonerTaskListSubmenu::Draw()
{
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	dict.clear();

	if (!selectedEntity.handle.Exists())
	{
		engine->GoBack();
		return;
	}
	selectedEntity.handle.RequestControlOnce();
	int thisEntityIndexInDb = sub::Spooner::EntityManagement::GetEntityIndexInDb(selectedEntity);
	const bool isThisEntityInDb = thisEntityIndexInDb >= 0;
	if (!isThisEntityInDb)
	{
		Game::Print::PrintBottomLeft("~r~Error:~s~ Entity is not in the Spooner Database");
		addlog(ige::LogType::LOG_WARNING, "Cannot display task list, Entity not in Spooner Database");
		engine->GoBack();
		return;
	}

	DrawTitle();

	auto& taskSequence = sub::Spooner::Databases::EntityDb[thisEntityIndexInDb].taskSequence;
	auto& taskList = taskSequence.AllTasks();
	selectedEntity.taskSequence = taskSequence;

	// Start / Stop status row (mirrors AddLocal). Tapping while active stops.
	const bool isActive = taskSequence.IsActive();
	if (engine->AddToggleStatus("Status", isActive))
	{
		if (!isActive)
		{
			if (!taskList.empty())
			{
				taskSequence.Start();
				Game::Print::PrintBottomLeft("Started a Spooner task sequence.");
			}
			else
			{
				Game::Print::PrintBottomCentre("~r~Error:~s~ Task list is empty.");
				addlog(ige::LogType::LOG_WARNING, "Cannot start tasks, Task list is empty");
			}
		}
		else
		{
			taskSequence.Reset(false);
			CLEAR_PED_TASKS_IMMEDIATELY(selectedEntity.handle.Handle());
			Game::Print::PrintBottomLeft("Ended a Spooner task sequence.");
		}
	}

	for (UINT16 i = 0; i < taskSequence.TaskCount();)
	{
		const auto nameIt = sub::Spooner::STSTaskGetName(taskList[i]->type);
		const std::string label = nameIt->second.first;
		const bool pressed = engine->AddOption(label);

		const bool rowSelected = (engine->printingOption == engine->ActiveSelection());
		bool removed = false;
		if (rowSelected)
		{
			// Remove hotkey
			bool removeTaskPressed = false;
			if (Menu::bitController)
			{
				engine->AddInstructionalButton(INPUT_SCRIPT_RLEFT, "Remove", false);
				removeTaskPressed = IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_SCRIPT_RLEFT) != 0;
			}
			else
			{
				engine->AddInstructionalButton(static_cast<int>(VirtualKey::B), "Remove", true);
				removeTaskPressed = IsKeyJustUp(VirtualKey::B);
			}

			// Move hotkey
			engine->AddInstructionalButton(INPUT_FRONTEND_RIGHT, std::string(), false);
			engine->AddInstructionalButton(INPUT_FRONTEND_LEFT, "Move", false);
			const char moveTaskPressed =
				MenuPressTimer::IsButtonHeldOrTapped(MenuPressTimer::Button::Left) ? -1
				: MenuPressTimer::IsButtonHeldOrTapped(MenuPressTimer::Button::Right) ? 1 : 0;

			if (removeTaskPressed)
			{
				taskSequence.RemoveTask(i);
				removed = true;
			}
			else if (moveTaskPressed > 0)
			{
				taskSequence.SwapTasks(i, i == taskSequence.TaskCount() - 1 ? 0 : i + 1);
			}
			else if (moveTaskPressed < 0)
			{
				taskSequence.SwapTasks(i, i == 0 ? taskSequence.TaskCount() - 1 : i - 1);
			}
		}

		if (pressed)
		{
			if (taskList[i]->duration != -1) // Has settings
			{
				_selectedSTST = taskList[i];
				engine->NavigateTo("spooner_tasksequence_in_task");
			}
		}

		if (!removed) ++i;
	}

	if (engine->AddOption("ADD NEW"))
	{
		engine->NavigateTo("spooner_tasksequence_add_task");
	}
}

void SpoonerTaskAddTaskSubmenu::Draw()
{
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	if (!selectedEntity.handle.Exists()) { engine->GoBack(); return; }
	int thisEntityIndexInDb = sub::Spooner::EntityManagement::GetEntityIndexInDb(selectedEntity);
	if (thisEntityIndexInDb < 0) { engine->GoBack(); return; }

	DrawTitle();

	auto& thisEntity = sub::Spooner::Databases::EntityDb[thisEntityIndexInDb];
	auto& taskSequence = thisEntity.taskSequence;

	for (auto& tn : sub::Spooner::vSTSTaskTypeNames)
	{
		if (tn.second.second == EntityType::ALL || tn.second.second == thisEntity.type)
		{
			if (engine->AddOption(tn.second.first))
			{
				taskSequence.AddTask(tn.first);
				selectedEntity.taskSequence = taskSequence;
				engine->GoBack();
				return;
			}
		}
	}
}

void SpoonerTaskInTaskSubmenu::Draw()
{
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	sub::Spooner::STSTask* tskPtr = _selectedSTST;
	if (tskPtr == nullptr) { engine->GoBack(); return; }

	// Title uses the selected task type's friendly name.
	const auto nameIt = sub::Spooner::STSTaskGetName(tskPtr->type);
	engine->AddTitle(nameIt->second.first);

	auto& thisDuration = tskPtr->duration;
	if (thisDuration >= 0)
	{
		float durationSeconds = static_cast<float>(thisDuration) / 1000.0f;
		::Menu::InputResult resDur = engine->AddNumber("Duration (In Seconds)",
			durationSeconds, 3);
		// Snapshot Duration-row selection BEFORE the next AddNumber bumps
		// printingOption (so Accept-tap below applies to Duration only).
		const bool durationRowSelected = (engine->printingOption == engine->ActiveSelection());

		::Menu::InputResult resPrec = engine->AddNumber("Scroll Sensitivity",
			_manualPlacementPrecision, 3);

		if (resDur.rightPressed)
		{
			const int delta = static_cast<int>(_manualPlacementPrecision * 1000);
			if (thisDuration <= INT_MAX - delta) thisDuration += delta;
		}
		if (resDur.leftPressed)
		{
			const int delta = static_cast<int>(_manualPlacementPrecision * 1000);
			if (thisDuration > delta) thisDuration -= delta;
		}
		if (resPrec.rightPressed) { if (_manualPlacementPrecision < 10.0f)  _manualPlacementPrecision *= 10; }
		if (resPrec.leftPressed)  { if (_manualPlacementPrecision > 0.001f) _manualPlacementPrecision /= 10; }

		if (durationRowSelected && MenuPressTimer::IsButtonTapped(MenuPressTimer::Button::Accept))
		{
			std::string seed = std::to_string(static_cast<float>(thisDuration) / 1000.0f);
			seed = seed.substr(0, seed.find('.') + 2);
			std::string inputStr = Game::InputBox(std::string(), 8,
				"Enter duration in seconds:", seed);
			if (!inputStr.empty())
			{
				int inputVal;
				try { inputVal = std::abs(static_cast<int>(std::stof(inputStr) * 1000)); }
				catch (std::out_of_range&) { inputVal = INT_MAX - 100; }
				catch (...) { inputVal = thisDuration; }
				thisDuration = inputVal;
			}
		}

		if (tskPtr->durationAfterLife >= 0)
		{
			if (engine->AddCheckbox("Keep Task Running After Allocated Time",
				tskPtr->durationAfterLife == 1, Checkbox::BOXTICK, Checkbox::BOXBLANK))
			{
				tskPtr->durationAfterLife = (tskPtr->durationAfterLife == 1) ? 0 : 1;
			}
		}
	}

	if (tskPtr->submenu != nullptr)
	{
		// Dispatch via task type rather than calling the legacy function
		// pointer (which renders into the V1 menu).
		TaskDraw::DispatchTaskSpecific(tskPtr);
	}
}

void SpoonerTaskScenarioActionListSubmenu::OnExit()
{
	searchStr.clear();
}

void SpoonerTaskScenarioActionListSubmenu::Draw()
{
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	if (_selectedSTST == nullptr) { engine->GoBack(); return; }
	auto tskPtr = _selectedSTST->GetTypeTask<sub::Spooner::STSTasks::ScenarioAction>();

	DrawTitle();

	const std::string searchLabel = searchStr.empty()
		? std::string("SEARCH") : ToUpperCopy(searchStr);
	if (DrawOption(searchLabel))
	{
		searchStr = Game::InputBox(searchStr, 126U, "SEARCH", searchStr);
		searchStr = ToLowerCopy(searchStr);
	}

	for (auto& sl : sub::AnimationTaskScenarios::vValues_TaskScenarios)
	{
		if (!searchStr.empty())
		{
			if (sl.find(searchStr) == std::string::npos) continue;
		}
		if (engine->AddCheckbox(sl, tskPtr->scenarioName == sl))
		{
			tskPtr->scenarioName = sl;
		}
	}
}

void SpoonerTaskPlayAnimationSettingsSubmenu::Draw()
{
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	if (_selectedSTST == nullptr) { engine->GoBack(); return; }
	auto tskPtr = _selectedSTST->GetTypeTask<sub::Spooner::STSTasks::PlayAnimation>();

	DrawTitle();

	{
		::Menu::InputResult res = engine->AddNumber("Blend-In Speed", tskPtr->speed, 1);
		if (res.rightPressed) { if (tskPtr->speed < FLT_MAX)  tskPtr->speed += 0.1f; }
		if (res.leftPressed)  { if (tskPtr->speed > -FLT_MAX) tskPtr->speed -= 0.1f; }

		const bool rowSelected = (engine->printingOption == engine->ActiveSelection());
		if (rowSelected && MenuPressTimer::IsButtonTapped(MenuPressTimer::Button::Accept))
		{
			PromptFloatInputBox(tskPtr->speed, 6);
		}
	}
	{
		::Menu::InputResult res = engine->AddNumber("Blend-Out Speed",
			tskPtr->speedMultiplier, 1);
		if (res.rightPressed) { if (tskPtr->speedMultiplier < FLT_MAX)  tskPtr->speedMultiplier += 0.1f; }
		if (res.leftPressed)  { if (tskPtr->speedMultiplier > -FLT_MAX) tskPtr->speedMultiplier -= 0.1f; }

		const bool rowSelected = (engine->printingOption == engine->ActiveSelection());
		if (rowSelected && MenuPressTimer::IsButtonTapped(MenuPressTimer::Button::Accept))
		{
			PromptFloatInputBox(tskPtr->speedMultiplier, 6);
		}
	}
	{
		const std::vector<std::string> options{ AnimFlag::vFlagNames[tskPtr->flag] };
		::Menu::InputResult res = engine->AddTextList("Flag", 0, options);
		if (res.rightPressed)
		{
			for (auto it = AnimFlag::vFlagNames.begin(); it != AnimFlag::vFlagNames.end(); ++it)
			{
				if (it->first == tskPtr->flag)
				{
					++it;
					if (it != AnimFlag::vFlagNames.end()) tskPtr->flag = it->first;
					break;
				}
			}
		}
		if (res.leftPressed)
		{
			for (auto it = AnimFlag::vFlagNames.rbegin(); it != AnimFlag::vFlagNames.rend(); ++it)
			{
				if (it->first == tskPtr->flag)
				{
					++it;
					if (it != AnimFlag::vFlagNames.rend()) tskPtr->flag = it->first;
					break;
				}
			}
		}
	}

	if (engine->AddCheckbox("Lock Position", tskPtr->lockPos,
		Checkbox::BOXTICK, Checkbox::BOXBLANK))
	{
		tskPtr->lockPos = !tskPtr->lockPos;
	}
}

void SpoonerTaskPlayAnimationAllPedAnimsSubmenu::OnEnter()
{
	if (!loaded)
	{
		sub::AnimationMenu::PopulateAllPedAnimsList();
		loaded = true;
	}
}

void SpoonerTaskPlayAnimationAllPedAnimsSubmenu::OnExit()
{
	searchStr.clear();
}

void SpoonerTaskPlayAnimationAllPedAnimsSubmenu::Draw()
{
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	if (_selectedSTST == nullptr) { engine->GoBack(); return; }

	sub::AnimationMenu::selectedAnimDictPtr = nullptr;

	DrawTitle();

	const std::string searchLabel = searchStr.empty()
		? std::string("SEARCH") : ToUpperCopy(searchStr);
	if (DrawOption(searchLabel))
	{
		searchStr = Game::InputBox(searchStr, 126U, "SEARCH", searchStr);
		searchStr = ToLowerCopy(searchStr);
	}

	for (auto& current : sub::AnimationMenu::allPedAnims)
	{
		if (current.second.empty()) continue;

		if (!searchStr.empty())
		{
			if (current.first.find(searchStr) == std::string::npos)
			{
				bool foundInValues = false;
				for (auto& current2 : current.second)
				{
					if (current2.find(searchStr) != std::string::npos)
					{
						foundInValues = true;
						break;
					}
				}
				if (!foundInValues) continue;
			}
		}

		if (engine->AddOption(current.first))
		{
			sub::AnimationMenu::selectedAnimDictPtr = &current;
			engine->NavigateTo("spooner_tasksequence_play_animation_all_ped_anims_in_dict");
		}
	}
}

void SpoonerTaskPlayAnimationAllPedAnimsInDictSubmenu::Draw()
{
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	if (_selectedSTST == nullptr || sub::AnimationMenu::selectedAnimDictPtr == nullptr)
	{
		engine->GoBack();
		return;
	}
	auto tskPtr = _selectedSTST->GetTypeTask<sub::Spooner::STSTasks::PlayAnimation>();
	auto& selectedDict = *sub::AnimationMenu::selectedAnimDictPtr;

	engine->AddTitle(selectedDict.first);

	for (auto& current : selectedDict.second)
	{
		const bool isCurrent = (selectedDict.first == tskPtr->animDict
			&& current == tskPtr->animName);
		if (engine->AddCheckbox(current, isCurrent))
		{
			tskPtr->animDict = selectedDict.first;
			tskPtr->animName = current;
		}

		const bool rowSelected = (engine->printingOption == engine->ActiveSelection());
		if (rowSelected)
		{
			const bool isFav = sub::IsAnimationAFavourite(selectedDict.first, current);
			const std::string hint = (!isFav ? "Add to" : "Remove from")
				+ std::string(" favourites");
			if (Menu::bitController)
			{
				engine->AddInstructionalButton(INPUT_SCRIPT_RLEFT, hint, false);
				if (IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_SCRIPT_RLEFT))
				{
					if (!isFav) sub::AddAnimationToFavourites(selectedDict.first, current);
					else        sub::RemoveAnimationFromFavourites(selectedDict.first, current);
				}
			}
			else
			{
				engine->AddInstructionalButton(static_cast<int>(VirtualKey::B), hint, true);
				if (IsKeyJustUp(VirtualKey::B))
				{
					if (!isFav) sub::AddAnimationToFavourites(selectedDict.first, current);
					else        sub::RemoveAnimationFromFavourites(selectedDict.first, current);
				}
			}
		}
	}

	if (engine->AddOption("Settings"))
	{
		engine->NavigateTo("spooner_tasksequence_play_animation_settings");
	}
}

void SpoonerTaskPlaySpeechWithVoiceInVoiceSubmenu::Draw()
{
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return;

	if (_selectedSTST == nullptr || sub::Speech::_currVoiceInfo == nullptr)
	{
		engine->GoBack();
		return;
	}
	auto tskPtr = _selectedSTST->GetTypeTask<sub::Spooner::STSTasks::PlaySpeechWithVoice>();
	auto& v = *sub::Speech::_currVoiceInfo;

	engine->AddTitle(v.voiceName);

	auto paramIter = std::find_if(sub::Speech::vSpeechParams.begin(),
		sub::Speech::vSpeechParams.end(),
		[tskPtr](const sub::Speech::SpeechParamS& item) {
			return item.label.compare(tskPtr->paramName) == 0;
		});
	if (paramIter == sub::Speech::vSpeechParams.end())
		paramIter = sub::Speech::vSpeechParams.begin();

	{
		const std::vector<std::string> options{ paramIter->title };
		::Menu::InputResult res = engine->AddTextList("Modifier", 0, options);
		if (res.rightPressed)
		{
			if (std::next(paramIter) != sub::Speech::vSpeechParams.end())
				tskPtr->paramName = (++paramIter)->label;
		}
		if (res.leftPressed)
		{
			if (paramIter != sub::Speech::vSpeechParams.begin())
				tskPtr->paramName = (--paramIter)->label;
		}
	}

	for (auto& s : v.speechNames)
	{
		if (engine->AddCheckbox(s, tskPtr->speechName == s,
			Checkbox::TICK, Checkbox::NONE))
		{
			tskPtr->voiceName = v.voiceName;
			tskPtr->speechName = s;
			GTAped(selectedEntity.handle).PlaySpeechWithVoice(
				tskPtr->speechName, tskPtr->voiceName, tskPtr->paramName);
		}
	}
}

}
REGISTER_SUBMENU(::Menu::SpoonerTaskListSubmenu)
REGISTER_SUBMENU(::Menu::SpoonerTaskAddTaskSubmenu)
REGISTER_SUBMENU(::Menu::SpoonerTaskInTaskSubmenu)
REGISTER_SUBMENU(::Menu::SpoonerTaskScenarioActionListSubmenu)
REGISTER_SUBMENU(::Menu::SpoonerTaskPlayAnimationSettingsSubmenu)
REGISTER_SUBMENU(::Menu::SpoonerTaskPlayAnimationAllPedAnimsSubmenu)
REGISTER_SUBMENU(::Menu::SpoonerTaskPlayAnimationAllPedAnimsInDictSubmenu)
REGISTER_SUBMENU(::Menu::SpoonerTaskPlaySpeechWithVoiceInVoiceSubmenu)
