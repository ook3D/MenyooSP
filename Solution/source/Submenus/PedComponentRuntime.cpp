#include "PedComponentRuntime.h"

#include "..\macros.h"

#include "..\Menu\Menu.h"
#include "..\Menu\Routine.h"
#include "PlayerRuntime.h"
#include "PedAnimationRuntime.h"   // SetPedFacialMood
#include "VehicleModShopRuntime.h"

#include "..\Memory\GTAmemory.h"

#include "..\Natives\natives2.h"
#include "..\Scripting\GTAped.h"
#include "..\Scripting\GTAentity.h"
#include "..\Scripting\Model.h"
#include "..\Scripting\Camera.h"
#include "..\Scripting\GameplayCamera.h"
#include "..\Scripting\World.h"
#include "..\Scripting\Game.h"
#include "..\Util\ExePath.h"
#include "..\Util\FileLogger.h"
#include "..\Util\StringManip.h"
#include "..\Util\keyboard.h"

#include "..\Menu\FolderPreviewBmps.h"
#include "PedModelRuntime.h"
#include "Spooner\SpoonerEntity.h"
#include "Spooner\Databases.h"
#include "Spooner\EntityManagement.h"
#include "Spooner\FileManagement.h"

#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include <string>
#include <vector>
#include <map>
//#include <utility>
#include <array>
#include <pugixml\src\pugixml.hpp>
#include <dirent\include\dirent.h>

namespace sub
{
	// Wraps an int value around [minVal, maxVal] on increment/decrement
	static int cycleInt(int current, bool increment, int minVal, int maxVal)
	{
		if (increment)
			return (current < maxVal) ? current + 1 : minVal;
		else
			return (current > minVal) ? current - 1 : maxVal;
	}

	// Wraps a float value around [minVal, maxVal] with a given step
	static float cycleFloat(float current, bool increment, float minVal, float maxVal, float step)
	{
		if (increment)
			return (current < maxVal) ? current + step : current;
		else
			return (current > minVal) ? current - step : current;
	}
	// Component changer

	Camera g_cam_componentChanger;

	void DrawPedVariationInfo(const std::string& info)
	{
		FLOAT x_coord = 0.066f + menuPos.x;
		FLOAT y_coord = OptionY + menuPos.y + 0.035f;

		Game::Print::SetupDraw(font_selection, Vector2(0.0f, (font_options == 0 ? 0.33f : 0.4f)), false, false, false, selectedtext);
		Game::Print::drawstring(info, x_coord, y_coord);
	}

    bool HasPedSpecificDrawable(int compon_drawable_new)
    {
        bool compon_drawable_correct = false;
        int drawableCurrent = GET_PED_DRAWABLE_VARIATION(g_Ped1, g_Ped4);
        if (compon_drawable_new == drawableCurrent)
        {
            compon_drawable_correct = true;
        }
        return compon_drawable_correct;
    }

	bool HasPedSpecificPropType(int propTypeNew)
	{
		bool propTypeCorrect = false;
		int propTypeCurrent = GET_PED_PROP_INDEX(g_Ped1, g_Ped4, 0);
		if (propTypeNew == propTypeCurrent)
		{
			propTypeCorrect = true;
		}
		return propTypeCorrect;
	}

	// Decals, tattoos & badges

	namespace PedDecals
	{
		std::map<Ped, std::vector<PedDecalValue>> vPedsAndDecals;

		bool g_tattooPreviewMode = false;
		const NamedPedDecal* g_previewTattoo = nullptr;

		void ClearPreviewTattoo()
		{
			if (g_previewTattoo && DOES_ENTITY_EXIST(g_Ped1))
			{
				g_previewTattoo->Remove(g_Ped1);
				g_previewTattoo = nullptr;
			}
		}

		bool NamedPedDecal::IsOnPed(GTAentity ped) const
		{
			auto it = vPedsAndDecals.find(ped.Handle());
			if (it == vPedsAndDecals.end())
			{
				return false;
			}

			for (auto& decal : it->second)
			{
				if (decal.collection == this->collection && decal.value == this->value)
				{
					return true;
				}
			}
			return false;
		}

		void NamedPedDecal::Apply(GTAentity ped) const
		{
			if (ped.Exists())
			{
				ped.RequestControl(200);
				PED::ADD_PED_DECORATION_FROM_HASHES(ped.Handle(), this->collection, this->value);
				vPedsAndDecals[ped.Handle()].push_back({ this->collection, this->value });
			}
			else
			{
				vPedsAndDecals.erase(ped.Handle());
			}
		}

		void NamedPedDecal::Remove(GTAentity ped) const
		{
			if (ped.Exists())
			{
				auto& decals = vPedsAndDecals[ped.Handle()];
				for (auto it = decals.begin(); it != decals.end();)
				{
					if (it->collection == this->collection && it->value == this->value)
					{
						it = decals.erase(it);
					}
					else ++it;
				}

				ped.RequestControl(200);
				CLEAR_PED_DECORATIONS(ped.Handle());
				for (auto& decal : decals)
				{
					ADD_PED_DECORATION_FROM_HASHES(ped.Handle(), decal.collection, decal.value);
				}
			}
			else
			{
				vPedsAndDecals.erase(ped.Handle());
			}
		}

		std::map<Hash, std::map<std::string, std::map<std::string, std::vector<NamedPedDecal>>>> vAllDecals; // PedHash[Type][Zone]
		void PopulateDecalsDict()
		{
			vAllDecals.clear();
			pugi::xml_document doc;
			if (doc.load_file((const char*)(GetPathffA(Pathff::Main, true) + "PedDecalOverlays.xml").c_str()).status != pugi::status_ok)
			{
				addlog(ige::LogType::LOG_ERROR, "Unable to open PedDecalOverlays.xml");
				return;
			}

			auto nodeRoot = doc.document_element();
			for (auto nodePed = nodeRoot.child("Ped"); nodePed; nodePed = nodePed.next_sibling("Ped"))
			{
				auto& dictType = vAllDecals[nodePed.attribute("hash").as_uint()];
				for (auto nodeType = nodePed.first_child(); nodeType; nodeType = nodeType.next_sibling())
				{
					auto& dictZone = dictType[nodeType.name()];
					for (auto nodeZone = nodeType.first_child(); nodeZone; nodeZone = nodeZone.next_sibling())
					{
						auto& listDecals = dictZone[nodeZone.name()];
						for (auto nodeDecal = nodeZone.child("OVERLAY"); nodeDecal; nodeDecal = nodeDecal.next_sibling("OVERLAY"))
						{
							NamedPedDecal decal;
							decal.collection = GET_HASH_KEY(nodeDecal.attribute("collection").as_string());
							decal.value = GET_HASH_KEY(nodeDecal.attribute("name").as_string());
							decal.caption = nodeDecal.attribute("caption").as_string();
							listDecals.push_back(decal);
						}
					}
				}
			}
		}

		std::pair<std::string, std::map<std::string, std::vector<NamedPedDecal>>>* selectedType;
		std::pair<std::string, std::vector<NamedPedDecal>>* selectedZone;
	}

	// Damage/blood textures

	namespace PedDamageTextures
	{
		auto& selectedPedHandle = g_Ped1;
		int boneToUse = 0;

		int& BoneToUse() { return boneToUse; }

		std::map<Ped, std::vector<std::string>> vPedsAndDamagePacks;

		void ClearAllBloodDamage(GTAped ped)
		{
			ped.ClearBloodDamage();
		}

		void ClearAllVisibleDamage(GTAped ped)
		{
			ped.ResetVisibleDamage();
			const auto& it = vPedsAndDamagePacks.find(ped.Handle());
			if (it != vPedsAndDamagePacks.end())
			{
				vPedsAndDamagePacks.erase(it);
			}
		}

		void ClearAll241BloodDamage()
		{
			ClearAllBloodDamage(selectedPedHandle);
		}

		void ClearAll241VisibleDamage()
		{
			ClearAllVisibleDamage(selectedPedHandle);
		}

#pragma region blood data
		using PedBloodDecals::vBloodDecals;
#pragma endregion
#pragma region damage decal data
		using PedDamageDecals::vDamageDecals;
#pragma endregion
#pragma region damage packs
		using PedDamagePacks::vDamagePacks;
#pragma endregion
	}

	// Head features (freemode m/f)

	namespace PedHeadFeatures_catind
	{

		std::map<Ped, sPedHeadFeatures> vPedHeads;
		std::map<Ped, sPedHeadFeatures>::mapped_type* pedHead;

#pragma region arrays
		const std::vector<std::pair<std::string, std::vector<std::string>>> vCaptions_headOverlays
		{
			{ /*"Skin Rash"*/"FACE_F_SUND",{ "Uneven", "Sandpaper", "Patchy", "Rough", "Leathery", "Textured", "Coarse", "Rugged", "Creased", "Cracked", "Gritty" } },
			{ /*"Beard"*/"FACE_F_BEARD",{ "HAIR_BEARD1", "HAIR_BEARD2", "HAIR_BEARD3", "HAIR_BEARD4", "HAIR_BEARD5", "HAIR_BEARD6", "HAIR_BEARD7", "HAIR_BEARD8", "HAIR_BEARD9", "HAIR_BEARD10", "HAIR_BEARD11", "HAIR_BEARD12", "HAIR_BEARD13", "HAIR_BEARD14", "HAIR_BEARD15", "HAIR_BEARD16", "HAIR_BEARD17", "HAIR_BEARD18", "HAIR_BEARD19" } }, // Beard HAIR_OPTION_0
			{ /*"Eyebrows"*/"FACE_F_EYEBR",{ "CC_EYEBRW_0", "CC_EYEBRW_1", "CC_EYEBRW_2", "CC_EYEBRW_3", "CC_EYEBRW_4", "CC_EYEBRW_5", "CC_EYEBRW_6", "CC_EYEBRW_7", "CC_EYEBRW_8", "CC_EYEBRW_9", "CC_EYEBRW_10", "CC_EYEBRW_11", "CC_EYEBRW_12", "CC_EYEBRW_13", "CC_EYEBRW_14", "CC_EYEBRW_15", "CC_EYEBRW_16", "CC_EYEBRW_17", "CC_EYEBRW_18", "CC_EYEBRW_19", "CC_EYEBRW_20", "CC_EYEBRW_21", "CC_EYEBRW_22", "CC_EYEBRW_23", "CC_EYEBRW_24", "CC_EYEBRW_25", "CC_EYEBRW_26", "CC_EYEBRW_27", "CC_EYEBRW_28", "CC_EYEBRW_29", "CC_EYEBRW_30", "CC_EYEBRW_31", "CC_EYEBRW_32", "CC_EYEBRW_33" } },
			{ /*"Wrinkles"*/"FACE_F_SKINA",{ "Crow's Feet", "First Signs", "Middle Aged", "Worry Lines", "Depression", "Distinguished", "Aged", "Weathered", "Wrinkled", "Sagging", "Tough Life", "Vintage", "Retired", "Junkie", "Geriatric" } },
			{ /*"Makeup & Face Paint"*/"HAIR_OPTION_2",{ "CC_MKUP_0", "CC_MKUP_1", "CC_MKUP_2", "CC_MKUP_3", "CC_MKUP_4", "CC_MKUP_5", "CC_MKUP_6", "CC_MKUP_7", "CC_MKUP_8", "CC_MKUP_9", "CC_MKUP_10", "CC_MKUP_11", "CC_MKUP_12", "CC_MKUP_13", "CC_MKUP_14", "CC_MKUP_15", "CC_MKUP_16", "CC_MKUP_17", "CC_MKUP_18", "CC_MKUP_19", "CC_MKUP_20", "CC_MKUP_21", "CC_MKUP_22", "CC_MKUP_23", "CC_MKUP_24", "CC_MKUP_25", "CC_MKUP_26", "CC_MKUP_27", "CC_MKUP_28", "CC_MKUP_29", "CC_MKUP_30", "CC_MKUP_31", "CC_MKUP_32", "CC_MKUP_33", "CC_MKUP_34", "CC_MKUP_35", "CC_MKUP_36", "CC_MKUP_37", "CC_MKUP_38", "CC_MKUP_39", "CC_MKUP_40", "CC_MKUP_41" } }, // Makeup & face paint HAIR_OPTION_2
			{ /*"Blush"*/"FACE_F_BLUSH",{ "CC_BLUSH_0", "CC_BLUSH_1", "CC_BLUSH_2", "CC_BLUSH_3", "CC_BLUSH_4", "CC_BLUSH_5", "CC_BLUSH_6" } },
			{ /*"Pigment 1 - Complexion"*/"FACE_F_SKC",{ "Rosy Cheeks", "Stubble Rash", "Hot Flush", "Sunburn", "Bruised", "Alchoholic", "Patchy", "Totem", "Blood Vessels", "Damaged", "Pale", "Ghostly" } },
			{ /*"Pigment 2 - Blemishes"*/"FACE_F_SKINB",{ "Measles", "Pimples", "Spots", "Break Out", "Blackheads", "Build Up", "Pustules", "Zits", "Full Acne", "Acne", "Cheek Rash", "Face Rash", "Picker", "Puberty", "Eyesore", "Chin Rash", "Two Face", "T Zone", "Greasy", "Marked", "Acne Scarring", "Full Acne Scarring", "Cold Sores", "Impetigo" } },
			{ /*"Lipstick"*/"FACE_F_LIPST",{ "CC_LIPSTICK_0", "CC_LIPSTICK_1", "CC_LIPSTICK_2", "CC_LIPSTICK_3", "CC_LIPSTICK_4", "CC_LIPSTICK_5", "CC_LIPSTICK_6", "CC_LIPSTICK_7", "CC_LIPSTICK_8", "CC_LIPSTICK_9" } },
			{ /*"Spots"*/"FACE_F_MOLE",{ "Cherub", "All Over", "Irregular", "Dot Dash", "Over the Bridge", "Baby Doll", "Pixie", "Sun Kissed", "Beauty Marks", "Line Up", "Modelesque", "Occasional", "Speckled", "Rain Drops", "Double Dip", "One Sided", "Pairs", "Growth" } },
			{ "Chest Hair",{ "CC_BODY_1_0", "CC_BODY_1_1", "CC_BODY_1_2", "CC_BODY_1_3", "CC_BODY_1_4", "CC_BODY_1_5", "CC_BODY_1_6", "CC_BODY_1_7", "CC_BODY_1_8", "CC_BODY_1_9", "CC_BODY_1_10", "CC_BODY_1_11", "CC_BODY_1_12", "CC_BODY_1_13", "CC_BODY_1_14", "CC_BODY_1_15", "CC_BODY_1_16", "CC_BODY_1_17" } },
			{ "Chest Blemishes",{} },
			{ "Chest Blemishes 2",{} },
		};
		const std::vector<std::string> vCaptions_facialFeatures
		{
			{ "Nose Width" },
			{ "Nose Bottom Height" },
			{ "Nose Tip Length" },
			{ "Nose Bridge Depth" },
			{ "Nose Tip Height" },
			{ "Nose Broken" },
			{ "Brow Height" },
			{ "Brow Depth" },
			{ "Cheekbone Height" },
			{ "Cheekbone Width" },
			{ "Cheek Depth" },
			{ "Eye Size" },
			{ "Lip Thickness" },
			{ "Jaw Width" },
			{ "Jaw Shape" },
			{ "Chin Height" },
			{ "Chin Depth" },
			{ "Chin Width" },
			{ "Chin Indent" },
			{ "Neck Width" }
		};
#pragma endregion

		inline int getMaxShapeAndSkinIds()
		{
			return g_unlockMaxIDs ? 255 : 46;
		}

		UINT8 GetPedHeadOverlayColourType(const PedHeadOverlay& overlayIndex)
		{
			switch (overlayIndex)
			{
			case PedHeadOverlay::Eyebrows:
			case PedHeadOverlay::Beard:
			case PedHeadOverlay::ChestHair:
			case PedHeadOverlay::Makeup:
				return 1;
			case PedHeadOverlay::Blush:
			case PedHeadOverlay::Lipstick:
				return 2;
			default:
				return 0;
			}
		}

		bool DoesPedModelSupportHeadFeatures(const GTAmodel::Model& pedModel)
		{
			return pedModel.hash == PedHash::FreemodeMale01 || pedModel.hash == PedHash::FreemodeFemale01;
		}

		void UpdatePedHeadBlendData(GTAped& ped, const PedHeadBlendData& blendData, bool bUnused)
		{
			ped.SetHeadBlendData(blendData);
		}

		void ApplyHeadOverlayTint(GTAped ped, int overlayIndex, int colourType, int primary, int secondary)
		{
			if (primary < 0 || colourType == 0)
			{
				SET_PED_HEAD_OVERLAY_TINT(ped.Handle(), overlayIndex, 0, 0, 0);
			}
			else
			{
				SET_PED_HEAD_OVERLAY_TINT(ped.Handle(), overlayIndex, colourType, primary, secondary);
			}
		}
	}

	// Outfits (saver)

	namespace ComponentChangerOutfit
	{
		UINT8 persistentAttachmentsTexterIndex = 0;

		bool Create(GTAentity ped, std::string filePath)
		{
			sub::Spooner::SpoonerEntity eped;
			eped.handle = ped;
			eped.type = EntityType::PED;
			eped.dynamic = true;

			bool bClearDecalOverlays = true;
			bool bAddAttachmentsToSpoonerDb = false;
			bool bStartTaskSeqsOnLoad = true;

			pugi::xml_document oldXml;
			if (oldXml.load_file((const char*)filePath.c_str()).status == pugi::status_ok)
			{
				auto nodeOldRoot = oldXml.child("OutfitPedData");
				bClearDecalOverlays = nodeOldRoot.child("ClearDecalOverlays").text().as_bool(bClearDecalOverlays);
				bAddAttachmentsToSpoonerDb = nodeOldRoot.child("SpoonerAttachments").attribute("SetAttachmentsPersistentAndAddToSpoonerDatabase").as_bool(bAddAttachmentsToSpoonerDb);
				bStartTaskSeqsOnLoad = nodeOldRoot.child("SpoonerAttachments").attribute("StartTaskSequencesOnLoad").as_bool(bStartTaskSeqsOnLoad);
			}

			pugi::xml_document doc;

			auto nodeDecleration = doc.append_child(pugi::node_declaration);
			nodeDecleration.append_attribute("version") = "1.0";
			nodeDecleration.append_attribute("encoding") = "ISO-8859-1";

			auto nodeEntity = doc.append_child("OutfitPedData"); // Root
			nodeEntity.append_child("ClearDecalOverlays").text() = bClearDecalOverlays;
			sub::Spooner::FileManagement::AddEntityToXmlNode(eped, nodeEntity);

			// Attachments
			auto nodeAttachments = nodeEntity.append_child("SpoonerAttachments");
			nodeAttachments.append_attribute("SetAttachmentsPersistentAndAddToSpoonerDatabase") = bAddAttachmentsToSpoonerDb;
			nodeAttachments.append_attribute("StartTaskSequencesOnLoad") = bStartTaskSeqsOnLoad;
			for (auto& e : sub::Spooner::Databases::EntityDb)
			{
				if (e.attachmentArgs.isAttached)
				{
					GTAentity att;
					if (sub::Spooner::EntityManagement::GetEntityThisEntityIsAttachedTo(e.handle, att))
					{
						if (att.Handle() == ped.Handle())
						{
							auto nodeAttachment = nodeAttachments.append_child("Attachment");
							sub::Spooner::FileManagement::AddEntityToXmlNode(e, nodeAttachment);
						}
					}
				}
			}

			return doc.save_file((const char*)filePath.c_str());
		}

		bool Apply(GTAped ep, const std::string& filePath, bool applyModelAndHead, bool applyProps, bool applyComps, bool applyDecals, bool applyDamageTextures, bool applyAttachedEntities)
		{
			pugi::xml_document doc;
			if (doc.load_file((const char*)filePath.c_str()).status != pugi::status_ok)
			{
				return false;
			}

			bool bNetworkIsGameInProgress = NETWORK::NETWORK_IS_GAME_IN_PROGRESS() != 0;
			auto nodeEntity = doc.child("OutfitPedData"); // Root
			ep.RequestControl(400);

			Model eModel = nodeEntity.child("ModelHash").text().as_uint();
			auto nodePedStuff = nodeEntity.child("PedProperties");

			if (applyModelAndHead)
			{
				if (ep.Handle() == PLAYER_PED_ID())
				{
					bool bWas241 = (g_Ped1 == ep.Handle());
					ChangeModel(eModel);
					ep = PLAYER_PED_ID();
					if (bWas241) g_Ped1 = ep.Handle();
				}

				if (nodePedStuff.child("HasShortHeight").text().as_bool()) SET_PED_CONFIG_FLAG(ep.Handle(), ePedConfigFlags::_Shrink, 1);

				auto nodePedHeadFeatures = nodePedStuff.child("HeadFeatures");
				if (sub::PedHeadFeatures_catind::DoesPedModelSupportHeadFeatures(eModel) && nodePedHeadFeatures)
				{
					auto nodePedHeadBlend = nodePedHeadFeatures.child("ShapeAndSkinTone");
					PED::SET_PED_HEAD_BLEND_DATA(ep.Handle(), 0, 0, 0, 1, 1, 1, 0.0f, 0.0f, 0.0f, false);
					PedHeadBlendData headBlend;
					headBlend.shapeFirstID = nodePedHeadBlend.child("ShapeFatherId").text().as_int();
					headBlend.shapeSecondID = nodePedHeadBlend.child("ShapeMotherId").text().as_int();
					headBlend.shapeThirdID = nodePedHeadBlend.child("ShapeOverrideId").text().as_int();
					headBlend.skinFirstID = nodePedHeadBlend.child("ToneFatherId").text().as_int();
					headBlend.skinSecondID = nodePedHeadBlend.child("ToneMotherId").text().as_int();
					headBlend.skinThirdID = nodePedHeadBlend.child("ToneOverrideId").text().as_int();
					headBlend.shapeMix = nodePedHeadBlend.child("ShapeVal").text().as_float();
					headBlend.skinMix = nodePedHeadBlend.child("ToneVal").text().as_float();
					headBlend.thirdMix = nodePedHeadBlend.child("OverrideVal").text().as_float();
					headBlend.isParent = nodePedHeadBlend.child("IsP").text().as_int();
					if (!g_unlockMaxIDs && (headBlend.shapeFirstID > 45 || headBlend.shapeSecondID > 45 || headBlend.shapeThirdID > 45))
					{
						Game::Print::PrintBottomCentre("~r~Warning:~s~ Parent Head Index outside normal range. Ensure Addon Heads are installed and Max Head IDs are unlocked");
						addlog(ige::LogType::LOG_WARNING, "Ped Head Index " + std::to_string(max(headBlend.shapeFirstID, max(headBlend.shapeSecondID, headBlend.shapeThirdID))) + " outside normal range of 0-45. Ensure Matching Addon Heads are installed from XML Source and Max Head IDs are unlocked.");
					}
					ep.SetHeadBlendData(headBlend);

					if (nodePedHeadFeatures.attribute("WasInArray").as_bool())
					{
						sub::PedHeadFeatures_catind::sPedHeadFeatures pedHead;
						pedHead.hairColour = nodePedHeadFeatures.child("HairColour").text().as_int();
						pedHead.hairColourStreaks = nodePedHeadFeatures.child("HairColourStreaks").text().as_int();
						pedHead.eyeColour = nodePedHeadFeatures.child("EyeColour").text().as_int();

						SET_PED_HAIR_TINT(ep.Handle(), pedHead.hairColour, pedHead.hairColourStreaks);
						SET_HEAD_BLEND_EYE_COLOR(ep.Handle(), SYSTEM::ROUND((float)pedHead.eyeColour)); // Sjaak says so

						auto nodePedFacialFeatures = nodePedHeadFeatures.child("FacialFeatures");
						int ii = 0;
						for (auto nodePedFacialFeature = nodePedFacialFeatures.first_child(); nodePedFacialFeature; nodePedFacialFeature = nodePedFacialFeature.next_sibling())
						{
							ii = stoi(std::string(nodePedFacialFeature.name()).substr(1));
							pedHead.facialFeatureData[ii] = nodePedFacialFeature.text().as_float();
							SET_PED_MICRO_MORPH(ep.Handle(), ii, pedHead.facialFeatureData[ii]);
						}

						auto nodePedHeadOverlays = nodePedHeadFeatures.child("Overlays");
						ii = 0;
						for (auto nodePedHeadOverlay = nodePedHeadOverlays.first_child(); nodePedHeadOverlay; nodePedHeadOverlay = nodePedHeadOverlay.next_sibling())
						{
							ii = stoi(std::string(nodePedHeadOverlay.name()).substr(1));
							auto overlayData_index = nodePedHeadOverlay.attribute("index").as_int();
							pedHead.overlayData[ii].colour = nodePedHeadOverlay.attribute("colour").as_int();
							pedHead.overlayData[ii].colourSecondary = nodePedHeadOverlay.attribute("colourSecondary").as_int();
							pedHead.overlayData[ii].opacity = nodePedHeadOverlay.attribute("opacity").as_float();
							SET_PED_HEAD_OVERLAY(ep.Handle(), ii, overlayData_index, pedHead.overlayData[ii].opacity);
							SET_PED_HEAD_OVERLAY_TINT(ep.Handle(), ii, sub::PedHeadFeatures_catind::GetPedHeadOverlayColourType((PedHeadOverlay)ii), pedHead.overlayData[ii].colour, pedHead.overlayData[ii].colourSecondary);
						}
						sub::PedHeadFeatures_catind::vPedHeads[ep.Handle()] = pedHead;
					}
				}

				auto nodeFacialMood = nodePedStuff.child("FacialMood");
				if (nodeFacialMood)
				{
					SetPedFacialMood(ep, nodeFacialMood.text().as_string());
				}

				int opacityLevel = nodeEntity.child("OpacityLevel").text().as_int(255);
				if (opacityLevel < 255)
				{
					ep.SetAlpha(opacityLevel);
				}
				ep.SetVisible(nodeEntity.child("IsVisible").text().as_bool());
			}

			if (nodeEntity.child("ClearDecalOverlays").text().as_bool(true))
			{
				CLEAR_PED_DECORATIONS(ep.Handle());
			}
			auto& decalsApplied = sub::PedDecals::vPedsAndDecals[ep.Handle()];
			decalsApplied.clear();
			if (applyDecals)
			{
				auto nodePedTattooLogoDecals = nodePedStuff.child("TattooLogoDecals");
				if (nodePedTattooLogoDecals)
				{
					for (auto nodeDecal = nodePedTattooLogoDecals.first_child(); nodeDecal; nodeDecal = nodeDecal.next_sibling())
					{
						sub::PedDecals::PedDecalValue decal(nodeDecal.attribute("collection").as_uint(), nodeDecal.attribute("value").as_uint());
						decalsApplied.push_back(decal);
						ADD_PED_DECORATION_FROM_HASHES(ep.Handle(), decal.collection, decal.value);
					}
				}
			}

			if (applyComps)
			{
				auto nodePedComps = nodePedStuff.child("PedComps");
				for (auto nodePedCompsObject = nodePedComps.first_child(); nodePedCompsObject; nodePedCompsObject = nodePedCompsObject.next_sibling())
				{
					int pedCompId = stoi(std::string(nodePedCompsObject.name()).substr(1));
					std::string pedCompIdValueStr = nodePedCompsObject.text().as_string();
					int pedCompIdValueDrawable = stoi(pedCompIdValueStr.substr(0, pedCompIdValueStr.find(",")));
					int pedCompIdValueTexture = stoi(pedCompIdValueStr.substr(pedCompIdValueStr.find(",") + 1));

					if (GET_NUMBER_OF_PED_DRAWABLE_VARIATIONS(ep.Handle(), pedCompId) >= pedCompIdValueDrawable && GET_NUMBER_OF_PED_TEXTURE_VARIATIONS(ep.Handle(), pedCompId, pedCompIdValueDrawable) >= pedCompIdValueTexture)
					{
						SET_PED_COMPONENT_VARIATION(ep.Handle(), pedCompId, pedCompIdValueDrawable, pedCompIdValueTexture, 0);
						addlog(ige::LogType::LOG_DEBUG, "Applied ped component " + std::to_string(pedCompId) + " with drawable " + std::to_string(pedCompIdValueDrawable) + " and texture " + std::to_string(pedCompIdValueTexture));
					}
					else
					{
						addlog(ige::LogType::LOG_WARNING, "Ped comp " + std::to_string(pedCompId) + " out of range - Drawable " + std::to_string(pedCompIdValueDrawable) + " and texture " + std::to_string(pedCompIdValueTexture));
					}
				}
			}

			if (applyProps)
			{
				CLEAR_ALL_PED_PROPS(ep.Handle(), 0);
				auto nodePedProps = nodePedStuff.child("PedProps");
				for (auto nodePedPropsObject = nodePedProps.first_child(); nodePedPropsObject; nodePedPropsObject = nodePedPropsObject.next_sibling())
				{
					int pedPropId = stoi(std::string(nodePedPropsObject.name()).substr(1));
					std::string pedPropIdValueStr = nodePedPropsObject.text().as_string();
					SET_PED_PROP_INDEX(ep.Handle(), pedPropId, stoi(pedPropIdValueStr.substr(0, pedPropIdValueStr.find(","))), stoi(pedPropIdValueStr.substr(pedPropIdValueStr.find(",") + 1)), bNetworkIsGameInProgress, 0);
				}
			}

			sub::PedDamageTextures::ClearAllBloodDamage(ep);
			sub::PedDamageTextures::ClearAllVisibleDamage(ep);
			if (applyDamageTextures)
			{
				auto nodePedDamagePacks = nodePedStuff.child("DamagePacks");
				if (nodePedDamagePacks)
				{
					auto& dmgPacksApplied = sub::PedDamageTextures::vPedsAndDamagePacks[ep.Handle()];
					dmgPacksApplied.clear();
					for (auto nodePedDamagePack = nodePedDamagePacks.first_child(); nodePedDamagePack; nodePedDamagePack = nodePedDamagePack.next_sibling())
					{
						const std::string dpnta = nodePedDamagePack.text().as_string();
						ep.ApplyDamagePack(dpnta, 1.0f, 1.0f);
						dmgPacksApplied.push_back(dpnta);
					}
				}
			}

			if (applyAttachedEntities)
			{
				std::unordered_set<Hash> vModelHashes;
				std::vector<sub::Spooner::SpoonerEntityWithInitHandle> vSpawnedAttachments;
				auto nodeAttachments = nodeEntity.child("SpoonerAttachments");
				bool bAddAttachmentsToSpoonerDb = nodeAttachments.attribute("SetAttachmentsPersistentAndAddToSpoonerDatabase").as_bool(false);
				bool bStartTaskSeqsOnLoad = nodeAttachments.attribute("StartTaskSequencesOnLoad").as_bool(true);
				switch (persistentAttachmentsTexterIndex)
				{
					case 0: break; // FileDecides
					case 1: bAddAttachmentsToSpoonerDb = false; break; // ForceOff
					case 2: bAddAttachmentsToSpoonerDb = true; break; // ForceOn
				}
				for (auto nodeAttachment = nodeAttachments.first_child(); nodeAttachment; nodeAttachment = nodeAttachment.next_sibling())
				{
					auto e = sub::Spooner::FileManagement::SpawnEntityFromXmlNode(nodeAttachment, vModelHashes);
					sub::Spooner::EntityManagement::AttachEntity(e.e, ep, e.e.attachmentArgs.boneIndex, e.e.attachmentArgs.offset, e.e.attachmentArgs.rotation);
					vSpawnedAttachments.push_back(e);
					if (bAddAttachmentsToSpoonerDb)
					{
						if (!e.e.taskSequence.empty())
						{
							auto& vTskPtrs = e.e.taskSequence.AllTasks();
							for (auto& u : vSpawnedAttachments)
							{
								for (auto& tskPtr : vTskPtrs)
								{
									tskPtr->LoadTargetingDressing(u.initHandle, u.e.handle.Handle());
								}
							}
							if (bStartTaskSeqsOnLoad) e.e.taskSequence.Start();
						}
						sub::Spooner::Databases::EntityDb.push_back(e.e);
					}
					else
					{
						e.e.handle.NoLongerNeeded();
					}
				}
				for (auto& amh : vModelHashes)
				{
					Model(amh).Unload();
				}
			}
			return true;
		}

	}
}
