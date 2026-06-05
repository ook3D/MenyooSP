#pragma once

#include <string>

namespace GTAmodel
{
	class Model;
}

namespace sub
{
	namespace PedFavourites
	{
		extern std::string xmlFavouritePeds;
		bool IsPedAFavourite(GTAmodel::Model model);
		bool AddPedToFavourites(GTAmodel::Model model, const std::string& customName);
		bool RemovePedFromFavourites(GTAmodel::Model model);
		void ShowInstructionalButton(GTAmodel::Model model);
		// Removed: PedFavouritesMenu — now owned elsewhere. Now owned by PedModelChangerSubmenus.cpp.
	}

	void ChangeModel(GTAmodel::Model model);
}