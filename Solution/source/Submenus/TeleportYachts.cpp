#include "TeleportYachts.h"

#include "../Menu/Engine.h"
#include "../Menu/SubmenuRegistry.h"
#include "../Menu/Menu.h"
#include "../Menu/Routine.h"
#include "PlayerRuntime.h"
#include "VehicleRuntime.h"

#include "../macros.h"

#include "../Natives/natives2.h"
#include "../Natives/types.h"     // RGBA
#include "../Scripting/enums.h"   // YachtPropPaintVariation::vNames
#include "../Scripting/Game.h"
#include "../Scripting/GTAblip.h"
#include "../Scripting/GTAentity.h"
#include "../Scripting/GTAped.h"
#include "../Scripting/GTAprop.h"
#include "../Scripting/GTAvehicle.h"
#include "../Scripting/GameplayCamera.h"
#include "../Scripting/Model.h"
#include "../Scripting/PTFX.h"
#include "../Scripting/Scaleform.h"
#include "../Scripting/World.h"
#include "../Memory/GTAmemory.h"
#include "../Util/GTAmath.h"

#include "SettingsRuntime.h"         // sub::g_settingsRGBA (colour-picker target)
#include "Teleport/TeleLocation.h"
#include "Teleport/TeleMethods.h"
#include "Teleport/Locations.h"

#include <Windows.h> // GetTickCount
#include <array>
#include <map>
#include <stdio.h>
#include <string>
#include <vector>

namespace Menu {

namespace YachtsNs = sub::TeleportLocations_catind::Yachts;


void TeleportYachtsSubmenu::Draw()
{
	DrawTitle();

	// Static "other yacht" teleport entries.
	for (std::size_t i = 0; i < YachtsNs::OtherYachtRelatedTeleportCount(); ++i)
	{
		if (DrawOption(YachtsNs::OtherYachtRelatedTeleportName(i)))
		{
			YachtsNs::InvokeOtherYachtRelatedTeleport(i);
		}
	}

	DrawBreak("---Build A Yacht---");

	// Restore previous build-state if a build was abandoned via back-nav.
	YachtsNs::RestoreFromOldYachtInfo();

	const auto& groupNames = YachtsNs::GroupLocationNames();
	for (std::size_t i = 0; i < groupNames.size(); ++i)
	{
		if (DrawOption(groupNames[i]))
		{
			YachtsNs::SetOldYachtInfoFromCurrent();
			YachtsNs::SetCurrentLocationByIndex(i);
			// Default the yacht-type pointer to the first option (matches the
			// legacy `&*vOptionNames.begin()` behaviour).
			YachtsNs::SetCurrentOptionByIndex(0);
			NavigateTo("teleport_yachts_in_grp");
		}
	}
}

void TeleportYachtsInGrpSubmenu::Draw()
{
	if (!YachtsNs::HasCurrentYachtLocation())
	{
		Engine* engine = Engine::Current();
		if (engine) engine->GoBack();
		return;
	}

	Engine* engine = Engine::Current();
	if (engine)
		engine->AddTitle(YachtsNs::CurrentLocationName());

	GTAped ped = g_Ped1;

	{
		int idx = YachtsNs::GetInternalLocationIndex();
		// Legacy used an empty Texter to show the raw int; mirror via DrawNumber.
		if (DrawNumber("Position Index", idx, 1, 1, 3))
		{
			YachtsNs::SetInternalLocationIndex(static_cast<unsigned __int8>(idx));
		}
	}

	{
		const auto& titles = YachtsNs::OptionDisplayTitles();
		const std::size_t cur = YachtsNs::CurrentOptionIndex();
		const std::vector<std::string> single{ cur < titles.size() ? titles[cur] : std::string() };

		int displayIdx = 0;
		const ::Menu::InputResult res = engine
			? engine->AddTextList("Yacht", displayIdx, single)
			: ::Menu::InputResult{};

		// Per-row preview sprite (anchored to the row Y).
		if (engine && IsCurrentRowSelected())
		{
			YachtsNs::DrawYachtBmpPreview(YachtsNs::CurrentOptionId(),
				engine->theme.position.x, engine->optionY, engine->theme.position.y);
		}

		if (res.rightPressed)
		{
			if (cur + 1 < titles.size())
				YachtsNs::SetCurrentOptionByIndex(cur + 1);
		}
		else if (res.leftPressed)
		{
			if (cur > 0)
				YachtsNs::SetCurrentOptionByIndex(cur - 1);
		}
	}

	{
		const auto& paintNames = YachtPropPaintVariation::vNames;
		int idx = YachtsNs::GetYachtPropTextureVariation();
		const ::Menu::InputResult res = engine
			? engine->AddTextList("Yacht Paint", idx, paintNames)
			: ::Menu::InputResult{};
		if (res.rightPressed)
		{
			if (YachtsNs::GetYachtPropTextureVariation() < paintNames.size() - 1)
				YachtsNs::SetYachtPropTextureVariation(YachtsNs::GetYachtPropTextureVariation() + 1);
		}
		else if (res.leftPressed)
		{
			if (YachtsNs::GetYachtPropTextureVariation() > 0)
				YachtsNs::SetYachtPropTextureVariation(YachtsNs::GetYachtPropTextureVariation() - 1);
		}
	}

	{
		const std::vector<std::string> opts{ "Chrome", "Gold" };
		int idx = YachtsNs::GetRailingColour() - 'a';
		const ::Menu::InputResult res = engine
			? engine->AddTextList("Railings", idx, opts)
			: ::Menu::InputResult{};
		if (res.rightPressed)
		{
			if (YachtsNs::GetRailingColour() < 'b')
				YachtsNs::SetRailingColour(YachtsNs::GetRailingColour() + 1);
		}
		else if (res.leftPressed)
		{
			if (YachtsNs::GetRailingColour() > 'a')
				YachtsNs::SetRailingColour(YachtsNs::GetRailingColour() - 1);
		}
	}

	{
		const std::vector<std::string> opts{ "Presidential", "Vivacious" };
		int idx = YachtsNs::GetLightingType() - 1;
		const ::Menu::InputResult res = engine
			? engine->AddTextList("Lighting", idx, opts)
			: ::Menu::InputResult{};
		if (res.rightPressed)
		{
			if (YachtsNs::GetLightingType() < 2)
				YachtsNs::SetLightingType(YachtsNs::GetLightingType() + 1);
		}
		else if (res.leftPressed)
		{
			if (YachtsNs::GetLightingType() > 1)
				YachtsNs::SetLightingType(YachtsNs::GetLightingType() - 1);
		}
	}

	{
		const std::vector<std::string> opts{ "Gold", "Blue", "Pink", "Green" };
		int idx = YachtsNs::GetLightingColour() - 'a';
		const ::Menu::InputResult res = engine
			? engine->AddTextList("Lighting Colour", idx, opts)
			: ::Menu::InputResult{};
		if (res.rightPressed)
		{
			if (YachtsNs::GetLightingColour() < 'd')
				YachtsNs::SetLightingColour(YachtsNs::GetLightingColour() + 1);
		}
		else if (res.leftPressed)
		{
			if (YachtsNs::GetLightingColour() > 'a')
				YachtsNs::SetLightingColour(YachtsNs::GetLightingColour() - 1);
		}
	}

	{
		const std::vector<std::string> opts{
			YachtsNs::GetDoorColour() == '\0' ? std::string("Chrome") : std::string("Gold")
		};
		int displayIdx = 0;
		const ::Menu::InputResult res = engine
			? engine->AddTextList("Doors", displayIdx, opts)
			: ::Menu::InputResult{};
		if (res.rightPressed)
		{
			if (YachtsNs::GetDoorColour() == '\0')
				YachtsNs::SetDoorColour('2');
		}
		else if (res.leftPressed)
		{
			if (YachtsNs::GetDoorColour() == '2')
				YachtsNs::SetDoorColour('\0');
		}
	}

	{
		const auto& flagNames = YachtsNs::FlagSuffixes();
		int idx = YachtsNs::GetFlagIndex();
		const ::Menu::InputResult res = engine
			? engine->AddTextList("Flag", idx, flagNames)
			: ::Menu::InputResult{};
		if (res.rightPressed)
		{
			if (YachtsNs::GetFlagIndex() < flagNames.size() - 1)
				YachtsNs::SetFlagIndex(YachtsNs::GetFlagIndex() + 1);
		}
		else if (res.leftPressed)
		{
			if (YachtsNs::GetFlagIndex() > 0)
				YachtsNs::SetFlagIndex(YachtsNs::GetFlagIndex() - 1);
		}
	}

	{
		RGBA& marker = YachtsNs::MarkerColour();
		const bool pressed = DrawOption("Marker Colour");
		if (engine && IsCurrentRowSelected())
		{
			engine->AddPresetColourOptionsPreview(
				static_cast<unsigned char>(marker.R),
				static_cast<unsigned char>(marker.G),
				static_cast<unsigned char>(marker.B));
		}
		if (pressed)
		{
			sub::g_settingsRGBA = &marker;
			NavigateTo("settings_colours2");
		}
	}

	if (DrawOption("Build Yacht"))
	{
		DO_SCREEN_FADE_OUT(50);
		YachtsNs::UnloadCurrentYachtScaleforms();
		YachtsNs::ResetCurrentYachtProp();
		YachtsNs::TeleportPedToCurrentYacht(ped);
		(ped.IsInVehicle() ? ped.CurrentVehicle() : GTAentity(ped)).FreezePosition(true);

		YachtsNs::DeleteOldYacht();

		WAIT(150);
		YachtsNs::CreateYachtCurrent();
		YachtsNs::TeleportPedToCurrentYacht(ped);
		(ped.IsInVehicle() ? ped.CurrentVehicle() : GTAentity(ped)).FreezePosition(false);
		WATER::SET_DEEP_OCEAN_SCALER(0.0f);
		WATER::REMOVE_EXTRA_CALMING_QUAD(-4);
		if (engine) engine->GoBack();
		DO_SCREEN_FADE_IN(200);
		return;
	}
}

}
REGISTER_SUBMENU(::Menu::TeleportYachtsSubmenu)
REGISTER_SUBMENU(::Menu::TeleportYachtsInGrpSubmenu)

namespace sub::TeleportLocations_catind
{
	namespace Yachts
	{
		const std::vector<TeleLocation> vOtherYachtRelatedTeleports
		{
			TeleLocation("Dignity Party (Vespucci Beach)", -2023.455f, -1038.181f, 8.0629f, IplNames::vYacht_Smboat1, IplNames::vYacht_Heist1, false),
			TeleLocation("Dignity Heist (Vespucci Beach)", -2023.455f, -1038.181f, 8.0629f, IplNames::vYacht_Heist1, IplNames::vYacht_Smboat1, true),
			TeleLocation("Gunrunning Heist (Paleto Bay)", -1368.7260f, 6736.3266f, 7.0000f, IplNames::vYacht_Heist2,{}, true)
		};
		struct CoordinatesWithHeading { Vector3 pos; float h; };
		struct YachtLocation { const std::string name; CoordinatesWithHeading poses[3]; };
		const std::map<UINT8, YachtLocation> vGroupLocations
		{
			{ 1,{ "Lago Zancudo",{ { { -3542.8215f, 1488.2498f, 5.4300f }, -123.0449f },{ { -3148.37912f, 2807.5549f, 5.4300f }, 91.95499f },{ { -3280.5012f, 2140.5071f, 5.4300f }, 86.9550f } } } },
			{ 2,{ "North Chumash",{ { { -2814.4895f, 4072.7398f, 5.4300f }, -108.04495f },{ { -3254.5520f, 3685.6765f, 5.4300f }, 81.9550f },{ { -2368.4412f, 4697.8740f, 5.4300f }, -133.0450f } } } },
			{ 3,{ "Pacific Bluffs",{ { { -3205.3440f, -219.0104f, 5.4300f }, 176.9550f },{ { -3448.2542f, 311.5018f, 5.4300f }, -83.04494f },{ { -2697.8616f, -540.6123f, 5.4300f }, 146.9550f } } } },
			{ 4,{ "Vespucci Beach",{ { { -1995.7343f, -1523.6902f, 5.4300f }, -38.0450f },{ { -2117.5806f, -2543.3460f, 5.4300f }, 36.9550f },{ { -1605.0737f, -1872.4680f, 5.4300f }, -68.0450f } } } },
			{ 5,{ "Los Santos International Airport",{ { { -753.0817f, -3919.0676f, 5.4300f }, 11.9550f },{ { -351.0608f, -3553.3235f, 5.4300f }, -123.0450f },{ { -1460.5361f, -3761.4670f, 5.4300f }, 161.9550f } } } },
			{ 6,{ "Terminal",{ { { 1546.8916f, -3045.6270f, 5.4300f }, -118.0445f },{ { 2490.8855f, -2428.8481f, 5.4300f }, -168.0450f },{ { 2049.7898f, -2821.6240f, 5.4300f }, 31.9550f } } } },
			{ 7,{ "Palomino Highlands",{ { { 3029.0183f, -1495.7024f, 5.4300f }, -108.0450f },{ { 3021.2537f, -723.3903f, 5.4300f }, 81.9550f },{ { 2976.6218f, -1994.7598f, 5.4300f }, -133.0450f } } } },
			{ 8,{ "Tataviam Mountains",{ { { 3404.5095f, 1977.0444f, 5.4300f }, -103.0450f },{ { 3411.1003f, 1193.4446f, 5.4300f }, 31.95502f },{ { 3784.8025f, 2548.5413f, 5.4300f }, 86.9551f } } } },
			{ 9,{ "San Chianski Mountain Range",{ { { 4225.0283f, 3988.0015f, 5.4300f }, 61.9551f },{ { 4250.5811f, 4576.5654f, 5.4300f }, 111.9550f },{ { 4204.3555f, 3373.7002f, 5.4300f }, 81.9550f } } } },
			{ 10,{ "Mount Gordo",{ { { 3751.6814f, 5753.5010f, 5.4300f }, 136.9550f },{ { 3490.1052f, 6305.7852f, 5.4300f }, 156.9550f },{ { 3684.8533f, 5212.2383f, 5.4300f }, -58.0450f } } } },
			{ 11,{ "Procopio Beach",{ { { 581.5955f, 7124.5576f, 5.4300f }, 121.9550f },{ { 2004.4617f, 6907.1572f, 5.4300f }, 6.9550f },{ { 1396.6379f, 6860.2031f, 5.4300f }, 176.9550f } } } },
			{ 12,{ "Paleto Bay",{ { { -1170.6904f, 5980.6816f, 5.4300f }, 91.9550f },{ { -777.4865f, 6566.9072f, 5.4300f }, 26.9549f },{ { -381.7739f, 6946.9600f, 5.4300f }, 71.9550f } } } }
		};
		namespace YachtOffsets
		{
			bool bPlayerJustTeleportedBetweenMarkers = false;
			std::vector<std::pair<Vector3, Vector3>> markerOffsets
			{
				{ { -37.5245f, -2.0054f, 0.2776f },{ -13.6966f, -1.9615f, 0.2808f } },
				{ { -13.6966f, -1.9615f, 0.2808f },{ -37.5245f, -2.0054f, 0.2776f } },
				{ { -0.5604f, -2.0216f, 6.3030f },{ 5.0348f, -1.9846f, 6.3074f } },
				{ { 5.0348f, -1.9846f, 6.3074f },{ -0.5604f, -2.0216f, 6.3030f } },
			};
			Vector3 teleportLocationOffset = { -57.6840f, -3.7550f, -3.2132f };
		}
		struct YachtBmp { const std::string title; const std::string imgName; };
		const std::map<UINT8, YachtBmp> vOptionNames
		{
			{ 1,{ "The Orion", "yacht_model_0_0" } },
			{ 2,{ "The Pisces", "yacht_model_1_0" } },
			{ 3,{ "The Aquarius", "yacht_model_2_0" } }
		};
		const std::vector<std::string> vFlagSuffixes
		{
			{ "Argentina" },
			{ "Australia" },
			{ "Austria" },
			{ "Belgium" },
			{ "Brazil" },
			{ "Canadat_yt" },
			{ "China" },
			{ "Columbia" },
			{ "Croatia" },
			{ "CzechRep" },
			{ "Denmark" },
			{ "England" },
			{ "EU_yt" },
			{ "Finland" },
			{ "France" },
			{ "German_yt" },
			{ "Hungary" },
			{ "Ireland" },
			{ "Israel" },
			{ "Italy" },
			{ "Jamaica" },
			{ "Japan_yt" },
			{ "Lstein" },
			{ "Malta" },
			{ "Mexico_yt" },
			{ "Netherlands" },
			{ "NewZealand" },
			{ "Nigeria" },
			{ "Norway" },
			{ "Palestine" },
			{ "Poland" },
			{ "Portugal" },
			{ "PuertoRico" },
			{ "Russia_yt" },
			{ "Scotland_yt" },
			{ "Script" },
			{ "Slovakia" },
			{ "Slovenia" },
			{ "SouthAfrica" },
			{ "Southkorea" },
			{ "Spain" },
			{ "Sweden" },
			{ "Switzerland" },
			{ "Turkey" },
			{ "UK_yt" },
			{ "US_yt" },
			{ "Wales" },
		};

		struct YachtBuildInfoStructure
		{
			std::pair<const UINT8, YachtLocation> const * location;
			UINT8 internalLocationIndex;
			std::pair<const UINT8, YachtBmp> const * option;
			GTAprop yachtProp;
			UINT8 yachtPropTextureVariation;
			char railingColour;
			UINT8 lightingType;
			char lightingColour;
			char doorColour;
			UINT8 flagIndex;
			GTAblip blip;
			RGBA markerColour;
			std::vector<GTAentity> vSpawnedEntities;
			std::vector<std::pair<Vector3, Vector3>> vMarkerPositions;
			std::array<std::pair<Scaleform, std::string>, 3> vScaleforms;

			void operator = (const YachtBuildInfoStructure& right)
			{
				this->location = right.location;
				this->internalLocationIndex = right.internalLocationIndex;
				this->option = right.option;
				this->yachtProp = right.yachtProp;
				this->yachtPropTextureVariation = right.yachtPropTextureVariation;
				this->railingColour = right.railingColour;
				this->lightingType = right.lightingType;
				this->lightingColour = right.lightingColour;
				this->doorColour = right.doorColour;
				this->blip = right.blip;
				this->markerColour = right.markerColour;
				this->vSpawnedEntities = right.vSpawnedEntities;
				this->vMarkerPositions = right.vMarkerPositions;
				this->vScaleforms = right.vScaleforms;
			}
		};
		YachtBuildInfoStructure currentYachtInfo = { nullptr, 1, nullptr, 0, 0, 'a', 1, 'a', '\0', 0, 0,{ 0, 220, 220, 130 } };
		YachtBuildInfoStructure oldYachtInfoVal;
		YachtBuildInfoStructure* oldYachtInfo = nullptr;

		void DrawYachtBmpPreview(UINT8 yachtId, float menuPosX, float optionY, float menuPosY)
		{
			Vector2 res = { 0.1f, 0.0889f };

			FLOAT x_coord = 0.324f + menuPosX;
			FLOAT y_coord = optionY + 0.044f + menuPosY;

			if (menuPosX > 0.45f) x_coord = menuPosX - 0.003f;

			DRAW_RECT(x_coord, y_coord, res.x + 0.003f, res.y + 0.003f, 0, 0, 0, 212, false);

			auto onit = vOptionNames.find(yachtId);
			if (onit != vOptionNames.end())
			{
				auto& vimg = onit->second;
				std::string imgDict = "dock_dlc_model";
				if (!HAS_STREAMED_TEXTURE_DICT_LOADED(imgDict.c_str()))
					REQUEST_STREAMED_TEXTURE_DICT(imgDict.c_str(), false);
				else
					DRAW_SPRITE(imgDict.c_str(), vimg.imgName.c_str(), x_coord, y_coord, res.x, res.y, 0, 255, 255, 255, 255, false, 0);
			}
			else
			{
				Game::Print::SetupDraw(0, Vector2(0, 0.185f), true, false, false);
				Game::Print::drawstring("No preview available", x_coord, y_coord - 0.0043f);
			}
		}
		void DrawYachtBmpPreview(UINT8 yachtId)
		{
			DrawYachtBmpPreview(yachtId, menuPos.x, OptionY, menuPos.y);
		}

		Vector3 GetYachtVectorWorldPosition(YachtBuildInfoStructure& yachtInfo, const Vector3& vec)
		{
			if (yachtInfo.location != nullptr)
			{
				auto& posh = yachtInfo.location->second.poses[yachtInfo.internalLocationIndex - 1];
				float vectorLength = vec.Length();
				float yaw = posh.h;
				Vector3 result = Vector3::DirectionToRotation(vec);
				result.z = yaw - 90.0f;
				result = Vector3::RotationToDirection(result);
				result.Normalize();
				return (posh.pos + (result * vectorLength));
			}
			return Vector3();
		}

		struct YachtPropCfg
		{
			bool dynamic = true;
			bool applyTint = false;          // SET_OBJECT_TINT_INDEX to yachts variation
			bool detachAfterAttach = false;  // attach for placement, then detach so world pos sticks
			bool freezeAfterAttach = false;
			bool makeDynamic = false;
		};

		// Create a prop, attach to parent, apply common setup, and record it for cleanup.
		static GTAprop AttachYachtProp(Model model, GTAentity parent,
			const Vector3& offset, const Vector3& rotation,
			YachtBuildInfoStructure& yachtInfo, const YachtPropCfg& cfg = {})
		{
			GTAprop p = World::CreateProp(model, Vector3(), cfg.dynamic, false);
			p.AttachTo(parent, 0, false, offset, rotation);
			p.SetIsCollisionEnabled(true);
			if (cfg.applyTint) SET_OBJECT_TINT_INDEX(p.Handle(), yachtInfo.yachtPropTextureVariation);
			SET_ENTITY_LIGHTS(p.Handle(), 0);
			if (cfg.detachAfterAttach) p.Detach();
			if (cfg.freezeAfterAttach) p.FreezePosition(true);
			if (cfg.makeDynamic) { p.FreezePosition(false); p.SetDynamic(true); }
			p.SetMissionEntity(true);
			yachtInfo.vSpawnedEntities.push_back(p);
			return p;
		}

		void CreateYacht(YachtBuildInfoStructure& yachtInfo)
		{
			if (worldObjects.size() > 2000)//GTA_MAX_ENTITIES - 48)
			{
				Game::Print::PrintBottomCentre("~r~Error:~s~ Too many entities in world.");
				return;
			}
			if (yachtInfo.location == nullptr || yachtInfo.option == nullptr)
				return;

			auto& posh = yachtInfo.location->second.poses[yachtInfo.internalLocationIndex - 1];
			char buffer[64];

			SET_INSTANCE_PRIORITY_MODE(true);
			ON_ENTER_MP();

			sprintf_s(buffer, "%02d", yachtInfo.location->first);
			const std::string groupIdStr = buffer;

			sprintf_s(buffer, "apa_yacht_grp%s_%i", groupIdStr.c_str(), yachtInfo.internalLocationIndex);
			const std::string iplInitials = buffer;

			REQUEST_IPL(iplInitials.c_str());
			REQUEST_IPL((iplInitials + "_int").c_str());
			REQUEST_IPL((iplInitials + "_lod").c_str());
			SET_INSTANCE_PRIORITY_MODE(false);

			WAIT(200);

			for (DWORD timeOut = GetTickCount() + 6500; GetTickCount() < timeOut;)
			{
				if (World::GetClosestPropOfType(posh.pos, 3.0f, 0x4FCAD2E0).Exists())
				{
					break; // apa_mp_apa_yacht
				}
			}

			const UINT8 optionNumber = yachtInfo.option->first;

			std::vector<Entity> propYachtArr;
			GTAmemory::GetPropHandles(propYachtArr, posh.pos, 3.0f, { 0x4FCAD2E0 }); // apa_mp_apa_yacht
			yachtInfo.yachtProp = World::GetClosestPropOfType(posh.pos, 3.0f, 0x4FCAD2E0);
			auto& propYacht = yachtInfo.yachtProp;
			if (!propYacht.Exists() && !propYachtArr.empty() && DOES_ENTITY_EXIST(propYachtArr.front()))
			{
				propYacht = propYachtArr.front();
			}
			if (!propYacht.Exists())
			{
				yachtInfo.vMarkerPositions.clear();
				Game::Print::PrintBottomLeft("~r~Error:~s~ Unable to build yacht. Try again.");
				return;
			}

			SET_OBJECT_TINT_INDEX(propYacht.Handle(), yachtInfo.yachtPropTextureVariation);
			auto& propYachtWin = propYacht; // kept as alias; original code selected a different handle here

			const Vector3 baseAttachOffset(0.0032f, 0.0028f, 14.5700f);

			// Railings
			sprintf_s(buffer, "apa_mp_apa_yacht_o%u_rail_%c", optionNumber, yachtInfo.railingColour);
			AttachYachtProp(std::string(buffer), propYachtWin, baseAttachOffset, Vector3(), yachtInfo, { .dynamic = false });

			// Collision / visual shells for the selected yacht option
			std::vector<std::string> optionColModels;
			switch (optionNumber)
			{
			case 1:
				optionColModels = { "apa_mp_apa_yacht_option1", "apa_mp_apa_yacht_option1_cola" };
				break;
			case 2:
				optionColModels = { "apa_mp_apa_yacht_option2", "apa_mp_apa_yacht_option2_cola", "apa_mp_apa_yacht_option2_colb" };
				break;
			case 3:
				optionColModels = { "apa_mp_apa_yacht_option3", "apa_mp_apa_yacht_option3_cola", "apa_mp_apa_yacht_option3_colb", "apa_mp_apa_yacht_option3_colc", "apa_mp_apa_yacht_option3_cold", "apa_mp_apa_yacht_option3_cole" };
				break;
			}
			for (const std::string& modelName : optionColModels)
			{
				AttachYachtProp(modelName, propYachtWin, baseAttachOffset, Vector3(), yachtInfo, { .dynamic = false, .applyTint = true });
			}

			// Lighting
			sprintf_s(buffer, "apa_mp_apa_y%u_l%u%c", optionNumber, yachtInfo.lightingType, yachtInfo.lightingColour);
			AttachYachtProp(std::string(buffer), propYachtWin, baseAttachOffset, Vector3(), yachtInfo, { .dynamic = false });

			// Doors
			sprintf_s(buffer, "apa_mp_apa_yacht_door%c", yachtInfo.doorColour);
			const std::string doorModel = buffer;
			const std::vector<std::pair<Vector3, Vector3>> doorOffsets = {
				{ { 0.01894f,  -3.3871f,  6.6600f }, { 0.0f, 0.0f,  90.2950f } },
				{ { 0.0046f,   -0.6018f,  6.6600f }, { 0.0f, 0.0f, -89.7050f } },
				{ { -36.8202f, -1.2778f,  0.6500f }, { 0.0f, 0.0f, -89.9550f } }
			};
			for (const auto& d : doorOffsets)
			{
				AttachYachtProp(doorModel, propYachtWin, d.first, d.second, yachtInfo, { .applyTint = true });
			}

			// Flag
			AttachYachtProp("apa_prop_flag_" + vFlagSuffixes[yachtInfo.flagIndex], propYachtWin, Vector3(-56.6221f, -2.0013f, 1.5937f), Vector3(49.6800f, 0.0f, -89.9500f), yachtInfo);

			// Keypad (prop_ld_keypad_01b)
			AttachYachtProp(0x25286EB9, propYachtWin, Vector3(-36.8196f, -2.8881f, 0.8880f), Vector3(0.0f, 0.0f, -84.7550f), yachtInfo);

			// Radomes (apa_mp_apa_yacht_radar_01a)
			static const std::map<UINT8, std::vector<std::pair<Vector3, float>>> radomeOffsets = {
				{ 1, { { { 0.95549f,  -2.1682f,  9.6040f },  90.0f }, { { 1.2820f,   -1.9895f, 13.4305f }, -180.0f }, { { 5.4844f,   -1.9817f, 18.1568f },  -90.0f } } },
				{ 2, { { { -2.2487f,  -1.9926f, 17.3200f }, -90.0f }, { { 1.6188f,   -1.9927f, 14.0505f }, -180.0f }, { { 7.6349f,   -1.9927f, 10.3491f },   90.0f } } },
				{ 3, { { { 10.8361f,  -1.9899f,  9.8530f },  90.0f }, { { -0.2231f,  -1.9601f, 12.8964f },  180.0f }, { { -15.0487f, -1.9918f,  9.0674f },   90.0f } } }
			};
			for (const auto& r : radomeOffsets.at(optionNumber))
			{
				AttachYachtProp(0x49566db0, propYachtWin, r.first, Vector3(0, 0, r.second), yachtInfo, { .applyTint = true, .detachAfterAttach = true, .freezeAfterAttach = true });
			}

			// Buoys (apa_prop_yacht_float_1a)
			static const std::vector<Vector3> buoyOffsets = {
				{ -53.4131f,   -10.0086f,  -6.0113f },
				{ -1631.3173f, -1819.5632f, -0.6474f },
				{ -58.8841f,     0.2735f,  -6.1121f },
				{ -1619.7332f, -1820.7730f, -0.8027f }
			};
			for (const Vector3& b : buoyOffsets)
			{
				AttachYachtProp(0x51d2a887, propYachtWin, b, Vector3(), yachtInfo, { .detachAfterAttach = true, .makeDynamic = true });
			}

			// Jacuzzi (apa_mp_apa_yacht_jacuzzi_ripple1)
			if (optionNumber >= 2)
			{
				AttachYachtProp(0x98B5E3D4, propYachtWin, Vector3(-50.8033f, -1.9774f, 0.1368f), Vector3(), yachtInfo);
			}

			// Vehicles
			static const std::map<UINT8, std::vector<std::tuple<Model, Vector3, Vector3>>> vehicleOffsets = {
				{ 1, {
					{ VEHICLE_TROPIC2,    { -54.3528f, -13.3907f, -5.1819f }, { 1.6733f,  1.9946f,  -109.0565f } },
					{ VEHICLE_SEASHARK3,  { -61.5043f,  -8.9306f, -5.5869f }, { 4.1636f, -0.8436f,   116.3082f } }
				} },
				{ 2, {
					{ VEHICLE_SEASHARK3,  { -61.5275f,   4.6035f, -5.3742f }, { 4.6320f, -1.7134f,    63.7883f } },
					{ VEHICLE_SEASHARK3,  { -61.5155f,  -8.8960f, -5.3594f }, { 4.5635f, -1.1910f,  -243.6808f } },
					{ VEHICLE_DINGHY4,    { -54.3522f, -13.3903f, -4.8438f }, { 4.8526f, -1.0505f,  -109.0528f } },
					{ VEHICLE_SPEEDER2,   { -53.7848f,   9.1620f, -4.6511f }, { 3.3195f,  0.8531f,   -69.9578f } },
					{ VEHICLE_SWIFT2,     { -30.8168f,  -1.8687f,  6.5134f }, { -0.2890f, 0.0000f,   -90.0000f } }
				} },
				{ 3, {
					{ VEHICLE_SEASHARK3,    { -61.5189f,   2.3503f, -5.8703f }, {  5.6612f,  0.2225f,   64.4417f } },
					{ VEHICLE_SEASHARK3,    { -61.5050f,  -8.9017f, -5.6858f }, {  7.4903f, -0.0753f,  116.2442f } },
					{ VEHICLE_SEASHARK3,    { -61.52280f,  4.5982f, -5.8566f }, {  5.9844f, -0.3815f,   64.5492f } },
					{ VEHICLE_SEASHARK3,    { -61.5087f,  -6.6536f, -5.7382f }, {  7.2774f,  0.9445f,  114.6004f } },
					{ VEHICLE_DINGHY4,      { -54.3447f, -13.3947f, -5.4129f }, {  3.6041f, -2.1484f, -108.2869f } },
					{ VEHICLE_TORO,         { -54.3726f,   9.1093f, -5.5979f }, { -1.8536f,  0.7891f,  -69.8533f } },
					{ VEHICLE_SUPERVOLITO2, { -30.8329f,  -1.8728f,  7.2883f }, {  0.0409f,  0.0000f,  -90.0000f } }
				} }
			};
			for (const auto& v : vehicleOffsets.at(optionNumber))
			{
				const Model& vehModel = std::get<0>(v);
				GTAvehicle veh = World::CreateVehicle(vehModel, Vector3(), 0.0f, false);
				veh.AttachTo(propYachtWin, 0, false, std::get<1>(v), std::get<2>(v));
				veh.Detach();
				veh.FreezePosition(false);
				veh.SetIsCollisionEnabled(true);
				SET_ENTITY_LIGHTS(veh.Handle(), 0);
				if (vehModel.IsBoat()) SET_BOAT_ANCHOR(veh.Handle(), true);
				yachtInfo.vSpawnedEntities.push_back(veh);
			}

			// Peds
			static const std::vector<std::tuple<Model, Vector3, Vector3, PTFX::sFxData>> pedSetup = {
				{ PedHash::BoatStaff01Male,   { 14.2079f, -2.1206f, 7.3519f }, { 0.0f, 0.0f, -88.2933f }, { std::string(), std::string() } },
				{ PedHash::BoatStaff01Female, { 23.3344f, -1.6929f, 3.5506f }, { 0.0f, 0.0f,  88.9650f }, { "anim@mini@yacht@bar@drink@idle_a", "idle_a_bartender" } }
			};
			for (const auto& p : pedSetup)
			{
				GTAped ped = World::CreatePed(std::get<0>(p), Vector3(), 0.0f, false);
				ped.AttachTo(propYachtWin, 0, false, std::get<1>(p), std::get<2>(p));
				ped.Detach();
				ped.FreezePosition(false);
				ped.SetIsCollisionEnabled(true);
				SET_ENTITY_LIGHTS(ped.Handle(), 0);
				ped.SetRelationshipGroup("PLAYER");
				ped.SetBlockPermanentEvent(true);
				const auto& animArgs = std::get<3>(p);
				if (animArgs.asset.empty())
				{
					if (!animArgs.effect.empty()) ped.Task().StartScenario(animArgs.effect);
				}
				else if (!animArgs.effect.empty())
				{
					ped.Task().PlayAnimation(animArgs.asset, animArgs.effect);
				}
				yachtInfo.vSpawnedEntities.push_back(ped);
			}

			// Blip
			GTAblip blipYacht = propYachtWin.AddBlip();
			blipYacht.SetIcon(BlipIcon::Yacht);
			blipYacht.SetBlipName("Menyoo Yacht");
			blipYacht.SetColour(BlipColour::Blue);
			blipYacht.SetAlpha(200);
			blipYacht.SetFriendly(true);
			blipYacht.SetScale(0.8f);

			for (const auto& v : YachtOffsets::markerOffsets)
			{
				currentYachtInfo.vMarkerPositions.push_back({propYachtWin.GetOffsetInWorldCoords(v.first), propYachtWin.GetOffsetInWorldCoords(v.second)});
			}
		}
		void DeleteYacht(YachtBuildInfoStructure& yachtInfo)
		{
			for (auto& entity : yachtInfo.vSpawnedEntities)
			{
				if (entity.Handle() != g_myVeh)
				{
					entity.RequestControl();
					entity.Delete(true);
				}
			}
			yachtInfo.vSpawnedEntities.clear();
			yachtInfo.yachtProp = 0;
			yachtInfo.blip.Remove();
		}
		void TeleportPedToYacht(const GTAentity ped, YachtBuildInfoStructure& yachtInfo)
		{
			if (yachtInfo.location != nullptr)
			{
				auto& posh = yachtInfo.location->second.poses[yachtInfo.internalLocationIndex - 1];
				Vector3 yachtPos = posh.pos;
				Vector3 teleOffset = YachtOffsets::teleportLocationOffset;
				if (yachtInfo.yachtProp.Exists())
					TeleportNetPed(ped, yachtInfo.yachtProp.GetOffsetInWorldCoords(teleOffset));
				else
					TeleportNetPed(ped, yachtPos + Vector3::RotationToDirection(Vector3::DirectionToRotation(teleOffset) + Vector3(0, 0, posh.h)) * teleOffset.Length());
			}
		}

		const std::vector<std::string>& GroupLocationNames()
		{
			static const std::vector<std::string> names = [] {
				std::vector<std::string> out;
				for (auto& kv : vGroupLocations) out.push_back(kv.second.name);
				return out;
			}();
			return names;
		}
		const std::vector<std::string>& OptionDisplayTitles()
		{
			static const std::vector<std::string> names = [] {
				std::vector<std::string> out;
				for (auto& kv : vOptionNames) out.push_back(kv.second.title);
				return out;
			}();
			return names;
		}
		const std::vector<std::string>& FlagSuffixes() { return vFlagSuffixes; }

		std::size_t OtherYachtRelatedTeleportCount() { return vOtherYachtRelatedTeleports.size(); }
		const std::string& OtherYachtRelatedTeleportName(std::size_t i) { return vOtherYachtRelatedTeleports[i].name; }
		void InvokeOtherYachtRelatedTeleport(std::size_t i) { TeleMethods::ToTeleLocation241(vOtherYachtRelatedTeleports[i]); }

		void  SetOldYachtInfoFromCurrent()
		{
			if (oldYachtInfo == nullptr)
			{
				oldYachtInfo = &oldYachtInfoVal;
				*oldYachtInfo = currentYachtInfo;
			}
		}
		void  ClearOldYachtInfo() { oldYachtInfo = nullptr; }
		bool  HasCurrentYachtLocation() { return currentYachtInfo.location != nullptr; }

		bool SetCurrentLocationByIndex(std::size_t i)
		{
			std::size_t k = 0;
			for (auto& kv : vGroupLocations)
			{
				if (k == i) { currentYachtInfo.location = &kv; return true; }
				++k;
			}
			return false;
		}
		bool SetCurrentOptionByIndex(std::size_t i)
		{
			std::size_t k = 0;
			for (auto& kv : vOptionNames)
			{
				if (k == i) { currentYachtInfo.option = &kv; return true; }
				++k;
			}
			return false;
		}

		void RestoreFromOldYachtInfo()
		{
			if (oldYachtInfo != nullptr)
			{
				if (oldYachtInfo->location != nullptr && oldYachtInfo->option != nullptr)
				{
					currentYachtInfo.location = oldYachtInfo->location;
					currentYachtInfo.internalLocationIndex = oldYachtInfo->internalLocationIndex;
					currentYachtInfo.option = oldYachtInfo->option;
					currentYachtInfo.vScaleforms = oldYachtInfo->vScaleforms;
				}
				oldYachtInfo = nullptr;
			}
		}

		std::size_t CurrentLocationIndex()
		{
			if (currentYachtInfo.location == nullptr) return 0;
			std::size_t k = 0;
			for (auto& kv : vGroupLocations)
			{
				if (&kv == currentYachtInfo.location) return k;
				++k;
			}
			return 0;
		}
		std::size_t CurrentOptionIndex()
		{
			if (currentYachtInfo.option == nullptr) return 0;
			std::size_t k = 0;
			for (auto& kv : vOptionNames)
			{
				if (&kv == currentYachtInfo.option) return k;
				++k;
			}
			return 0;
		}

		static const std::string s_emptyName;
		const std::string& CurrentLocationName()
		{
			return currentYachtInfo.location != nullptr ? currentYachtInfo.location->second.name : s_emptyName;
		}
		UINT8 CurrentOptionId()
		{
			return currentYachtInfo.option != nullptr ? currentYachtInfo.option->first : (UINT8)0;
		}

		UINT8 GetInternalLocationIndex()        { return currentYachtInfo.internalLocationIndex; }
		void  SetInternalLocationIndex(UINT8 i) { currentYachtInfo.internalLocationIndex = i; }
		UINT8 GetYachtPropTextureVariation()    { return currentYachtInfo.yachtPropTextureVariation; }
		void  SetYachtPropTextureVariation(UINT8 v) { currentYachtInfo.yachtPropTextureVariation = v; }
		char  GetRailingColour()                { return currentYachtInfo.railingColour; }
		void  SetRailingColour(char c)          { currentYachtInfo.railingColour = c; }
		UINT8 GetLightingType()                 { return currentYachtInfo.lightingType; }
		void  SetLightingType(UINT8 v)          { currentYachtInfo.lightingType = v; }
		char  GetLightingColour()               { return currentYachtInfo.lightingColour; }
		void  SetLightingColour(char c)         { currentYachtInfo.lightingColour = c; }
		char  GetDoorColour()                   { return currentYachtInfo.doorColour; }
		void  SetDoorColour(char c)             { currentYachtInfo.doorColour = c; }
		UINT8 GetFlagIndex()                    { return currentYachtInfo.flagIndex; }
		void  SetFlagIndex(UINT8 v)             { currentYachtInfo.flagIndex = v; }
		RGBA& MarkerColour()                    { return currentYachtInfo.markerColour; }

		void ResetCurrentYachtProp()            { currentYachtInfo.yachtProp.Handle() = 0; }
		void UnloadCurrentYachtScaleforms()
		{
			for (auto& sclf : currentYachtInfo.vScaleforms) sclf.first.Unload();
		}
		void CreateYachtCurrent()               { CreateYacht(currentYachtInfo); }
		void DeleteOldYacht()
		{
			if (oldYachtInfo != nullptr)
			{
				DeleteYacht(*oldYachtInfo);
				oldYachtInfo = nullptr;
			}
		}
		void TeleportPedToCurrentYacht(GTAentity ped)
		{
			TeleportPedToYacht(ped, currentYachtInfo);
		}

		void Tick()
		{
			auto& yachtInfo = currentYachtInfo;
			if (yachtInfo.location != nullptr && yachtInfo.option != nullptr)
			{
				GTAentity myPed = PLAYER_PED_ID();
				const Vector3& myPos = myPed.GetPosition();
				auto& posh = yachtInfo.location->second.poses[yachtInfo.internalLocationIndex - 1];

				if (myPos.DistanceTo(posh.pos) < 100.0f)
				{
					bool bMyPedIsInVehicle = IS_PED_IN_ANY_VEHICLE(myPed.Handle(), false) != 0;

					auto& colour = yachtInfo.markerColour;
					float scale = 0.75f;
					auto& vPositions = yachtInfo.vMarkerPositions;

					for (auto& p : vPositions)
					{
						World::DrawMarker(MarkerType::VerticalCylinder, p.first + Vector3::WorldDown() * 0.8301f, Vector3(), Vector3(), Vector3::One() * scale, colour);

						if (!YachtOffsets::bPlayerJustTeleportedBetweenMarkers)
						{
							if (!bMyPedIsInVehicle)
							{
								if (myPos.DistanceTo(p.first) < scale)
								{
									YachtOffsets::bPlayerJustTeleportedBetweenMarkers = true;
									DO_SCREEN_FADE_OUT(5);
									myPed.RequestControl();
									WAIT(20);
									myPed.SetPosition(p.second);
									TaskSequence sq;
									const Vector3& outsideMarkerPos = myPed.GetOffsetInWorldCoords(0, scale + 1.0f, 0);
									TASK_GO_STRAIGHT_TO_COORD(0, outsideMarkerPos.x, outsideMarkerPos.y, outsideMarkerPos.z, 1.5f, 2000, Vector3::DirectionToRotation(Vector3::Normalize(outsideMarkerPos - p.second)).z, 0.0f);
									sq.Close();
									sq.MakePedPerform(myPed);
									sq.Clear();
									GameplayCamera::SetRelativeHeading(0.0f);
									GameplayCamera::SetRelativePitch(0.0f);
									WAIT(100);
									DO_SCREEN_FADE_IN(300);
									return;
								}
							}
						}
						else
						{
							YachtOffsets::bPlayerJustTeleportedBetweenMarkers = false;
							for (auto& pp : yachtInfo.vMarkerPositions)
							{
								if (myPos.DistanceTo(pp.first) < scale + 0.36f)
								{
									YachtOffsets::bPlayerJustTeleportedBetweenMarkers = true;
									return;
								}
							}

						}
					}
				}
			}
		}

	}

}
