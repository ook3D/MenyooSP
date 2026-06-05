#pragma once

#include <memory>
#include <string_view>
#include <vector>

namespace Menu
{
	class Submenu;

	class SubmenuRegistry
	{
	public:
		// Take ownership and store. Normally called by the AutoRegister
		// trampoline; can also be called manually for dynamically-created
		// submenus.
		static void Register(std::unique_ptr<Submenu> submenu);

		// Look up by Id(). Returns nullptr if no matching submenu exists.
		static Submenu* Find(std::string_view id);

		// Convenience: find by Id and call Draw(). Returns false if Id is not
		// registered (caller can fall back).
		static bool Draw(std::string_view id);

		// All registered submenus, in registration order. Useful for building
		// a debug index or for an auto-generated landing page.
		static const std::vector<std::unique_ptr<Submenu>>& All();

	private:
		static std::vector<std::unique_ptr<Submenu>>& Get();
	};

	template <typename T>
	struct AutoRegister
	{
		AutoRegister() { SubmenuRegistry::Register(std::make_unique<T>()); }
	};
}

#define MENU_CONCAT_INNER(a, b) a##b
#define MENU_CONCAT(a, b) MENU_CONCAT_INNER(a, b)
#define REGISTER_SUBMENU(ClassName) \
	static ::Menu::AutoRegister<ClassName> MENU_CONCAT(_menuAutoReg_, __COUNTER__);
