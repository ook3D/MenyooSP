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

#include <string>
#include <unordered_set>
#include <map>

typedef signed short INT16;
typedef unsigned short UINT16;
typedef signed char INT8;
typedef unsigned char UINT8;
typedef unsigned char BYTE;
typedef int INT;
typedef int Entity;
typedef int Ped;
typedef int Vehicle;
typedef int Player;
typedef unsigned long DWORD;
typedef unsigned long Hash;
typedef float FLOAT;
typedef char* PCHAR;
typedef const char* LPCSTR;

class RgbS;
class Vector3;
class Camera;

namespace GTAmodel
{
	class Model;
}
class GTAplayer;
class GTAentity;
class GTAvehicle;
class GTAped;

namespace PTFX
{
	class sFxData;
}

void ThreadMenyooMain();
void ThreadMenuLoops2();
void TickMenyooConfig();
