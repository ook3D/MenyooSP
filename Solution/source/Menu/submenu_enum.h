/*
* Menyoo PC - Grand Theft Auto V single-player trainer mod
* Copyright (C) 2019  MAFINS
*
* Slim remnant of the legacy V1 SUB:: enum. The V1 menu dispatcher is gone;
* the only remaining consumers are:
*   - Submenus/Teleport/Locations.cpp (stores SUB::TELEPORTOPS_* values as
*     void* sentinels in vAllCategories[].nextNamedLocListList)
*   - Submenus/TeleportSubmenus.cpp (maps those values to Menu submenu ids
*     via SubEnumToMenuId)
*
* The first slot must remain a non-zero TELEPORTOPS_* value so the
* reinterpret_cast<>(rawPtr) < MAX_SUBS sentinel check still distinguishes
* "this is a SUB enum index" from "this is a real pointer". Adding new entries
* anywhere is fine; reordering or removing will break the existing
* Locations.cpp data table.
*/
#pragma once

namespace SUB {
	enum SUB : int
	{
		_RESERVED0 = 0,

		TELEPORTOPS_OFFICEGARAGES,
		TELEPORTOPS_OFFICEGARAGES_INLOC,
		TELEPORTOPS_IEVEHICLEWAREHOUSES,
		TELEPORTOPS_IEVEHICLEWAREHOUSES_INLOC,
		TELEPORTOPS_BIKERCLUBHOUSES,
		TELEPORTOPS_BIKERCLUBHOUSES_INLOC,
		TELEPORTOPS_BUSINESSES,
		TELEPORTOPS_BUSINESSES_INLOC,
		TELEPORTOPS_BUNKERS,
		TELEPORTOPS_BUNKERS_INLOC,
		TELEPORTOPS_MOC,
		TELEPORTOPS_MOC_INLOC,
		TELEPORTOPS_HANGARS,
		TELEPORTOPS_HANGARS_INLOC,
		TELEPORTOPS_HANGARS_INOPTION,
		TELEPORTOPS_FACILITIES,
		TELEPORTOPS_FACILITIES_INLOC,
		TELEPORTOPS_FACILITIES_INOPTION,
		TELEPORTOPS_NIGHTCLUBS,
		TELEPORTOPS_ARENAWAR,
		TELEPORTOPS_YACHTS,
		TELEPORTOPS_YACHTS_INGRP,

		MAX_SUBS,
	};
}
