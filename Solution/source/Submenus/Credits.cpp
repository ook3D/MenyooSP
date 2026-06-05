#include "Credits.h"

#include "../Menu/SubmenuRegistry.h"

#include "../Util/ExePath.h"
#include "../Util/FileLogger.h"

#include <pugixml/src/pugixml.hpp>

#include <windows.h>
#include <shellapi.h>

#pragma comment(lib, "shell32.lib")

namespace Menu {

void CreditsSubmenu::OnEnter()
{
	if (!loaded)
		LoadFromXml();
}

void CreditsSubmenu::LoadFromXml()
{
	addlog(ige::LogType::LOG_DEBUG, "Loading credits from XML...");
	tiers.clear();

	pugi::xml_document doc;
	const std::string path = GetPathffA(Pathff::Main, true) + "Supporters.xml";
	addlog(ige::LogType::LOG_TRACE, "Credits path: " + path);

	const pugi::xml_parse_result result = doc.load_file(path.c_str());
	if (result.status != pugi::status_ok)
	{
		addlog(ige::LogType::LOG_ERROR, std::string("Credits XML load failed: ") + result.description());
		loaded = true; // don't retry every frame
		return;
	}

	const pugi::xml_node root = doc.child("Credits");
	if (!root)
	{
		loaded = true;
		return;
	}

	for (pugi::xml_node tierNode : root.children("Tier"))
	{
		Tier tier;
		tier.name = tierNode.attribute("name").as_string();
		for (pugi::xml_node memberNode : tierNode.children("Member"))
		{
			std::string member = memberNode.text().as_string();
			if (!member.empty())
				tier.members.push_back(std::move(member));
		}
		if (!tier.members.empty())
			tiers.push_back(std::move(tier));
	}

	loaded = true;
}

void CreditsSubmenu::Draw()
{
	DrawTitle();

	// Lazy load on first Draw if OnEnter didn't fire (e.g. direct draw).
	if (!loaded)
		LoadFromXml();

	if (DrawOption("Support Menyoo on Patreon"))
	{
		ShellExecuteA(nullptr, "open",
			"https://www.patreon.com/cw/ItsJustCurtis",
			nullptr, nullptr, SW_SHOWNORMAL);
	}

	if (tiers.empty())
	{
		DrawOption("No credits found - Check Supporters.xml from your download");
		return;
	}

	for (const Tier& tier : tiers)
	{
		DrawBreak(tier.name);
		for (const std::string& member : tier.members)
		{
			DrawOption(member);
		}
	}
}

}
REGISTER_SUBMENU(::Menu::CreditsSubmenu)
