/*
* Menyoo PC - Grand Theft Auto V single-player trainer mod
* Copyright (C) 2019  MAFINS
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*/
#include "SpoonerTaskSequenceStubs.h"

#include "STSTask.h"

// All Sub_TaskSequence_* draw functions ported to Menu submenus -
// see Solution/source/Submenus/SpoonerTaskSequenceSubmenus.cpp.
//
// The per-task drawer symbols under namespace Sub_TaskSequence (Nothing,
// SetHealth, AddBlip, ...) are kept here as empty stubs because each STSTask
// subclass stores one of them as a `submenu` function pointer in
// STSTasks.cpp. the port inspects the pointer for non-null (as a "has a settings
// submenu" flag) but dispatches via task type instead of calling the pointer,
// so the bodies are unreachable - they only need to exist as linkable symbols.

namespace sub::Spooner
{
	namespace Submenus
	{
		// Currently-selected task in the task-sequence editor. Owned here so the existing legacy callers keep linking; the port reads
		// and writes this via `using sub::Spooner::Submenus::_selectedSTST`.
		STSTask* _selectedSTST = nullptr;

		namespace Sub_TaskSequence
		{
			void Nothing() {}
			void SetHealth() {}

			void AddBlip() {}
			void RemoveBlip() {}

			void Pause() {}
			void UsePhone() {}
			void ThrowProjectile() {}
			void Writhe() {}
			void FaceDirection() {}
			void FaceEntity() {}
			void LookAtCoord() {}
			void LookAtEntity() {}
			void LookAtCoordEyesOnly() {}
			void LookAtEntityEyesOnly() {}
			void TeleportToCoord() {}
			void SeekCoverAtCoord() {}
			void SlideToCoord() {}
			void GoToCoord() {}
			void FollowRoute() {}
			void FollowEntity() {}
			void PatrolInRange() {}
			void WanderFreely() {}
			void FleeFromCoord() {}
			void NearestAppropriateAction() {}
			void ScenarioAction() {}
			void ScenarioAction_list() {}
			void PlayAnimation() {}
			void PlayAnimation_settings() {}
			void PlayAnimation_allPedAnims() {}
			void PlayAnimation_allPedAnims_inDict() {}
			void SetActiveWeapon() {}
			void AimAtCoord() {}
			void AimAtEntity() {}
			void ShootAtCoord() {}
			void ShootAtEntity() {}
			void FightHatedTargets() {}
			void FightPed() {}
			void SpeakToPed() {}
			void PlaySpeechWithVoice() {}
			void PlaySpeechWithVoice_inVoice() {}

			void WarpIntoVehicle() {}
			void EnterVehicle() {}
			void DriveWander() {}
			void DriveToCoord() {}
			void DriveFollowEntity() {}
			void DriveLandPlane() {}

			void AchieveVehicleForwardSpeed() {}

			void ChangeTextureVariation() {}

			void AchieveVelocity() {}
			void AchievePushForce() {}
			void OscillateToPoint() {}
			void OscillateToEntity() {}
			void FreezeInPlace() {}
			void SetRotation() {}
			void ChangeOpacity() {}
			void TriggerFx() {}
		}
	}
}
