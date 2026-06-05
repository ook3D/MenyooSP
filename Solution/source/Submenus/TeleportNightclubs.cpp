#include "TeleportNightclubs.h"

#include "../Menu/Engine.h"
#include "../Menu/SubmenuRegistry.h"

#include "Teleport/TeleMethods.h"

#include <string_view>

namespace Menu {

using sub::TeleportLocations_catind::TeleLocation;
using sub::TeleportLocations_catind::TeleMethods::ToTeleLocation241;

namespace {

const std::vector<std::string_view> vNcPropsROOTCAUSE{
	"Int01_ba_security_upgrade"_sv,
	"Int01_ba_equipment_setup"_sv,
	"Int01_ba_Style01"_sv,
	"Int01_ba_Style02"_sv,
	"Int01_ba_Style03"_sv,
	"Int01_ba_style01_podium"_sv,
	"Int01_ba_style02_podium"_sv,
	"Int01_ba_style03_podium"_sv,
	"int01_ba_lights_screen"_sv,
	"Int01_ba_Screen"_sv,
	"Int01_ba_bar_content"_sv,
	"Int01_ba_booze_01"_sv,
	"Int01_ba_booze_02"_sv,
	"Int01_ba_booze_03"_sv,
	"Int01_ba_dj01"_sv,
	"Int01_ba_dj02"_sv,
	"Int01_ba_dj03"_sv,
	"Int01_ba_dj04"_sv,
	"DJ_01_Lights_01"_sv,
	"DJ_01_Lights_02"_sv,
	"DJ_01_Lights_03"_sv,
	"DJ_01_Lights_04"_sv,
	"DJ_02_Lights_01"_sv,
	"DJ_02_Lights_02"_sv,
	"DJ_02_Lights_03"_sv,
	"DJ_02_Lights_04"_sv,
	"DJ_03_Lights_01"_sv,
	"DJ_03_Lights_02"_sv,
	"DJ_03_Lights_03"_sv,
	"DJ_03_Lights_04"_sv,
	"DJ_04_Lights_01"_sv,
	"DJ_04_Lights_02"_sv,
	"DJ_04_Lights_03"_sv,
	"DJ_04_Lights_04"_sv,
	"light_rigs_off"_sv,
	"Int01_ba_lightgrid_01"_sv,
	"Int01_ba_Clutter"_sv,
	"Int01_ba_equipment_upgrade"_sv,
	"Int01_ba_clubname_01"_sv,
	"Int01_ba_clubname_02"_sv,
	"Int01_ba_clubname_03"_sv,
	"Int01_ba_clubname_04"_sv,
	"Int01_ba_clubname_05"_sv,
	"Int01_ba_clubname_06"_sv,
	"Int01_ba_clubname_07"_sv,
	"Int01_ba_clubname_08"_sv,
	"Int01_ba_clubname_09"_sv,
	"Int01_ba_dry_ice"_sv,
	"Int01_ba_deliverytruck"_sv,
	"Int01_ba_trophy04"_sv,
	"Int01_ba_trophy05"_sv,
	"Int01_ba_trophy07"_sv,
	"Int01_ba_trophy09"_sv,
	"Int01_ba_trophy08"_sv,
	"Int01_ba_trophy11"_sv,
	"Int01_ba_trophy10"_sv,
	"Int01_ba_trophy03"_sv,
	"Int01_ba_trophy01"_sv,
	"Int01_ba_trophy02"_sv,
	"Int01_ba_trad_lights"_sv,
	"Int01_ba_Worklamps"_sv,
};

const std::vector<std::string_view> vNcPropsROOTCAUSE2{
	"Int02_ba_floor01"_sv,
	"Int02_ba_floor02"_sv,
	"Int02_ba_floor03"_sv,
	"Int02_ba_floor04"_sv,
	"Int02_ba_floor05"_sv,
	"Int02_ba_sec_upgrade_grg"_sv,
	"Int02_ba_sec_upgrade_strg"_sv,
	"Int02_ba_sec_upgrade_desk"_sv,
	"Int02_ba_storage_blocker"_sv,
	"Int02_ba_garage_blocker"_sv,
	"Int02_ba_FanBlocker01"_sv,
	"Int02_ba_equipment_upgrade"_sv,
	"Int02_ba_coke01"_sv,
	"Int02_ba_coke02"_sv,
	"Int02_ba_meth01"_sv,
	"Int02_ba_meth02"_sv,
	"Int02_ba_meth03"_sv,
	"Int02_ba_meth04"_sv,
	"Int02_ba_Weed01"_sv,
	"Int02_ba_Weed02"_sv,
	"Int02_ba_Weed03"_sv,
	"Int02_ba_Weed04"_sv,
	"Int02_ba_Weed05"_sv,
	"Int02_ba_Weed06"_sv,
	"Int02_ba_Weed07"_sv,
	"Int02_ba_Weed08"_sv,
	"Int02_ba_Weed09"_sv,
	"Int02_ba_Weed10"_sv,
	"Int02_ba_Weed11"_sv,
	"Int02_ba_Weed12"_sv,
	"Int02_ba_Weed13"_sv,
	"Int02_ba_Weed14"_sv,
	"Int02_ba_Weed15"_sv,
	"Int02_ba_Weed16"_sv,
	"Int02_ba_Forged01"_sv,
	"Int02_ba_Forged02"_sv,
	"Int02_ba_Forged03"_sv,
	"Int02_ba_Forged04"_sv,
	"Int02_ba_Forged05"_sv,
	"Int02_ba_Forged06"_sv,
	"Int02_ba_Forged07"_sv,
	"Int02_ba_Forged08"_sv,
	"Int02_ba_Forged09"_sv,
	"Int02_ba_Forged10"_sv,
	"Int02_ba_Forged11"_sv,
	"Int02_ba_Forged12"_sv,
	"Int02_ba_Cash01"_sv,
	"Int02_ba_Cash02"_sv,
	"Int02_ba_Cash03"_sv,
	"Int02_ba_Cash04"_sv,
	"Int02_ba_Cash05"_sv,
	"Int02_ba_Cash06"_sv,
	"Int02_ba_Cash07"_sv,
	"Int02_ba_Cash08"_sv,
	"Int02_ba_truckmod"_sv,
	"Int02_ba_truckmod"_sv,
	"Int02_ba_coke_EQP"_sv,
	"Int02_ba_Cash_EQP"_sv,
	"Int02_ba_Forged_EQP"_sv,
	"Int02_ba_meth_EQP"_sv,
	"Int02_ba_Weed_EQP"_sv,
	"Int02_ba_DeskPC"_sv,
	"Int02_ba_sec_desks_L1"_sv,
	"Int02_ba_sec_desks_L2345"_sv,
	"Int02_ba_sec_upgrade_desk02"_sv,
	"Int02_ba_clutterstuff"_sv,
};

const std::vector<std::string_view> vNcPropsROOTCAUSE3{
	"Int_03_ba_weapons_mod"_sv,
	"Int_03_ba_drone"_sv,
	"Int_03_ba_Design_01"_sv,
	"Int_03_ba_Design_02"_sv,
	"Int_03_ba_Design_03"_sv,
	"Int_03_ba_Design_04"_sv,
	"Int_03_ba_Design_05"_sv,
	"Int_03_ba_Design_06"_sv,
	"Int_03_ba_Design_07"_sv,
	"Int_03_ba_Design_08"_sv,
	"Int_03_ba_Design_09"_sv,
	"Int_03_ba_Design_10"_sv,
	"Int_03_ba_Design_11"_sv,
	"Int_03_ba_Design_12"_sv,
	"Int_03_ba_Design_13"_sv,
	"Int_03_ba_Design_14"_sv,
	"Int_03_ba_Design_15"_sv,
	"Int_03_ba_Design_16"_sv,
	"Int_03_ba_Design_17"_sv,
	"Int_03_ba_Design_18"_sv,
	"Int_03_ba_Design_19"_sv,
	"Int_03_ba_Design_20"_sv,
	"Int_03_ba_Design_21"_sv,
	"Int_03_ba_Design_22"_sv,
	"Int_03_ba_Design_23"_sv,
	"Int_03_ba_Design_24"_sv,
	"Int_03_ba_Design_25"_sv,
	"Int_03_ba_bikemod"_sv,
	"Int_03_ba_Tint"_sv,
	"Int_03_ba_Light_Rig1"_sv,
	"Int_03_ba_Light_Rig2"_sv,
	"Int_03_ba_Light_Rig3"_sv,
	"Int_03_ba_Light_Rig4"_sv,
	"Int_03_ba_Light_Rig5"_sv,
	"Int_03_ba_Light_Rig6"_sv,
	"Int_03_ba_Light_Rig7"_sv,
	"Int_03_ba_Light_Rig8"_sv,
	"Int_03_ba_Light_Rig9"_sv,
};

} // anonymous namespace

const std::vector<TeleLocation>
TeleportNightclubsSubmenu::vOtherNightclubRelatedTeleports
{
	TeleLocation("Nightclub (Edgy)",                -1569.2500f, -3017.3900f, -73.2200f,
		{ "ba_int_placement_ba_interior_0_dlc_int_01_ba_milo_"_sv }, {}, vNcPropsROOTCAUSE,  true, false, true),
	TeleLocation("Nightclub (Glamorous)",           -1569.2500f, -3017.3900f, -73.2200f,
		{ "ba_int_placement_ba_interior_0_dlc_int_01_ba_milo_"_sv }, {}, vNcPropsROOTCAUSE,  true, false, true),
	TeleLocation("Night Club Interior (Traditional)", -1569.2500f, -3017.3900f, -73.2200f,
		{ "ba_int_placement_ba_interior_0_dlc_int_01_ba_milo_"_sv }, {}, vNcPropsROOTCAUSE,  true, false, true),
	TeleLocation("Night Club Basement",             -1509.3100f, -2990.4400f, -79.7400f,
		{ "ba_int_placement_ba_interior_1_dlc_int_02_ba_milo_"_sv }, {}, vNcPropsROOTCAUSE2, true, false, true),
	TeleLocation("Terrorbyte",                      -1421.0150f, -3012.5870f, -80.0000f,
		{ "ba_int_placement_ba_interior_2_dlc_int_03_ba_milo_"_sv }, {}, vNcPropsROOTCAUSE3, true, false, true),
};

void TeleportNightclubsSubmenu::Draw()
{
	DrawTitle();

	for (auto& otherTele : vOtherNightclubRelatedTeleports)
	{
		if (DrawOption(otherTele.name))
		{
			ToTeleLocation241(otherTele);
		}
	}
}

}
REGISTER_SUBMENU(::Menu::TeleportNightclubsSubmenu)
