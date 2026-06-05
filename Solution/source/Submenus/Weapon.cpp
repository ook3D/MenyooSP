#include "Weapon.h"

#include "../Menu/Engine.h"
#include "../Menu/SubmenuRegistry.h"
#include "../Menu/Menu.h"      // Checkbox, Menu::bitController, VirtualKey
#include "../Menu/Routine.h"   // shared globals
#include "PlayerRuntime.h"
#include "Time.h"              // currentTimescale
#include "Misc.h"              // dict, dict2, dict3

#include "../Natives/natives2.h"
#include "../Natives/types.h"  // RGBA, RgbS

#include "../Scripting/Game.h"
#include "../Scripting/GameplayCamera.h"
#include "../Scripting/GTAplayer.h"
#include "../Scripting/GTAped.h"
#include "../Scripting/GTAentity.h"
#include "../Scripting/Model.h"
#include "../Scripting/World.h"
#include "../Scripting/WeaponIndivs.h"

#include "../Util/keyboard.h"
#include "../Util/StringManip.h"
#include "../Util/ExePath.h"

#include "../Misc/RopeGun.h"
#include "../Misc/MagnetGun.h"
#include "../Misc/FlameThrower.h"

#include "WeaponRuntime.h"
#include "PtfxData.h"
#include "VehicleSpawnerRuntime.h"
#include "VehicleModShopRuntime.h" // bitMSPaintsRGBMode
#include "SettingsRuntime.h"       // sub::g_settingsRGBA

#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include <pugixml/src/pugixml.hpp>
#include <dirent/include/dirent.h>

#include <algorithm>
#include <cctype>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

namespace Menu {

namespace {
inline void toLowerInPlace(std::string& s)
{
	std::transform(s.begin(), s.end(), s.begin(),
		[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
}
inline void toUpperInPlace(std::string& s)
{
	std::transform(s.begin(), s.end(), s.begin(),
		[](unsigned char c) { return static_cast<char>(std::toupper(c)); });
}
inline std::string toLowerCopy(const std::string& s)
{
	std::string out(s); toLowerInPlace(out); return out;
}
inline std::string toUpperCopy(const std::string& s)
{
	std::string out(s); toUpperInPlace(out); return out;
}
} // namespace

void WeaponSubmenu::Draw()
{
	DrawTitle();

	dict2.clear();
	dict3.clear();

	if (g_WeaponOpsPedOverride != 0)
	{
		g_Ped1 = g_WeaponOpsPedOverride;
		g_Ped2 = g_WeaponOpsPlayerOverride;
	}
	else
	{
		g_Ped1 = PLAYER_PED_ID();
		g_Ped2 = PLAYER_ID();
	}

	if (DrawOption("Individual Weapons & Customisation")) NavigateTo("weapon_categories");
	if (DrawOption("Loadouts"))                            NavigateTo("weapon_loadouts");

	if (DrawOption("Give All Weapons"))
	{
		GiveAllWeaponsToPed(g_Ped1);
		WAIT(15);
		GivePedMaxAmmo(g_Ped1);
		return;
	}
	if (DrawOption("Give Digiscanner"))
	{
		GIVE_WEAPON_TO_PED(g_Ped1, WEAPON_DIGISCANNER, 1, true, true);
		SET_CURRENT_PED_WEAPON(g_Ped1, WEAPON_DIGISCANNER, true);
		SET_PED_CURRENT_WEAPON_VISIBLE(g_Ped1, 1, 1, 1, 0);
		return;
	}
	if (DrawOption("Give Briefcase"))
	{
		GIVE_WEAPON_TO_PED(g_Ped1, WEAPON_BRIEFCASE, 1, true, true);
		SET_CURRENT_PED_WEAPON(g_Ped1, WEAPON_BRIEFCASE, true);
		SET_PED_CURRENT_WEAPON_VISIBLE(g_Ped1, 1, 1, 1, 0);
		return;
	}
	if (DrawOption("Give Max Ammo"))
	{
		GivePedMaxAmmo(g_Ped1);
		return;
	}
	if (DrawOption("Remove All Weapons"))
	{
		REMOVE_ALL_PED_WEAPONS(g_Ped1, 1);
		return;
	}

	DrawToggle("Infinite Parachutes", selfInfiniteParachutes);

	if (DrawToggle("Infinite Ammo In Clip", bitInfiniteAmmo))
	{
		if (bitInfiniteAmmo)
		{
			GivePedMaxAmmo(g_Ped1);
			SET_PED_INFINITE_AMMO_CLIP(g_Ped1, true);
		}
		else
		{
			SET_PED_INFINITE_AMMO_CLIP(g_Ped1, false);
		}
		return;
	}

	DrawToggle("Explosive Melee", explosiveMelee);
	DrawToggle("Explosive Ammo", explosiveRounds);
	DrawToggle("Tenfold Bullets", tripleBullets);
	DrawToggle("Flaming Bullets", flamingRounds);

	{
		std::stringstream wdmgSs;
		wdmgSs << std::fixed << std::setprecision(2) << (weaponDamageIncrease / 0.72f) << "x";
		Engine* engine = Engine::Current();
		const ::Menu::InputResult res = engine
			? engine->AddTextList("Weapon Damage", 0, std::vector<std::string>{ wdmgSs.str() })
			: ::Menu::InputResult{};
		if (res.accepted)
		{
			weaponDamageIncrease = (weaponDamageIncrease == 1.0f ? 100.0f : 1.0f) * 0.72f;
			SET_PLAYER_WEAPON_DAMAGE_MODIFIER(g_Ped2, weaponDamageIncrease);
			SET_PLAYER_MELEE_WEAPON_DAMAGE_MODIFIER(g_Ped2, weaponDamageIncrease, true);
		}
		if (res.rightPressed)
		{
			if (weaponDamageIncrease / 0.72f < 100.0f) weaponDamageIncrease += (0.1f * 0.72f);
			SET_PLAYER_WEAPON_DAMAGE_MODIFIER(g_Ped2, weaponDamageIncrease);
			SET_PLAYER_MELEE_WEAPON_DAMAGE_MODIFIER(g_Ped2, weaponDamageIncrease, true);
		}
		else if (res.leftPressed)
		{
			if (weaponDamageIncrease / 0.72f > -100.0f) weaponDamageIncrease -= (0.1f * 0.72f);
			SET_PLAYER_WEAPON_DAMAGE_MODIFIER(g_Ped2, weaponDamageIncrease);
			SET_PLAYER_MELEE_WEAPON_DAMAGE_MODIFIER(g_Ped2, weaponDamageIncrease, true);
		}
	}

	if (DrawToggle("Bullet Time", bulletTime) && !bulletTime)
	{
		SET_TIME_SCALE(currentTimescale);
		return;
	}

	DrawToggle("Triggerbot", selfTriggerbot);

	if (DrawToggle("Rapid Fire", rapidFire) && rapidFire)
	{
		Game::Print::PrintBottomLeft("~b~Note:~s~ You cannot use other Menyoo weapon mods with this on.");
		return;
	}

	if (DrawToggle("Soul-Switch Gun (SP)", soulSwitchGun) && soulSwitchGun)
	{
		if (NETWORK_IS_IN_SESSION()) soulSwitchGun = false;
		else Game::Print::PrintBottomLeft("Shoot ~b~scrubs~s~ with the ~b~combat pistol~s~ for hax!");
		return;
	}

	if (DrawToggleExternal("Rope Gun (Glitchy)", RopeGun::g_ropeGun.Enabled()))
		RopeGun::ToggleOnOff();
	if (DrawToggleExternal("Magnet Gun", MagnetGun::g_magnetGun.Enabled()))
		MagnetGun::ToggleOnOff();
	if (DrawToggleExternal("Flamethrower " + GetWeaponLabel(FlameThrower::_whash, true),
		FlameThrower::IsPlayerAdded(g_Ped2)))
	{
		if (FlameThrower::IsPlayerAdded(g_Ped2))
			FlameThrower::RemoveSelf();
		else
			FlameThrower::AddSelf();
	}

	if (DrawToggle("Teleport Gun", teleportGun) && teleportGun)
	{
		Game::Print::PrintBottomLeft("Use the ~b~Heavy pistol~s~ to teleport to places.");
		return;
	}

	if (DrawToggle("Ped Revival Gun", selfResurrectionGun) && selfResurrectionGun)
	{
		Game::Print::PrintBottomLeft("Shoot ~b~dead people~s~ with the ~b~stun gun~s~ to bring them back from the other side.");
		return;
	}

	if (DrawToggle("Entity Removal Gun", selfDeleteGun) && selfDeleteGun)
	{
		Game::Print::PrintBottomLeft("Shoot ~b~anything*~s~ with the ~b~SNS pistol~s~ to delete it.");
		return;
	}

	DrawToggle("Light Gun", lightGun);

	if (DrawOption("Laser Sight"))      NavigateTo("weapon_laser_sight");
	if (DrawOption("Forge Gun"))        NavigateTo("weapon_forge_gun");
	if (DrawOption("Gravity Gun"))      NavigateTo("weapon_gravity_gun");
	if (DrawOption("TriggerFX Gun"))    NavigateTo("weapon_trigger_fx_gun");
	if (DrawOption("Kaboom Gun"))       NavigateTo("weapon_kaboom_gun");
	if (DrawOption("Bullet Gun"))       NavigateTo("weapon_bullet_gun");
	if (DrawOption("Ped Gun"))          NavigateTo("weapon_ped_gun");
	if (DrawOption("Object & Vehicle Gun")) NavigateTo("weapon_object_gun");
}

void ForgeGunSubmenu::Draw()
{
	DrawTitle();

	if (DrawToggle("Gun Toggle", forgeGun) && forgeGun)
	{
		Game::Print::PrintBottomLeft("Use the ~b~pistol~s~ for hax.");
		if (!Menu::bitController)
		{
			Game::Print::PrintBottomLeft("~b~Mouse Scroll~s~ for distance.");
			Game::Print::PrintBottomLeft(oss_ << "~b~ [ ~s~& ~b~ ] ~s~for pitch." << " \n"
				<< "~b~ ; ~s~& ~b~ ' ~s~for roll." << " \n"
				<< "~b~ , ~s~& ~b~ . ~s~for yaw.");
		}
		else
		{
			Game::Print::PrintBottomLeft("~b~RS~s~ & ~b~LS~s~ for distance.");
			Game::Print::PrintBottomLeft(oss_ << "~b~Up~s~ & ~b~Down~s~ for pitch." << " \n"
				<< "~b~Right~s~ & ~b~Left~s~ for roll." << " \n"
				<< "~b~RB~s~ & ~b~LB~s~ for yaw.");
		}
		Game::Print::PrintBottomLeft("~b~Shoot~s~ for launch.");
		return;
	}

	DrawToggle("Freeze Pickups In Place", objectSpawnForgeAssistance);
	Engine* engine = Engine::Current();
	{
		const ::Menu::InputResult res = engine
			? engine->AddNumber("Rotation Precision", g_forgeGunPrecision, 4)
			: ::Menu::InputResult{};
		if (res.rightPressed)
		{
			if (g_forgeGunPrecision < 10.0f) g_forgeGunPrecision *= 10;
			return;
		}
		if (res.leftPressed)
		{
			if (g_forgeGunPrecision > 0.0001f) g_forgeGunPrecision /= 10;
			return;
		}
	}

	{
		const ::Menu::InputResult res = engine
			? engine->AddNumber("Launch Force", g_forgeGunShootForce, 0)
			: ::Menu::InputResult{};
		if (res.accepted)
		{
			std::string inputStr = Game::InputBox("", 11U, "", std::to_string(g_forgeGunShootForce));
			if (inputStr.length() > 0)
			{
				try { g_forgeGunShootForce = std::stof(inputStr); }
				catch (...) { Game::Print::PrintErrorInvalidInput(inputStr); }
			}
		}
		if (res.rightPressed)
		{
			if (g_forgeGunShootForce < FLT_MAX) g_forgeGunShootForce++;
			return;
		}
		if (res.leftPressed)
		{
			if (g_forgeGunShootForce > 0) g_forgeGunShootForce--;
			return;
		}
	}
}

void GravityGunSubmenu::Draw()
{
	DrawTitle();

	using namespace sub::GravityGun_catind;
	bool& enabled = Enabled();
	bool& multi = MultipleEntities();
	float& shootForce = ShootForce();
	unsigned char& typeIdx = TypeToPickUpIndex();

	static const std::vector<std::string> kEntityTypes{ "All", "Peds", "Vehicles", "Objects" };

	if (DrawToggle("Toggle", enabled) && enabled)
	{
		Game::Print::PrintBottomLeft("Use the ~b~" + GetWeaponLabel(WHash(), true) + "~s~ for hax.");
		Game::Print::PrintBottomLeft((std::string)"Use ~b~"
			+ (Menu::bitController ? "RS/LS" : "mouse scroll") + "~s~ to change the hold distance.");
		Game::Print::PrintBottomLeft("Shoot to launch.");
		return;
	}

	DrawToggle("Multiple Pick Ups", multi);

	Engine* engine = Engine::Current();
	{
		int idx = typeIdx;
		const ::Menu::InputResult res = engine
			? engine->AddTextList("Pick Up Type", idx, kEntityTypes)
			: ::Menu::InputResult{};
		if (res.rightPressed && typeIdx < static_cast<unsigned char>(kEntityTypes.size() - 1))
		{
			++typeIdx;
			return;
		}
		if (res.leftPressed && typeIdx > 0)
		{
			--typeIdx;
			return;
		}
	}

	{
		const ::Menu::InputResult res = engine
			? engine->AddNumber("Launch Force", shootForce, 0)
			: ::Menu::InputResult{};
		if (res.accepted)
		{
			std::string inputStr = Game::InputBox("", 11U, "", std::to_string(shootForce));
			if (inputStr.length() > 0)
			{
				try { shootForce = std::stof(inputStr); }
				catch (...) { Game::Print::PrintErrorInvalidInput(inputStr); }
			}
		}
		if (res.rightPressed) { if (shootForce < FLT_MAX) shootForce++; return; }
		if (res.leftPressed) { if (shootForce > 0) shootForce--; return; }
	}
}

void TriggerFxGunSubmenu::Draw()
{
	DrawTitle();

	DrawToggle("Gun Toggle", triggerFXGun);

	if (DrawOption("Select FX"))
	{
		NavigateTo("ptfx");
	}
}

namespace {

struct KaboomEntry { const char* name; Hash hash; };
constexpr KaboomEntry kKaboomEntries[] = {
	{ "Molotov",         3 },
	{ "Steam",           11 },
	{ "Flame",           12 },
	{ "Water Hydrant",   13 },
	{ "Flare",           22 },
	{ "Huge",            29 },
	{ "SuperFlame",      30 },
	{ "Firework",        38 },
	{ "Snowball",        39 },
	{ "Valkyrie Cannon", 40 },
	{ "Monkey",          3268439891u },
	{ "Mani",            3367706194u },
	{ "Zombie",          2890614022u },
	{ "Pogo",            3696858125u },
	{ "Police Bus",      2287941233u },
	{ "Slamvan",         833469436u },
	{ "Fixter",          3458454463u },
};

} // namespace

void KaboomGunSubmenu::Draw()
{
	DrawTitle();

	DrawToggle("Gun Toggle", kaboomGun);
	DrawToggle("Invisibility Toggle", kaboomGunInvis);
	DrawToggle("Random Toggle", kaboomGunRandBit);

	for (const auto& e : kKaboomEntries)
	{
		if (DrawSelectionItem(e.name, kaboomGunHash == e.hash))
		{
			if (e.hash > 70)
				Model(e.hash).Load();
			kaboomGunHash = e.hash;
		}
	}

	if (DrawOption("All Vehicles"))
		NavigateTo("weapon_vehicle_cats");

	if (DrawOption("~b~Input~s~ Model"))
	{
		std::string inputStr = Game::InputBox("", 64U, "Enter vehicle model name:");
		if (inputStr.length() > 0)
		{
			Model model(inputStr);
			if (model.IsInCdImage())
			{
				kaboomGunHash = model.hash;
				model.Load();
			}
			else Game::Print::PrintErrorInvalidModel(inputStr);
		}
	}
}

namespace {

struct BulletEntry { const char* name; Hash hash; };
const std::vector<BulletEntry>& BulletEntries()
{
	static const std::vector<BulletEntry> kBulletEntries{
		{ "Green Laser",        VEHICLE_WEAPON_PLAYER_LASER },
		{ "Red Laser",          VEHICLE_WEAPON_ENEMY_LASER },
		{ "Insurgent Turret",   VEHICLE_WEAPON_TURRET_INSURGENT },
		{ "Technical Turret",   VEHICLE_WEAPON_TURRET_TECHNICAL },
		{ "Valkyrie Turret",    VEHICLE_WEAPON_NOSE_TURRET_VALKYRIE },
		{ "Airstrike Rocket",   WEAPON_AIRSTRIKE_ROCKET },
		{ "Tazer",              WEAPON_STUNGUN },
		{ "Firework",           WEAPON_FIREWORK },
		{ "Snowball",           WEAPON_SNOWBALL },
		{ "Ball",               WEAPON_BALL },
		{ "Flare",              WEAPON_FLAREGUN },
		{ "Flare 2",            WEAPON_FLARE }
	};
	return kBulletEntries;
}

void ApplyBulletGunHash(Hash hash)
{
	Hash currentWeapon;
	BOOL bCurrentWeapon = GET_CURRENT_PED_WEAPON(g_Ped1, &currentWeapon, 1);

	if (!HAS_WEAPON_ASSET_LOADED(hash))
		REQUEST_WEAPON_ASSET(hash, 31, 0);
	GIVE_WEAPON_TO_PED(g_Ped1, hash, 120, 1, 1);

	if (bCurrentWeapon)
		SET_CURRENT_PED_WEAPON(g_Ped1, currentWeapon, true);

	bullet_gun_hash = hash;
}

} // namespace

void BulletGunSubmenu::Draw()
{
	DrawTitle();

	DrawToggle("Gun Toggle", bulletGun);

	for (const auto& e : BulletEntries())
	{
		if (DrawSelectionItem(e.name, bullet_gun_hash == e.hash))
			ApplyBulletGunHash(e.hash);
	}
}

namespace {

struct PedEntry { const char* name; Hash hash; };
const std::vector<PedEntry>& PedEntries()
{
	static const std::vector<PedEntry> kPedEntries{
		{ "Pogo",          3696858125u },
		{ "Mime",          1021093698u },
		{ "Mani",          3367706194u },
		{ "Imponent Rage",  880829941u },
		{ "Lester",        1302784073u },
		{ "Fattie",        3050275044u },
		{ "Cop",            368603149u },
		{ "Pilot",         2872052743u },
		{ "Tracy",         3728026165u },
		{ "Jimmy",         1459905209u },
		{ "Chimp",         2825402133u },
		{ "Monkey",        3268439891u },
		{ "Hen",           1794449327u },
		{ "Seagull",       3549666813u },
		{ "Shark",          113504370u },
		{ "Fish",           802685111u },
		{ "Cow",           4244282910u },
		{ "Mountain Lion",  307287994u },
		{ "Husky",         1318032802u },
		{ "Chop",           351016938u }
	};
	return kPedEntries;
}

void ApplyPedGunHash(Model newModel)
{
	if (newModel.IsInCdImage())
	{
		pedGunRandBit = false;

		Model oldModel = pedGunHash;
		if (oldModel.IsLoaded())
			oldModel.Unload();
		if (!newModel.IsLoaded())
			newModel.Load();

		pedGunHash = newModel;
	}
}

} // namespace

void PedGunSubmenu::Draw()
{
	DrawTitle();

	DrawToggle("Gun Toggle", pedGun);
	DrawToggle("Random Toggle", pedGunRandBit);

	if (DrawOption("All Peds"))
		NavigateTo("weapon_ped_gun_all");

	for (const auto& e : PedEntries())
	{
		if (DrawSelectionItem(e.name, pedGunHash.hash == e.hash))
			ApplyPedGunHash(Model(e.hash));
	}

	if (DrawOption("~b~Input~s~ Model"))
	{
		std::string inputStr = Game::InputBox("", 64U, "Enter ped model name:");
		if (inputStr.length() > 0)
		{
			Model model(inputStr);
			if (model.IsInCdImage())
			{
				pedGunHash = model;
				model.Load();
			}
			else Game::Print::PrintErrorInvalidModel(inputStr);
		}
	}
}

void PedGunAllPedsSubmenu::Draw()
{
	DrawTitle();

	if (DrawOption("Favourites")) NavigateTo("ped_model_changer_favourites");
	if (DrawOption("All Peds"))   NavigateTo("ped_model_changer");
}

namespace {

struct ObjectEntry { const char* name; Hash hash; };
const std::vector<ObjectEntry>& ObjectEntries()
{
	static const std::vector<ObjectEntry> kObjectEntries{
		{ "Poo",             2452367939u },
		{ "Body Parts",      3026386862u },
		{ "Corpse",          3283329087u },
		{ "Cone",            3235319999u },
		{ "Shopping Cart",   1395334609u },
		{ "Gold Bar",        4031179319u },
		{ "Jukebox",         1945457558u },
		{ "Christmas Tree S", 238789712u },
		{ "Bank Safe",       1089807209u },
		{ "W33D",            3989082015u },
		{ "EMP",              932490441u },
		{ "Laptop",          3618439924u },
		{ "Molten Gate",      735855031u },
		{ "Case",            1037912790u },
		{ "Snowman",         2677555217u },
		{ "Doors",            672525579u },
		{ "Fixter",          3458454463u },
		{ "Barracks",         630371791u },
		{ "Lectro",           640818791u },
		{ "Hakuchou",        1265391242u },
		{ "Thrust",          1836027715u },
		{ "Panto",           3863274624u }
	};
	return kObjectEntries;
}

void ApplyObjectGunHash(Model newModel)
{
	if (newModel.IsInCdImage())
	{
		objectGunRandBitO = false;
		objectGunRandBitV = false;

		Model oldModel = objectGunHash;
		if (oldModel.IsLoaded())
			oldModel.Unload();
		if (!newModel.IsLoaded())
			newModel.Load();

		objectGunHash = newModel.hash;
	}
}

} // namespace

void ObjectGunSubmenu::Draw()
{
	DrawTitle();

	DrawToggle("Gun Toggle", objectGun);

	if (DrawOption("All Objects"))
	{
		dict.clear();
		NavigateTo("weapon_object_spawner");
	}
	if (DrawOption("All Vehicles"))
		NavigateTo("weapon_vehicle_cats");

	for (const auto& e : ObjectEntries())
	{
		if (DrawSelectionItem(e.name, objectGunHash.hash == e.hash))
			ApplyObjectGunHash(Model(e.hash));
	}

	if (DrawOption("~b~Input~s~ Model"))
	{
		std::string inputStr = Game::InputBox("", 64U, "Enter object/vehicle model name:");
		if (inputStr.length() > 0)
		{
			Model model(inputStr);
			if (model.IsInCdImage())
			{
				objectGunHash = model;
				model.Load();
			}
			else Game::Print::PrintErrorInvalidModel(inputStr);
		}
	}
}

void WeaponVehicleCatsSubmenu::Draw()
{
	DrawTitle();

	if (DrawOption("Favourites"))     NavigateTo("vehicle_spawner_favourites");
	if (DrawOption("Vehicle Spawner")) NavigateTo("vehicle_spawner");
}

void WeaponObjectSpawnerSubmenu::Draw()
{
	DrawTitle();

	std::string& searchStr = dict;

	if (DrawOption(searchStr.empty() ? "SEARCH" : toUpperCopy(searchStr)))
	{
		searchStr = Game::InputBox(searchStr, 126U, "SEARCH", searchStr);
		toLowerInPlace(searchStr);
	}

	for (const auto& current : objectModels)
	{
		if (!searchStr.empty())
		{
			if (current.find(searchStr) == std::string::npos)
				continue;
		}

		Hash hash = GET_HASH_KEY(current.c_str());
		if (DrawSelectionItem(current, objectGunHash.hash == hash))
			ApplyObjectGunHash(Model(hash));
	}
}

namespace {

const char* kFavouriteWeaponsXml = "FavouriteWeapons.xml";

} // namespace

void WeaponFavouritesSubmenu::Draw()
{
	DrawTitle();

	using sub::WeaponFavourites_catind::AddWeaponToFavourites;
	using sub::WeaponFavourites_catind::IsWeaponAFavourite;
	using sub::WeaponFavourites_catind::RemoveWeaponFromFavourites;

	GTAped ped = g_Ped1;
	Hash currentPedWeapon = ped.GetWeapon();

	pugi::xml_document doc;
	const std::string xmlPath = (std::string)GetPathffA(Pathff::Main, true) + kFavouriteWeaponsXml;
	if (doc.load_file(xmlPath.c_str()).status != pugi::status_ok)
	{
		doc.reset();
		auto nodeDecleration = doc.append_child(pugi::node_declaration);
		nodeDecleration.append_attribute("version") = "1.0";
		nodeDecleration.append_attribute("encoding") = "ISO-8859-1";
		doc.append_child("FavouriteWeapons");
		doc.save_file(xmlPath.c_str());
		return;
	}
	pugi::xml_node nodeRoot = doc.document_element();

	if (DrawOption("Add New Weapon"))
	{
		std::string hashNameStr = Game::InputBox("", 40U, "Enter name (e.g. WEAPON_FLAMETHROWER):");
		if (hashNameStr.length() > 0)
		{
			WAIT(500);
			Hash hashNameHash = GET_HASH_KEY(hashNameStr.c_str());
			if (IS_WEAPON_VALID(hashNameHash))
			{
				std::string customNameStr = Game::InputBox("", 28U, "Enter custom name:", GetWeaponLabel(hashNameHash, true));
				if (customNameStr.length() > 0)
				{
					if (AddWeaponToFavourites(hashNameHash, customNameStr))
						Game::Print::PrintBottomLeft("Weapon ~b~added~s~.");
					else
						Game::Print::PrintBottomLeft("~r~Error:~s~ Unable to add weapon.");
				}
				else
					Game::Print::PrintErrorInvalidInput(customNameStr);
			}
			else
				Game::Print::PrintErrorInvalidInput(std::to_string(hashNameHash));
		}
		else
			Game::Print::PrintErrorInvalidInput(hashNameStr);
	}

	if (currentPedWeapon != WEAPON_UNARMED)
	{
		bool bIsCurrentWeaponAFav = IsWeaponAFavourite(currentPedWeapon);
		Engine* engine = Engine::Current();
		const bool pressed = engine
			? engine->AddCheckbox("Currently Held Weapon", bIsCurrentWeaponAFav,
				Checkbox::BOXTICK, Checkbox::BOXBLANK)
			: false;
		if (pressed)
		{
			if (!bIsCurrentWeaponAFav)
			{
				std::string customNameStr = Game::InputBox("", 28U, "Enter custom name:", GetWeaponLabel(currentPedWeapon, true));
				if (customNameStr.length())
				{
					if (AddWeaponToFavourites(currentPedWeapon, customNameStr))
						Game::Print::PrintBottomLeft("Weapon ~b~added~s~.");
					else
						Game::Print::PrintBottomLeft("~r~Error:~s~ Unable to add weapon.");
				}
				else
					Game::Print::PrintErrorInvalidInput(customNameStr);
			}
			else
			{
				if (RemoveWeaponFromFavourites(currentPedWeapon))
					Game::Print::PrintBottomLeft("Weapon ~b~removed~s~.");
				else
					Game::Print::PrintBottomLeft("~r~Error:~s~ Unable to remove weapon.");
			}
		}
	}

	if (nodeRoot.first_child())
	{
		DrawBreak("---Added Weapons---");

		Engine* engine = Engine::Current();
		for (auto nodeLocToLoad = nodeRoot.first_child(); nodeLocToLoad;
			nodeLocToLoad = nodeLocToLoad.next_sibling())
		{
			const std::string customName = nodeLocToLoad.attribute("customName").as_string();
			Hash whash = nodeLocToLoad.attribute("hash").as_uint();

			if (DrawOption(customName))
			{
				INT& selectedCategoryForInItem = g_Ped4;
				INT& selectedWeaponForInItem = msCurrentPaintIndex;
				auto& vAllAddedWeaponsArr = *WeaponIndivs::vAllWeapons.back();
				for (UINT i = 0; i < vAllAddedWeaponsArr.size(); i++)
				{
					if (vAllAddedWeaponsArr[i].weaponHash == whash)
					{
						selectedCategoryForInItem = (INT)(WeaponIndivs::vAllWeapons.size() - 1);
						selectedWeaponForInItem = (INT)i;
						NavigateTo("weapon_in_item");
						break;
					}
				}
			}

			// "Remove" contextual hotkey on the highlighted row.
			if (engine && IsCurrentRowSelected())
			{
				if (Menu::bitController)
				{
					engine->AddInstructionalButton(INPUT_SCRIPT_RLEFT, "Remove", /*isKey=*/false);
					if (IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_SCRIPT_RLEFT))
					{
						nodeLocToLoad.parent().remove_child(nodeLocToLoad);
						doc.save_file(xmlPath.c_str());
						return;
					}
				}
				else
				{
					engine->AddInstructionalButton(static_cast<int>(VirtualKey::B), "Remove", /*isKey=*/true);
					if (IsKeyJustUp(VirtualKey::B))
					{
						nodeLocToLoad.parent().remove_child(nodeLocToLoad);
						doc.save_file(xmlPath.c_str());
						return;
					}
				}
			}
		}
	}
}

void WeaponCategoriesSubmenu::Draw()
{
	DrawTitle();

	using WeaponIndivs::WEAPONTYPE;
	using WeaponIndivs::vCategoryNames;

	if (g_WeaponOpsPedOverride != 0)
	{
		g_Ped1 = g_WeaponOpsPedOverride;
		g_Ped2 = g_WeaponOpsPlayerOverride;
	}
	else
	{
		g_Ped1 = PLAYER_PED_ID();
		g_Ped2 = PLAYER_ID();
	}

	auto& ped = g_Ped1;
	INT& selectedCategory = g_Ped4;

	Hash pedCurrentWeapon;
	GET_CURRENT_PED_WEAPON(ped, &pedCurrentWeapon, 1);

	if (DrawOption("Currently Held Weapon"))
	{
		if (pedCurrentWeapon == WEAPON_UNARMED)
		{
			Game::Print::PrintBottomCentre(oss_ << "This weapon isn't exactly valid: ~b~"
				<< GetWeaponLabel(pedCurrentWeapon, true) << "~s~.");
		}
		else
		{
			selectedCategory = WEAPONTYPE::WEAPE_CURRENTLYHELD;
			NavigateTo("weapon_in_item");
		}
	}

	if (GET_PLAYER_PED(g_Ped2) == ped)
	{
		if (DrawOption("Parachutes")) NavigateTo("weapon_parachute");
	}

	for (int i = 0; i < (int)vCategoryNames.size() - 1 /*Minus Added weapons*/; i++)
	{
		if (DrawOption(vCategoryNames[i]))
		{
			selectedCategory = i;
			NavigateTo("weapon_in_category");
		}
	}

	if (DrawOption("Favourites"))
	{
		selectedCategory = (INT)(vCategoryNames.size() - 1);
		NavigateTo("weapon_favourites");
	}
}

void WeaponInCategorySubmenu::Draw()
{
	using WeaponIndivs::vAllWeapons;
	using WeaponIndivs::vCategoryNames;
	using sub::WeaponFavourites_catind::IsWeaponAFavourite;
	using sub::WeaponFavourites_catind::AddWeaponToFavourites;
	using sub::WeaponFavourites_catind::RemoveWeaponFromFavourites;

	INT& selectedCategory = g_Ped4;
	INT& selectedWeapon = msCurrentPaintIndex;

	Engine* engineForTitle = Engine::Current();
	if (engineForTitle && selectedCategory >= 0 && selectedCategory < (INT)vCategoryNames.size())
		engineForTitle->AddTitle(vCategoryNames[selectedCategory]);
	else
		DrawTitle();

	if (selectedCategory < 0 || selectedCategory >= (INT)vAllWeapons.size())
		return;

	Engine* engine = Engine::Current();
	auto& weaponsInCat = *vAllWeapons[selectedCategory];

	for (int i = 0; i < (int)weaponsInCat.size(); i++)
	{
		auto& thisWeaponInfo = weaponsInCat[i];

		if (DrawOption(GetWeaponLabel(thisWeaponInfo.weaponHash, true)))
		{
			selectedWeapon = i;
			NavigateTo("weapon_in_item");
		}

		if (engine && IsCurrentRowSelected())
		{
			const bool bIsAFav = IsWeaponAFavourite(thisWeaponInfo.weaponHash);
			const std::string hintLabel = (!bIsAFav ? "Add to" : "Remove from")
				+ (std::string)" favourites";

			if (Menu::bitController)
			{
				engine->AddInstructionalButton(INPUT_SCRIPT_RLEFT, hintLabel, /*isKey=*/false);
				if (IS_DISABLED_CONTROL_JUST_PRESSED(2, INPUT_SCRIPT_RLEFT))
				{
					if (!bIsAFav)
						AddWeaponToFavourites(thisWeaponInfo.weaponHash,
							Game::InputBox("", 28U, "Enter custom name:",
								GetWeaponLabel(thisWeaponInfo.weaponHash, true)));
					else
						RemoveWeaponFromFavourites(thisWeaponInfo.weaponHash);
				}
			}
			else
			{
				engine->AddInstructionalButton(static_cast<int>(VirtualKey::B), hintLabel, /*isKey=*/true);
				if (IsKeyJustUp(VirtualKey::B))
				{
					if (!bIsAFav)
						AddWeaponToFavourites(thisWeaponInfo.weaponHash,
							Game::InputBox("", 28U, "Enter custom name:",
								GetWeaponLabel(thisWeaponInfo.weaponHash, true)));
					else
						RemoveWeaponFromFavourites(thisWeaponInfo.weaponHash);
				}
			}
		}
	}
}

void WeaponInItemSubmenu::Draw()
{
	DrawTitle();

	using WeaponIndivs::WEAPONTYPE;
	using WeaponIndivs::vAllWeapons;
	using WeaponIndivs::get_weapon;

	auto& ped = g_Ped1;
	NETWORK_REQUEST_CONTROL_OF_ENTITY(ped);

	INT& selectedCategory = g_Ped4;
	INT& selectedWeapon = msCurrentPaintIndex;

	Hash pedCurrentWeapon;
	GET_CURRENT_PED_WEAPON(ped, &pedCurrentWeapon, 1);

	Hash whash;
	const WeaponAndComponents* selWeapon = nullptr;
	const std::vector<NamedWeaponComponent>* selWeaponComponents = nullptr;
	const std::vector<std::string>* selWeaponTints = nullptr;

	if (selectedCategory == WEAPONTYPE::WEAPE_CURRENTLYHELD)
	{
		whash = pedCurrentWeapon;
		selWeapon = get_weapon(whash);
		if (selWeapon == nullptr)
			return;
		selWeaponComponents = &selWeapon->components;
		selWeaponTints = selWeapon->tintCaptions;
	}
	else
	{
		if (selectedCategory < 0 || selectedCategory >= (INT)vAllWeapons.size())
			return;
		auto& weaponsInCat = *vAllWeapons[selectedCategory];
		if (selectedWeapon < 0 || selectedWeapon >= (INT)weaponsInCat.size())
			return;
		selWeapon = &weaponsInCat[selectedWeapon];
		whash = selWeapon->weaponHash;
		selWeaponComponents = &selWeapon->components;
		selWeaponTints = selWeapon->tintCaptions;
	}

	if (DrawSelectionItem("Equip Weapon", pedCurrentWeapon == whash))
	{
		if (DOES_ENTITY_EXIST(ped))
		{
			if (!HAS_PED_GOT_WEAPON(ped, whash, false))
			{
				GIVE_DELAYED_WEAPON_TO_PED(ped, whash, 1000, 0);
				int maxAmmo = 0;
				GET_MAX_AMMO(ped, whash, &maxAmmo);
				SET_AMMO_IN_CLIP(ped, whash, GET_MAX_AMMO_IN_CLIP(ped, whash, false));
				SET_PED_AMMO(ped, whash, maxAmmo, 0);
			}
			SET_CURRENT_PED_WEAPON(ped, whash, true);
		}
	}

	if (DrawOption("Fill Ammo"))
	{
		int maxAmmo = 0;
		GET_MAX_AMMO(ped, whash, &maxAmmo);
		SET_AMMO_IN_CLIP(ped, whash, GET_MAX_AMMO_IN_CLIP(ped, whash, false));
		SET_PED_AMMO(ped, whash, maxAmmo, 0);
	}

	if (DrawOption("Empty Ammo"))
	{
		SET_AMMO_IN_CLIP(ped, whash, -1);
		SET_PED_AMMO(ped, whash, -1, 0);
	}

	if (DrawOption("Remove Weapon"))
	{
		if (HAS_PED_GOT_WEAPON(ped, whash, false))
			REMOVE_WEAPON_FROM_PED(ped, whash);
	}

	if (selWeapon != nullptr)
	{
		if (selWeaponComponents->size() > 0 || selWeaponTints != nullptr)
		{
			if (DrawOption("Attachments & Tints"))
				NavigateTo("weapon_in_item_mods");
		}
	}
}

void WeaponInItemModsSubmenu::Draw()
{
	DrawTitle();

	using WeaponIndivs::WEAPONTYPE;
	using WeaponIndivs::vAllWeapons;
	using WeaponIndivs::get_weapon;

	auto& ped = g_Ped1;
	NETWORK_REQUEST_CONTROL_OF_ENTITY(ped);

	INT& selectedCategory = g_Ped4;
	INT& selectedWeapon = msCurrentPaintIndex;

	Hash pedCurrentWeapon;
	GET_CURRENT_PED_WEAPON(ped, &pedCurrentWeapon, 1);

	Hash whash;
	const WeaponAndComponents* selWeapon = nullptr;
	const std::vector<NamedWeaponComponent>* selWeaponComponents = nullptr;
	const std::vector<std::string>* selWeaponTints = nullptr;

	if (selectedCategory == WEAPONTYPE::WEAPE_CURRENTLYHELD)
	{
		whash = pedCurrentWeapon;
		selWeapon = get_weapon(whash);
		if (selWeapon == nullptr)
			return;
		selWeaponComponents = &selWeapon->components;
		selWeaponTints = selWeapon->tintCaptions;
	}
	else
	{
		if (selectedCategory < 0 || selectedCategory >= (INT)vAllWeapons.size())
			return;
		auto& weaponsInCat = *vAllWeapons[selectedCategory];
		if (selectedWeapon < 0 || selectedWeapon >= (INT)weaponsInCat.size())
			return;
		selWeapon = &weaponsInCat[selectedWeapon];
		whash = selWeapon->weaponHash;
		selWeaponComponents = &selWeapon->components;
		selWeaponTints = selWeapon->tintCaptions;
	}

	int currentTint = GET_PED_WEAPON_TINT_INDEX(ped, whash);

	WEAPON::GIVE_WEAPON_TO_PED(ped, whash, 9999, false, true);
	WEAPON::SET_CURRENT_PED_WEAPON(ped, whash, true);

	Engine* engine = Engine::Current();

	for (auto& comp : *selWeaponComponents)
	{
		if (!WEAPON::DOES_WEAPON_TAKE_WEAPON_COMPONENT(whash, comp.hash))
			continue;

		bool bHasComponent = HAS_PED_GOT_WEAPON_COMPONENT(ped, whash, comp.hash) != 0;

		if (bHasComponent && toUpperCopy(comp.name).find("CAMO") != std::string::npos)
		{
			int currentLivery = GET_PED_WEAPON_COMPONENT_TINT_INDEX(ped, whash, comp.hash);
			std::string label = Game::GetGXTEntry("WCT_C_TINT_" + std::to_string(currentLivery));

			const ::Menu::InputResult res = engine
				? engine->AddTextList(comp.name, 0, std::vector<std::string>{ label })
				: ::Menu::InputResult{};

			bool changed = false;
			if (res.rightPressed && currentLivery < 31)
			{
				currentLivery++;
				changed = true;
			}
			else if (res.leftPressed && currentLivery > 0)
			{
				currentLivery--;
				changed = true;
			}

			if (changed)
				SET_PED_WEAPON_COMPONENT_TINT_INDEX(ped, whash, comp.hash, currentLivery);
		}
		else
		{
			const bool pressed = engine
				? engine->AddCheckbox(comp.name, bHasComponent, Checkbox::WEAPONTHING, Checkbox::NONE)
				: false;
			if (pressed)
			{
				if (bHasComponent)
					REMOVE_WEAPON_COMPONENT_FROM_PED(ped, whash, comp.hash);
				else
					GIVE_WEAPON_COMPONENT_TO_PED(ped, whash, comp.hash);
			}
		}
	}

	if (selWeaponTints != nullptr)
	{
		if (!selWeaponComponents->empty())
			DrawBreak("---Tints---");

		for (int i = 0; i < (int)selWeaponTints->size(); i++)
		{
			const bool pressed = engine
				? engine->AddCheckbox(selWeaponTints->at(i), currentTint == i,
					Checkbox::WEAPONTHING, Checkbox::NONE)
				: false;
			if (pressed)
				SET_PED_WEAPON_TINT_INDEX(ped, whash, i);
		}
	}
}

void WeaponParachuteSubmenu::Draw()
{
	DrawTitle();

	using WeaponIndivs::vCaptions_ChuteTints;

	auto& ped = g_Ped1;
	NETWORK_REQUEST_CONTROL_OF_ENTITY(ped);
	auto& player = g_Ped2;

	int currentChuteTint;
	GET_PLAYER_PARACHUTE_TINT_INDEX(player, &currentChuteTint);

	RGBA paraSmokeCol;
	GET_PLAYER_PARACHUTE_SMOKE_TRAIL_COLOR(player, &paraSmokeCol.R, &paraSmokeCol.G, &paraSmokeCol.B);

	Hash whash = GADGET_PARACHUTE;
	bool doesPedHavePara = HAS_PED_GOT_WEAPON(ped, whash, false) != 0;

	if (DrawSelectionItem("Equip Primary", doesPedHavePara))
	{
		if (DOES_ENTITY_EXIST(ped))
			GivePedParachute(ped);
	}

	if (DrawSelectionItem("Equip Secondary", doesPedHavePara))
	{
		if (DOES_ENTITY_EXIST(ped))
		{
			GivePedParachute(ped);
			SET_PLAYER_HAS_RESERVE_PARACHUTE(g_Ped2);
		}
	}

	if (DrawOption("Remove"))
	{
		if (doesPedHavePara)
			REMOVE_WEAPON_FROM_PED(ped, whash);
	}

	Engine* engine = Engine::Current();
	if (DrawOption("Set Smoke Colour"))
	{
		bitMSPaintsRGBMode = 7;
		NavigateTo("vehicle_modshop_paints_rgb");
	}
	if (engine && IsCurrentRowSelected())
		engine->AddPresetColourOptionsPreview(paraSmokeCol);

	DrawBreak("---Tints---");

	for (UINT i = 0; i < vCaptions_ChuteTints.size(); i++)
	{
		if (DrawSelectionItem(vCaptions_ChuteTints[i], currentChuteTint == (int)i,
			Checkbox::WEAPONTHING, Checkbox::NONE))
		{
			SET_PLAYER_PARACHUTE_TINT_INDEX(player, i);
			if (IS_DLC_PRESENT(0xC40B8B70))
				SET_PLAYER_PARACHUTE_MODEL_OVERRIDE(player, 0x73268708);
			else if (IS_DLC_PRESENT(0x55292CC7))
				SET_PLAYER_PARACHUTE_MODEL_OVERRIDE(player, 0x815E52EB);
		}
	}
}

void WeaponLaserSightSubmenu::Draw()
{
	DrawTitle();

	DrawToggle("Toggle", sub::LaserSight_catind::bEnabled);

	if (DrawOption("Set Colour"))
	{
		sub::g_settingsRGBA = &sub::LaserSight_catind::_colour;
		NavigateTo("settings_colours2");
	}
	Engine* engine = Engine::Current();
	if (engine && IsCurrentRowSelected())
		engine->AddPresetColourOptionsPreview(sub::LaserSight_catind::_colour);
}

void WeaponLoadoutsSubmenu::Draw()
{
	DrawTitle();

	using sub::WeaponsLoadouts_catind::Create;

	std::string& searchStr = dict2;
	std::string& name = dict;
	std::string& dirStr = dict3;
	auto& ped = g_Ped1;

	bool save2Pressed = DrawOption("Save Current Loadout");
	bool createFolderPressed = DrawOption("Create New Folder");

	if (dirStr.empty()) dirStr = GetPathffA(Pathff::WeaponsLoadout, false);

	std::vector<std::string> vfilnames;
	DIR* dir_point = opendir(dirStr.c_str());
	if (dir_point)
	{
		dirent* entry = readdir(dir_point);
		while (entry)
		{
			vfilnames.push_back(entry->d_name);
			entry = readdir(dir_point);
		}
		closedir(dir_point);
	}

	DrawBreak("---Found Files---");

	if (DrawOption(".."))
	{
		dirStr = dirStr.substr(0, dirStr.rfind("\\"));
	}

	if (!vfilnames.empty())
	{
		if (DrawOption(searchStr.empty() ? "SEARCH" : searchStr))
		{
			searchStr = Game::InputBox(searchStr, 126U, "SEARCH", toLowerCopy(searchStr));
			toUpperInPlace(searchStr);
		}

		for (auto& filname : vfilnames)
		{
			if (filname.front() == '.' || filname.front() == ',')
				continue;
			if (!searchStr.empty())
			{
				if (toUpperCopy(filname).find(searchStr) == std::string::npos)
					continue;
			}

			bool isFolder = PathIsDirectoryA((dirStr + "\\" + filname).c_str()) != 0;
			bool isXml = filname.length() > 4 && filname.rfind(".xml") == filname.length() - 4;
			Checkbox icon = Checkbox::NONE;
			if (isFolder) icon = Checkbox::ARROWRIGHT;
			else if (isXml) icon = Checkbox::TICK2;

			Engine* engine = Engine::Current();

			if (isFolder)
			{
				const bool pressed = engine
					? engine->AddCheckbox(filname + " >>>", true, icon, Checkbox::NONE)
					: false;
				if (pressed)
				{
					dirStr = dirStr + "\\" + filname;
				}
			}
			else if (isXml)
			{
				const bool pressed = engine
					? engine->AddCheckbox(filname, true, icon, Checkbox::NONE)
					: false;
				if (pressed)
				{
					name = filname.substr(0, filname.rfind('.'));
					NavigateTo("weapon_loadout_item");
					return;
				}
			}
		}
	}

	if (save2Pressed)
	{
		std::string inputStr = Game::InputBox("", 28U, "Enter loadout name:");
		if (inputStr.length() > 0)
		{
			if (!IsSafePath(inputStr))
			{
				Game::Print::PrintBottomCentre("~r~Error:~s~ Invalid characters in name.");
			}
			else if (Create(ped, dirStr + "\\" + inputStr + ".xml"))
				Game::Print::PrintBottomLeft("Loadout ~b~saved~s~.");
			else
				Game::Print::PrintBottomLeft("~r~Error:~s~ Unable to save loadout.");
		}
		else
			Game::Print::PrintErrorInvalidInput(inputStr);
	}

	if (createFolderPressed)
	{
		std::string inputStr = Game::InputBox("", 28U, "Enter folder name:");
		if (inputStr.length() > 0)
		{
			if (!IsSafePath(inputStr))
			{
				Game::Print::PrintBottomCentre("~r~Error:~s~ Invalid characters in name.");
			}
			else if (CreateDirectoryA((dirStr + "\\" + inputStr).c_str(), NULL)
				|| GetLastError() == ERROR_ALREADY_EXISTS)
			{
				dirStr = dirStr + "\\" + inputStr;
				Game::Print::PrintBottomLeft("Folder ~b~created~s~.");
			}
			else
			{
				Game::Print::PrintBottomCentre("~r~Failed~s~ to create folder.");
			}
		}
		else
			Game::Print::PrintErrorInvalidInput(inputStr);
		return;
	}
}

void WeaponLoadoutItemSubmenu::Draw()
{
	DrawTitle();

	using sub::WeaponsLoadouts_catind::Apply;
	using sub::WeaponsLoadouts_catind::Create;

	std::string& name = dict;
	std::string& dirStr = dict3;
	auto& ped = g_Ped1;
	const std::string filePath = dirStr + "\\" + name + ".xml";

	if (DrawOption("Apply"))
	{
		if (Apply(ped, filePath))
			Game::Print::PrintBottomLeft("Loadout ~b~applied~s~.");
		else
			Game::Print::PrintBottomLeft("~r~Error~s~ applying loadout.");
	}

	if (DrawOption("Rename File"))
	{
		std::string inputStr = Game::InputBox("", 28U, "Enter new name:", name);
		if (inputStr.length())
		{
			if (!IsSafePath(inputStr))
			{
				Game::Print::PrintBottomCentre("~r~Error:~s~ Invalid characters in name.");
			}
			else if (rename(filePath.c_str(), (dirStr + "\\" + inputStr + ".xml").c_str()) == 0)
			{
				name = inputStr;
				Game::Print::PrintBottomLeft("File ~b~renamed~s~.");
			}
			else
				Game::Print::PrintBottomCentre("~r~Error~s~ renaming file.");
		}
		else
			Game::Print::PrintErrorInvalidInput(inputStr);
	}

	if (DrawOption("Overwrite File"))
	{
		if (Create(ped, filePath))
			Game::Print::PrintBottomLeft("File ~b~overwritten~s~.");
		else
			Game::Print::PrintBottomCentre("~r~Error:~s~ Unable to overwrite file.");
	}

	if (DrawOption("Delete File"))
	{
		if (remove(filePath.c_str()) == 0)
			Game::Print::PrintBottomLeft("File ~b~deleted~s~.");
		else
			Game::Print::PrintBottomCentre("~r~Error~s~ deleting file.");

		Engine* engine = Engine::Current();
		if (engine) engine->GoBack();
		return;
	}
}

}
REGISTER_SUBMENU(::Menu::WeaponSubmenu)
REGISTER_SUBMENU(::Menu::WeaponFavouritesSubmenu)
REGISTER_SUBMENU(::Menu::WeaponCategoriesSubmenu)
REGISTER_SUBMENU(::Menu::WeaponInCategorySubmenu)
REGISTER_SUBMENU(::Menu::WeaponInItemSubmenu)
REGISTER_SUBMENU(::Menu::WeaponInItemModsSubmenu)
REGISTER_SUBMENU(::Menu::WeaponLoadoutsSubmenu)
REGISTER_SUBMENU(::Menu::WeaponLoadoutItemSubmenu)
REGISTER_SUBMENU(::Menu::WeaponParachuteSubmenu)
REGISTER_SUBMENU(::Menu::WeaponLaserSightSubmenu)
REGISTER_SUBMENU(::Menu::ForgeGunSubmenu)
REGISTER_SUBMENU(::Menu::GravityGunSubmenu)
REGISTER_SUBMENU(::Menu::KaboomGunSubmenu)
REGISTER_SUBMENU(::Menu::TriggerFxGunSubmenu)
REGISTER_SUBMENU(::Menu::BulletGunSubmenu)
REGISTER_SUBMENU(::Menu::PedGunSubmenu)
REGISTER_SUBMENU(::Menu::PedGunAllPedsSubmenu)
REGISTER_SUBMENU(::Menu::ObjectGunSubmenu)
REGISTER_SUBMENU(::Menu::WeaponVehicleCatsSubmenu)
REGISTER_SUBMENU(::Menu::WeaponObjectSpawnerSubmenu)
