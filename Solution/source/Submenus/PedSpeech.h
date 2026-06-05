#pragma once

#include "../Menu/Submenu.h"

#include <string>
#include <array>
#include <vector>

typedef unsigned short uint16_t;

namespace sub
{
	namespace Speech
	{
		struct SpeechNameS { std::string title, label; };
		extern std::vector<SpeechNameS> vSpeechNames;
		struct VoiceNameS { std::string title, label; };
		extern std::vector<VoiceNameS> vVoiceNames;
		struct AmbientSpeechDataS { std::string title, voiceName, speechName, paramName; };
		extern const std::vector<AmbientSpeechDataS> vSpeechData;
		struct SpeechParamS { std::string title, label; };
		extern const std::array<SpeechParamS, 37> vSpeechParams;

		struct AmbientVoice_t
		{
			std::string voiceName;
			std::vector<std::string> speechNames;
		};
		extern std::vector<AmbientVoice_t> vVoiceData;
		extern AmbientVoice_t* _currVoiceInfo;
		extern uint16_t _currSpeechParamIndex;
		extern std::string& searchStr; // alias of Routine.h `dict2`; shared search buffer

		bool PopulateVoiceData();
	}
}

namespace Menu {

class VoiceChangerSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_voice_changer"; }
	const char* Title() const override { return "Voice Changer"; }
	void Draw() override;
};

class SpeechPlayerSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_speech_player"; }
	const char* Title() const override { return "Speech"; }
	void Draw() override;
};

class SpeechPlayerInVoiceSubmenu final : public ::Menu::Submenu
{
public:
	const char* Id() const override    { return "ped_speech_player_in_voice"; }
	const char* Title() const override { return "Voice"; }
	void Draw() override;
};

}