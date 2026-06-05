#include "FunnyVehicles.h"

#include "../Menu/SubmenuRegistry.h"

#include "FunnyVehiclesActions.h"

namespace Menu {

void FunnyVehiclesSubmenu::Draw()
{
	DrawTitle();

	namespace FV = ::Menu::FunnyVehicles;
	using ActionFn = void (*)();
	struct Row { const char* label; ActionFn fn; };

	static const Row rows[] = {
		{ "Adderuma",                            &FV::Adderuma                       },
		{ "Zentornuma",                          &FV::Zentornuma                     },
		{ "TurismoRuma",                         &FV::TurismoRuma                    },
		{ "EnturumaXF",                          &FV::EnturumaXF                     },
		{ "Osirisuma",                           &FV::Osirisuma                      },
		{ "T20uma",                              &FV::T20uma                         },
		{ "Feltzeruma",                          &FV::Feltzeruma                     },
		{ "Banshuma",                            &FV::Banshuma                       },
		{ "Bulletuma",                           &FV::Bulletuma                      },
		{ "Land Jetski",                         &FV::LandJetski                     },
		{ "Chino O Death",                       &FV::ChinoODeath                    },
		{ "RV-Building",                         &FV::RVBuilding                     },
		{ "Monster Train Truck",                 &FV::MonsterTrainTruck              },
		{ "MonsterTruck (Boat Chassis)",         &FV::MonsterTruckBoatChassis        },
		{ "MonsterTruck (RV Chassis)",           &FV::MonsterTruckRVChassis          },
		{ "MonsterTruck (Helicopter Chassis)",   &FV::MonsterTruckHelicopterChassis  },
		{ "MonsterTruck (Fighter Jet Chassis)",  &FV::MonsterTruckFighterJetChassis  },
		{ "MonsterTruck (Tank Chassis)",         &FV::MonsterTruckTankChassis        },
		{ "Cow Car",                             &FV::CowCar                         },
		{ "Deer Car",                            &FV::DeerCar                        },
		{ "Shark Car",                           &FV::SharkCar                       },
		{ "Poo Mobile",                          &FV::PooMobile                      },
		{ "Wheelchair",                          &FV::Wheelchair                     },
		{ "Toy Car",                             &FV::ToyCar                         },
		{ "Bumper Car",                          &FV::BumperCar                      },
		{ "Roller Car",                          &FV::RollerCar                      },
		{ "Hydra UFO",                           &FV::HydraUFO                       },
		{ "Missile Surano",                      &FV::MissileSurano                  },
		{ "'Murican Surano",                     &FV::MuricanSurano                  },
		{ "Moveable Platform",                   &FV::MoveablePlatform               },
		{ "Speakers Up",                         &FV::SpeakersUp                     },
		{ "Yacht Airship",                       &FV::YachtAirshipWithFans           },
		{ "Yacht Airship (Without Fans)",        &FV::YachtAirship                   },
		{ "FIB Building",                        &FV::FibBuilding                    },
		{ "Go-Kart",                             &FV::GoKart                         },
		{ "Go-Bike",                             &FV::DragsterBike                   },
		{ "Weed-Wheels Bike",                    &FV::WeedWheelsBike                 }
	};

	for (const Row& r : rows)
	{
		if (DrawOption(r.label))
			r.fn();
	}
}

}
REGISTER_SUBMENU(::Menu::FunnyVehiclesSubmenu)
