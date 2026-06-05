#pragma once

#include <string>

#include "../Menu/Routine.h"

typedef signed short INT16;
typedef signed char INT8;
typedef unsigned __int8 UINT8;
typedef int INT;
typedef int Entity;
typedef int Ped;
typedef int Player;
typedef float FLOAT;

class Camera;
class Vector3;
class GTAplayer;
class GTAentity;
class GTAvehicle;
class GTAped;

extern INT16 BindNoClip;

extern INT g_Ped1;
extern INT g_Ped2;
extern INT g_Ped3;
extern INT g_Ped4;

extern const char* g_PlayerName;
extern bool bitNightVision;

extern FLOAT swimSpeedMult;
extern FLOAT playerNoiseMult;
extern FLOAT selfSweatMult;
extern FLOAT g_playerVerticalElongationMultiplier;

extern Entity g_driveWaterObject;

extern INT8 spectatePlayer;

extern bool checkSelfDeathModel;

extern UINT8 forceField;
extern UINT8 selfFreezeWantedLevel;

extern bool playerNoRagdoll;
extern bool playerSeatbelt;
extern bool playerUnlimitedAbility;
extern bool playerAutoClean;
extern bool playerWalkUnderwater;
extern bool superJump;
extern bool selfRefillHealthInCover;
extern bool playerInvincibility;
extern bool noClip;
extern bool noClipToggle;
extern bool superRun;
extern bool ignoredByEveryone;
extern bool neverWanted;
extern bool superman;
extern bool supermanAuto;
extern bool driveOnWater;
extern bool playerBurn;

extern bool bitNoclipAlreadyInvisible;
extern bool bitNoclipAlreadyCollision;
extern bool bitNoclipShowHelp;
extern Camera g_cam_noClip;

void SetSelfNearbyPedsCalm();
void NetworkSetEveryoneIgnorePlayer(Player player);
void SetBecomePed(GTAped ped);
void SetPedInvincibleOn(Ped ped);
void SetPedInvincibleOff(Ped ped);
void SetPedNoRagdollOn(Ped ped);
void SetPedNoRagdollOff(Ped ped);
void SetPedSeatbeltOn(Ped ped);
void SetPedSeatbeltOff(Ped ped);

void SetNoclipOff1();
void SetNoclipOff2();
void SetNoclip();
void SetLocalButtonSuperRun();
void SetSelfRefillHealthWhenInCover();
void SetLocalSupermanManual();
void SetPedSupermanAuto(Ped ped);
void SetLocalForcefield();

void DriveOnWater(GTAped ped, Entity& waterobject);
void SetPedBurnMode(GTAped ped, bool enable);

void SetWalkUnderwater(Entity PlayerPed);
