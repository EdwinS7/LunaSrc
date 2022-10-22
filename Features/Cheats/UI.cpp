//Menu
#include "../ImGui/code_editor.h"
#include "UI.h"

//Logs
#include "../cheats/misc/logs.h"
#include "../logging.h"

//Others
#include "../Others/constchars.h"
#include <ShlObj_core.h>
#include <unordered_map>
#include <string>

//Used for lua
#define ALPHA (ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar| ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float)
#define NOALPHA (ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float)

std::vector <std::string> files;
std::vector <std::string> scripts;
IDirect3DTexture9* all_skins[36];
auto selected_script = 0;

std::string get_config_dir()
{
	std::string folder;

	folder = crypt_str("luna\\configs");

	CreateDirectory(folder.c_str(), NULL);

	return folder;
}

void load_config(std::string selected_config)
{
	if (cfg_manager->files.empty())
		return;

	cfg_manager->load(selected_config, false);
	c_lua::get().unload_all_scripts();

	for (auto& script : cfg::g_cfg.scripts.scripts)
		c_lua::get().load_script(c_lua::get().get_script_id(script));

	scripts = c_lua::get().scripts;

	if (selected_script >= scripts.size())
		selected_script = scripts.size() - 1; //-V103

	for (auto& current : scripts)
	{
		if (current.size() >= 5 && current.at(current.size() - 1) == 'c')
			current.erase(current.size() - 5, 5);
		else if (current.size() >= 4)
			current.erase(current.size() - 4, 4);
	}

	for (auto i = 0; i < cfg::g_cfg.skins.skinChanger.size(); ++i)
		all_skins[i] = nullptr;

	cfg::g_cfg.scripts.scripts.clear();

	CMenu::get().loaded_config = selected_config;
	cfg_manager->load(selected_config, true);
	cfg_manager->config_files();

	eventlogs::get().add(crypt_str("Loaded ") + selected_config);

}

void save_config(std::string selected_config)
{
	if (selected_config == "")
		return;

	if (cfg_manager->files.empty())
		return;

	cfg::g_cfg.scripts.scripts.clear();

	for (auto i = 0; i < c_lua::get().scripts.size(); ++i)
	{
		auto script = c_lua::get().scripts.at(i);

		if (c_lua::get().loaded.at(i))
			cfg::g_cfg.scripts.scripts.emplace_back(script);
	}

	cfg_manager->save(selected_config);
	cfg_manager->config_files();

	eventlogs::get().add(crypt_str("Saved ") + selected_config);
}

void remove_config(std::string selected_config)
{
	eventlogs::get().add(crypt_str("Deleted ") + selected_config);

	cfg_manager->remove(selected_config);
	cfg_manager->config_files();

	files = cfg_manager->files;

	if (cfg::g_cfg.selected_config >= files.size())
		cfg::g_cfg.selected_config = files.size() - 1;

	for (auto& current : files)
		if (current.size() > 2)
			current.erase(current.size() - 3, 3);
}

void add_config(std::string name)
{
	if (name.empty())
		name = crypt_str("config");

	if (name.find(crypt_str(".cfg")) == std::string::npos)
		name += crypt_str(".cfg");

	eventlogs::get().add(crypt_str("Added ") + name + crypt_str(" config"));

	cfg_manager->save(name);
}

//For lua api
void draw_combo(const char* name, int& variable, const char* labels[], int count) { ImGui::Combo(std::string(name).c_str(), &variable, labels, count); }
void draw_combo(const char* name, int& variable, bool (*items_getter)(void*, int, const char**), void* data, int count) { ImGui::Combo(std::string(name).c_str(), &variable, items_getter, data, count); }

void draw_multicombo(std::string name, std::vector<int>& variable, const char* labels[], int count, std::string& preview)
{
	for (auto i = 0, j = 0; i < count; i++)
	{
		if (variable[i])
		{
			if (j)
				preview += crypt_str(", ") + (std::string)labels[i];
			else
				preview = labels[i];

			j++;
		}

	}

	if (ImGui::BeginCombo(name.c_str(), preview.c_str())) //draw
	{
		ImGui::BeginGroup();
		{

			for (auto i = 0; i < count; i++)
				ImGui::Selectable(labels[i], (bool*)&variable[i], ImGuiSelectableFlags_DontClosePopups);

		}
		ImGui::EndGroup();

		ImGui::EndCombo();
	}
	preview = crypt_str("None");
}

bool LabelClick(const char* label, bool* v, const char* unique_id) {
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	// The concatoff/on thingies were for my weapon config system so if we're going to make that, we still need this aids.
	char Buf[64];
	_snprintf(Buf, 62, crypt_str("%s"), label);

	char getid[128];
	sprintf_s(getid, 128, crypt_str("%s%s"), label, unique_id);


	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(getid);
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

	const ImRect check_bb(window->DC.CursorPos, ImVec2(label_size.y + style.FramePadding.y * 2 + window->DC.CursorPos.x, window->DC.CursorPos.y + label_size.y + style.FramePadding.y * 2));
	ImGui::ItemSize(check_bb, style.FramePadding.y);

	ImRect total_bb = check_bb;

	if (label_size.x > 0)
	{
		ImGui::SameLine(0, style.ItemInnerSpacing.x);
		const ImRect text_bb(ImVec2(window->DC.CursorPos.x, window->DC.CursorPos.y + style.FramePadding.y), ImVec2(window->DC.CursorPos.x + label_size.x, window->DC.CursorPos.y + style.FramePadding.y + label_size.y));

		ImGui::ItemSize(ImVec2(text_bb.GetWidth(), check_bb.GetHeight()), style.FramePadding.y);
		total_bb = ImRect(ImMin(check_bb.Min, text_bb.Min), ImMax(check_bb.Max, text_bb.Max));
	}

	if (!ImGui::ItemAdd(total_bb, id))
		return false;

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
	if (pressed)
		*v = !(*v);

	if (*v)
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(cfg::g_cfg.menu.menu_color_col.r() / 255.f, cfg::g_cfg.menu.menu_color_col.g() / 255.f, cfg::g_cfg.menu.menu_color_col.b() / 255.f, 1.f));
	if (label_size.x > 0.0f)
		ImGui::RenderText(ImVec2(check_bb.GetTL().x + 12, check_bb.GetTL().y), Buf);
	if (*v)
		ImGui::PopStyleColor();

	return pressed;

}

void draw_keybind(const char* label, key_bind* key_bind, const char* unique_id, bool with_bool = false, bool with_color = false)
{
	// reset bind if we re pressing esc
	if (key_bind->key == KEY_ESCAPE)
		key_bind->key = KEY_NONE;


	auto clicked = false;
	auto text = (std::string)m_inputsys()->ButtonCodeToString(key_bind->key);
	auto s = ImGui::GetWindowSize();
	if (key_bind->key <= KEY_NONE || key_bind->key >= KEY_MAX) {
		text = crypt_str("< >");
	}
	else

	// if we clicked on keybind
	if (hooks::input_shouldListen && hooks::input_receivedKeyval == &key_bind->key)
	{
		clicked = true;
		text = crypt_str("...");
	}

	if (text == crypt_str("MOUSE5"))
		text = crypt_str("M5");
	else if (text == crypt_str("MOUSE4"))
		text = crypt_str("M4");
	else if (text == crypt_str("MOUSE3"))
		text = crypt_str("M3");
	else if (text == crypt_str("MOUSE1"))
		text = crypt_str("M1");
	else if (text == crypt_str("MOUSE2"))
		text = crypt_str("M2");
	else if (text == crypt_str("INSERT"))
		text = crypt_str("INS");
	else if (text == crypt_str("DELETE"))
		text = crypt_str("DEL");



	auto textsize = ImGui::CalcTextSize(text.c_str()).x + 2;

	auto labelsize = ImGui::CalcTextSize(label);

	if (with_bool)
		ImGui::SameLine(-1, (s.x) - ImGui::CalcTextSize(text.c_str()).x - 60);
	else if (with_color)
		ImGui::SameLine(-1, (s.x) - ImGui::CalcTextSize(text.c_str()).x - 42);
	else
		ImGui::SameLine(-1, (s.x) - ImGui::CalcTextSize(text.c_str()).x - 17);



	if (ImGui::KeybindButton(text.c_str(), unique_id, ImVec2(ImGui::CalcTextSize(text.c_str()).x + 8, ImGui::CalcTextSize(text.c_str()).y + 6), clicked, ImGuiButtonFlags_::ImGuiButtonFlags_None))
		clicked = true;

	if (clicked)
	{
		hooks::input_shouldListen = true;
		hooks::input_receivedKeyval = &key_bind->key;
	}

	static auto hold = false, toggle = false, always = false;

	switch (key_bind->mode)
	{
	case HOLD:
		hold = true;
		toggle = false;
		always = false;
		break;
	case TOGGLE:
		toggle = true;
		hold = false;
		always = false;
		break;
	case ALWAYS:
		toggle = false;
		hold = false;
		always = true;
		break;
	}

	ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_PopupBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_PopupRounding, 8);
	ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(15 / 255.f, 15 / 255.f, 15 / 255.f, 1 * 0.85f));


	if (ImGui::BeginPopup(unique_id))
	{
		if (LabelClick(crypt_str("Always"), &always, unique_id))
		{
			if (always)
			{
				always = false;
				key_bind->mode = ALWAYS;
			}
			else if (toggle)
			{
				hold = false;
				key_bind->mode = TOGGLE;
			}
			else if (hold)
			{
				toggle = false;
				key_bind->mode = HOLD;
			}
			else
			{
				hold = false;
				key_bind->mode = TOGGLE;
			}

			ImGui::CloseCurrentPopup();
		}

		if (LabelClick(crypt_str("Toggle"), &toggle, unique_id))
		{
			if (toggle)
			{
				hold = false;
				key_bind->mode = TOGGLE;
			}
			else if (hold)
			{
				toggle = false;
				key_bind->mode = HOLD;
			}
			else if (always)
			{
				always = false;
				key_bind->mode = ALWAYS;
			}
			else
			{
				hold = false;
				key_bind->mode = TOGGLE;
			}

			ImGui::CloseCurrentPopup();
		}

		if (LabelClick(crypt_str("On hold"), &hold, unique_id))
		{
			if (hold)
			{
				toggle = false;
				key_bind->mode = HOLD;
			}
			else if (toggle)
			{
				hold = false;
				key_bind->mode = TOGGLE;
			}
			else if (always)
			{
				always = false;
				key_bind->mode = ALWAYS;
			}
			else
			{
				toggle = false;
				key_bind->mode = HOLD;
			}

			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}
	ImGui::PopStyleVar(2);    ImGui::PopStyleColor(1);
}

//#define BUFFER 8192
void CMenu::Draw(bool is_open) {
	if (is_open && public_alpha < 1)
		CMenu::get().public_alpha += 0.55f;
	else if (!is_open && public_alpha > 0)
		CMenu::get().public_alpha -= 0.55f;
	if (public_alpha < 0.01f)
		return;

	//Save config if we do ctrl(R or L) + S
	if (GetAsyncKeyState(VK_CONTROL) && GetAsyncKeyState(0x53) && !config_bind) {
		save_config(loaded_config);
		config_bind = true;
	}
	else if (config_bind && !GetAsyncKeyState(VK_CONTROL) && !GetAsyncKeyState(0x53))
		config_bind = false;

	static int tab = 0;

	//vars  //vars  //vars  //vars  //vars  //vars
	static bool keybinds[5], spectating[5];

	enum tabs {
		Ragebot,
		Legitbot,
		Antiaim,
		Players,
		World,
		Miscellaneous,
		Settings,
		lua,
		Skinchanger
	};

	int alpha = 255;
	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha / 255.f);
	ImGui::MainBegin("gayassmenu", nullptr, ImGuiWindowFlags_NoDecoration);
	{

		//some menu identifiers
		ImVec2 p = ImGui::GetWindowPos();
		ImDrawList* draw = ImGui::GetWindowDrawList();
		//menu size
		ImGui::SetWindowSize({ 765 * dpi_scale, 600 * dpi_scale });


		/*top textbox (logo)*/
		{
			//Logo Background
			draw->AddRectFilled({ p.x + 12, p.y + 18 }, { p.x + 42, p.y + 48 }, ImColor(35, 35, 35), 8);

			//Logo
			ImGui::PushFont(kwfont);
			draw->AddText(ImVec2(p.x + 22, p.y - 6 + ImGui::CalcTextSize("L").y), ImColor(cfg::g_cfg.menu.menu_color_col.r(), cfg::g_cfg.menu.menu_color_col.g(), cfg::g_cfg.menu.menu_color_col.b(), 255), "L");
			ImGui::PopFont();
		}

		/*bottom build data*/
		{
			ImGui::PushFont(logs_font);
			draw->AddText({ p.x + 755 - ImGui::CalcTextSize("Build: beta").x, p.y + 595 - ImGui::CalcTextSize("Build: beta").y }, ImColor(150, 150, 150, 255), "Build: ");
			draw->AddText({ p.x + 755 - ImGui::CalcTextSize("beta").x, p.y + 595 - ImGui::CalcTextSize("beta").y }, ImColor(cfg::g_cfg.menu.menu_color_col.r(), cfg::g_cfg.menu.menu_color_col.g(), cfg::g_cfg.menu.menu_color_col.b(), 255), "beta");
			ImGui::PopFont();
		}

		//vars

		static int ragebotsubtab = 0;
		static int legitbotsubtab = 0;
		static int player = 0;

		/*tabs*/   /*tabs*/   /*tabs*/   /*tabs*/   /*tabs*/
		ImGui::SetCursorPos({ 12, 65 });
		ImGui::BeginGroup();
		{
			
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 18));
			
			if (ImGui::tabby("1", "Ragebot", tab == Ragebot, CMenu::get().tab_icons))
				tab = Ragebot;
			if (ImGui::tabby("2", "Legitbot", tab == Legitbot, CMenu::get().tab_icons))
			    tab = Legitbot;
			if (ImGui::tabby("3", "Anti Aim", tab == Antiaim, CMenu::get().tab_icons))
				tab = Antiaim;
			if (ImGui::tabby("4", "Players", tab == Players, CMenu::get().tab_icons))
				tab = Players;
			if (ImGui::tabby("5", "World", tab == World, CMenu::get().tab_icons))
				tab = World;
			if (ImGui::tabby("6", "Miscellaneous", tab == Miscellaneous, CMenu::get().tab_icons))
				tab = Miscellaneous;
			if (ImGui::tabby("9", "Skinchanger", tab == Skinchanger, CMenu::get().tab_icons_skin))
				tab = Skinchanger;
			if (ImGui::tabby("7", "Settings", tab == Settings, CMenu::get().tab_icons))
				tab = Settings;
			if (ImGui::tabby("8", "Lua", tab == lua, CMenu::get().tab_icons))
				tab = lua;

			ImGui::PopStyleVar();

		}
		ImGui::EndGroup();
		/*tabs*/   /*tabs*/   /*tabs*/   /*tabs*/   /*tabs*/

		/*subtabs*/  /*subtabs*/  /*subtabs*/  /*subtabs*/
		ImGui::PushFont(MenuFont);
		switch (tab) {
		case Ragebot:
		{

			ImGui::SetCursorPos({ 65, 18 });
			ImGui::BeginGroup();
			{
				if (ImGui::subtab("main", ragebotsubtab == 0)) ragebotsubtab = 0;  ImGui::SameLine();
				if (ImGui::subtab("weapons", ragebotsubtab == 1)) ragebotsubtab = 1; ImGui::SameLine();
			}
			ImGui::EndGroup();

		}break;
		case Players:
		{

			ImGui::SetCursorPos({ 65, 18 });
			ImGui::BeginGroup();
			{
				if (ImGui::subtab("enemy", player == 0)) player = 0; ImGui::SameLine();
				if (ImGui::subtab("team", player == 1)) player = 1; ImGui::SameLine();
				if (ImGui::subtab("local", player == 2)) player = 2;
			}
			ImGui::EndGroup();

		}break;



		}
		ImGui::PopFont();
		/*subtabs*/  /*subtabs*/  /*subtabs*/  /*subtabs*/


		float startpos = 0;
		float added = 0;

		if (tab == Ragebot || tab == Players)
			startpos = 60;
		else
		{
			startpos = 15;
			added = 45;
			/*
			startpos = 60;
			added = 0;
			*/
		}


		ImGui::SetCursorPos({ 60, startpos });
		ImGui::BeginMainChild("Main", { 690, 505 + added }, false /*ImGuiWindowFlags_NoBackground*/);
		{
			ImGui::PushFont(MenuFont);
	
			switch (tab)
			{
			case Ragebot:
			{
				switch (ragebotsubtab)
				{
				case 0:
				{

					ImGui::SetCursorPos({ 5, 35 });
					ImGui::BeginChild("Main", { 332, 465 + added }, false, ImGuiWindowFlags_NoBackground);
					{


						ImGui::Checkbox(crypt_str("Master Switch"), &cfg::g_cfg.ragebot.enable);
						ImGui::Checkbox(crypt_str("Automatic fire"), &cfg::g_cfg.ragebot.autoshoot);
						ImGui::Checkbox(crypt_str("Extrapolation"), &cfg::g_cfg.ragebot.extrapolation);
						/*ImGui::Checkbox(crypt_str("Prediction (!)"), &cfg::g_cfg.ragebot.prediction);

						if (cfg::g_cfg.ragebot.prediction)
							ImGui::SliderInt(crypt_str("Length"), &cfg::g_cfg.ragebot.prediction_ticks, 1, 8, true);*/

						ImGui::Checkbox(crypt_str("Accurate fakeduck"), &cfg::g_cfg.ragebot.accurate_fd);
						ImGui::Checkbox(crypt_str("Dormant aimbot"), &cfg::g_cfg.ragebot.dormant_aimbot);

						if (cfg::g_cfg.ragebot.enable)
							cfg::g_cfg.legitbot.enabled = false;
					}
					ImGui::EndMenuChild();

					ImGui::SetCursorPos({ 342 , 35 });
					ImGui::BeginChild("Exploits", { 343, 250 - 35 }, false, ImGuiWindowFlags_NoBackground);
					{

						ImGui::Checkbox(crypt_str("Double Tap"), &cfg::g_cfg.ragebot.double_tap);
						ImGui::SameLine();
						draw_keybind(crypt_str(""), &cfg::g_cfg.ragebot.double_tap_key, crypt_str("##HOTKEY_DOUBLETAP"), true);


						ImGui::Checkbox(crypt_str("Hide Shots"), &cfg::g_cfg.antiaim.hide_shots);
						ImGui::SameLine();
						draw_keybind(crypt_str(""), &cfg::g_cfg.antiaim.hide_shots_key, crypt_str("##HOTKEY_HIDESHOTS"), true);

						draw_multicombo(crypt_str("Exploit modifiers"), cfg::g_cfg.ragebot.exploit_modifiers, exploit_modifiers, ARRAYSIZE(exploit_modifiers), preview);
					}
					ImGui::EndMenuChild();

					ImGui::SetCursorPos({ 342 , 250 + 35 });
					ImGui::BeginChild("Other", { 343, 250 - 35 + added }, false, ImGuiWindowFlags_NoBackground);
					{
						ImGui::Checkbox(crypt_str("Resolver"), &cfg::g_cfg.ragebot.resolver);
						ImGui::SameLine();
						draw_keybind(crypt_str(""), &cfg::g_cfg.ragebot.roll_correction_bind, crypt_str("##HOTKEY_ROLL_CORRECTION"), true);

						ImGui::Checkbox(crypt_str("Knife Bot"), &cfg::g_cfg.ragebot.knife_bot);
						ImGui::Checkbox(crypt_str("Zeus Bot"), &cfg::g_cfg.ragebot.zeus_bot);
						if (cfg::g_cfg.ragebot.zeus_bot)
							ImGui::SliderInt(crypt_str("Hit chance"), &cfg::g_cfg.ragebot.zeus_bot_hitchance, 0, 100);

						ImGui::Checkbox(crypt_str("Headshot only"), &cfg::g_cfg.ragebot.headshot_only);


					}
					ImGui::EndMenuChild();


				}break;
				case 1:
				{
					ImGui::SetCursorPos({ 5, 35 });
					ImGui::BeginChild("Settings", { 332, 465 + added }, false, ImGuiWindowFlags_NoBackground);
					{
						const char* rage_weapon[9] = { crypt_str("Pistols"), crypt_str("Revolver"), crypt_str("Deagle"), crypt_str("SMG"), crypt_str("Rifle"), crypt_str("Heavy"), crypt_str("Scout"), crypt_str("Auto sniper") , crypt_str("AWP") };

						ImGui::Combo(crypt_str("Weapon"), &hooks::rage_weapon, rage_weapon, ARRAYSIZE(rage_weapon));

						ImGui::Combo(crypt_str("Target Selection"), &cfg::g_cfg.ragebot.weapon[hooks::rage_weapon].selection_type, selection, ARRAYSIZE(selection));

						draw_multicombo(crypt_str("Hitboxes"), cfg::g_cfg.ragebot.weapon[hooks::rage_weapon].hitboxes, hitboxes, ARRAYSIZE(hitboxes), preview);

						draw_multicombo(crypt_str("Multipoints"), cfg::g_cfg.ragebot.weapon[hooks::rage_weapon].multipoints, hitboxes, ARRAYSIZE(hitboxes), preview);

						auto multipoints = cfg::g_cfg.ragebot.weapon[hooks::rage_weapon].multipoints;
						if (multipoints.at(0))
						    ImGui::SliderFloat(crypt_str("Head Scale"), &cfg::g_cfg.ragebot.weapon[hooks::rage_weapon].head_scale, 0, 1, cfg::g_cfg.ragebot.weapon[hooks::rage_weapon].head_scale ? crypt_str("%.2f") : crypt_str("Adaptive"));

						if (multipoints.at(1) || multipoints.at(2) || multipoints.at(3) || multipoints.at(4) || multipoints.at(5))
						    ImGui::SliderFloat(crypt_str("Body Scale"), &cfg::g_cfg.ragebot.weapon[hooks::rage_weapon].body_scale, 0, 1, cfg::g_cfg.ragebot.weapon[hooks::rage_weapon].body_scale ? crypt_str("%.2f") : crypt_str("Adaptive"));

						ImGui::Checkbox(crypt_str("Prefer Body Aim"), &cfg::g_cfg.ragebot.weapon[hooks::rage_weapon].prefer_body_aim);
						if (cfg::g_cfg.ragebot.weapon[hooks::rage_weapon].prefer_body_aim)
							ImGui::Combo(crypt_str("Mode"), &cfg::g_cfg.ragebot.weapon[hooks::rage_weapon].prefer_body_aim_mode, prefer_body_mode, ARRAYSIZE(prefer_body_mode));

						ImGui::SetCursorPosX(10); ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);	ImGui::Text("Force Body Aim");
						draw_keybind(crypt_str("##forcebaim"), &cfg::g_cfg.ragebot.body_aim_key, crypt_str("##HOKEY_FORCE_BODY_AIM"));

					}
					ImGui::EndMenuChild();

					ImGui::SetCursorPos({ 342 , 35 });
					ImGui::BeginChild("Accuracy / Minimum Damage", { 343, 465 + added }, false, ImGuiWindowFlags_NoBackground);
					{
						ImGui::Checkbox(crypt_str("Auto Stop"), &cfg::g_cfg.ragebot.weapon[hooks::rage_weapon].autostop);

						if (hooks::rage_weapon > 3 && hooks::rage_weapon != 3 && hooks::rage_weapon != 5) {
							ImGui::Checkbox(crypt_str("Auto Scope"), &cfg::g_cfg.ragebot.weapon[hooks::rage_weapon].autoscope);
							if (cfg::g_cfg.ragebot.weapon[hooks::rage_weapon].autoscope)
								ImGui::Combo(crypt_str("Auto Scope Type"), &cfg::g_cfg.ragebot.weapon[hooks::rage_weapon].autoscope_mode, auto_scope_mode, ARRAYSIZE(auto_scope_mode));

							if (cfg::g_cfg.ragebot.weapon[hooks::rage_weapon].autostop)
								draw_multicombo(crypt_str("Auto Stop Modifiers"), cfg::g_cfg.ragebot.weapon[hooks::rage_weapon].autostop_modifiers, autostop_modifiers, ARRAYSIZE(autostop_modifiers), preview);
						}

						ImGui::SliderInt(crypt_str("Hit Chance"), &cfg::g_cfg.ragebot.weapon[hooks::rage_weapon].hitchance_amount, 0, 100, cfg::g_cfg.ragebot.weapon[hooks::rage_weapon].hitchance_amount ? crypt_str("%d") : crypt_str("None"));
						
						if (hooks::rage_weapon != 1 && hooks::rage_weapon != 6 && hooks::rage_weapon != 8)
						    ImGui::SliderInt(crypt_str("Doubletap Hit Chance"), &cfg::g_cfg.ragebot.weapon[hooks::rage_weapon].double_tap_hitchance_amount, 0, 100, cfg::g_cfg.ragebot.weapon[hooks::rage_weapon].double_tap_hitchance_amount ? crypt_str("%d") : crypt_str("None"));

						ImGui::SliderInt(crypt_str("Minimum Damage"), &cfg::g_cfg.ragebot.weapon[hooks::rage_weapon].min_dmg, 1, 120, true);

						ImGui::SetCursorPosX(10); ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);	ImGui::Text("Damage Override");

						draw_keybind(crypt_str("Damage Override"), &cfg::g_cfg.ragebot.damage_override_key, crypt_str("##HOTKEY__DAMAGE_OVERRIDE"));

						if (cfg::g_cfg.ragebot.damage_override_key.key > KEY_NONE && cfg::g_cfg.ragebot.damage_override_key.key < KEY_MAX)
							ImGui::SliderInt(crypt_str("Damage Override Amount"), &cfg::g_cfg.ragebot.weapon[hooks::rage_weapon].min_override_dmg, 1, 120, true);

					}
					ImGui::EndMenuChild();

				}break;
				}
			}break;
			case Legitbot:
			{
				const char* legit_weapons[9] = { crypt_str("Pistols"), crypt_str("Revolver"), crypt_str("Deagle"), crypt_str("SMGs"), crypt_str("Rifles"), crypt_str("Heavies"), crypt_str("SSG-08"), crypt_str("SCAR-20 / G3SG1") , crypt_str("AWP") };

				const char* hitbox_legit[3] = { crypt_str("Nearest"), crypt_str("Head"), crypt_str("Body") };

				ImGui::SetCursorPos({ 5, 35 });
				ImGui::BeginChild("Main", { 332, 465 + added }, false, ImGuiWindowFlags_NoBackground);
				{


					ImGui::Checkbox(crypt_str("Enable"), &cfg::g_cfg.legitbot.enabled);
					ImGui::SameLine();
					draw_keybind(crypt_str(""), &cfg::g_cfg.legitbot.key, crypt_str("##legitbot_key"), true);

					//ImGui::Checkbox(crypt_str("Friendly Fire"), &cfg::g_cfg.legitbot.friendly_fire);
					ImGui::Checkbox(crypt_str("Auto Pistols"), &cfg::g_cfg.legitbot.autopistol);
					draw_multicombo(crypt_str("Disablers"), cfg::g_cfg.legitbot.disasblers, legitbot_disablers, ARRAYSIZE(legitbot_disablers), preview);
					ImGui::Combo(crypt_str("Field Of View Type"), &cfg::g_cfg.legitbot.weapon[hooks::legit_weapon].fov_type, LegitFov, ARRAYSIZE(LegitFov));
					ImGui::SliderFloat(crypt_str("Field Of View Amount"), &cfg::g_cfg.legitbot.weapon[hooks::legit_weapon].fov, 0.f, 50.f, crypt_str("%.2f"));


					ImGui::Checkbox(crypt_str("Backtrack"), &cfg::g_cfg.legitbot.backtrack);
					if (cfg::g_cfg.legitbot.backtrack)
					{
						ImGui::SliderInt(crypt_str("Backtrack Amount"), &cfg::g_cfg.legitbot.backtrack_ticks, 1, 100);
					}

					if (cfg::g_cfg.legitbot.enabled)
						cfg::g_cfg.ragebot.enable = false;


				}
				ImGui::EndMenuChild();

				ImGui::SetCursorPos({ 342 , 35 });
				ImGui::BeginChild("Settings", { 343, 465 + added }, false, ImGuiWindowFlags_NoBackground);
				{


					ImGui::Combo(crypt_str("Weapon"), &hooks::legit_weapon, legit_weapons, ARRAYSIZE(legit_weapons));
					ImGui::Combo(crypt_str("Hitbox"), &cfg::g_cfg.legitbot.weapon[hooks::legit_weapon].priority, hitbox_legit, ARRAYSIZE(hitbox_legit));


					ImGui::Combo(crypt_str("Smoothing Type"), &cfg::g_cfg.legitbot.weapon[hooks::legit_weapon].smooth_type, LegitSmooth, ARRAYSIZE(LegitSmooth));
					ImGui::SliderFloat(crypt_str("Smoothing Amount"), &cfg::g_cfg.legitbot.weapon[hooks::legit_weapon].smooth, 1.f, 12.f, crypt_str("%.1f"));


					ImGui::Combo(crypt_str("RCS Type"), &cfg::g_cfg.legitbot.weapon[hooks::legit_weapon].rcs_type, RCSType, ARRAYSIZE(RCSType));
					ImGui::SliderFloat(crypt_str("RCS Amount"), &cfg::g_cfg.legitbot.weapon[hooks::legit_weapon].rcs, 0.f, 100.f, crypt_str("%.0f%%"), 1.f);
					if (cfg::g_cfg.legitbot.weapon[hooks::legit_weapon].rcs > 0)
						ImGui::SliderFloat(crypt_str("RCS Smoothness"), &cfg::g_cfg.legitbot.weapon[hooks::legit_weapon].custom_rcs_smooth, 0.f, 12.f, (!cfg::g_cfg.legitbot.weapon[hooks::legit_weapon].custom_rcs_smooth ? crypt_str("None") : crypt_str("%.1f"))); //-V550


				}
				ImGui::EndMenuChild();

			}break;
			case Antiaim:
			{
				ImGui::SetCursorPos({ 5, 35 });
				ImGui::BeginChild("General", { 332, 250 - 35 }, false, ImGuiWindowFlags_NoBackground);
				{

					ImGui::Checkbox(crypt_str("Enable"), &cfg::g_cfg.antiaim.enable);

					if (cfg::g_cfg.antiaim.enable)
					{
						ImGui::Combo(crypt_str("Pitch"), &cfg::g_cfg.antiaim.pitch, pitch, ARRAYSIZE(pitch));
						if (cfg::g_cfg.antiaim.pitch == 3)
						    ImGui::SliderInt(crypt_str("Pitch"), &cfg::g_cfg.antiaim.custom_pitch, -89, 89);

						ImGui::Combo(crypt_str("Direction"), &cfg::g_cfg.antiaim.yaw_direction, yaw_direction, ARRAYSIZE(yaw_direction));

						ImGui::Combo(crypt_str("Modifiers"), &cfg::g_cfg.antiaim.yaw, yaw, ARRAYSIZE(yaw));

						ImGui::Combo(crypt_str("Base Angle"), &cfg::g_cfg.antiaim.base_angle, baseangle, ARRAYSIZE(baseangle));

						if (cfg::g_cfg.antiaim.yaw)
							ImGui::SliderInt(crypt_str("Jitter Range"), &cfg::g_cfg.antiaim.range, -180, 180);

						ImGui::SliderInt(crypt_str("Yaw add left"), &cfg::g_cfg.antiaim.yaw_add_left, -90, 90);
						ImGui::SliderInt(crypt_str("Yaw add right"), &cfg::g_cfg.antiaim.yaw_add_right, -90, 90);
					}
				}
				ImGui::EndMenuChild();
				ImGui::SetCursorPos({ 5, 250 + 35 });
				ImGui::BeginChild("Fake Lag", { 332, 250 - 35 + added }, false, ImGuiWindowFlags_NoBackground);
				{
					ImGui::Checkbox(crypt_str("Enable"), &cfg::g_cfg.antiaim.fakelag);

					if (cfg::g_cfg.antiaim.fakelag)
					{
						ImGui::Combo(crypt_str("Fake Lag Type"), &cfg::g_cfg.antiaim.fakelag_type, fakelags, ARRAYSIZE(fakelags));
						ImGui::SliderInt(crypt_str("Limit"), &cfg::g_cfg.antiaim.fakelag_amount, 1, 14);

						draw_multicombo(crypt_str("Fake Lag Triggers"), cfg::g_cfg.antiaim.fakelag_enablers, lagstrigger, ARRAYSIZE(lagstrigger), preview);

						auto enabled_fakelag_triggers = false;

						for (auto i = 0; i < ARRAYSIZE(lagstrigger); i++)
							if (cfg::g_cfg.antiaim.fakelag_enablers[i])
								enabled_fakelag_triggers = true;

						if (enabled_fakelag_triggers)
							ImGui::SliderInt(crypt_str("Triggers Limit"), &cfg::g_cfg.antiaim.triggers_fakelag_amount, 1, 14);
					}
				}
				ImGui::EndMenuChild();
				ImGui::SetCursorPos({ 342 , 35 });
				ImGui::BeginChild("Fake", { 343, 250 - 35 }, false, ImGuiWindowFlags_NoBackground);
				{
					if (cfg::g_cfg.antiaim.enable)
					{
						ImGui::Combo(crypt_str("Desync"), &cfg::g_cfg.antiaim.desync, desync, ARRAYSIZE(desync));

						if (cfg::g_cfg.antiaim.desync)
						{
							ImGui::SliderInt(crypt_str("Desync Range"), &cfg::g_cfg.antiaim.desync_range, 0, 100);
							ImGui::SliderInt(crypt_str("Roll"), &cfg::g_cfg.antiaim.roll, 0, 45);

							ImGui::SetCursorPosX(10); ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2); ImGui::Text("Invert Desync");
							draw_keybind(crypt_str("Invert Desync"), &cfg::g_cfg.antiaim.flip_desync, crypt_str("##HOTKEY_INVERT_DESYNC"));
						}
					}

				}
				ImGui::EndMenuChild();
				ImGui::SetCursorPos({ 342 , 250 + 35 });
				ImGui::BeginChild("Misc", { 343, 250 - 35 + added }, false, ImGuiWindowFlags_NoBackground);
				{

					if (cfg::g_cfg.antiaim.enable)
					{
						ImGui::Checkbox(crypt_str("Infinite duck"), &cfg::g_cfg.misc.noduck);
						if (cfg::g_cfg.misc.noduck)
						{

							ImGui::SetCursorPosX(10); ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2); ImGui::Text(crypt_str("Fake Duck"));
							draw_keybind(crypt_str("fakeduck"), &cfg::g_cfg.misc.fakeduck_key, crypt_str("##FAKEDUCK__KEY"));

						}

						ImGui::SetCursorPosX(10); ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2); ImGui::Text(crypt_str("Slow Walk"));
						draw_keybind(crypt_str("##slowwalk_key"), &cfg::g_cfg.misc.slowwalk_key, crypt_str("##SLOWWALK__KEY"));


						if (cfg::g_cfg.misc.slowwalk_key.key > KEY_NONE && cfg::g_cfg.misc.slowwalk_key.key < KEY_MAX) {

							ImGui::Combo("Slow Walk Type", &cfg::g_cfg.misc.slowwalk_type, slowwalk_type, ARRAYSIZE(slowwalk_type));

							if (cfg::g_cfg.misc.slowwalk_type == 1)
							{
								ImGui::SliderFloat(crypt_str("Speed"), &cfg::g_cfg.misc.slowwalk_speed, 0.01, 0.5);
							}
						}

						ImGui::SetCursorPosX(10); ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2); ImGui::Text("Manual Backward");
						draw_keybind(crypt_str("Mback"), &cfg::g_cfg.antiaim.manual_back, crypt_str("##backman"));
						ImGui::SetCursorPosX(10); ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2); ImGui::Text("Manual Right");
						draw_keybind(crypt_str("Mright"), &cfg::g_cfg.antiaim.manual_right, crypt_str("##rightman"));
						ImGui::SetCursorPosX(10); ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2); ImGui::Text("Manual Left");
						draw_keybind(crypt_str("Mleft"), &cfg::g_cfg.antiaim.manual_left, crypt_str("##leftman"));

						ImGui::Checkbox(crypt_str("Freestanding"), &cfg::g_cfg.antiaim.freestand);
						draw_keybind(XorStr("FSbind"), &cfg::g_cfg.antiaim.freestand_key, XorStr("##fsbind"), true);

					}
				}
				ImGui::EndMenuChild();
			}break;
			case Players:
			{
				//auto player = players_section;

				static int local_tab = 0;
				static int enemyorteam_tab = 0;
				const char* local_chams_sel[] =
				{
					"Player",
					"Desync",
					"Arms",
					"Weapon",
					"Attachments"
				};
				const char* team_sel[] =
				{
					"Visible",
					"Hidden",
					"Backtrack"
				};
				const char* enemy_sel[] =
				{
					"Visible",
					"Hidden",
					"Backtrack",
					"Shot"
				};

				ImGui::SetCursorPos({ 5, 35 });
				ImGui::BeginChild("ESP", { 332, 465 + added }, false, ImGuiWindowFlags_NoBackground);
				{
					ImGui::Checkbox("Master Switch", &cfg::g_cfg.player.enable);
					ImGui::SliderFloat(XorStr("Dormant time"), &cfg::g_cfg.player.type[player].dormant_time, 0.0f, 15.0f);
					ImGui::Checkbox(crypt_str("Box"), &cfg::g_cfg.player.type[player].box);
					ImGui::ColorEdit(crypt_str("##boxcolor"), &cfg::g_cfg.player.type[player].box_color, true);

					ImGui::Checkbox(crypt_str("Name"), &cfg::g_cfg.player.type[player].name);
					ImGui::ColorEdit(crypt_str("##namecolor"), &cfg::g_cfg.player.type[player].name_color, true);

					ImGui::Checkbox(crypt_str("Health Bar"), &cfg::g_cfg.player.type[player].health);

					ImGui::Checkbox(crypt_str("Custom Health Color"), &cfg::g_cfg.player.type[player].custom_health_color);
					ImGui::ColorEdit(crypt_str("##healthcolor"), &cfg::g_cfg.player.type[player].health_color, true);
					ImGui::SetCursorPosX(10); ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2); ImGui::Text(XorStr("Secondary color"));
					ImGui::ColorEdit(crypt_str("##healthcolortwo"), &cfg::g_cfg.player.type[player].health_color_two, false);

					for (auto i = 0, j = 0; i < ARRAYSIZE(flags); i++)
					{
						if (cfg::g_cfg.player.type[player].flags[i])
						{
							if (j)
								preview += crypt_str(", ") + (std::string)flags[i];
							else
								preview = flags[i];

							j++;
						}
					}

					draw_multicombo(crypt_str("Flags"), cfg::g_cfg.player.type[player].flags, flags, ARRAYSIZE(flags), preview);
					draw_multicombo(crypt_str("Weapon"), cfg::g_cfg.player.type[player].weapon, weaponplayer, ARRAYSIZE(weaponplayer), preview);


					if (cfg::g_cfg.player.type[player].weapon[WEAPON_ICON] || cfg::g_cfg.player.type[player].weapon[WEAPON_TEXT])
					{

						ImGui::SetCursorPosX(10); ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2); ImGui::Text(crypt_str("Color"));
						ImGui::ColorEdit(crypt_str("##weapcolor"), &cfg::g_cfg.player.type[player].weapon_color, true);
					}

					ImGui::Checkbox(crypt_str("Skeleton"), &cfg::g_cfg.player.type[player].skeleton);
					ImGui::ColorEdit(crypt_str("##skeletoncolor"), &cfg::g_cfg.player.type[player].skeleton_color, true);

					ImGui::Checkbox(crypt_str("Ammo bar"), &cfg::g_cfg.player.type[player].ammo);
					ImGui::ColorEdit(crypt_str("##ammocolor"), &cfg::g_cfg.player.type[player].ammobar_color, true);

					ImGui::Checkbox(crypt_str("Glow"), &cfg::g_cfg.player.type[player].glow);
					ImGui::SameLine();
					ImGui::ColorEdit(crypt_str("##glowcolor"), &cfg::g_cfg.player.type[player].glow_color, true);

					ImGui::Checkbox(crypt_str("Footsteps"), &cfg::g_cfg.player.type[player].footsteps);
					ImGui::ColorEdit(crypt_str("##footstepscolor"), &cfg::g_cfg.player.type[player].footsteps_color, true);

					if (cfg::g_cfg.player.type[player].footsteps)
					{
						ImGui::SliderInt(crypt_str("Thickness"), &cfg::g_cfg.player.type[player].thickness, 1, 10);
						ImGui::SliderInt(crypt_str("Radius"), &cfg::g_cfg.player.type[player].radius, 50, 500);
					}

				}
				ImGui::EndMenuChild();

				ImGui::SetCursorPos({ 342 , 35 });
				ImGui::BeginChild("Chams", { 343, 250 - 35 }, false, ImGuiWindowFlags_NoBackground);
				{

					if (player == LOCAL)
					{

						ImGui::Combo(crypt_str("Selection"), &local_tab, local_chams_sel, ARRAYSIZE(local_chams_sel));



						if (local_tab == 0)
						{
							//ImGui::Text("Player Color");
							ImGui::Checkbox(crypt_str("Enable Chams"), &cfg::g_cfg.player.type[player].enable_chams);
							ImGui::ColorEdit("##local.color", &cfg::g_cfg.player.type[player].chams_color, true);

							if (cfg::g_cfg.player.type[LOCAL].enable_chams)
								ImGui::Combo(crypt_str("Material"), &cfg::g_cfg.player.type[player].chams_type, chamstype, ARRAYSIZE(chamstype));

							ImGui::Checkbox(crypt_str("Overlay Chams"), &cfg::g_cfg.player.type[player].double_material);
							ImGui::ColorEdit(crypt_str("##Secondlocalcolor"), &cfg::g_cfg.player.type[player].double_material_color, true);

							if (cfg::g_cfg.player.type[player].double_material)
								ImGui::Combo(crypt_str("Overlay Chams Material"), &cfg::g_cfg.player.type[player].double_material_material, chamstype, ARRAYSIZE(chamstype));

							ImGui::Checkbox(crypt_str("Transparent When Scoped"), &cfg::g_cfg.player.transparency_in_scope);
							if (cfg::g_cfg.player.transparency_in_scope)
								ImGui::SliderInt(crypt_str("Transparency Amount"), &cfg::g_cfg.player.transparency_in_scope_amount, 0, 100, crypt_str("%d %%"));


						}
						else if (local_tab == 1)
						{
							ImGui::Checkbox(crypt_str("Desync Chams"), &cfg::g_cfg.player.fake_chams_enable);
							ImGui::ColorEdit(crypt_str("##desynccolor"), &cfg::g_cfg.player.fake_chams_color, true);

							if (cfg::g_cfg.player.fake_chams_enable)
							{
								ImGui::Checkbox(crypt_str("Overlayed Desync Chams"), &cfg::g_cfg.player.layered);
								ImGui::Checkbox(crypt_str("Visualise Lag"), &cfg::g_cfg.player.visualize_lag);

								ImGui::Combo(crypt_str("Desync Chams Material"), &cfg::g_cfg.player.fake_chams_type, chamstype, ARRAYSIZE(chamstype));

								ImGui::Checkbox(crypt_str("Overlay Desync Chams"), &cfg::g_cfg.player.fake_double_material);
								ImGui::ColorEdit(crypt_str("##second.desync.color"), &cfg::g_cfg.player.fake_double_material_color, true);

								if (cfg::g_cfg.player.fake_double_material)
									ImGui::Combo(crypt_str("Overlay Desync Chams Material"), &cfg::g_cfg.player.fake_double_material_material, chamstype, ARRAYSIZE(chamstype));
							}
						}
						else if (local_tab == 2)
						{
							ImGui::Checkbox(crypt_str("Arm Chams"), &cfg::g_cfg.visuals.arms_chams);
							ImGui::ColorEdit("##armscolor", &cfg::g_cfg.visuals.arms_chams_color, true);

							if (cfg::g_cfg.visuals.arms_chams)
								ImGui::Combo(crypt_str("Arm Chams Material"), &cfg::g_cfg.visuals.arms_chams_type, chamstype, ARRAYSIZE(chamstype));

							ImGui::Checkbox(crypt_str("Overlay Arm Chams"), &cfg::g_cfg.visuals.arms_double_material);
							ImGui::ColorEdit(crypt_str("##Secondmaterialarmscolor"), &cfg::g_cfg.visuals.arms_double_material_color, true);

							if (cfg::g_cfg.visuals.arms_double_material)
								ImGui::Combo(crypt_str("Overlay Arm Chams Material"), &cfg::g_cfg.visuals.arms_double_material_material, chamstype, ARRAYSIZE(chamstype));


						}
						else if (local_tab == 3)
						{

							ImGui::Checkbox(crypt_str("Weapon Chams"), &cfg::g_cfg.visuals.weapon_chams);
							ImGui::ColorEdit(crypt_str("##weaponchamscolor"), &cfg::g_cfg.visuals.weapon_chams_color, true);

							if (cfg::g_cfg.visuals.weapon_chams)
								ImGui::Combo(crypt_str("Weapon Chams material"), &cfg::g_cfg.visuals.weapon_chams_type, chamstype, ARRAYSIZE(chamstype));

							ImGui::Checkbox(crypt_str("Overlay Weapon Chams"), &cfg::g_cfg.visuals.weapon_double_material);
							ImGui::ColorEdit(crypt_str("##Secondmaterialweaponcolor"), &cfg::g_cfg.visuals.weapon_double_material_color, true);

							if (cfg::g_cfg.visuals.weapon_double_material)
								ImGui::Combo(crypt_str("Overlay Weapon Chams Material"), &cfg::g_cfg.visuals.weapon_double_material_material, chamstype, ARRAYSIZE(chamstype));
						}

					}
					else
					{
						if (player == ENEMY)
						    ImGui::Combo(crypt_str("Selection"), &enemyorteam_tab, enemy_sel, ARRAYSIZE(enemy_sel));
						else
							ImGui::Combo(crypt_str("Selection"), &enemyorteam_tab, team_sel, ARRAYSIZE(team_sel));


						if (enemyorteam_tab == 0)
						{
							ImGui::Checkbox(crypt_str("Enable Visible Chams"), &cfg::g_cfg.player.type[player].enable_chams);
							ImGui::ColorEdit(crypt_str("##visiblecolor"), &cfg::g_cfg.player.type[player].chams_color, true);

							if (cfg::g_cfg.player.type[player].enable_chams)
								ImGui::Combo(crypt_str("Chams Material"), &cfg::g_cfg.player.type[player].chams_type, chamstype, ARRAYSIZE(chamstype));


							ImGui::Checkbox(crypt_str("Visible Overlay"), &cfg::g_cfg.player.type[player].double_material);
							ImGui::ColorEdit("##secondcolor", &cfg::g_cfg.player.type[player].double_material_color, true);

							if (cfg::g_cfg.player.type[player].double_material)
								ImGui::Combo(crypt_str("Visible Overlay Material"), &cfg::g_cfg.player.type[player].double_material_material, chamstype, ARRAYSIZE(chamstype));


						}
						else if (enemyorteam_tab == 1)
						{

							ImGui::Checkbox(crypt_str("Enable Hidden Chams"), &cfg::g_cfg.player.type[player].xqz_enable);
							ImGui::ColorEdit(crypt_str("##xqzcolor"), &cfg::g_cfg.player.type[player].xqz_color, true);

							if (cfg::g_cfg.player.type[player].xqz_enable)
								ImGui::Combo(crypt_str("Hidden Chams Material"), &cfg::g_cfg.player.type[player].chams_xqz, chamstype, ARRAYSIZE(chamstype));


							ImGui::Checkbox(crypt_str("Hidden Overlay"), &cfg::g_cfg.player.type[player].double_material_xqz);
							ImGui::ColorEdit(crypt_str("##second.xqz.material"), &cfg::g_cfg.player.type[player].double_material_color_xqz, true);

							if (cfg::g_cfg.player.type[player].double_material_xqz)
								ImGui::Combo(crypt_str("Hidden Overlay Material"), &cfg::g_cfg.player.type[player].double_material_material_xqz, chamstype, ARRAYSIZE(chamstype));



						}
						else if (enemyorteam_tab == 2)
						{

							ImGui::Checkbox(crypt_str("Backtrack Chams"), &cfg::g_cfg.player.backtrack_chams);
							ImGui::ColorEdit("##backtrack.color", &cfg::g_cfg.player.backtrack_chams_color, true);

							if (cfg::g_cfg.player.backtrack_chams)
								ImGui::Combo(crypt_str("Backtrack Chams Material"), &cfg::g_cfg.player.backtrack_chams_material, chamstype, ARRAYSIZE(chamstype));

						}
						else if (enemyorteam_tab == 3)
						{

							ImGui::Checkbox(crypt_str("Enable Shot Chams"), &cfg::g_cfg.player.type[player].shot_enable);
							ImGui::ColorEdit(crypt_str("##shotcolor"), &cfg::g_cfg.player.type[player].shot_color, true);

							if (cfg::g_cfg.player.type[player].xqz_enable)
								ImGui::Combo(crypt_str("Shot Chams Material"), &cfg::g_cfg.player.type[player].chams_shot, chamstype, ARRAYSIZE(chamstype));


							ImGui::Checkbox(crypt_str("Shot Overlay"), &cfg::g_cfg.player.type[player].double_material_shot);
							ImGui::ColorEdit(crypt_str("##second.shot.material"), &cfg::g_cfg.player.type[player].double_material_color_shot, true);

							if (cfg::g_cfg.player.type[player].double_material_shot)
								ImGui::Combo(crypt_str("Shot Overlay Material"), &cfg::g_cfg.player.type[player].double_material_material_shot, chamstype, ARRAYSIZE(chamstype));

							ImGui::SliderFloat(crypt_str("Shot time"), &cfg::g_cfg.player.type[player].shot_time, 0.0f, 5.0f);

						}
					}


				}
				ImGui::EndMenuChild();


				ImGui::SetCursorPos({ 342 , 250 + 35 });
				ImGui::BeginChild("Other", { 343, 250 - 35 + added }, false, ImGuiWindowFlags_NoBackground);
				{
					if (player == ENEMY)
					{
						ImGui::Checkbox(crypt_str("FOV Arrows"), &cfg::g_cfg.player.arrows);
						ImGui::ColorEdit(crypt_str("##arrowscolor"), &cfg::g_cfg.player.arrows_color, true);

						if (cfg::g_cfg.player.arrows)
						{
							ImGui::SliderInt(crypt_str("Arrow Distance"), &cfg::g_cfg.player.distance, 5, 100);
							ImGui::SliderInt(crypt_str("Arrow Size"), &cfg::g_cfg.player.size, 5, 30);
						}
					}

					if (cfg::g_cfg.ragebot.enable)
					{
						ImGui::Checkbox(crypt_str("Aimbot Points"), &cfg::g_cfg.player.show_multi_points);
						ImGui::ColorEdit(crypt_str("##aimcolors"), &cfg::g_cfg.player.show_multi_points_color, true);
					}

					ImGui::Checkbox(crypt_str("Shot Capsule"), &cfg::g_cfg.player.lag_hitbox);
					ImGui::ColorEdit(crypt_str("##hitcolor"), &cfg::g_cfg.player.lag_hitbox_color, true);
					

				}
				ImGui::EndMenuChild();

			}break;
			case World:
			{

				ImGui::SetCursorPos({ 5, 35 });
				ImGui::BeginChild("View", { 332, 250 - 35 }, false, ImGuiWindowFlags_NoBackground);
				{
					ImGui::Checkbox("Master Switch", &cfg::g_cfg.visuals.enable);


					draw_multicombo(crypt_str("Visual Removals"), cfg::g_cfg.visuals.removals, removals, ARRAYSIZE(removals), CMenu::get().preview);

					ImGui::SliderInt(crypt_str("Fov"), &cfg::g_cfg.visuals.fov, 0, 75);

					ImGui::SliderInt(crypt_str("Viewmodel Fov"), &cfg::g_cfg.visuals.viewmodel_fov, -10, 20);

					ImGui::SliderInt(crypt_str("Viewmodel X"), &cfg::g_cfg.visuals.viewmodel_x, -30, 30);

					ImGui::SliderInt(crypt_str("Viewmodel Y"), &cfg::g_cfg.visuals.viewmodel_y, -30, 30);

					ImGui::SliderInt(crypt_str("Viewmodel Z"), &cfg::g_cfg.visuals.viewmodel_z, -30, 30);

					ImGui::SetCursorPosX(10); ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2); ImGui::Text("Third Person"); draw_keybind(crypt_str("##Thirdperson"), &cfg::g_cfg.misc.thirdperson_toggle, crypt_str("##TPKEY__HOTKEY"));
					if (cfg::g_cfg.misc.thirdperson_toggle.key > KEY_NONE && cfg::g_cfg.misc.thirdperson_toggle.key < KEY_MAX) {
						ImGui::SliderInt(crypt_str("Thirdperson Distance"), &cfg::g_cfg.misc.thirdperson_distance, 50, 300);

					}
				}
				ImGui::EndMenuChild();
				ImGui::SetCursorPos({ 5, 250 + 35 });
				ImGui::BeginChild("Modulation", { 332, 250 - 35 + added }, false, ImGuiWindowFlags_NoBackground);
				{
					ImGui::Checkbox(crypt_str("Night Mode"), &cfg::g_cfg.visuals.nightmode);
					if (cfg::g_cfg.visuals.nightmode)
						ImGui::SliderInt(crypt_str("Night mode amount"), &cfg::g_cfg.visuals.nightmode_amount, 1, 100);

					ImGui::Checkbox(crypt_str("Ambient lighting"), &cfg::g_cfg.visuals.ambient_lighting);
					ImGui::ColorEdit("##worldcolor", &cfg::g_cfg.visuals.world_color, true);

					if (cfg::g_cfg.visuals.nightmode || cfg::g_cfg.visuals.ambient_lighting)
					    ImGui::SliderInt(crypt_str("Prop alpha"), &cfg::g_cfg.visuals.prop_alpha, 0, 255);

					//ImGui::Checkbox(crypt_str("Sunset mode"), &cfg::g_cfg.visuals.sunset_mode);

					ImGui::Combo(crypt_str("Skybox"), &cfg::g_cfg.visuals.skybox, skybox, ARRAYSIZE(skybox));

					if (cfg::g_cfg.visuals.skybox == 21)
					{
						static char sky_custom[124] = "";

						if (!cfg::g_cfg.visuals.custom_skybox.empty())
							strcpy_s(sky_custom, sizeof(sky_custom), cfg::g_cfg.visuals.custom_skybox.c_str());

						ImGui::SetCursorPosX(10); ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2); ImGui::Text(crypt_str("Custom Skybox Name:"), true);

						if (ImGui::InputText(crypt_str("##customsky"), sky_custom, sizeof(sky_custom)), ImGuiInputTextFlags_EnterReturnsTrue)
							cfg::g_cfg.visuals.custom_skybox = sky_custom;

					}

					ImGui::SetCursorPosX(10); ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2); ImGui::Text(crypt_str("Custom Skybox Color"));
					ImGui::ColorEdit(crypt_str("##skyboxcolor"), &cfg::g_cfg.visuals.skybox_color);



					/*ImGui::Checkbox(crypt_str("Ambient Modulation"), &cfg::g_cfg.visuals.world_modulation);
					if (cfg::g_cfg.visuals.world_modulation)
					{
						ImGui::SliderFloat(crypt_str("Bloom"), &cfg::g_cfg.visuals.bloom, 0.0f, 500.0f);
						ImGui::SliderFloat(crypt_str("Ambience"), &cfg::g_cfg.visuals.ambient, 0.0f, 250.0f);
						ImGui::SliderFloat(crypt_str("Exposure"), &cfg::g_cfg.visuals.exposure, 0.0f, 2000.0f);
					}*/

					ImGui::Checkbox(crypt_str("Fog Modulation"), &cfg::g_cfg.visuals.fog);
					ImGui::SameLine();
					ImGui::ColorEdit(crypt_str("##fogcolor"), &cfg::g_cfg.visuals.fog_color, true);
					if (cfg::g_cfg.visuals.fog) {
						ImGui::SliderInt(crypt_str("Distance"), &cfg::g_cfg.visuals.fog_distance, 0, 2000);
						ImGui::SliderInt(crypt_str("Density"), &cfg::g_cfg.visuals.fog_density, 0, 100);
					}

					ImGui::Checkbox(crypt_str("Precipitation"), &cfg::g_cfg.visuals.precipitation);
					ImGui::Combo(crypt_str("Precipitation type"), &cfg::g_cfg.visuals.precipitation_mode, weather, ARRAYSIZE(weather));
				}
				ImGui::EndMenuChild();
				ImGui::SetCursorPos({ 342 , 35 });
				ImGui::BeginChild("Grenades", { 343, 250 - 35 }, false, ImGuiWindowFlags_NoBackground);
				{
					ImGui::Checkbox(crypt_str("Grenade Prediction"), &cfg::g_cfg.visuals.grenade_prediction);
					ImGui::ColorEdit(crypt_str("##TRCRCLR"), &cfg::g_cfg.visuals.grenade_prediction_tracer_color, true);


					ImGui::Checkbox(crypt_str("Grenade Warning"), &cfg::g_cfg.visuals.grenade_proximity_warning);

					if (cfg::g_cfg.visuals.grenade_proximity_warning)
					{
						ImGui::SetCursorPosX(10); ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2); ImGui::Text(crypt_str("Warning Color"));
						ImGui::ColorEdit(crypt_str("##progressgrenpredcolor"), &cfg::g_cfg.visuals.grenade_proximity_warning_progress_color);
					}
					
					//broken
					/*
					draw_multicombo(crypt_str("Grenade ESP"), cfg::g_cfg.visuals.grenade_esp, proj_combo, ARRAYSIZE(proj_combo), preview);
					{
						if (cfg::g_cfg.visuals.grenade_esp[GRENADE_ICON] || cfg::g_cfg.visuals.grenade_esp[GRENADE_TEXT])
						{
							ImGui::SetCursorPosX(10); ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2); ImGui::Text(crypt_str("Icon/Text Color"));
					
							ImGui::ColorEdit(crypt_str("##projectcolor"), &cfg::g_cfg.visuals.projectiles_color);
						}

						if (cfg::g_cfg.visuals.grenade_esp[GRENADE_BOX])
						{
							ImGui::SetCursorPosX(10); ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2); ImGui::Text(crypt_str("Box Color"));
						
							ImGui::ColorEdit(crypt_str("##grenade_box_color"), &cfg::g_cfg.visuals.grenade_box_color);
						}

						if (cfg::g_cfg.visuals.grenade_esp[GRENADE_GLOW])
						{
							ImGui::SetCursorPosX(10); ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2); ImGui::Text(crypt_str("Glow Color"));
			
							ImGui::ColorEdit(crypt_str("##grenade_glow_color"), &cfg::g_cfg.visuals.grenade_glow_color);
						}
					}*/

				}
				ImGui::EndMenuChild();
				ImGui::SetCursorPos({ 342 , 250 + 35 });
				ImGui::BeginChild("Other", { 343, 250 - 35 + added }, false, ImGuiWindowFlags_NoBackground);
				{


					ImGui::Checkbox(crypt_str("Hit Marker"), &cfg::g_cfg.visuals.hitmarker);
					ImGui::Checkbox(crypt_str("Hit Effect"), &cfg::g_cfg.visuals.hiteffect);
					ImGui::Checkbox(crypt_str("Damage Marker"), &cfg::g_cfg.visuals.damage_marker);
					ImGui::ColorEdit("##dmgcolor", &cfg::g_cfg.visuals.damage_marker_color, true);
					ImGui::Checkbox(crypt_str("Force Crosshair"), &cfg::g_cfg.visuals.forcecrosshair);
					ImGui::Checkbox(crypt_str("Penetration Crosshair"), &cfg::g_cfg.visuals.penetration_crosshair);

					if (!cfg::g_cfg.visuals.removals[REMOVALS_RECOIL])
						ImGui::Checkbox(crypt_str("Recoil Crosshair"), &cfg::g_cfg.visuals.recoil_crosshair);

					ImGui::Checkbox(crypt_str("Client Bullet Impacts"), &cfg::g_cfg.visuals.client_bullet_impacts);
					ImGui::ColorEdit(crypt_str("##clientbulletimpacts"), &cfg::g_cfg.visuals.client_bullet_impacts_color, true);

					ImGui::Checkbox(crypt_str("Server Bullet Impacts"), &cfg::g_cfg.visuals.server_bullet_impacts);
					ImGui::ColorEdit(crypt_str("##serverbulletimpacts"), &cfg::g_cfg.visuals.server_bullet_impacts_color, true);

					ImGui::SliderFloat(crypt_str("Impact size"), &cfg::g_cfg.visuals.bullet_impacts_size, 0.5f, 5.f);

					ImGui::Checkbox(crypt_str("Local Bullet Tracers"), &cfg::g_cfg.visuals.bullet_tracer);
					ImGui::ColorEdit(crypt_str("##bulltracecolor"), &cfg::g_cfg.visuals.bullet_tracer_color, true);

					ImGui::Checkbox(crypt_str("Enemy Bullet Tracers"), &cfg::g_cfg.visuals.enemy_bullet_tracer);
					ImGui::ColorEdit(crypt_str("##enemybulltracecolor"), &cfg::g_cfg.visuals.enemy_bullet_tracer_color, true);

					ImGui::Combo(crypt_str("Tracer type"), &cfg::g_cfg.visuals.bullet_tracers_type, tracers_type, ARRAYSIZE(tracers_type));
					ImGui::SliderFloat(crypt_str("Tracer size"), &cfg::g_cfg.visuals.bullet_tracers_width, 0.1f, 10.0f);
				}
				ImGui::EndMenuChild();

			}break;
			case Miscellaneous:
			{
				ImGui::SetCursorPos({ 5, 35 });
				ImGui::BeginChild("General", { 332, 250 - 35 }, false, ImGuiWindowFlags_NoBackground);
				{

					ImGui::Checkbox(crypt_str("Anti-Untrusted"), &cfg::g_cfg.misc.anti_untrusted);

					ImGui::Checkbox(crypt_str("Rank Reveal"), &cfg::g_cfg.misc.rank_reveal);

					ImGui::Checkbox(crypt_str("Console Filter"), &cfg::g_cfg.misc.console_filter);

					ImGui::Checkbox(crypt_str("Gravity Ragdolls"), &cfg::g_cfg.misc.ragdolls);

					ImGui::Checkbox(crypt_str("Preserve Killfeed"), &cfg::g_cfg.visuals.preserve_killfeed);

					ImGui::Checkbox(crypt_str("Extended Backtrack"), &cfg::g_cfg.misc.extended_backtracking);

					if (cfg::g_cfg.misc.extended_backtracking)
						ImGui::SliderInt(crypt_str("Extended Backtrack Amount"), &cfg::g_cfg.misc.extended_backtracking_value, 1, 1000);

					ImGui::Checkbox(crypt_str("Aspect Ratio"), &cfg::g_cfg.misc.aspect_ratio);

					if (cfg::g_cfg.misc.aspect_ratio)
						ImGui::SliderFloat(crypt_str("Aspect Ratio Amount"), &cfg::g_cfg.misc.aspect_ratio_amount, .50f, 2.0f);




				}
				ImGui::EndMenuChild();
				ImGui::SetCursorPos({ 5, 250 + 35 });
				ImGui::BeginChild("Movement", { 332, 250 - 35 + added }, false, ImGuiWindowFlags_NoBackground);
				{


					ImGui::Checkbox(crypt_str("Auto Jump"), &cfg::g_cfg.misc.bunnyhop);
					ImGui::Combo(crypt_str("Auto Strafer"), &cfg::g_cfg.misc.airstrafe, strafes, ARRAYSIZE(strafes));
					ImGui::Checkbox(crypt_str("Fast Stop"), &cfg::g_cfg.misc.fast_stop);




					ImGui::SetCursorPosX(10); ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);	ImGui::Text(crypt_str("Auto Peek"));
					draw_keybind(crypt_str("Auto peek"), &cfg::g_cfg.misc.automatic_peek, crypt_str("##APHKEY"));


				}
				ImGui::EndMenuChild();
				ImGui::SetCursorPos({ 342 , 35 });
				ImGui::BeginChild("Extra", { 343, 250 - 35 }, false, ImGuiWindowFlags_NoBackground);
				{
					ImGui::Checkbox(crypt_str("Menu Color"), &cfg::g_cfg.menu.menu_color);
					ImGui::ColorEdit("##menucolmain", &cfg::g_cfg.menu.menu_color_col, true);
					//Quick fix
					if (!cfg::g_cfg.menu.menu_color)
						cfg::g_cfg.menu.menu_color_col = Color(0, 153, 255);

					ImGui::Checkbox(crypt_str("Watermark"), &cfg::g_cfg.menu.watermark);

					ImGui::Checkbox(crypt_str("Indicators"), &cfg::g_cfg.menu.keybind_list);

					ImGui::Checkbox(crypt_str("Clan Tag"), &cfg::g_cfg.misc.clantag_spammer);

					ImGui::Combo(crypt_str("Hitsound"), &cfg::g_cfg.visuals.hitsound, sounds, ARRAYSIZE(sounds));
				}
				ImGui::EndMenuChild();
				ImGui::SetCursorPos({ 342 , 250 + 35 });
				ImGui::BeginChild("Other", { 343, 250 - 35 + added }, false, ImGuiWindowFlags_NoBackground);
				{


					ImGui::Checkbox(crypt_str("Logs"), &cfg::g_cfg.misc.log_output);

					if (cfg::g_cfg.misc.log_output)
					{
						draw_multicombo(crypt_str("Logged events"), cfg::g_cfg.misc.events_to_log, events, ARRAYSIZE(events), preview);

						ImGui::SetCursorPosX(10); ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);	ImGui::Text(crypt_str("Log Color"));
						ImGui::ColorEdit("##logclr", &cfg::g_cfg.misc.log_color);

					}

					ImGui::Checkbox(crypt_str("Buy Bot"), &cfg::g_cfg.misc.buybot_enable);

					if (cfg::g_cfg.misc.buybot_enable)
					{
						ImGui::Combo(crypt_str("Primary"), &cfg::g_cfg.misc.buybot1, mainwep, ARRAYSIZE(mainwep));

						ImGui::Combo(crypt_str("Pistols"), &cfg::g_cfg.misc.buybot2, secwep, ARRAYSIZE(secwep));

						draw_multicombo(crypt_str("Other"), cfg::g_cfg.misc.buybot3, grenades, ARRAYSIZE(grenades), preview);
					}
				}
				ImGui::EndMenuChild();

			}break;
			case Settings:
			{
				static bool is_sure_check = false;
				static float started_think = 0;
				static std::string selected_name = "";
				static char config_name[30] = "\0";


				ImGui::SetCursorPos({ 5, 35 });
				ImGui::BeginChild("Config List", { 332, 465 + added }, false, ImGuiWindowFlags_NoBackground);
				{
					ImGui::InputText(crypt_str("Config Name"), config_name, sizeof(config_name));

					cfg_manager->config_files();
					files = cfg_manager->files;

					for (auto file : files) {
						bool is_selected = selected_name == file;

						if (ImGui::cfgtab(file.c_str(), is_selected, ImVec2(314, 35))) {
							selected_name = is_selected ? "" : file;


							is_sure_check = false;
							started_think = 0;
						}
					}

					if (selected_name.empty())
					{

						selected_name = "";

					}

				}
				ImGui::EndMenuChild();

				ImGui::SetCursorPos({ 342 , 35 });
				ImGui::BeginChild("Config Settings", { 343, 465 + added }, false, ImGuiWindowFlags_NoBackground);
				{

					if ((ImGui::CustomButton(crypt_str("Create new..."), crypt_str("##CreateConfig"), ImVec2(333, 30), true, CMenu::get().settingicons, "3")))
						add_config(config_name);

					if ((ImGui::CustomButton(crypt_str("Open Config Directory"), crypt_str("##OpenConfigDirectory"), ImVec2(333, 26), true, CMenu::get().settingicons, "2")))
					{

						std::string folder;

						auto get_dir = [&folder]() -> void
						{

							folder = crypt_str("luna\\configs");

							CreateDirectory(folder.c_str(), NULL);
						};

						get_dir();

						ShellExecute(NULL, crypt_str("open"), folder.c_str(), NULL, NULL, SW_SHOWNORMAL);


					}

					if (!selected_name.empty())
					{
						//Load confirmation
						{
							if (prenext_load && m_globals()->m_realtime < load_time + 3.f) {
								if ((ImGui::CustomButton(crypt_str(" Confirm?"), crypt_str("##ConfirmLoad"), ImVec2(333, 26), true, CMenu::get().settingicons, "5")) && !selected_name.empty()) {
									load_config(selected_name);
									prenext_load = false;
								}
							}
							else
								prenext_load = false;
						}

						//Load button
						{
							if (!prenext_load) {
								if ((ImGui::CustomButton(crypt_str(" Load"), crypt_str("##load"), ImVec2(333, 26), true, CMenu::get().settingicons, "5")) && !selected_name.empty()) {
									load_time = m_globals()->m_realtime;
									prenext_load = true;
									//load_config(selected_name);
								}
							}
						}

						//Save confirmation
						{
							if (prenext_save && m_globals()->m_realtime < save_time + 3.f) {

								if ((ImGui::CustomButton(crypt_str(" Confirm?"), crypt_str("##ConfirmSave"), ImVec2(333, 26), true, CMenu::get().settingicons, "4")) && !selected_name.empty())
								{
									save_config(selected_name);
									prenext_save = false;
								}
							}
							else
								prenext_save = false;
						}

						//Save button
						{
							if (!prenext_save) {
								if ((ImGui::CustomButton(crypt_str(" Save"), crypt_str("##Save"), ImVec2(333, 26), true, CMenu::get().settingicons, "4")) && !selected_name.empty())
								{
									save_time = m_globals()->m_realtime;
									prenext_save = true;
								}
							}
						}

						//Delete confirmation
						{
							if (prenext_delete && m_globals()->m_realtime < delete_time + 3.f) {
								if ((ImGui::CustomButton(crypt_str("Confirm?"), crypt_str("##ConfirmDelete"), ImVec2(333, 26), true, CMenu::get().settingicons, "7")) && !selected_name.empty())
								{
									prenext_delete = false;
									remove_config(selected_name); selected_name = "";
								}
							}
							else
								prenext_delete = false;
						}

						//Delete button
						{
							if (!prenext_delete) {
								if ((ImGui::CustomButton(crypt_str("Delete"), crypt_str("##Delete"), ImVec2(333, 26), true, CMenu::get().settingicons, "7")) && !selected_name.empty())
								{
									delete_time = m_globals()->m_realtime;
									prenext_delete = true;
								}
							}
						}

						if ((ImGui::CustomButton(crypt_str("Reset"), crypt_str("##Resetconfig"), ImVec2(333, 26), false, CMenu::get().settingicons, "")) && !selected_name.empty())
							cfg_manager->setup();

						if (!try_not_to_blue_screen) {
							if ((ImGui::CustomButton(crypt_str("Try not to miss!"), crypt_str("##challenge"), ImVec2(333, 26), false, CMenu::get().settingicons, "")) && !selected_name.empty())
							{
								try_not_to_blue_screen = true;
							}
						}
					}




				}
				ImGui::EndMenuChild();


			}break;
			case lua:
			{

				ImGui::SetCursorPos({ 5, 35 });
				ImGui::BeginChild("Scripts", { 332, 465 + added }, false, ImGuiWindowFlags_NoBackground);
				{
					static auto should_update = true;

					if (should_update)
					{
						should_update = false;
						scripts = c_lua::get().scripts;

						for (auto& current : scripts)
						{
							if (current.size() >= 5 && current.at(current.size() - 1) == 'c')
								current.erase(current.size() - 5, 5);
							else if (current.size() >= 4)
								current.erase(current.size() - 4, 4);
						}
					}

					if (ImGui::CustomButton("Open folder", "##LUAS__FOLDER", ImVec2(323, 26)))
					{
						std::string folder;

						auto get_dir = [&folder]() -> void
						{

							folder = crypt_str("luna\\scripts");

							CreateDirectory(folder.c_str(), NULL);
						};

						get_dir();

						ShellExecute(NULL, crypt_str("open"), folder.c_str(), NULL, NULL, SW_SHOWNORMAL);
					}

					ImGui::SetCursorPos({ 10, 76 });

					if (scripts.empty())
						ImGui::ListBoxConfigArray(crypt_str("Lua scripts [empty]"), &selected_script, scripts, 5);
					else
					{
						auto backup_scripts = scripts;

						for (auto& script : scripts)
						{
							auto script_id = c_lua::get().get_script_id(script + crypt_str(".lua"));

							if (script_id == -1)
								continue;

							if (c_lua::get().loaded.at(script_id))
								scripts.at(script_id) += crypt_str(" [loaded]");
						}

						ImGui::ListBoxConfigArray(crypt_str("Lua scripts"), &selected_script, scripts, 5);

						scripts = std::move(backup_scripts);
					}


					if (ImGui::CustomButton("Refresh", "##LUA__REFRESH", ImVec2(323, 26)))
					{
						c_lua::get().refresh_scripts();
						scripts = c_lua::get().scripts;

						if (selected_script >= scripts.size())
							selected_script = scripts.size() - 1; //-V103

						for (auto& current : scripts)
						{
							if (current.size() >= 5 && current.at(current.size() - 1) == 'c')
								current.erase(current.size() - 5, 5);
							else if (current.size() >= 4)
								current.erase(current.size() - 4, 4);
						}
					}

					if (ImGui::CustomButton("Load script", "##LUA__LOAD", ImVec2(323, 26)))
					{
						c_lua::get().load_script(selected_script);
						c_lua::get().refresh_scripts();

						scripts = c_lua::get().scripts;

						if (selected_script >= scripts.size())
							selected_script = scripts.size() - 1; //-V103

						for (auto& current : scripts)
						{
							if (current.size() >= 5 && current.at(current.size() - 1) == 'c')
								current.erase(current.size() - 5, 5);
							else if (current.size() >= 4)
								current.erase(current.size() - 4, 4);
						}
					}

					if (ImGui::CustomButton("Unload script", "##LUA__UNLOAD", ImVec2(323, 26)))
					{
						c_lua::get().unload_script(selected_script);
						c_lua::get().refresh_scripts();

						scripts = c_lua::get().scripts;

						if (selected_script >= scripts.size())
							selected_script = scripts.size() - 1; //-V103

						for (auto& current : scripts)
						{
							if (current.size() >= 5 && current.at(current.size() - 1) == 'c')
								current.erase(current.size() - 5, 5);
							else if (current.size() >= 4)
								current.erase(current.size() - 4, 4);
						}
					}

					if (ImGui::CustomButton("Reload all scripts", "##LUA__RELOADALL", ImVec2(323, 26)))
					{
						c_lua::get().reload_all_scripts();
						c_lua::get().refresh_scripts();

						scripts = c_lua::get().scripts;

						if (selected_script >= scripts.size())
							selected_script = scripts.size() - 1; //-V103

						for (auto& current : scripts)
						{
							if (current.size() >= 5 && current.at(current.size() - 1) == 'c')
								current.erase(current.size() - 5, 5);
							else if (current.size() >= 4)
								current.erase(current.size() - 4, 4);
						}
					}

					if (ImGui::CustomButton("Unload all scripts", "##LUA__UNLOADALL", ImVec2(323, 26)))
					{
						c_lua::get().unload_all_scripts();
						c_lua::get().refresh_scripts();

						scripts = c_lua::get().scripts;

						if (selected_script >= scripts.size())
							selected_script = scripts.size() - 1; //-V103

						for (auto& current : scripts)
						{
							if (current.size() >= 5 && current.at(current.size() - 1) == 'c')
								current.erase(current.size() - 5, 5);
							else if (current.size() >= 4)
								current.erase(current.size() - 4, 4);
						}
					}
				}
				ImGui::EndMenuChild();

				ImGui::SetCursorPos({ 342 , 35 });
				ImGui::BeginChild("Loaded Scripts", { 343, 465 + added }, false, ImGuiWindowFlags_NoBackground);
				{
					//Funny gay code!!!
					auto previous_check_box = false;

					for (auto& current : c_lua::get().scripts)
					{
						auto& items = c_lua::get().items.at(c_lua::get().get_script_id(current));

						for (auto& item : items)
						{
							std::string item_name;

							auto first_point = false;
							auto item_str = false;

							for (auto& c : item.first)
							{
								if (c == '.')
								{
									if (first_point)
									{
										item_str = true;
										continue;
									}
									else
										first_point = true;
								}

								if (item_str)
									item_name.push_back(c);
							}

							switch (item.second.type)
							{
							case NEXT_LINE:
								previous_check_box = false;
								break;
							case CHECK_BOX:
								previous_check_box = true;
								ImGui::Checkbox(item_name.c_str(), &item.second.check_box_value);
								break;
							case COMBO_BOX:
								previous_check_box = false;
								draw_combo(item_name.c_str(), item.second.combo_box_value, [](void* data, int idx, const char** out_text)
									{
										auto labels = (std::vector <std::string>*)data;
										*out_text = labels->at(idx).c_str(); //-V106
										return true;
									}, &item.second.combo_box_labels, item.second.combo_box_labels.size());
								break;
							case SLIDER_INT:
								previous_check_box = false;
								ImGui::SliderInt(item_name.c_str(), &item.second.slider_int_value, item.second.slider_int_min, item.second.slider_int_max);
								break;
							case SLIDER_FLOAT:
								previous_check_box = false;
								ImGui::SliderFloat(item_name.c_str(), &item.second.slider_float_value, item.second.slider_float_min, item.second.slider_float_max);
								break;
							case COLOR_PICKER:
								if (previous_check_box)
									previous_check_box = false;
								else
									ImGui::Text((item_name + ' ').c_str());

								ImGui::SameLine();
								ImGui::ColorEdit((crypt_str("##") + item_name).c_str(), &item.second.color_picker_value, ALPHA, true);
								break;
							}
						}
					}
				}
				ImGui::EndMenuChild();

			}break;
			case Skinchanger:
			{
				static char custom_name[124] = "";
				static int seed = 0;
				bool stat_trak = false;
				static float wear = 0.0f;

				ImGui::SetCursorPos({ 5, 35 });
				ImGui::BeginChild("General", { 332, 465 + added }, false, ImGuiWindowFlags_NoBackground);
				{
					static int next_id = -1;

					// we need to count our items in 1 line
					auto same_line_counter = 0;
					auto current_weapon = 0;

					if (CMenu::get().current_profile == -1)
					{
						for (auto i = 0; i < cfg::g_cfg.skins.skinChanger.size(); i++)
						{
							// do we need update our preview for some reasons?
							if (!all_skins[i])
							{
								cfg::g_cfg.skins.skinChanger.at(i).update();
							}

							next_id = i;
							CMenu::get().current_profile = next_id;

							//L::Print("Skin id logged: " + std::to_string(CMenu::get().current_profile));

							if (CMenu::get().current_profile == 35)
								CMenu::get().current_profile = 0;
						}
					}
					else
					{
						//ui::BeginGroup();
						//ui::PushItemWidth(260 * dpi_scale);
						auto& selected_entry = cfg::g_cfg.skins.skinChanger[CMenu::get().current_profile];
						selected_entry.itemIdIndex = CMenu::get().current_profile;

						// search input later
						static char search_skins[64] = "\0";
						static auto item_index = selected_entry.paint_kit_vector_index;


						// Enabled
						//ImGui::Checkbox("Enabled", &selected_entry.enabled);

						ImGui::Combo(crypt_str("Weapon"), &current_profile, skinchanger_weapons, ARRAYSIZE(skinchanger_weapons));

						if (!current_profile)
						{


							//ImGui::Text(crypt_str("Knife"));
							//ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5 * CMenu::get().dpi_scale);
							if (ImGui::Combo(crypt_str("Knife"), &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text)
								{
									*out_text = game_data::knife_names[idx].name;
									return true;
								}, nullptr, IM_ARRAYSIZE(game_data::knife_names)))
							{
								SkinChanger::scheduleHudUpdate();
							}
						}
						else if (current_profile == 1)
						{
							//ImGui::Text(crypt_str("Gloves"));
							//ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 5 * CMenu::get().dpi_scale);

							if (ImGui::Combo(crypt_str("Gloves"), &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text)
								{
									*out_text = game_data::glove_names[idx].name;
									return true;
								}, nullptr, IM_ARRAYSIZE(game_data::glove_names)))
							{
								item_index = 0; // set new generated paintkits element to 0;
								SkinChanger::scheduleHudUpdate();
							}
						}

						if (CMenu::get().current_profile != 1)
						{
							ImGui::SetCursorPosX(10); ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2); ImGui::Text(crypt_str("Search"));

							ImGui::InputText(crypt_str("##search"), search_skins, sizeof(search_skins));
						}

						auto main_kits = CMenu::get().current_profile == 1 ? SkinChanger::gloveKits : SkinChanger::skinKits;

						//Broken, kinda, does not pop up when you search the weapons name. also some items are named knife_... for some reason.
						//auto main_kits = CMenu::get().current_profile == 1 ? (cfg::g_cfg.skins.show_names ? SkinChanger::gloveKits2 : SkinChanger::gloveKits) : (cfg::g_cfg.skins.show_names ? SkinChanger::skinKits2 : SkinChanger::skinKits);
						
						auto display_index = 0;

						SkinChanger::displayKits = main_kits;

						// we dont need custom gloves
						if (CMenu::get().current_profile == 1)
						{
							for (auto i = 0; i < main_kits.size(); i++)
							{
								auto main_name = main_kits.at(i).name;

								for (auto i = 0; i < main_name.size(); i++)
									if (iswalpha((main_name.at(i))))
										main_name.at(i) = towlower(main_name.at(i));

								char search_name[64];

								if (!strcmp(game_data::glove_names[selected_entry.definition_override_vector_index].name, crypt_str("Hydra")))
									strcpy_s(search_name, sizeof(search_name), crypt_str("Bloodhound"));
								else
									strcpy_s(search_name, sizeof(search_name), game_data::glove_names[selected_entry.definition_override_vector_index].name);

								for (auto i = 0; i < sizeof(search_name); i++)
									if (iswalpha(search_name[i]))
										search_name[i] = towlower(search_name[i]);

								if (main_name.find(search_name) != std::string::npos)
								{
									SkinChanger::displayKits.at(display_index) = main_kits.at(i);
									display_index++;
								}
							}

							SkinChanger::displayKits.erase(SkinChanger::displayKits.begin() + display_index, SkinChanger::displayKits.end());
						}
						else
						{
							if (strcmp(search_skins, crypt_str(""))) //-V526
							{
								for (auto i = 0; i < main_kits.size(); i++)
								{

									//Setup skin names
									auto main_name = main_kits.at(i).name;

									for (auto i = 0; i < main_name.size(); i++)
											main_name.at(i) = towlower(main_name.at(i));

									//Setup search name
									char search_name[64];
									strcpy_s(search_name, sizeof(search_name), search_skins);

									for (auto i = 0; i < sizeof(search_name); i++)
											search_name[i] = towlower(search_name[i]);


									//Compare.
									if (!main_name.find(search_name))
									{
										SkinChanger::displayKits.at(display_index) = main_kits.at(i);
										display_index++;
									}
								}

								//Finish
								SkinChanger::displayKits.erase(SkinChanger::displayKits.begin() + display_index, SkinChanger::displayKits.end());
							}
							else
								item_index = selected_entry.paint_kit_vector_index;
						}
					    skip:

						ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
						if (!SkinChanger::displayKits.empty())
						{
							static float shitpad = 0;

							if (current_profile == 0)
								shitpad = 32;
							else if (current_profile == 1)
								shitpad = -32;
							else
								shitpad = -18;

							ImGui::SetCursorPos({ 10, 166 + shitpad });

							if (item_index < 0 || item_index > 20000)
							{
								L::Print(std::to_string(item_index));
								item_index = 0;
							}

							if (ImGui::ListBox(crypt_str("Skin List"), &item_index, [](void* data, int idx, const char** out_text) {
								while (SkinChanger::displayKits.at(idx).name.find(crypt_str("С‘")) != std::string::npos)
									SkinChanger::displayKits.at(idx).name.replace(SkinChanger::displayKits.at(idx).name.find(crypt_str("С‘")), 2, crypt_str("Рµ"));
								*out_text = SkinChanger::displayKits.at(idx).name.c_str();
								return true; }, nullptr, SkinChanger::displayKits.size(), SkinChanger::displayKits.size() > 9 ? 9 : SkinChanger::displayKits.size()))
							{

								SkinChanger::scheduleHudUpdate();

								auto i = 0;

								while (i < main_kits.size())
								{
									if (main_kits.at(i).id == SkinChanger::displayKits.at(item_index).id)
									{
										selected_entry.paint_kit_vector_index = i;
										break;
									}

									i++;
								}

							}


						}
						ImGui::PopStyleVar();

						//Broken, kinda, does not pop up when you search the weapons name. also some items are named knife_... for some reason.
						//ImGui::Checkbox(crypt_str("Show weapon names"), &cfg::g_cfg.skins.show_names);

						selected_entry.update();
					}
				}
				ImGui::EndMenuChild();

				ImGui::SetCursorPos({ 342 , 35 });
				ImGui::BeginChild("Other", { 343, 465 + added }, false, ImGuiWindowFlags_NoBackground);
				{
					auto& selected_entry = cfg::g_cfg.skins.skinChanger[CMenu::get().current_profile];
					selected_entry.itemIdIndex = CMenu::get().current_profile;

					if (ImGui::Checkbox(crypt_str("StatTrak"), &selected_entry.stat_trak))
						SkinChanger::scheduleHudUpdate();

					if (ImGui::SliderFloat(crypt_str("Wear"), &selected_entry.wear, 0.0f, 1.0f))
						SkinChanger::scheduleHudUpdate();

					ImGui::SetCursorPosX(10); ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2);  ImGui::Text("Seed");
					if (ImGui::InputInt(crypt_str("##Seed"), &selected_entry.seed, 1, 100))
						SkinChanger::scheduleHudUpdate();

					if (CMenu::get().current_profile != 1)
					{
						if (!cfg::g_cfg.skins.custom_name_tag[CMenu::get().current_profile].empty())
							strcpy_s(selected_entry.custom_name, sizeof(selected_entry.custom_name), cfg::g_cfg.skins.custom_name_tag[CMenu::get().current_profile].c_str());

						ImGui::SetCursorPosX(10); ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2); ImGui::Text(crypt_str("Name Tag"));
						ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);

						if (ImGui::InputText(crypt_str("##nametag"), selected_entry.custom_name, sizeof(selected_entry.custom_name)))
						{
							cfg::g_cfg.skins.custom_name_tag[CMenu::get().current_profile] = selected_entry.custom_name;
							SkinChanger::scheduleHudUpdate();
						}

						ImGui::PopStyleVar();
					}

					ImGui::Combo(crypt_str("Terrorist Player Model"), &cfg::g_cfg.player.player_model_t, player_models, ARRAYSIZE(player_models));
					ImGui::Combo(crypt_str("Counter-Terrorist Player Model"), &cfg::g_cfg.player.player_model_ct, player_models, ARRAYSIZE(player_models));

					if ((ImGui::CustomButton(crypt_str("Force Update"), crypt_str("##updater"), ImVec2(333, 26), false, CMenu::get().settingicons, "0")))
						SkinChanger::scheduleHudUpdate();
				}
				ImGui::EndMenuChild();
			}break;
			}

			ImGui::PopFont();
		}
		ImGui::EndChild();

	}
	ImGui::End();
	ImGui::PopStyleVar();
}