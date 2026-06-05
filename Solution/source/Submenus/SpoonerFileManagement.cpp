#include "SpoonerFileManagement.h"

#include "../Menu/Engine.h"
#include "../Menu/SubmenuRegistry.h"
#include "../Menu/Menu.h"      // Checkbox
#include "../Menu/Routine.h"
#include "../Menu/FolderPreviewBmps.h"
#include "Misc.h"              // dict, dict3 (file name + directory shared with legacy nav)
#include "PlayerRuntime.h"
#include "Weather.h"           // currentTimecycleStrength

#include "../Memory/GTAmemory.h"

#include "../Natives/natives2.h"
#include "../Natives/types.h"  // RGBA
#include "../Scripting/enums.h"
#include "../Scripting/Camera.h"
#include "../Scripting/Game.h"
#include "../Scripting/GTAblip.h"
#include "../Scripting/GTAentity.h"
#include "../Scripting/GTAped.h"
#include "../Scripting/TimecycleModification.h"
#include "../Scripting/World.h"

#include "../Util/ExePath.h"
#include "../Util/FileLogger.h"
#include "../Util/GTAmath.h"
#include "../Util/StringManip.h"

#include "Spooner/Databases.h"
#include "Spooner/EntityManagement.h"
#include "Spooner/FileManagement.h"
#include "Spooner/MarkerManagement.h"
#include "Spooner/SpoonerMarker.h"
#include "Spooner/SpoonerMode.h"

#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")
#include <pugixml/src/pugixml.hpp>
#include <dirent/include/dirent.h>

#include <algorithm>
#include <cctype>
#include <cfloat>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace Menu {

namespace {

inline void toUpperInPlace(std::string& s)
{
	std::transform(s.begin(), s.end(), s.begin(),
		[](unsigned char c) { return static_cast<char>(std::toupper(c)); });
}
inline std::string toLowerCopy(const std::string& s)
{
	std::string out(s);
	std::transform(out.begin(), out.end(), out.begin(),
		[](unsigned char c) { return static_cast<char>(std::tolower(c)); });
	return out;
}
inline std::string toUpperCopy(const std::string& s)
{
	std::string out(s); toUpperInPlace(out); return out;
}

bool RowIsActive()
{
	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (!engine) return false;
	return engine->ActiveSelection() == engine->printingOption;
}

} // namespace

void SpoonerSaveFilesSubmenu::OnExit()
{
	searchStr.clear();
}

void SpoonerSaveFilesSubmenu::Draw()
{
	std::string& name = dict;
	std::string& dirStr = dict3;

	GTAentity myPed = PLAYER_PED_ID();
	const Vector3 myPos = myPed.GetPosition();

	DrawTitle();

	// --- Save Database To File ---
	if (DrawOption("Save Database To File (" + std::to_string(sub::Spooner::Databases::EntityDb.size()) + ")"))
	{
		std::string inputStr = Game::InputBox("", 28U, "Enter file name:");
		if (inputStr.length() > 0)
		{
			if (!IsSafePath(inputStr))
			{
				Game::Print::PrintBottomCentre("~r~Error:~s~ Invalid characters in name.");
			}
			else if (sub::Spooner::FileManagement::SaveDbToFile(dirStr + "\\" + inputStr + ".xml", true))
			{
				Game::Print::PrintBottomLeft("File ~b~saved~s~.");
			}
			else
			{
				Game::Print::PrintBottomCentre("~r~Error:~s~ Unable to save file.");
				addlog(ige::LogType::LOG_ERROR, "Attempt to save Database file " + inputStr + ".xml failed");
			}
		}
	}

	if (DrawOption("Save World To File (" + std::to_string(worldEntities.size()) + ")"))
	{
		std::string inputStr = Game::InputBox("", 28U, "Enter file name:");
		if (inputStr.length() > 0)
		{
			if (!IsSafePath(inputStr))
			{
				Game::Print::PrintBottomCentre("~r~Error:~s~ Invalid characters in name.");
			}
			else if (sub::Spooner::FileManagement::SaveWorldToFile(dirStr + "\\" + inputStr + ".xml", worldEntities, sub::Spooner::Databases::MarkerDb))
			{
				Game::Print::PrintBottomLeft("File ~b~saved~s~.");
			}
			else
			{
				Game::Print::PrintBottomCentre("~r~Error:~s~ Unable to save file.");
				addlog(ige::LogType::LOG_ERROR, "Attempt to save World file " + inputStr + ".xml failed");
			}
		}
	}

	std::vector<Entity> vSaveRangeEntities;
	GTAmemory::GetEntityHandles(vSaveRangeEntities, myPos, fSaveRangeRadius);
	{
		::Menu::Engine* engine = ::Menu::Engine::Current();
		::Menu::InputResult res = engine
			? engine->AddNumber("Save Range To File (" + std::to_string(vSaveRangeEntities.size()) + ")", fSaveRangeRadius, 0)
			: ::Menu::InputResult{};
		if (RowIsActive())
		{
			sub::Spooner::EntityManagement::DrawRadiusDisplayingMarker(myPos, fSaveRangeRadius);
		}
		if (res.rightPressed) { if (fSaveRangeRadius < FLT_MAX) fSaveRangeRadius += 1.0f; }
		if (res.leftPressed)  { if (fSaveRangeRadius > 0.0f)    fSaveRangeRadius -= 1.0f; }
		if (res.accepted)
		{
			std::string inputStr = Game::InputBox("", 28U, "Enter file name:");
			if (inputStr.length() > 0)
			{
				if (!IsSafePath(inputStr))
				{
					Game::Print::PrintBottomCentre("~r~Error:~s~ Invalid characters in name.");
				}
				else
				{
					std::vector<sub::Spooner::SpoonerMarker> vSaveRangeMarkers;
					sub::Spooner::MarkerManagement::GetAllMarkersInRange(vSaveRangeMarkers, myPos, fSaveRangeRadius);

					if (sub::Spooner::FileManagement::SaveWorldToFile(dirStr + "\\" + inputStr + ".xml", vSaveRangeEntities, vSaveRangeMarkers))
					{
						Game::Print::PrintBottomLeft("File ~b~saved~s~.");
					}
					else
					{
						Game::Print::PrintBottomCentre("~r~Error:~s~ Unable to save file.");
						addlog(ige::LogType::LOG_ERROR, "Attempt to save Range Markers file " + inputStr + ".xml failed");
					}
				}
			}
		}
	}

	if (dirStr.empty())
		dirStr = GetPathffA(Pathff::Spooner, false);

	std::vector<std::string> vfilnames;
	if (DIR* dirPoint = opendir(dirStr.c_str()))
	{
		dirent* entry = readdir(dirPoint);
		while (entry)
		{
			vfilnames.push_back(entry->d_name);
			entry = readdir(dirPoint);
		}
		closedir(dirPoint);
	}

	if (DrawOption("Create New Folder"))
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
				addlog(ige::LogType::LOG_ERROR, "Attempt to create new folder " + inputStr + " failed");
			}
		}
		else
		{
			Game::Print::PrintErrorInvalidInput(inputStr);
		}
		return;
	}

	DrawBreak("---Found Files---");

	// --- ".." (parent dir) ---
	if (DrawOption(".."))
	{
		std::string baseDir = GetPathffA(Pathff::Spooner, false);
		if (dirStr.length() > baseDir.length() && dirStr.find(baseDir) == 0)
		{
			dirStr = dirStr.substr(0, dirStr.rfind("\\"));
		}
		else
		{
			dirStr = baseDir;
		}
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

			const bool isFolder = PathIsDirectoryA((dirStr + "\\" + filname).c_str()) != 0;
			const bool isXml = filname.length() > 4 && filname.rfind(".xml") == filname.length() - 4;
			const bool isSp00n = filname.length() > 6 && filname.rfind(".SP00N") == filname.length() - 6;

			Checkbox icon = Checkbox::NONE;
			if (isFolder) icon = Checkbox::ARROWRIGHT;
			else if (isXml || isSp00n) icon = Checkbox::TICK2;

			::Menu::Engine* engine = ::Menu::Engine::Current();

			if (isFolder)
			{
				const bool pressed = engine
					? engine->AddCheckbox(filname + " >>>", true, icon, Checkbox::NONE)
					: false;
				if (pressed)
				{
					dirStr = dirStr + "\\" + filname;
				}
				else if (RowIsActive())
				{
					if (sub::FolderPreviewBmps_catind::bFolderBmpsEnabled)
						sub::FolderPreviewBmps_catind::DrawBmp(dirStr + "\\" + filname);
				}
			}
			else if (isXml || isSp00n)
			{
				const bool pressed = engine
					? engine->AddCheckbox(filname, true, icon, Checkbox::NONE)
					: false;
				if (pressed)
				{
					if (isXml)
					{
						name = filname.substr(0, filname.rfind('.'));
						NavigateTo("spooner_save_files_load");
					}
					else if (isSp00n)
					{
						name = filname.substr(0, filname.rfind('.'));
						NavigateTo("spooner_save_files_load_legacy");
					}
				}
			}
		}
	}
}

void SpoonerSaveFilesLoadSubmenu::Draw()
{
	std::string& name = dict;
	std::string& dirStr = dict3;
	const std::string filePath = dirStr + "\\" + name + ".xml";

	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (engine) engine->AddTitle(name);

	if (DrawOption("Teleport To Reference"))
	{
		sub::Spooner::FileManagement::TeleportToReference(filePath);
	}

	if (DrawOption("Load Placements"))
	{
		if (sub::Spooner::FileManagement::LoadPlacementsFromFile(filePath))
		{
			Game::Print::PrintBottomLeft("File ~b~loaded~s~.");
		}
		else
		{
			Game::Print::PrintBottomCentre("~r~Error:~s~ Unable to load file.");
			addlog(ige::LogType::LOG_ERROR, "Attempt to load placements from " + filePath + ".xml failed");
		}
	}

	if (DrawOption("Rename File"))
	{
		std::string inputStr = Game::InputBox("", 28U, "Enter new name:", name);
		if (inputStr.length() > 0)
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
			{
				Game::Print::PrintBottomCentre("~r~Error:~s~ Unable to rename file.");
				addlog(ige::LogType::LOG_ERROR, "Attempt to rename file " + name + ".xml to " + inputStr + " failed");
			}
		}
	}

	if (DrawOption("Overwrite Placements In File (With DB)"))
	{
		if (sub::Spooner::FileManagement::SaveDbToFile(filePath, false))
		{
			Game::Print::PrintBottomLeft("File ~b~overwritten~s~. Extra settings kept.");
		}
		else
		{
			Game::Print::PrintBottomCentre("~r~Error:~s~ Unable to overwrite file.");
			addlog(ige::LogType::LOG_ERROR, "Attempt to overwrite " + filePath + " failed");
		}
	}

	if (DrawOption("Delete File"))
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

	// ----- Attribute editing -----
	pugi::xml_document doc;
	if (doc.load_file(filePath.c_str()).status != pugi::status_ok)
		return;

	DrawBreak("---Attributes---");
	auto nodeRoot = doc.child("SpoonerPlacements");

	// Note
	if (auto nodeNote = nodeRoot.child("Note"))
	{
		std::string noteStr = nodeNote.text().as_string();
		const std::string preview = noteStr.length() > 0
			? (noteStr.length() < 10 ? noteStr : noteStr.substr(0, 10) + "...")
			: "~italic~None";
		const std::vector<std::string> items{ preview };
		::Menu::InputResult res = engine
			? engine->AddTextList("Note", 0, items)
			: ::Menu::InputResult{};
		if (res.accepted)
		{
			std::string inputStr = Game::InputBox("~`", 300U, "Enter note:", noteStr);
			if (inputStr.compare("~`") != 0)
			{
				noteStr = inputStr;
				nodeNote.text() = noteStr.c_str();
				doc.save_file(filePath.c_str());
			}
		}
	}

	// AudioFile
	if (auto nodeAudioFile = nodeRoot.child("AudioFile"))
	{
		std::string audioFileName = nodeAudioFile.text().as_string();
		const std::vector<std::string> items{ audioFileName.length() > 0 ? audioFileName : std::string("~italic~None") };
		::Menu::InputResult res = engine
			? engine->AddTextList("Audio File To Play", 0, items)
			: ::Menu::InputResult{};
		if (res.accepted)
		{
			std::string inputStr = Game::InputBox("~`", 64U, "Enter filename with extension (file should be in menyooStuff\\Audio):", audioFileName);
			if (inputStr.compare("~`") != 0)
			{
				audioFileName = inputStr;
				nodeAudioFile.text() = audioFileName.c_str();
				doc.save_file(filePath.c_str());
			}
		}
	}

	// StartTaskSequencesOnLoad
	if (auto nodeStartTaskSeqOnLoad = nodeRoot.child("StartTaskSequencesOnLoad"))
	{
		const bool current = nodeStartTaskSeqOnLoad.text().as_bool();
		if (engine && engine->AddCheckbox("Start Task Sequences Immediately", current, Checkbox::BOXTICK, Checkbox::BOXBLANK))
		{
			nodeStartTaskSeqOnLoad.text() = !current;
			doc.save_file(filePath.c_str());
		}
	}

	// ClearDatabase
	auto nodeClearDatabase = nodeRoot.child("ClearDatabase");
	if (nodeClearDatabase)
	{
		const bool current = nodeClearDatabase.text().as_bool();
		if (engine && engine->AddCheckbox("Delete Database Entities", current, Checkbox::BOXTICK, Checkbox::BOXBLANK))
		{
			nodeClearDatabase.text() = !current;
			doc.save_file(filePath.c_str());
		}
	}

	// ClearMarkers
	if (auto nodeClearMarkers = nodeRoot.child("ClearMarkers"))
	{
		const bool current = nodeClearMarkers.text().as_bool();
		if (engine && engine->AddCheckbox("Delete Database Markers", current, Checkbox::BOXTICK, Checkbox::BOXBLANK))
		{
			nodeClearMarkers.text() = !current;
			doc.save_file(filePath.c_str());
		}
	}

	// Reference + ImgLoading coords previews
	Vector3 refCoords;
	Vector3 imgLoadingCoords;
	auto nodeReferenceCoords = nodeRoot.child("ReferenceCoords");
	auto nodeImgLoadingCoords = nodeRoot.child("ImgLoadingCoords");
	for (auto& nas : std::vector<std::pair<Vector3*, pugi::xml_node*>>{
			{ &refCoords, &nodeReferenceCoords },
			{ &imgLoadingCoords, &nodeImgLoadingCoords }
		})
	{
		if (*nas.second)
		{
			nas.first->x = nas.second->child("X").text().as_float();
			nas.first->y = nas.second->child("Y").text().as_float();
			nas.first->z = nas.second->child("Z").text().as_float();

			sub::Spooner::EntityManagement::DrawRadiusDisplayingMarker(*nas.first, 0.5f, RGBA(0, 102, 204, 130));
			Vector2 scrnPos;
			if (World::WorldToScreen(*nas.first, scrnPos))
			{
				Game::Print::SetupDraw(GTAfont::Impact, Vector2(0.3f, 0.3f), true, false, true);
				Game::Print::drawstring(nas.second->name(), scrnPos.x, scrnPos.y);
			}
		}
	}

	// ClearWorld radius (requires ClearDatabase + ReferenceCoords present)
	auto nodeClearWorld = nodeRoot.child("ClearWorld");
	if (nodeClearDatabase && nodeReferenceCoords)
	{
		float clearWorldRadius = nodeClearWorld.text().as_float();
		::Menu::InputResult res = engine
			? engine->AddNumber("Delete World Entities (Within Radius)", clearWorldRadius, 0)
			: ::Menu::InputResult{};
		if (RowIsActive())
		{
			sub::Spooner::EntityManagement::DrawRadiusDisplayingMarker(refCoords, clearWorldRadius, RGBA(255, 0, 0, 130));
		}
		if (res.rightPressed && clearWorldRadius < FLT_MAX)
		{
			clearWorldRadius += 1.0f;
			nodeClearWorld.text() = abs(clearWorldRadius);
			doc.save_file(filePath.c_str());
		}
		else if (res.leftPressed && clearWorldRadius > 0.0f)
		{
			clearWorldRadius -= 1.0f;
			nodeClearWorld.text() = abs(clearWorldRadius);
			doc.save_file(filePath.c_str());
		}
	}

	// Per-coord-set editors
	for (auto& nas : std::vector<std::tuple<std::string, Vector3*, pugi::xml_node*, std::string>>{
			std::make_tuple(std::string("Reference Coordinates"),       &refCoords,        &nodeReferenceCoords,  std::string("ReferenceCoords")),
			std::make_tuple(std::string("CD-Image Loading Coordinates"), &imgLoadingCoords, &nodeImgLoadingCoords, std::string("ImgLoadingCoords"))
		})
	{
		auto& xNode = *std::get<2>(nas);
		DrawBreak(std::get<0>(nas));
		DrawOption("~italic~" + (xNode ? std::get<1>(nas)->ToString() : std::string("Not Set")));

		auto& spoocam = sub::Spooner::SpoonerMode::spoonerModeCamera;
		if (!spoocam.IsActive())
		{
			if (DrawOption("Set To Player Position"))
			{
				Vector3 myPos = GTAentity(PLAYER_PED_ID()).GetPosition();
				*std::get<1>(nas) = myPos;
				if (!xNode)
				{
					xNode = nodeRoot.append_child(std::get<3>(nas).c_str());
					xNode.append_child("X");
					xNode.append_child("Y");
					xNode.append_child("Z");
				}
				xNode.child("X").text() = std::get<1>(nas)->x;
				xNode.child("Y").text() = std::get<1>(nas)->y;
				xNode.child("Z").text() = std::get<1>(nas)->z;
				doc.save_file(filePath.c_str());
			}
		}
		else
		{
			if (DrawOption("Set To Camera Target"))
			{
				Vector3 hitCoords = spoocam.RaycastForCoord(Vector2(0.0f, 0.0f), 0, 160.0f, 3.0f);
				*std::get<1>(nas) = hitCoords;
				if (!xNode)
				{
					xNode = nodeRoot.append_child(std::get<3>(nas).c_str());
					xNode.append_child("X");
					xNode.append_child("Y");
					xNode.append_child("Z");
				}
				xNode.child("X").text() = std::get<1>(nas)->x;
				xNode.child("Y").text() = std::get<1>(nas)->y;
				xNode.child("Z").text() = std::get<1>(nas)->z;
				doc.save_file(filePath.c_str());
			}
		}

		if (IS_WAYPOINT_ACTIVE())
		{
			if (DrawOption("Set To Waypoint"))
			{
				GTAblip wpBlip = GET_FIRST_BLIP_INFO_ID(BlipIcon::Waypoint);
				Vector3 wpCoords = wpBlip.GetPosition();
				wpCoords.z = World::GetGroundHeight(wpCoords);
				*std::get<1>(nas) = wpCoords;
				if (!xNode)
				{
					xNode = nodeRoot.append_child(std::get<3>(nas).c_str());
					xNode.append_child("X");
					xNode.append_child("Y");
					xNode.append_child("Z");
				}
				xNode.child("X").text() = std::get<1>(nas)->x;
				xNode.child("Y").text() = std::get<1>(nas)->y;
				xNode.child("Z").text() = std::get<1>(nas)->z;
				doc.save_file(filePath.c_str());
			}
		}
	}

	// TimecycleModifier
	if (auto nodeTimecycMod = nodeRoot.child("TimecycleModifier"))
	{
		DrawBreak("Vision Hax");

		std::string timecycModStr = nodeTimecycMod.text().as_string();
		const std::vector<std::string> tmItems{ timecycModStr.length() ? timecycModStr : std::string("None") };
		::Menu::InputResult tmRes = engine
			? engine->AddTextList("Timecycle Mod", 0, tmItems)
			: ::Menu::InputResult{};

		if (tmRes.rightPressed)
		{
			auto tit = std::find_if(TimecycleModification::vTimecycles.begin(), TimecycleModification::vTimecycles.end(),
				[&timecycModStr](const std::pair<std::string, std::string>& item) {
					return item.first.compare(timecycModStr) == 0;
				});
			if (tit == TimecycleModification::vTimecycles.end())
			{
				timecycModStr = TimecycleModification::vTimecycles.front().first;
				nodeTimecycMod.text() = timecycModStr.c_str();
				doc.save_file(filePath.c_str());
			}
			else
			{
				++tit;
				if (tit != TimecycleModification::vTimecycles.end())
				{
					timecycModStr = tit->first;
					nodeTimecycMod.text() = timecycModStr.c_str();
					doc.save_file(filePath.c_str());
				}
			}
		}
		else if (tmRes.leftPressed)
		{
			auto tit = std::find_if(TimecycleModification::vTimecycles.begin(), TimecycleModification::vTimecycles.end(),
				[&timecycModStr](const std::pair<std::string, std::string>& item) {
					return item.first.compare(timecycModStr) == 0;
				});
			if (tit == TimecycleModification::vTimecycles.begin())
			{
				timecycModStr.clear();
				nodeTimecycMod.text() = timecycModStr.c_str();
				doc.save_file(filePath.c_str());
			}
			else if (tit != TimecycleModification::vTimecycles.end())
			{
				--tit;
				timecycModStr = tit->first;
				nodeTimecycMod.text() = timecycModStr.c_str();
				doc.save_file(filePath.c_str());
			}
		}
		else if (tmRes.accepted)
		{
			std::string inputStr = Game::InputBox(timecycModStr, 28U, "Enter timecycle mod name:", timecycModStr);
			if (inputStr.compare(timecycModStr) != 0)
			{
				timecycModStr = inputStr;
				nodeTimecycMod.text() = timecycModStr.c_str();
				doc.save_file(filePath.c_str());
			}
		}

		float timecycModStrength = nodeTimecycMod.attribute("strength").as_float(1.0f);
		::Menu::InputResult strRes = engine
			? engine->AddNumber("Strength", timecycModStrength, 2)
			: ::Menu::InputResult{};
		if (strRes.rightPressed)
		{
			if (timecycModStrength < 3.0f)
			{
				timecycModStrength += 0.02f;
				nodeTimecycMod.attribute("strength") = timecycModStrength;
				doc.save_file(filePath.c_str());
			}
		}
		else if (strRes.leftPressed)
		{
			if (timecycModStrength > 0.0f)
			{
				currentTimecycleStrength -= 0.02f;
				nodeTimecycMod.attribute("strength") = timecycModStrength;
				doc.save_file(filePath.c_str());
			}
		}
	}

	// WeatherToSet
	if (auto nodeWeatherToSet = nodeRoot.child("WeatherToSet"))
	{
		DrawBreak("Weather To Set");
		std::string weatherToSetStr = nodeWeatherToSet.text().as_string();
		const int weatherToSetInt = static_cast<int>(World::GetWeather(weatherToSetStr));

		if (engine && engine->AddCheckbox("None", weatherToSetInt == -1))
		{
			nodeWeatherToSet.text() = "";
			doc.save_file(filePath.c_str());
		}
		for (auto& ws : World::sWeatherNames)
		{
			if (engine && engine->AddCheckbox(ws.first, weatherToSetStr == ws.second))
			{
				nodeWeatherToSet.text() = ws.second.c_str();
				doc.save_file(filePath.c_str());
			}
		}
	}
}

void SpoonerSaveFilesLoadLegacySubmenu::Draw()
{
	std::string& name = dict;
	std::string& dirStr = dict3;
	const std::string filePath = dirStr + "\\" + name + ".SP00N";

	::Menu::Engine* engine = ::Menu::Engine::Current();
	if (engine) engine->AddTitle(name);

	if (DrawOption("Load Placements"))
	{
		if (sub::Spooner::FileManagement::LoadPlacementsFromSP00NFile(filePath))
		{
			Game::Print::PrintBottomLeft("File ~b~loaded~s~.");
		}
		else
		{
			Game::Print::PrintBottomCentre("~r~Error:~s~ Unable to load file.");
			addlog(ige::LogType::LOG_ERROR, "Attempt to load Placements file from" + filePath + " failed");
		}
	}

	if (DrawOption("Rename File"))
	{
		std::string inputStr = Game::InputBox("", 28U, "Enter new name:", name);
		if (inputStr.length() > 0)
		{
			if (!IsSafePath(inputStr))
			{
				Game::Print::PrintBottomCentre("~r~Error:~s~ Invalid characters in name.");
			}
			else if (rename(filePath.c_str(), (dirStr + "\\" + inputStr + ".SP00N").c_str()) == 0)
			{
				name = inputStr;
				Game::Print::PrintBottomLeft("File ~b~renamed~s~.");
			}
			else
			{
				addlog(ige::LogType::LOG_ERROR, "Attempt to rename file from" + name + " to " + inputStr + ".SP00N failed");
			}
		}
	}

	if (DrawOption("Delete File"))
	{
		if (remove(filePath.c_str()) == 0)
		{
			Game::Print::PrintBottomLeft("File ~b~deleted~s~.");
		}
		else
		{
			Game::Print::PrintBottomCentre("~r~Error:~s~ Unable to delete file.");
			addlog(ige::LogType::LOG_ERROR, "Attempt to delete file" + filePath + " failed");
		}
		if (engine) engine->GoBack();
		return;
	}
}

}
REGISTER_SUBMENU(::Menu::SpoonerSaveFilesSubmenu)
REGISTER_SUBMENU(::Menu::SpoonerSaveFilesLoadSubmenu)
REGISTER_SUBMENU(::Menu::SpoonerSaveFilesLoadLegacySubmenu)
