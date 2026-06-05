#include "SettingsRuntime.h"

namespace sub
{
	RGBA* g_settingsRGBA;
	int*g_settingsRGBA2;
	INT8* g_settingsFont;
	UINT8 settingsHUDColor = 0ui8;

	class MenyooTheme
	{
	private:
		bool grads;
		bool rainbow;
		bool thinLineOverFooter;

		RGBA ttbox;
		RGBA bgbox;
		RGBA tttext;
		RGBA optext;
		RGBA seltext;
		RGBA opbreak;
		RGBA opcount;
		RGBA selhi;
		RGBA pedtrackers;

		INT8 f_title;
		INT8 f_options;
		INT8 f_selection;
		INT8 f_breaks;
		INT8 f_hud;
		INT8 f_speedo;
	public:
		MenyooTheme()
		{
		}
		MenyooTheme(bool _grads, bool _rainbow, bool _thinLineOverFooter,
			RGBA _ttbox, RGBA _bgbox, RGBA _tttext, RGBA _optext, RGBA _seltext, RGBA _opbreak, RGBA _opcount, RGBA _selhi, RGBA _pedtrackers,
			INT8 _f_title, INT8 _f_options, INT8 _f_selection, INT8 _f_breaks, INT8 _f_hud, INT8 _f_speedo)
		{
			grads = _grads;
			rainbow = _rainbow;
			thinLineOverFooter = _thinLineOverFooter;

			ttbox = _ttbox;
			bgbox = _bgbox;
			tttext = _tttext;
			optext = _optext;
			seltext = _seltext;
			opbreak = _opbreak;
			opcount = _opcount;
			selhi = _selhi;
			pedtrackers = _pedtrackers;

			f_title = _f_title;
			f_options = _f_options;
			f_selection = _f_selection;
			f_breaks = _f_breaks;
			f_hud = _f_hud;
			f_speedo = _f_speedo;
		}

		void SetActive()
		{
			Menu::gradients = grads;
			rainbowBoxes = rainbow;
			Menu::thinLineOverScrect = thinLineOverFooter;

			titlebox = ttbox;
			BG = bgbox;
			titletext = tttext;
			optiontext = optext;
			selectedtext = seltext;
			optionbreaks = opbreak;
			optioncount = opcount;
			selectionhi = selhi;
			_globalPedTrackers_Col = pedtrackers;

			font_title = f_title;
			font_options = f_options;
			font_selection = f_selection;
			font_breaks = f_breaks;
			font_hud = f_hud;
			font_speedo = f_speedo;
		}
		bool IsActive()
		{
			return
				Menu::gradients == grads &&
				rainbowBoxes == rainbow &&
				Menu::thinLineOverScrect == thinLineOverFooter &&

				titlebox == ttbox &&
				BG == bgbox &&
				titletext == tttext &&
				optiontext == optext &&
				selectedtext == seltext &&
				optionbreaks == opbreak &&
				optioncount == opcount &&
				selectionhi == selhi &&
				_globalPedTrackers_Col == pedtrackers &&

				font_title == f_title &&
				font_options == f_options &&
				font_selection == f_selection &&
				font_breaks == f_breaks &&
				font_hud == f_hud &&
				font_speedo == f_speedo;
		}

		static MenyooTheme CurrentlyActiveTheme()
		{
			MenyooTheme curr;

			curr.grads = Menu::gradients;
			curr.rainbow = rainbowBoxes;
			curr.thinLineOverFooter = Menu::thinLineOverScrect;

			curr.ttbox = titlebox;
			curr.bgbox = BG;
			curr.tttext = titletext;
			curr.optext = optiontext;
			curr.seltext = selectedtext;
			curr.opbreak = optionbreaks;
			curr.opcount = optioncount;
			curr.selhi = selectionhi;
			curr.pedtrackers = _globalPedTrackers_Col;

			curr.f_title = font_title;
			curr.f_options = font_options;
			curr.f_selection = font_selection;
			curr.f_breaks = font_breaks;
			curr.f_hud = font_hud;
			curr.f_speedo = font_speedo;

			return curr;
		}

		bool operator ==(const MenyooTheme& value2) const
		{
			const MenyooTheme& value1 = *this;

			return
				value1.grads == value2.grads &&
				value1.rainbow == value2.rainbow &&
				value1.thinLineOverFooter == value2.thinLineOverFooter &&

				value1.ttbox == value2.ttbox &&
				value1.bgbox == value2.bgbox &&
				value1.tttext == value2.tttext &&
				value1.optext == value2.optext &&
				value1.seltext == value2.seltext &&
				value1.opbreak == value2.opbreak &&
				value1.opcount == value2.opcount &&
				value1.selhi == value2.selhi &&
				value1.pedtrackers == value2.pedtrackers &&

				value1.f_title == value2.f_title &&
				value1.f_options == value2.f_options &&
				value1.f_selection == value2.f_selection &&
				value1.f_breaks == value2.f_breaks &&
				value1.f_hud == value2.f_hud &&
				value1.f_speedo == value2.f_speedo;
		}
		bool Equals(MenyooTheme const& value2)
		{
			return (this->operator==(value2));
		}

	};
	class MenyooThemeNamed
	{
	public:
		std::string name;
		MenyooTheme theme;

		MenyooThemeNamed(const std::string& newName, const MenyooTheme& newTheme)
		{
			this->name = newName;
			this->theme = newTheme;
		}
	};

	std::vector<MenyooThemeNamed> vValues_MenyooThemes
	{
		{ "Black & Green", MenyooTheme(true, false, false, RGBA(0, 0, 0, 255), RGBA(0, 0, 0, 100), RGBA(0, 255, 100, 255), RGBA(0, 255, 100, 255), RGBA(255, 255, 255, 255), RGBA(0, 255, 100, 255), RGBA(0, 255, 100, 255), RGBA(0, 255, 100, 255), RGBA(0, 255, 100, 205), GTAfont::Italic, GTAfont::Impact, GTAfont::Impact, GTAfont::Italic, GTAfont::Arial, GTAfont::Pricedown) },
		{ "Black & Teal", MenyooTheme(true, false, false, RGBA(0, 0, 0, 255), RGBA(0, 0, 0, 150), RGBA(0, 255, 255, 255), RGBA(0, 255, 255, 255), RGBA(255, 255, 255, 255), RGBA(0, 255, 255, 255), RGBA(0, 255, 255, 255), RGBA(0, 255, 255, 150), RGBA(0, 255, 255, 205), GTAfont::Italic, GTAfont::Impact, GTAfont::Impact, GTAfont::Italic, GTAfont::Arial, GTAfont::Pricedown) },
		{ "Black & Red", MenyooTheme(true, false, false, RGBA(0, 0, 0, 255), RGBA(0, 0, 0, 175), RGBA(255, 0, 0, 255), RGBA(255, 0, 0, 255), RGBA(255, 255, 255, 255), RGBA(255, 0, 0, 255), RGBA(255, 0, 0, 255), RGBA(255, 0, 0, 100), RGBA(255, 0, 0, 205), GTAfont::Italic, GTAfont::Impact, GTAfont::Impact, GTAfont::Italic, GTAfont::Arial, GTAfont::Pricedown) },
		{ "Black & White", MenyooTheme(true, false, false, RGBA(0, 0, 0, 255), RGBA(0, 0, 0, 50), RGBA(255, 255, 255, 255), RGBA(255, 255, 255, 255), RGBA(255, 255, 255, 255), RGBA(255, 255, 255, 255), RGBA(255, 255, 255, 255), RGBA(255, 255, 255, 150), RGBA(255, 255, 255, 205), GTAfont::Italic, GTAfont::Impact, GTAfont::Impact, GTAfont::Italic, GTAfont::Arial, GTAfont::Pricedown) },
		{ "Black & Yellow", MenyooTheme(true, false, false, RGBA(0, 0, 0, 255), RGBA(0, 0, 0, 100), RGBA(255, 255, 100, 255), RGBA(255, 255, 100, 255), RGBA(255, 255, 255, 255), RGBA(255, 255, 100, 255), RGBA(255, 255, 100, 255), RGBA(255, 255, 100, 125), RGBA(255, 255, 100, 205), GTAfont::Italic, GTAfont::Impact, GTAfont::Impact, GTAfont::Italic, GTAfont::Arial, GTAfont::Pricedown) },
		{ "White & Green", MenyooTheme(true, false, false, RGBA(255, 255, 255, 255), RGBA(0, 0, 0, 100), RGBA(0, 255, 100, 255), RGBA(0, 255, 100, 255), RGBA(255, 255, 255, 255), RGBA(0, 255, 100, 255), RGBA(0, 255, 100, 255), RGBA(0, 255, 125, 150), RGBA(0, 255, 100, 255), GTAfont::Italic, GTAfont::Impact, GTAfont::Impact, GTAfont::Italic, GTAfont::Arial, GTAfont::Pricedown) },
		{ "White & Teal", MenyooTheme(true, false, false, RGBA(255, 255, 255, 255), RGBA(0, 0, 0, 100), RGBA(0, 255, 255, 255), RGBA(0, 255, 255, 255), RGBA(255, 255, 255, 255), RGBA(0, 255, 255, 255), RGBA(0, 255, 255, 255), RGBA(0, 255, 255, 150), RGBA(0, 255, 255, 205), GTAfont::Italic, GTAfont::Impact, GTAfont::Impact, GTAfont::Italic, GTAfont::Arial, GTAfont::Pricedown) },
		{ "White & Red", MenyooTheme(true, false, false, RGBA(255, 255, 255, 255), RGBA(0, 0, 0, 175), RGBA(255, 0, 0, 255), RGBA(255, 255, 255, 255), RGBA(255, 255, 255, 255), RGBA(255, 0, 0, 255), RGBA(255, 0, 0, 255), RGBA(255, 0, 0, 150), RGBA(255, 0, 0, 205), GTAfont::Italic, GTAfont::Impact, GTAfont::Impact, GTAfont::Italic, GTAfont::Arial, GTAfont::Pricedown) },
		{ "White & Black", MenyooTheme(true, false, false, RGBA(255, 255, 255, 255), RGBA(0, 0, 0, 150), RGBA(0, 0, 0, 255), RGBA(255, 255, 255, 255), RGBA(0, 0, 0, 255), RGBA(0, 0, 0, 255), RGBA(0, 0, 0, 255), RGBA(255, 255, 255, 150), RGBA(255, 255, 255, 205), GTAfont::Italic, GTAfont::Impact, GTAfont::Impact, GTAfont::Italic, GTAfont::Arial, GTAfont::Pricedown) },
		{ "White & Yellow", MenyooTheme(true, false, false, RGBA(255, 255, 255, 255), RGBA(0, 0, 0, 100), RGBA(255, 255, 100, 255), RGBA(255, 255, 100, 255), RGBA(255, 255, 100, 255), RGBA(255, 255, 100, 255), RGBA(255, 255, 100, 255), RGBA(255, 255, 100, 125), RGBA(255, 255, 100, 205), GTAfont::Italic, GTAfont::Impact, GTAfont::Impact, GTAfont::Italic, GTAfont::Arial, GTAfont::Pricedown) },
		{ "Halloween", MenyooTheme(true, false, false, RGBA(22, 161, 18, 255), RGBA(96, 62, 148, 170), RGBA(255, 51, 0, 255), RGBA(255, 255, 255, 255), RGBA(0, 0, 0, 255), RGBA(255, 255, 255, 255), RGBA(255, 51, 0, 255), RGBA(255, 51, 0, 150), RGBA(22, 161, 18, 205), GTAfont::Pricedown, GTAfont::Impact, GTAfont::Impact, GTAfont::Italic, GTAfont::Arial, GTAfont::Pricedown) },
		{ "Elegant Purple", MenyooTheme(true, false, true, RGBA(102, 0, 204, 255), RGBA(10, 10, 10, 255), RGBA(255, 255, 255, 255), RGBA(255, 255, 255, 255), RGBA(0, 0, 0, 255), RGBA(255, 255, 255, 240), RGBA(255, 255, 255, 255), RGBA(255, 255, 255, 211), RGBA(153, 51, 255, 205), GTAfont::Pricedown, GTAfont::Impact, GTAfont::Impact, GTAfont::Italic, GTAfont::Arial, GTAfont::Pricedown) },
	};

	int SettingsThemeCount()
	{
		return static_cast<int>(vValues_MenyooThemes.size());
	}
	const std::string& SettingsThemeName(int index)
	{
		return vValues_MenyooThemes[index].name;
	}
	void SettingsThemeApply(int index)
	{
		vValues_MenyooThemes[index].theme.SetActive();
	}
	bool SettingsThemeIsActive(int index)
	{
		return vValues_MenyooThemes[index].theme == MenyooTheme::CurrentlyActiveTheme();
	}
}
