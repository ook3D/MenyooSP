#pragma once

#include "../Menu/Submenu.h"

#include "../Util/GTAmath.h"

#include <string>
#include <vector>

namespace Menu {

struct BunkerLocation
{
	std::string name;
	Vector3 pos;
	std::string ipl;
};

struct BunkerInteriorOption
{
	std::string name;
	std::string value;
};

struct BunkerInfoStructure
{
	const BunkerLocation* location;
	unsigned char styleOption;
	unsigned char setOption;
	unsigned char securityOption;
	unsigned char officeOption;
	unsigned char gunRangeOption;
	unsigned char gunLockerOption;
	unsigned char gunSchematicOption;
};

class TeleportBunkersSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "teleport_bunkers"; }
	const char* Title() const override { return "Bunkers"; }
	void Draw() override;

	static const std::vector<BunkerLocation>& Locations();
	static const std::vector<BunkerInteriorOption>& StyleOptions();
	static const std::vector<BunkerInteriorOption>& SetOptions();
	static const std::vector<BunkerInteriorOption>& SecurityOptions();
	static const std::vector<BunkerInteriorOption>& OfficeOptions();
	static const std::vector<BunkerInteriorOption>& GunRangeOptions();
	static const std::vector<BunkerInteriorOption>& GunLockerOptions();
	static const std::vector<BunkerInteriorOption>& GunSchematicOptions();
	static BunkerInfoStructure& CurrentInfo();
};

class TeleportBunkersInLocSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "teleport_bunkers_in_loc"; }
	const char* Title() const override { return "Bunker"; }
	void Draw() override;
};

// ---- MOC ----

struct MocLocation
{
	std::string name;
	Vector3 pos;
	std::string ipl;
};

struct MocInteriorOption
{
	std::string name;
	std::string value;
};

struct MocInfoStructure
{
	const MocLocation* location;
	unsigned char styleOption;
};

class TeleportMocSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "teleport_moc"; }
	const char* Title() const override { return "MOCs"; }
	void Draw() override;

	static const std::vector<MocLocation>& Locations();
	static const std::vector<MocInteriorOption>& StyleOptions();
	static MocInfoStructure& CurrentInfo();
};

class TeleportMocInLocSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "teleport_moc_in_loc"; }
	const char* Title() const override { return "MOC"; }
	void Draw() override;
};

}