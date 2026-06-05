/*
* Menyoo PC - Grand Theft Auto V single-player trainer mod
* Copyright (C) 2019  MAFINS
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*/
#pragma once

#include <tuple>
#include <string>

#include "..\..\Util\GTAmath.h"

typedef unsigned char UINT8, BYTE;
typedef unsigned int UINT;
typedef unsigned long DWORD, Hash;

class GTAentity;

// All Sub_*Menu declarations removed - Those submenus are owned elsewhere
// (see Solution/source/Submenus/SpoonerSubmenus.cpp et al.). Only the
// shared file-scope state below is still consumed externally.
namespace sub
{
	namespace Spooner::Submenus
	{
		extern std::string& _searchStr;
		extern std::tuple<GTAentity, Vector3*, Vector3*> SpoonerVector3ManualPlacementPtrs;
		extern float _manualPlacementPrecision;
		extern UINT8 _copyEntTexterValue;
	}
}
