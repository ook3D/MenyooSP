#include "PedComponent.h"

#include "../Menu/Engine.h"
#include "../Menu/SubmenuRegistry.h"
#include "../Menu/Menu.h"
#include "../Menu/Routine.h"
#include "PlayerRuntime.h"
#include "VehicleModShopRuntime.h"
#include "../Menu/FolderPreviewBmps.h"
#include "Misc.h"              // dict2, dict3

#include "../Memory/GTAmemory.h"

#include "../Natives/natives2.h"
#include "../Scripting/enums.h"
#include "../Scripting/Game.h"
#include "../Scripting/World.h"
#include "../Scripting/Model.h"
#include "../Scripting/Camera.h"
#include "../Scripting/GameplayCamera.h"
#include "../Scripting/GTAped.h"
#include "../Scripting/GTAentity.h"
#include "../Util/ExePath.h"
#include "../Util/StringManip.h"
#include "../Util/FileLogger.h"
#include "../Util/keyboard.h"

#include "PedComponentRuntime.h"
#include "PedModelRuntime.h"

#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include <pugixml\src\pugixml.hpp>
#include <dirent\include\dirent.h>

#include <algorithm>
#include <array>
#include <cctype>
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


int cycleInt(int current, bool increment, int minVal, int maxVal)
{
	if (increment) return (current < maxVal) ? current + 1 : minVal;
	return (current > minVal) ? current - 1 : maxVal;
}

float cycleFloat(float current, bool increment, float minVal, float maxVal, float step)
{
	if (increment) return (current < maxVal) ? current + step : current;
	return (current > minVal) ? current - step : current;
}

bool RowIsActive()
{
	Engine* engine = Engine::Current();
	if (!engine) return false;
	return engine->ActiveSelection() == engine->printingOption;
}

void ClearPreviewTattoo()
{
	using namespace sub::PedDecals;
	if (g_previewTattoo && DOES_ENTITY_EXIST(g_Ped1))
	{
		g_previewTattoo->Remove(g_Ped1);
		g_previewTattoo = nullptr;
	}
}

void DrawPedVariationInfo(const std::string& info)
{
	FLOAT x_coord = 0.066f + menuPos.x;
	FLOAT y_coord = OptionY + menuPos.y + 0.035f;

	Game::Print::SetupDraw(font_selection, Vector2(0.0f, (font_options == 0 ? 0.33f : 0.4f)), false, false, false, selectedtext);
	Game::Print::drawstring(info, x_coord, y_coord);
}

} // namespace

void PedComponentsSubmenu::Draw()
{
	DrawTitle();

	dict2.clear();
	dict3.clear();

	GTAped thisPed = g_Ped1;
	const Model& thisPedModel = thisPed.Model();

	if (sub::g_cam_componentChanger.Exists())
	{
		sub::g_cam_componentChanger.AttachTo(thisPed, Vector3(0.0f, 2.6f + thisPed.Dim1().y, 0.5f));
		sub::g_cam_componentChanger.PointAt(thisPed);
	}

	// Front view (external state toggle: camera exists -> on).
	if (DrawToggleExternal("Front View", sub::g_cam_componentChanger.Exists()))
	{
		if (sub::g_cam_componentChanger.Exists())
		{
			sub::g_cam_componentChanger.SetActive(false);
			sub::g_cam_componentChanger.Destroy();
			World::SetRenderingCamera(0);
		}
		else
		{
			Camera gmCam = CREATE_CAM("DEFAULT_SCRIPTED_CAMERA", 1);
			sub::g_cam_componentChanger = CREATE_CAM("DEFAULT_SCRIPTED_CAMERA", 1);

			sub::g_cam_componentChanger.SetFieldOfView(40.0f);
			sub::g_cam_componentChanger.AttachTo(thisPed, Vector3(0.0f, 1.5f + thisPed.Dim1().y, 0.5f));
			sub::g_cam_componentChanger.PointAt(thisPed);

			gmCam.SetPosition(World::GetRenderingCamera().Handle() == 0
				? GameplayCamera::GetPosition() : World::GetRenderingCamera().GetPosition());
			gmCam.SetRotation(World::GetRenderingCamera().Handle() == 0
				? GameplayCamera::GetRotation() : World::GetRenderingCamera().GetRotation());

			gmCam.InterpTo(sub::g_cam_componentChanger, 1000, true, true);
			while (gmCam.IsInterpolating()) WAIT(0);
			gmCam.Destroy();
			World::SetRenderingCamera(sub::g_cam_componentChanger);
		}
		return;
	}

	if (DrawOption("Outfits"))         NavigateTo("ped_components_outfits");
	if (DrawOption("Decal Overlays"))
	{
		const bool allowed = sub::PedDecals::vAllDecals.find(thisPedModel.hash)
			!= sub::PedDecals::vAllDecals.end();
		if (allowed) NavigateTo("ped_decals_types");
		else Game::Print::PrintBottomCentre("~r~Error:~s~ No decal overlays available for this ped model.");
	}
	if (DrawOption("Damage Overlays")) NavigateTo("ped_damage_categories");
	if (DrawOption("Head Features"))   NavigateTo("ped_head_features");
	if (DrawOption("Accessories"))     NavigateTo("ped_components_props");

	const std::vector<std::string> components{
		"Head", "Beard/Mask", "Hair", "Torso", "Legs",
		"Hands/Back", "Shoes", "Teeth/Scarf/Necklace/Bracelets",
		"Accessory/Tops", "Task/Armour", "Emblem", "Tops2 (Outer)"
	};

	DrawBreak("---Components---");

	for (int i = 0; i < PV_COMP_MAX; ++i)
	{
		if (GET_NUMBER_OF_PED_DRAWABLE_VARIATIONS(g_Ped1, i) > 0)
		{
			if (DrawOption(components[i]))
			{
				g_Ped4 = i;
				NavigateTo("ped_components_set");
			}
		}
	}

	if (DrawOption("Random Components"))
	{
		thisPed.RequestControlOnce();
		SET_PED_RANDOM_COMPONENT_VARIATION(thisPed.GetHandle(), 0);
		return;
	}

	if (DrawOption("Default Components"))
	{
		thisPed.RequestControlOnce();
		SET_PED_DEFAULT_COMPONENT_VARIATION(thisPed.GetHandle());
		return;
	}

	switch (thisPedModel.hash)
	{
	case PedHash::FreemodeMale01:
	case PedHash::FreemodeFemale01:
	case PedHash::Michael:
		DrawBreak("---Premade Outfits---"); break;
	}

	if (thisPedModel.hash == PedHash::FreemodeMale01)
	{
		if (DrawOption("Police (Freemode Male)"))
		{
			SET_PED_PROP_INDEX(g_Ped1, 0, 47, 0, NETWORK_IS_GAME_IN_PROGRESS(), 0);
			SET_PED_PROP_INDEX(g_Ped1, 1, 10, 0, NETWORK_IS_GAME_IN_PROGRESS(), 0);
			SET_PED_PROP_INDEX(g_Ped1, 2, 3, 0, NETWORK_IS_GAME_IN_PROGRESS(), 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 0, 0, 0, 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 1, 0, 0, 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 3, 0, 0, 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 4, 35, 0, 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 5, 0, 0, 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 6, 25, 0, 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 7, 0, 0, 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 8, 58, 0, 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 9, 0, 0, 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 10, 0, 0, 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 11, 55, 0, 0);
			return;
		}
	}
	if (thisPedModel.hash == PedHash::FreemodeFemale01)
	{
		if (DrawOption("Police (Freemode Female)"))
		{
			SET_PED_PROP_INDEX(g_Ped1, 0, 45, 0, NETWORK_IS_GAME_IN_PROGRESS(), 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 3, 100, 0, 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 4, 34, 0, 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 11, 48, 0, 0);
			return;
		}
	}
	if (thisPedModel.hash == PedHash::FreemodeMale01)
	{
		if (DrawOption("Robber (Freemode Male)"))
		{
			SET_PED_PROP_INDEX(g_Ped1, 0, 48, 1, NETWORK_IS_GAME_IN_PROGRESS(), 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 3, 29, 1, 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 4, 34, 0, 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 5, 45, 0, 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 6, 24, 0, 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 7, 40, 0, 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 8, 25, 1, 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 11, 0, 16, 0);
			return;
		}
		if (DrawOption("Garbage Man (Freemode Male)"))
		{
			SET_PED_PROP_INDEX(g_Ped1, 1, 4, 0, NETWORK_IS_GAME_IN_PROGRESS(), 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 0, 0, 0, 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 1, 0, 0, 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 3, 64, 0, 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 4, 36, 0, 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 5, 0, 0, 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 6, 23, 0, 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 7, 0, 0, 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 8, 59, 0, 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 9, 0, 0, 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 10, 0, 0, 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 11, 57, 0, 0);
			return;
		}
	}
	if (thisPedModel.hash == PedHash::Michael)
	{
		if (DrawOption("Police (Michael)"))
		{
			SET_PED_PROP_INDEX(g_Ped1, 0, 10, 0, NETWORK_IS_GAME_IN_PROGRESS(), 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 3, 6, 0, 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 4, 6, 0, 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 6, 6, 0, 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 8, 8, 0, 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 11, 0, 0, 0);
			return;
		}
		if (DrawOption("Firefighter (Michael)"))
		{
			SET_PED_PROP_INDEX(g_Ped1, 0, 0, 0, NETWORK_IS_GAME_IN_PROGRESS(), 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 3, 1, 0, 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 4, 1, 0, 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 5, 1, 0, 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 6, 1, 0, 0);
			SET_PED_COMPONENT_VARIATION(g_Ped1, 8, 1, 0, 0);
			return;
		}
	}
}

void PedComponentsSetSubmenu::Draw()
{
	DrawTitle();

	Engine* engine = Engine::Current();

	int drawableCurrent = GET_PED_DRAWABLE_VARIATION(g_Ped1, g_Ped4);
	int textureCurrent = GET_PED_TEXTURE_VARIATION(g_Ped1, g_Ped4);
	int paletteCurrent = GET_PED_PALETTE_VARIATION(g_Ped1, g_Ped4);

	const int drawableOld = drawableCurrent;
	const int textureOld = textureCurrent;
	const int paletteOld = paletteCurrent;

	const int maxDrawable = GET_NUMBER_OF_PED_DRAWABLE_VARIATIONS(g_Ped1, g_Ped4) - 1;
	const int maxTexture = GET_NUMBER_OF_PED_TEXTURE_VARIATIONS(g_Ped1, g_Ped4, drawableCurrent);

	bool drawablePlus = false;
	bool drawableMinus = false;
	bool texturePlus = false;
	bool textureMinus = false;
	bool drawableAccepted = false;

	if (GET_NUMBER_OF_PED_DRAWABLE_VARIATIONS(g_Ped1, g_Ped4) > 0 && engine)
	{
		const ::Menu::InputResult res = engine->AddNumber("Type", drawableCurrent, 0);
		drawablePlus = res.rightPressed;
		drawableMinus = res.leftPressed;
		drawableAccepted = res.accepted;
	}
	if (GET_NUMBER_OF_PED_TEXTURE_VARIATIONS(g_Ped1, g_Ped4, drawableCurrent) && engine)
	{
		const ::Menu::InputResult res = engine->AddNumber("Texture", textureCurrent, 0);
		texturePlus = res.rightPressed;
		textureMinus = res.leftPressed;
	}

	if (!g_isEnhanced)
	{
		DrawPedVariationInfo(GTAmemory::GetPedDrawableCollectionString(g_Ped1, g_Ped4));
	}

	// Accept on the "Type" row opens an InputBox for direct drawable index entry.
	if (drawableAccepted)
	{
		std::string inputStr = Game::InputBox("", 5U, "", std::to_string(drawableOld));
		if (inputStr.length() > 0)
		{
			try
			{
				int requested = std::stoi(inputStr);
				if (requested > maxDrawable)
				{
					Game::Print::PrintErrorInvalidInput(inputStr);
				}
				else
				{
					drawableCurrent = requested;
				}
			}
			catch (...)
			{
				Game::Print::PrintErrorInvalidInput(inputStr);
			}
		}
	}

	if (drawablePlus || drawableMinus)
	{
		drawableCurrent = cycleInt(drawableCurrent, drawablePlus, 0, maxDrawable);
		textureCurrent = 0;
	}
	else if (texturePlus || textureMinus)
	{
		textureCurrent = cycleInt(textureCurrent, texturePlus, 0, maxTexture);
	}

	if (drawableOld != drawableCurrent
		|| textureOld != textureCurrent
		|| paletteOld != paletteCurrent)
	{
		if (g_Ped4 == PV_COMP_ACCS && !GET_PED_CONFIG_FLAG(g_Ped1, ePedConfigFlags::DisableTakeOffScubaGear, true))
		{
			SET_PED_CONFIG_FLAG(g_Ped1, ePedConfigFlags::DisableTakeOffScubaGear, true);
		}
		SET_PED_COMPONENT_VARIATION(g_Ped1, g_Ped4, drawableCurrent, textureCurrent, paletteCurrent);

		while (!sub::HasPedSpecificDrawable(drawableCurrent))
		{
			if (drawablePlus)
			{
				if (drawableCurrent < GET_NUMBER_OF_PED_DRAWABLE_VARIATIONS(g_Ped1, g_Ped4) - 1)
				{
					++drawableCurrent;
					textureCurrent = 0;
				}
				else
				{
					drawableCurrent = 0;
					textureCurrent = 0;
				}
			}
			else if (drawableMinus)
			{
				if (drawableCurrent > -1)
				{
					--drawableCurrent;
					textureCurrent = 0;
				}
				else
				{
					drawableCurrent = GET_NUMBER_OF_PED_DRAWABLE_VARIATIONS(g_Ped1, g_Ped4) - 1;
					textureCurrent = 0;
				}
			}
			else
			{
				break; // accepted path: no skip required
			}
			SET_PED_COMPONENT_VARIATION(g_Ped1, g_Ped4, drawableCurrent, textureCurrent, paletteCurrent);
		}
	}
}

void PedComponentsPropsSubmenu::Draw()
{
	DrawTitle();

	GTAped thisPed = g_Ped1;

	if (sub::g_cam_componentChanger.Exists())
	{
		sub::g_cam_componentChanger.AttachTo(thisPed, Bone::Head, Vector3(0.0f, 0.6f, 0.0f));
		sub::g_cam_componentChanger.PointAt(thisPed, Bone::Head);
	}

	const std::vector<std::string> propNames{
		"Hats", "Glasses", "Ear Pieces", "Unknown 3", "Unknown 4",
		"Unknown 5", "Watches", "Bangles", "Unknown 8", "Unknown 9"
	};

	for (int i = 0; i < static_cast<int>(propNames.size()); ++i)
	{
		if (GET_NUMBER_OF_PED_PROP_DRAWABLE_VARIATIONS(g_Ped1, i) > 0)
		{
			if (DrawOption(propNames[i]))
			{
				g_Ped4 = i;
				NavigateTo("ped_components_props_set");
			}
		}
	}

	DrawBreak("---Utilities---");

	if (DrawOption("Random Accessories"))
	{
		thisPed.RequestControlOnce();
		SET_PED_RANDOM_PROPS(thisPed.Handle());
		return;
	}

	Engine* engine = Engine::Current();
	if (engine && engine->AddCheckbox("Clear Accessories", true, Checkbox::CROSS, Checkbox::NONE))
	{
		thisPed.RequestControlOnce();
		CLEAR_ALL_PED_PROPS(thisPed.Handle(), 0);
		return;
	}
}

void PedComponentsPropsSetSubmenu::Draw()
{
	DrawTitle();

	GTAentity ped = g_Ped1;
	auto& propId = g_Ped4;

	int propTypeCurrent = GET_PED_PROP_INDEX(g_Ped1, g_Ped4, 0);
	int propTextureCurrent = GET_PED_PROP_TEXTURE_INDEX(g_Ped1, g_Ped4);
	const int propTypeOld = propTypeCurrent;
	const int propTextureOld = propTextureCurrent;

	Engine* engine = Engine::Current();
	bool typePlus = false;
	bool typeMinus = false;
	bool texPlus = false;
	bool texMinus = false;

	if (GET_NUMBER_OF_PED_PROP_DRAWABLE_VARIATIONS(g_Ped1, g_Ped4) > 0 && engine)
	{
		const ::Menu::InputResult res = engine->AddNumber("Type", propTypeCurrent, 0);
		typePlus = res.rightPressed;
		typeMinus = res.leftPressed;
	}
	if (GET_NUMBER_OF_PED_PROP_TEXTURE_VARIATIONS(g_Ped1, g_Ped4, propTypeCurrent) > 0 && engine)
	{
		const ::Menu::InputResult res = engine->AddNumber("Texture", propTextureCurrent, 0);
		texPlus = res.rightPressed;
		texMinus = res.leftPressed;
	}

	if (!g_isEnhanced)
	{
		DrawPedVariationInfo(GTAmemory::GetPedPropCollectionString(g_Ped1, g_Ped4));
	}

	if (typePlus)
	{
		if (propTypeCurrent < GET_NUMBER_OF_PED_PROP_DRAWABLE_VARIATIONS(g_Ped1, g_Ped4) - 1)
		{
			++propTypeCurrent;
			propTextureCurrent = 0;
		}
		else
		{
			propTypeCurrent = -1;
			propTextureCurrent = 0;
		}
	}
	else if (typeMinus)
	{
		if (propTypeCurrent > -1)
		{
			--propTypeCurrent;
			propTextureCurrent = 0;
		}
		else
		{
			propTypeCurrent = GET_NUMBER_OF_PED_PROP_DRAWABLE_VARIATIONS(g_Ped1, g_Ped4) - 1;
			propTextureCurrent = 0;
		}
	}
	else if (texPlus)
	{
		if (propTextureCurrent < GET_NUMBER_OF_PED_PROP_TEXTURE_VARIATIONS(g_Ped1, g_Ped4, propTypeCurrent) - 1)
			++propTextureCurrent;
		else
			propTextureCurrent = 0;
	}
	else if (texMinus)
	{
		if (propTextureCurrent > 0)
			--propTextureCurrent;
		else
			propTextureCurrent = GET_NUMBER_OF_PED_PROP_TEXTURE_VARIATIONS(g_Ped1, g_Ped4, propTypeCurrent) - 1;
	}

	if (ped.Exists() && (propTypeCurrent != propTypeOld || propTextureCurrent != propTextureOld))
	{
		Game::Print::PrintBottomCentre("propTypeCurrent: " + std::to_string(propTypeCurrent));
		if (propTypeCurrent == -1)
		{
			CLEAR_PED_PROP(ped.Handle(), propId, 0);
		}
		else
		{
			SET_PED_PROP_INDEX(ped.Handle(), propId, propTypeCurrent, propTextureCurrent, NETWORK_IS_GAME_IN_PROGRESS(), 0);

			while (!sub::HasPedSpecificPropType(propTypeCurrent))
			{
				if (typePlus)
				{
					if (propTypeCurrent < GET_NUMBER_OF_PED_PROP_DRAWABLE_VARIATIONS(g_Ped1, g_Ped4) - 1)
					{
						++propTypeCurrent;
						propTextureCurrent = 0;
					}
					else
					{
						propTypeCurrent = -1;
						propTextureCurrent = 0;
					}
				}
				else if (typeMinus)
				{
					if (propTypeCurrent > -1)
					{
						--propTypeCurrent;
						propTextureCurrent = 0;
					}
					else
					{
						propTypeCurrent = GET_NUMBER_OF_PED_PROP_DRAWABLE_VARIATIONS(g_Ped1, g_Ped4) - 1;
						propTextureCurrent = 0;
					}
				}
				else
				{
					break;
				}
				SET_PED_PROP_INDEX(ped.Handle(), propId, propTypeCurrent, propTextureCurrent, NETWORK_IS_GAME_IN_PROGRESS(), 0);
			}
		}
	}
}

void PedDecalsTypesSubmenu::Draw()
{
	DrawTitle();

	GTAped ped = g_Ped1;
	const auto& pedModel = ped.Model();

	const auto& vPed = sub::PedDecals::vAllDecals.find(pedModel.hash);
	if (vPed == sub::PedDecals::vAllDecals.end())
	{
		Engine* engine = Engine::Current();
		if (engine) engine->GoBack();
		return;
	}

	for (auto& type : vPed->second)
	{
		if (DrawOption(type.first))
		{
			sub::PedDecals::selectedType = (std::pair<std::string, std::map<std::string, std::vector<sub::PedDecals::NamedPedDecal>>>*)&type;
			NavigateTo("ped_decals_zones");
		}
	}

	Engine* engine = Engine::Current();
	if (engine && engine->AddCheckbox("CLEAR ALL", true, Checkbox::CROSS, Checkbox::NONE))
	{
		ped.RequestControl(600);
		CLEAR_PED_DECORATIONS(ped.Handle());
		sub::PedDecals::vPedsAndDecals.erase(ped.Handle());
	}
}

void PedDecalsZonesSubmenu::Draw()
{
	if (!sub::PedDecals::selectedType)
		return;

	DrawTitle();

	for (auto& zone : sub::PedDecals::selectedType->second)
	{
		if (DrawOption(zone.first))
		{
			sub::PedDecals::selectedZone = (std::pair<std::string, std::vector<sub::PedDecals::NamedPedDecal>>*)&zone;
			NavigateTo("ped_decals_in_zone");
		}
	}
}

void PedDecalsInZoneSubmenu::OnExit()
{
	ClearPreviewTattoo();
}

void PedDecalsInZoneSubmenu::Draw()
{
	if (!sub::PedDecals::selectedZone)
		return;

	DrawTitle();

	GTAentity ped = g_Ped1;
	Engine* engine = Engine::Current();

	for (const auto& decal : sub::PedDecals::selectedZone->second)
	{
		const bool isHovered = RowIsActive();
		const bool isOnPed = decal.IsOnPed(ped);

		// translates to a checkbox row that flips: apply on press if not on, remove on press if on.
		const bool pressed = engine
			? engine->AddCheckbox(decal.caption, isOnPed, Checkbox::TATTOOTHING, Checkbox::NONE)
			: false;

		if (sub::PedDecals::g_tattooPreviewMode && isHovered)
		{
			if (sub::PedDecals::g_previewTattoo != &decal)
			{
				ClearPreviewTattoo();
				if (!isOnPed)
				{
					decal.Apply(ped);
					sub::PedDecals::g_previewTattoo = &decal;
				}
			}
		}

		if (pressed)
		{
			if (isOnPed)
			{
				if (sub::PedDecals::g_previewTattoo == &decal)
				{
					// Promote the preview to a permanent application.
					ClearPreviewTattoo();
					decal.Apply(ped);
				}
				else
				{
					decal.Remove(ped);
				}
			}
			else
			{
				decal.Apply(ped);
			}
		}
	}

	// Preview-mode toggle hotkey (B).
	if (engine)
	{
		engine->AddInstructionalButton(static_cast<int>(VirtualKey::B),
			sub::PedDecals::g_tattooPreviewMode ? "Preview: ON " : "Preview: OFF ", /*isKey=*/true);
		if (IsKeyJustUp(VirtualKey::B))
		{
			sub::PedDecals::g_tattooPreviewMode = !sub::PedDecals::g_tattooPreviewMode;
			if (!sub::PedDecals::g_tattooPreviewMode)
				ClearPreviewTattoo();
		}
	}
}

void PedDamageCategoriesSubmenu::Draw()
{
	DrawTitle();

	if (DrawOption("Damage Packs"))
		NavigateTo("ped_damage_packs");

	DrawBreak("---Wash Up---");

	Engine* engine = Engine::Current();
	if (engine && engine->AddCheckbox("Clear Blood Damage", true, Checkbox::CROSS, Checkbox::NONE))
		sub::PedDamageTextures::ClearAllBloodDamage(g_Ped1);
	if (engine && engine->AddCheckbox("Clear All Visible Damage", true, Checkbox::CROSS, Checkbox::NONE))
		sub::PedDamageTextures::ClearAllVisibleDamage(g_Ped1);
}

void PedDamageBoneSubmenu::Draw()
{
	DrawTitle();

	Engine* engine = Engine::Current();
	int& boneToUse = sub::PedDamageTextures::BoneToUse();

	for (const auto& bn : Bone::vBoneNames)
	{
		const bool isCurrent = (boneToUse == bn.boneid);
		const bool pressed = engine
			? engine->AddCheckbox(bn.name, isCurrent, Checkbox::SKULL_DM, Checkbox::NONE)
			: false;
		if (pressed)
			boneToUse = bn.boneid;
	}
}

void PedDamageBloodSubmenu::Draw()
{
	DrawTitle();

	GTAped thisPed = g_Ped1;
	const int thisBone = sub::PedDamageTextures::BoneToUse();

	for (const auto& bdn : PedBloodDecals::vBloodDecals)
	{
		if (DrawOption(bdn))
		{
			thisPed.ApplyBlood(bdn, thisBone, Vector3());
		}
	}
}

void PedDamageDecalsSubmenu::Draw()
{
	DrawTitle();

	GTAped thisPed = g_Ped1;
	const int thisBone = sub::PedDamageTextures::BoneToUse();

	for (const auto& ddn : PedDamageDecals::vDamageDecals)
	{
		if (DrawOption(ddn))
		{
			thisPed.ApplyDamageDecal(ddn, thisBone, Vector3(), 1.0f, 1.0f, 1, true);
		}
	}
}

void PedDamagePacksSubmenu::Draw()
{
	DrawTitle();

	GTAped thisPed = g_Ped1;
	auto& dmgPacksApplied = sub::PedDamageTextures::vPedsAndDamagePacks[thisPed.Handle()];

	Engine* engine = Engine::Current();

	for (const auto& dpn : PedDamagePacks::vDamagePacks)
	{
		const bool isApplied = std::find(dmgPacksApplied.begin(), dmgPacksApplied.end(), dpn) != dmgPacksApplied.end();
		const bool pressed = engine
			? engine->AddCheckbox(dpn, isApplied, Checkbox::MAKEUPTHING, Checkbox::NONE)
			: false;

		if (pressed)
		{
			if (!isApplied)
			{
				thisPed.ApplyDamagePack(dpn, 1.0f, 1.0f);
				dmgPacksApplied.push_back(dpn);
			}
			else
			{
				thisPed.ResetVisibleDamage();
				for (auto it = dmgPacksApplied.begin(); it != dmgPacksApplied.end();)
				{
					if (it->compare(dpn) == 0)
					{
						it = dmgPacksApplied.erase(it);
						continue;
					}
					thisPed.ApplyDamagePack(*it, 1.0f, 1.0f);
					++it;
				}
			}
		}
	}
}

namespace {

int getMaxShapeAndSkinIds() { return g_unlockMaxIDs ? 255 : 46; }

} // namespace

void PedHeadFeaturesSubmenu::Draw()
{
	GTAped ped = g_Ped1;
	Model pedModel = ped.Model();

	if (sub::g_cam_componentChanger.Exists())
	{
		sub::g_cam_componentChanger.AttachTo(ped, Bone::Head, Vector3(0.0f, 0.645f, 0.0f));
		sub::g_cam_componentChanger.PointAt(ped, Bone::Head);
	}

	Engine* engine = Engine::Current();
	if (!ped.Exists() || !sub::PedHeadFeatures_catind::DoesPedModelSupportHeadFeatures(pedModel.hash))
	{
		auto pit = sub::PedHeadFeatures_catind::vPedHeads.find(ped.Handle());
		if (pit != sub::PedHeadFeatures_catind::vPedHeads.end())
			sub::PedHeadFeatures_catind::vPedHeads.erase(pit);
		if (engine) engine->GoBack();
		Game::Print::PrintBottomLeft("~r~Error:~s~ Either the ped died or it isn't an MP freemode model.");
		return;
	}

	sub::PedHeadFeatures_catind::pedHead = &sub::PedHeadFeatures_catind::vPedHeads[ped.Handle()];
	auto* pedHead = sub::PedHeadFeatures_catind::pedHead;

	const int maxIds = getMaxShapeAndSkinIds();

	auto headBlend = ped.GetHeadBlendData();
	if (headBlend.shapeFirstID < 0 || headBlend.shapeFirstID > maxIds
		|| headBlend.shapeSecondID < 0 || headBlend.shapeSecondID > maxIds
		|| headBlend.shapeThirdID < 0 || headBlend.shapeThirdID > maxIds
		|| headBlend.skinFirstID < 0 || headBlend.skinFirstID > maxIds
		|| headBlend.skinSecondID < 0 || headBlend.skinSecondID > maxIds
		|| headBlend.skinThirdID < 0 || headBlend.skinThirdID > maxIds)
	{
		headBlend.shapeFirstID = 0;
		headBlend.shapeSecondID = 0;
		headBlend.shapeThirdID = 0;
		headBlend.skinFirstID = 1;
		headBlend.skinSecondID = 1;
		headBlend.skinThirdID = 1;
		headBlend.shapeMix = 0.0f;
		headBlend.skinMix = 0.0f;
		headBlend.thirdMix = 0.0f;
		headBlend.isParent = false;
		ped.SetHeadBlendData(headBlend);
	}

	DrawTitle();

	if (DrawOption("Overlays"))         NavigateTo("ped_head_overlays");
	if (DrawOption("Facial Features"))  NavigateTo("ped_face_features");
	if (DrawOption("Shape & Skin Tone")) NavigateTo("ped_skin_tone");

	DrawBreak("---Hair---");

	const int maxHairColours = GET_NUM_PED_HAIR_TINTS() - 1;
	const int maxEyeColours = 32;

	if (DrawNumber("Hair Colour", pedHead->hairColour, 1, 0, maxHairColours))
	{
		SET_PED_HAIR_TINT(ped.Handle(), pedHead->hairColour, pedHead->hairColourStreaks);
	}
	if (DrawNumber("Hair Streaks Colour", pedHead->hairColourStreaks, 1, 0, maxHairColours))
	{
		SET_PED_HAIR_TINT(ped.Handle(), pedHead->hairColour, pedHead->hairColourStreaks);
	}

	DrawBreak("---Eyes---");
	if (DrawNumber(Game::GetGXTEntry("FACE_APP_EYE", "Eye Colour"), pedHead->eyeColour, 1, 0, maxEyeColours))
	{
		SET_HEAD_BLEND_EYE_COLOR(ped.Handle(), SYSTEM::ROUND(static_cast<float>(pedHead->eyeColour)));
	}
}

void PedHeadOverlaysSubmenu::Draw()
{
	DrawTitle();

	auto& overlayIndex = g_Ped4;
	const auto& captions = sub::PedHeadFeatures_catind::vCaptions_headOverlays;

	for (UINT i = 0; i < captions.size(); ++i)
	{
		if (DrawOption(captions[i].first))
		{
			overlayIndex = static_cast<int>(i);
			NavigateTo("ped_head_overlays_item");
		}
	}
}

void PedHeadOverlaysItemSubmenu::Draw()
{
	using namespace sub::PedHeadFeatures_catind;

	if (!pedHead) return;

	auto& overlayIndex = g_Ped4;
	GTAped ped = g_Ped1;
	Engine* engine = Engine::Current();

	const UINT8 colourType = GetPedHeadOverlayColourType(static_cast<PedHeadOverlay>(overlayIndex));
	const bool coloursAvailable = (colourType != 0);

	auto& currentOverlayData = pedHead->overlayData[overlayIndex];
	int overlayValue = GET_PED_HEAD_OVERLAY(ped.Handle(), overlayIndex);
	const int maxOverlays = GET_PED_HEAD_OVERLAY_NUM(overlayIndex) - 1;
	const int maxColours = 64;

	DrawTitle();
	{
		const auto& names = vCaptions_headOverlays[overlayIndex].second;
		int idx = overlayValue;
		const ::Menu::InputResult res = engine
			? engine->AddTextList("Variation", idx, names)
			: ::Menu::InputResult{};
		if (res.rightPressed)
		{
			if (overlayValue < maxOverlays)
				++overlayValue;
			else
				overlayValue = (overlayValue == 255) ? 0 : 255;
			SET_PED_HEAD_OVERLAY(ped.Handle(), overlayIndex, overlayValue, currentOverlayData.opacity);
			ApplyHeadOverlayTint(ped, overlayIndex, colourType, currentOverlayData.colour, currentOverlayData.colourSecondary);
		}
		else if (res.leftPressed)
		{
			if (overlayValue > 0)
				overlayValue = overlayValue > maxOverlays ? maxOverlays : overlayValue - 1;
			else
				overlayValue = 255;
			SET_PED_HEAD_OVERLAY(ped.Handle(), overlayIndex, overlayValue, currentOverlayData.opacity);
			ApplyHeadOverlayTint(ped, overlayIndex, colourType, currentOverlayData.colour, currentOverlayData.colourSecondary);
		}
	}

	// ---- OPACITY ----
	if (DrawNumber(Game::GetGXTEntry("FACE_OPAC", "Opacity"), currentOverlayData.opacity, 0.01f, 3, 0.0f, 1.0f))
	{
		SET_PED_HEAD_OVERLAY(ped.Handle(), overlayIndex, overlayValue, currentOverlayData.opacity);
	}

	if (!coloursAvailable)
		return;

	// ---- PRIMARY COLOUR (wraps -1..maxColours) ----
	{
		int idx = currentOverlayData.colour;
		const ::Menu::InputResult res = engine
			? engine->AddNumber(Game::GetGXTEntry("CMOD_COL0_0", "Primary Colour"), currentOverlayData.colour, 0)
			: ::Menu::InputResult{};
		if (res.rightPressed)
		{
			if (currentOverlayData.colour < maxColours) ++currentOverlayData.colour;
			else currentOverlayData.colour = -1;
			ApplyHeadOverlayTint(ped, overlayIndex, colourType, currentOverlayData.colour, currentOverlayData.colourSecondary);
		}
		else if (res.leftPressed)
		{
			if (currentOverlayData.colour > -1) --currentOverlayData.colour;
			else currentOverlayData.colour = maxColours;
			ApplyHeadOverlayTint(ped, overlayIndex, colourType, currentOverlayData.colour, currentOverlayData.colourSecondary);
		}
		(void)idx;
	}

	// ---- SECONDARY COLOUR (only shown when primary >= 0) ----
	if (currentOverlayData.colour > -1)
	{
		const ::Menu::InputResult res = engine
			? engine->AddNumber(Game::GetGXTEntry("CMOD_COL0_1", "Secondary Colour"), currentOverlayData.colourSecondary, 0)
			: ::Menu::InputResult{};
		if (res.rightPressed)
		{
			if (currentOverlayData.colourSecondary < maxColours) ++currentOverlayData.colourSecondary;
			ApplyHeadOverlayTint(ped, overlayIndex, colourType, currentOverlayData.colour, currentOverlayData.colourSecondary);
		}
		else if (res.leftPressed)
		{
			if (currentOverlayData.colourSecondary > -1) --currentOverlayData.colourSecondary;
			ApplyHeadOverlayTint(ped, overlayIndex, colourType, currentOverlayData.colour, currentOverlayData.colourSecondary);
		}
	}
}

void PedFaceFeaturesSubmenu::Draw()
{
	using namespace sub::PedHeadFeatures_catind;
	if (!pedHead) return;

	DrawTitle();

	GTAped ped = g_Ped1;

	for (int i = 0; i < static_cast<int>(vCaptions_facialFeatures.size()); ++i)
	{
		auto& featureValue = pedHead->facialFeatureData[i];
		if (DrawNumber(vCaptions_facialFeatures[i], featureValue, 0.05f, 2, -1.0f, 1.0f))
		{
			SET_PED_MICRO_MORPH(ped.Handle(), i, featureValue);
		}
	}
}

void PedSkinToneSubmenu::Draw()
{
	using namespace sub::PedHeadFeatures_catind;

	GTAped ped = g_Ped1;
	PedHeadBlendData blendData;
	GET_PED_HEAD_BLEND_DATA(ped.Handle(), (Any*)&blendData);
	std::vector<std::string> idNames; // empty — legacy texter just shows the index as number

	const float maxMix = 1.0f;
	const float minMix = 0.0f;
	const float mixStep = 0.01f;

	DrawTitle();
	DrawToggle("Unlock ID Limits", g_unlockMaxIDs);

	const int maxIds = getMaxShapeAndSkinIds();

	Engine* engine = Engine::Current();

	auto addBlendIdTexter = [&](const char* label, int& idValue, bool isShape)
	{
		int idx = idValue;
		const ::Menu::InputResult res = engine
			? engine->AddTextList(label, idx, idNames)
			: ::Menu::InputResult{};
		if (res.rightPressed || res.leftPressed)
		{
			idValue = cycleInt(idValue, res.rightPressed, 0, maxIds);
			UpdatePedHeadBlendData(ped, blendData, isShape);
		}
	};

	addBlendIdTexter("Shape Inherited From Father",   blendData.shapeFirstID,  true);
	addBlendIdTexter("Shape Inherited From Mother",   blendData.shapeSecondID, true);
	addBlendIdTexter("Shape Inherited From Ancestor", blendData.shapeThirdID,  true);

	addBlendIdTexter("Tone Inherited From Father",    blendData.skinFirstID,  true);
	addBlendIdTexter("Tone Inherited From Mother",    blendData.skinSecondID, true);
	addBlendIdTexter("Tone Inherited From Ancestor",  blendData.skinThirdID,  true);

	DrawBreak("---Adjustment---");

	auto addMixSlider = [&](const char* label, float& mixValue)
	{
		bool plus = false;
		bool minus = false;
		const ::Menu::InputResult res = engine
			? engine->AddNumber(label, mixValue, 2)
			: ::Menu::InputResult{};
		plus = res.rightPressed;
		minus = res.leftPressed;
		if (plus || minus)
		{
			mixValue = cycleFloat(mixValue, plus, minMix, maxMix, mixStep);
			UpdatePedHeadBlendData(ped, blendData, false);
		}
	};

	addMixSlider("Shape", blendData.shapeMix);
	addMixSlider("Tone",  blendData.skinMix);
	addMixSlider("Ancestor (Shape & Tone)", blendData.thirdMix);
}

void PedOutfitsSubmenu::Draw()
{
	using sub::ComponentChangerOutfit::persistentAttachmentsTexterIndex;

	std::string& name = dict;
	std::string& searchStr = dict2;
	std::string& dir = dict3;

	DrawTitle();

	Engine* engine = Engine::Current();

	// AddAttachmentsToSpoonerDB Texter (non-wrapping 0..2).
	{
		const std::vector<std::string> opts{ "FileDecides", "ForceOff", "ForceOn" };
		int idx = persistentAttachmentsTexterIndex;
		const ::Menu::InputResult res = engine
			? engine->AddTextList("AddAttachmentsToSpoonerDB", idx, opts)
			: ::Menu::InputResult{};
		if (res.rightPressed && persistentAttachmentsTexterIndex < 2) ++persistentAttachmentsTexterIndex;
		else if (res.leftPressed && persistentAttachmentsTexterIndex > 0) --persistentAttachmentsTexterIndex;
	}

	bool savePressed = DrawOption("Save Outfit To File");
	bool createFolderPressed = DrawOption("Create New Folder");

	if (dir.empty())
		dir = GetPathffA(Pathff::Outfit, false);

	std::vector<std::string> fileNames;
	if (DIR* dirPoint = opendir(dir.c_str()))
	{
		dirent* entry = readdir(dirPoint);
		while (entry)
		{
			fileNames.push_back(entry->d_name);
			entry = readdir(dirPoint);
		}
		closedir(dirPoint);
	}

	DrawBreak("---Found Files---");

	if (DrawOption(".."))
	{
		dir = dir.substr(0, dir.rfind("\\"));
		if (engine) engine->currentOption = 5;
	}

	if (!fileNames.empty())
	{
		if (DrawOption(searchStr.empty() ? "SEARCH" : searchStr))
		{
			searchStr = Game::InputBox(searchStr, 126U, "SEARCH", toLowerCopy(searchStr));
			toUpperInPlace(searchStr);
		}

		for (auto& fileName : fileNames)
		{
			if (fileName.front() == '.' || fileName.front() == ',') continue;
			if (!searchStr.empty())
			{
				if (toUpperCopy(fileName).find(searchStr) == std::string::npos)
					continue;
			}

			const bool isFolder = PathIsDirectoryA((dir + "\\" + fileName).c_str()) != 0;
			const bool isXml = fileName.length() > 4 && fileName.rfind(".xml") == fileName.length() - 4;
			Checkbox icon = Checkbox::NONE;
			if (isFolder) icon = Checkbox::ARROWRIGHT;
			else if (isXml) icon = Checkbox::TICK2;

			if (isFolder)
			{
				const bool pressed = engine
					? engine->AddCheckbox(fileName + " >>>", true, icon, Checkbox::NONE)
					: false;
				if (pressed)
				{
					dir = dir + "\\" + fileName;
					if (engine) engine->currentOption = 5;
				}
				else if (RowIsActive() && sub::FolderPreviewBmps_catind::bFolderBmpsEnabled)
				{
					sub::FolderPreviewBmps_catind::DrawBmp(dir + "\\" + fileName);
				}
			}
			else if (isXml)
			{
				const bool pressed = engine
					? engine->AddCheckbox(fileName, true, icon, Checkbox::NONE)
					: false;
				if (pressed)
				{
					name = fileName.substr(0, fileName.rfind('.'));
					NavigateTo("ped_components_outfits_item");
					return;
				}
			}
		}
	}

	if (savePressed)
	{
		std::string inputStr = Game::InputBox("", 28U, "FMMC_KEY_TIP9");
		if (inputStr.length() > 0)
		{
			if (!IsSafePath(inputStr))
			{
				Game::Print::PrintBottomCentre("~r~Error:~s~ Invalid characters in name.");
			}
			else
			{
				sub::ComponentChangerOutfit::Create(g_Ped1, dir + "\\" + inputStr + ".xml");
				Game::Print::PrintBottomLeft("File ~b~created~s~.");
			}
		}
		else Game::Print::PrintErrorInvalidInput(inputStr);
		return;
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
			else if (CreateDirectoryA((dir + "\\" + inputStr).c_str(), NULL) || GetLastError() == ERROR_ALREADY_EXISTS)
			{
				dir = dir + "\\" + inputStr;
				if (engine) engine->currentOption = 5;
				Game::Print::PrintBottomLeft("Folder ~b~created~s~.");
			}
			else
			{
				Game::Print::PrintBottomCentre("~r~Failed~s~ to create folder.");
				addlog(ige::LogType::LOG_ERROR, "Attempt to create folder " + inputStr + " failed");
			}
		}
		else Game::Print::PrintErrorInvalidInput(inputStr);
		return;
	}
}

void PedOutfitsItemSubmenu::Draw()
{
	std::string& name = dict;
	std::string& dir = dict3;
	const std::string filePath = dir + "\\" + name + ".xml";

	DrawTitle();

	Engine* engine = Engine::Current();

	bool applyPressed = DrawOption("Apply");
	bool applyAllFeatures = DrawOption("Apply Clothing & Attachments");
	bool applyModel = DrawOption(std::string("Apply ") + (g_Ped1 == PLAYER_PED_ID() ? "Ped Model" : "Head Features"));
	bool applySetDefault = DrawOption("Apply and Set as Default");
	bool renamePressed = DrawOption("Rename File");
	bool overwritePressed = DrawOption("Overwrite File");
	bool deletePressed = DrawOption("Delete File");

	if (applyPressed)
	{
		applyModel = true;
		applyAllFeatures = true;
	}

	if (applyModel)
	{
		bool s1isme = (g_Ped1 == PLAYER_PED_ID());
		sub::ComponentChangerOutfit::Apply(g_Ped1, filePath, true, false, false, false, false, false);
		if (s1isme) g_Ped1 = PLAYER_PED_ID();
	}

	if (applyAllFeatures)
	{
		sub::ComponentChangerOutfit::Apply(g_Ped1, filePath, false, true, true, true, true, true);
	}

	if (applySetDefault)
	{
		sub::ComponentChangerOutfit::Apply(PLAYER_PED_ID(), filePath, true, false, false, false, false, false);
		sub::ComponentChangerOutfit::Apply(PLAYER_PED_ID(), filePath, false, true, true, true, true, true);
		if (sub::ComponentChangerOutfit::Create(PLAYER_PED_ID(), "menyooStuff/defaultPed.xml"))
			Game::Print::PrintBottomLeft("Set as ~b~Default~s~, Outfit will be auto loaded on next game launch.");
		else
		{
			Game::Print::PrintBottomCentre("~r~Error:~s~ Unable to create file.");
			addlog(ige::LogType::LOG_ERROR, "Attempt to create file menyooStuff/defaultPed.xml failed");
		}
	}

	if (overwritePressed)
	{
		if (sub::ComponentChangerOutfit::Create(g_Ped1, filePath))
			Game::Print::PrintBottomLeft("File ~b~overwritten~s~.");
		else
		{
			Game::Print::PrintBottomCentre("~r~Error:~s~ Unable to overwrite file.");
			addlog(ige::LogType::LOG_ERROR, "Attempt to overwrite file " + filePath + " failed");
		}
	}

	if (renamePressed)
	{
		std::string newName = Game::InputBox("", 28U, "FMMC_KEY_TIP9", name);
		if (newName.length() > 0)
		{
			if (!IsSafePath(newName))
				Game::Print::PrintBottomCentre("~r~Error:~s~ Invalid characters in name.");
			else if (rename(filePath.c_str(), (dir + "\\" + newName + ".xml").c_str()) == 0)
			{
				name = newName;
				Game::Print::PrintBottomLeft("File ~b~renamed~s~.");
			}
			else Game::Print::PrintBottomCentre("~r~Error:~s~ Unable to rename file.");
			addlog(ige::LogType::LOG_ERROR, "Attempt to rename file " + name + " to " + newName + " failed");
		}
		else Game::Print::PrintErrorInvalidInput(newName);
	}

	if (deletePressed)
	{
		if (remove(filePath.c_str()) == 0)
		{
			Game::Print::PrintBottomLeft("File ~b~deleted~s~.");
		}
		else
		{
			Game::Print::PrintBottomCentre("~r~Error:~s~ Unable to delete file.");
			addlog(ige::LogType::LOG_ERROR, "Attempt to delete file " + filePath + " failed");
		}
		if (engine) engine->GoBack();
		return;
	}

	pugi::xml_document doc;
	if (doc.load_file(filePath.c_str()).status != pugi::status_ok)
		return;

	DrawBreak("---Attributes---");
	auto nodeEntity = doc.child("OutfitPedData");
	auto nodePedStuff = nodeEntity.child("PedProperties");

	auto nodeClearDecalOverlays = nodeEntity.child("ClearDecalOverlays");
	if (engine && engine->AddCheckbox("Clear Previous Decals",
		nodeClearDecalOverlays.text().as_bool(true), Checkbox::BOXTICK, Checkbox::BOXBLANK))
	{
		if (!nodeClearDecalOverlays)
			nodeClearDecalOverlays = nodeEntity.append_child("ClearDecalOverlays");
		nodeClearDecalOverlays.text() = !nodeClearDecalOverlays.text().as_bool(true);
		doc.save_file(filePath.c_str());
	}

	auto nodeShortHeighted = nodePedStuff.child("HasShortHeight");
	if (nodeShortHeighted)
	{
		if (engine && engine->AddCheckbox("Short Height",
			nodeShortHeighted.text().as_bool(), Checkbox::BOXTICK, Checkbox::BOXBLANK))
		{
			nodeShortHeighted.text() = !nodeShortHeighted.text().as_bool();
			doc.save_file(filePath.c_str());
		}
	}

	auto nodeAddAttachmentsToSpoonerDb = nodeEntity.child("SpoonerAttachments")
		.attribute("SetAttachmentsPersistentAndAddToSpoonerDatabase");
	bool addAttachemntsToSpoonerDb = nodeAddAttachmentsToSpoonerDb.as_bool();
	if (nodeAddAttachmentsToSpoonerDb)
	{
		if (engine && engine->AddCheckbox("Persistent Attachments (AddToSpoonerDb)",
			addAttachemntsToSpoonerDb, Checkbox::BOXTICK, Checkbox::BOXBLANK))
		{
			nodeAddAttachmentsToSpoonerDb = !nodeAddAttachmentsToSpoonerDb.as_bool();
			addAttachemntsToSpoonerDb = !addAttachemntsToSpoonerDb;
			doc.save_file(filePath.c_str());
		}
	}

	if (addAttachemntsToSpoonerDb)
	{
		auto nodeStartTaskSeqOnLoad = nodeEntity.child("SpoonerAttachments")
			.attribute("StartTaskSequencesOnLoad");
		if (nodeStartTaskSeqOnLoad)
		{
			if (engine && engine->AddCheckbox("Start Task Sequences Immediately",
				nodeStartTaskSeqOnLoad.as_bool(), Checkbox::BOXTICK, Checkbox::BOXBLANK))
			{
				nodeStartTaskSeqOnLoad = !nodeStartTaskSeqOnLoad.as_bool();
				doc.save_file(filePath.c_str());
			}
		}
	}
}

}
REGISTER_SUBMENU(::Menu::PedComponentsSubmenu)
REGISTER_SUBMENU(::Menu::PedComponentsSetSubmenu)
REGISTER_SUBMENU(::Menu::PedComponentsPropsSubmenu)
REGISTER_SUBMENU(::Menu::PedComponentsPropsSetSubmenu)
REGISTER_SUBMENU(::Menu::PedOutfitsSubmenu)
REGISTER_SUBMENU(::Menu::PedOutfitsItemSubmenu)
REGISTER_SUBMENU(::Menu::PedDecalsTypesSubmenu)
REGISTER_SUBMENU(::Menu::PedDecalsZonesSubmenu)
REGISTER_SUBMENU(::Menu::PedDecalsInZoneSubmenu)
REGISTER_SUBMENU(::Menu::PedDamageCategoriesSubmenu)
REGISTER_SUBMENU(::Menu::PedDamageBoneSubmenu)
REGISTER_SUBMENU(::Menu::PedDamageBloodSubmenu)
REGISTER_SUBMENU(::Menu::PedDamageDecalsSubmenu)
REGISTER_SUBMENU(::Menu::PedDamagePacksSubmenu)
REGISTER_SUBMENU(::Menu::PedHeadFeaturesSubmenu)
REGISTER_SUBMENU(::Menu::PedHeadOverlaysSubmenu)
REGISTER_SUBMENU(::Menu::PedHeadOverlaysItemSubmenu)
REGISTER_SUBMENU(::Menu::PedFaceFeaturesSubmenu)
REGISTER_SUBMENU(::Menu::PedSkinToneSubmenu)
