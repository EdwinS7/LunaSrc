#pragma once
#include "../includes.hpp"
#include "exploits/TickBase.h"

struct m_keybind
{
	std::string m_name;
	std::string m_mode;

	m_keybind(std::string name, std::string mode) :
		m_name(name), m_mode(mode)
	{

	}
};


class CMenu : public singleton<CMenu> {
public:
	void Draw(bool is_open);

	float dpi_scale = 1.f;


	ImFont* name;
	ImFont* HEADER;
	ImFont* montherrat_12px;
	ImFont* MenuFont;
	ImFont* tab_icons;
	ImFont* tab_icons_skin;
	ImFont* kwfont;
	ImFont* ugen;
	ImFont* settingicons;
	ImFont* binds_spec;
	ImFont* mainfont;
	ImFont* mainfont_bigger;
	ImFont* logs_font;

	bool config_bind = false;
	bool try_not_to_blue_screen = false;

	float save_time = m_globals()->m_realtime;
	bool  prenext_save = false;

	float load_time = m_globals()->m_realtime;
	bool  prenext_load = false;

	float delete_time = m_globals()->m_realtime;
	bool  prenext_delete = false;

	std::string loaded_config = XorStr("");



	float public_alpha;
	IDirect3DDevice9* device;
	float color_buffer[4] = { 1.f, 1.f, 1.f, 1.f };
private:
	struct {
		ImVec2 WindowPadding;
		float  WindowRounding;
		ImVec2 WindowMinSize;
		float  ChildRounding;
		float  PopupRounding;
		ImVec2 FramePadding;
		float  FrameRounding;
		ImVec2 ItemSpacing;
		ImVec2 ItemInnerSpacing;
		ImVec2 TouchExtraPadding;
		float  IndentSpacing;
		float  ColumnsMinSpacing;
		float  ScrollbarSize;
		float  ScrollbarRounding;
		float  GrabMinSize;
		float  GrabRounding;
		float  TabRounding;
		float  TabMinWidthForUnselectedCloseButton;
		ImVec2 DisplayWindowPadding;
		ImVec2 DisplaySafeAreaPadding;
		float  MouseCursorScale;
	} styles;

	bool update_dpi = false;
	bool update_scripts = false;

	int active_tab_index;
	ImGuiStyle style;
	int width = 850, height = 560;
	float child_height;

	float preview_alpha = 1.f;

	int active_tab;

	int rage_section;
	int legit_section;
	int visuals_section;
	int players_section;
	int misc_section;
	int settings_section;
	int current_profile = -1;
	int last_id = 0;

	std::vector<m_keybind> m_keybinds;
	std::string preview = crypt_str("None");
};