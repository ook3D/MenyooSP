#pragma once

class Vector3;
class GTAvehicle;
namespace GTAmodel { class Model; }

namespace sub
{
	void AttachPedToVehicle(GTAmodel::Model model, GTAvehicle vehicle, const Vector3& offset, const Vector3& rotation, bool invis = false, bool piggyback = false);
	void AttachObjectToVehicle(GTAmodel::Model model, GTAvehicle vehicle, float X, float Y, float Z, float Pitch, float Roll, float Yaw, bool invis, int boneIndex = -1, bool dynamic = false, bool collisionEnabled = false, bool destroyVar = true);
	void AttachVehicleToVehicle(GTAmodel::Model model, GTAvehicle vehicle, int primColour, int secColour, float X, float Y, float Z, float Pitch, float Roll, float Yaw, bool invis, int boneIndex = -1, bool collisionEnabled = false);
}

namespace Menu { namespace FunnyVehicles {

void GoKart();
void DragsterBike();
void WeedWheelsBike();
void YachtAirship();
void YachtAirshipWithFans();
void FibBuilding();
void BlackNoisyUFO();
void ToyCar();
void Adderuma();
void Zentornuma();
void TurismoRuma();
void EnturumaXF();
void Osirisuma();
void T20uma();
void Feltzeruma();
void Banshuma();
void Nightshuma();
void Bulletuma();
void LandJetski();
void MonsterTruckBoatChassis();
void MonsterTruckTankChassis();
void MonsterTruckHelicopterChassis();
void MonsterTruckRVChassis();
void MonsterTruckFighterJetChassis();
void ChinoODeath();
void RVBuilding();
void MonsterTrainTruck();

void CowCar();
void DeerCar();
void SharkCar();
void PooMobile();
void Wheelchair();
void BumperCar();
void RollerCar();
void MissileSurano();
void MoveablePlatform();
void HydraUFO();
void MuricanSurano();
void SpeakersUp();

}}
