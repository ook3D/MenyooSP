/*
* Menyoo PC - Grand Theft Auto V single-player trainer mod
* Copyright (C) 2019  MAFINS
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*/
#include "SpoonerShared.h"

#include "..\..\Menu\Routine.h"
#include "..\Misc.h"
#include "..\..\Scripting\GTAentity.h"
#include "..\..\Util\GTAmath.h"

#include <string>
#include <tuple>

// All Sub_*Menu function bodies removed. Those submenus are owned elsewhere -
// see Solution/source/Submenus/SpoonerSubmenus.cpp,
// SpoonerFileManagementSubmenus.cpp and SpoonerTaskSequenceSubmenus.cpp
// for the ports. The shared file-scope state below is still read/written
// by this code and by other legacy Spooner files (SpoonerMode.cpp etc.).

namespace sub
{
	namespace Spooner::Submenus
	{
		// Shared search-string slot - alias of Menu::dict2. Kept for ABI parity
		// with legacy callers; the spawn submenus manage their own search state.
		std::string& _searchStr = dict2;

		// Vector3 placement context for Sub_Vector3_ManualPlacement legacy path.
		// Sub_Vector3_ManualPlacement port (and the task-sequence port)
		// reads/writes this tuple to feed the "enter coords" placement flow.
		std::tuple<GTAentity, Vector3*, Vector3*> SpoonerVector3ManualPlacementPtrs = { 0, nullptr, nullptr };

		// Scroll-wheel sensitivity for manual placement / size manipulation.
		// Mutated by manual-placement code and by Sub_TaskSequence_InTask
		// (now legacy-removed); kept here so this code and any remaining callers see
		// the same value.
		float _manualPlacementPrecision = 0.01f;

		// Copy-entity texter selector (0 = copy, 1 = clone). Read by
		// SpoonerMode.cpp's clipboard paste paths.
		UINT8 _copyEntTexterValue = 0;
	}
}
