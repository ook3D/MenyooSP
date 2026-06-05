#include "SubmenuRegistry.h"
#include "Submenu.h"

namespace Menu
{
	std::vector<std::unique_ptr<Submenu>>& SubmenuRegistry::Get()
	{
		static std::vector<std::unique_ptr<Submenu>> submenus;
		return submenus;
	}

	void SubmenuRegistry::Register(std::unique_ptr<Submenu> submenu)
	{
		if (!submenu) return;
		Get().push_back(std::move(submenu));
	}

	Submenu* SubmenuRegistry::Find(std::string_view id)
	{
		for (const auto& sub : Get())
		{
			if (sub->Id() == id) return sub.get();
		}
		return nullptr;
	}

	bool SubmenuRegistry::Draw(std::string_view id)
	{
		Submenu* sub = Find(id);
		if (!sub) return false;
		sub->Draw();
		return true;
	}

	const std::vector<std::unique_ptr<Submenu>>& SubmenuRegistry::All()
	{
		return Get();
	}
}
