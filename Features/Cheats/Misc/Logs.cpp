// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "logs.h"
#include "..\..\utils\render.h"
#include <Features\logging.h>
#include "../ui.h"
#include <Features/Cheats/Visuals/OtherEsp.h>

float absolute_time() {
	return (float)(clock() / (float)1000.f);
}

std::map<std::string, std::string> event_to_normal =
{
	// Other
	{ "weapon_taser", "Zeus" },
	{ "item_kevlar", "Kevlar" },
	{ "item_defuser", "Defuse kit" },
	{ "item_assaultsuit", "Kevlar + Helmet" },

	// Pistols
	{ "weapon_p250", "P250" },
	{ "weapon_tec9", "TEC-9" },
	{ "weapon_cz75a", "CZ75A" },
	{ "weapon_glock", "Glock" },
	{ "weapon_elite", "Double-Berretas" },
	{ "weapon_deagle", "Desert-Eagle" },
	{ "weapon_hkp2000", "P2000" },
	{ "weapon_usp_silencer", "USP-S" },
	{ "weapon_revolver", "R8 Revolver" },
	{ "weapon_fiveseven", "Five-Seven" },

	// PP
	{ "weapon_mp9", "MP-9" },
	{ "weapon_mac10", "MAC-10" },
	{ "weapon_mp7", "MP-7" },
	{ "weapon_mp5sd", "MP5-SD" },
	{ "weapon_ump45", "UMP-45" },
	{ "weapon_p90", "P90" },
	{ "weapon_bizon", "PP-Bizon" },

	//rifles
	{ "weapon_famas", "FAMAS" },
	{ "weapon_m4a1_silencer", "M4A1-s" },
	{ "weapon_m4a1", "M4A1" },
	{ "weapon_ssg08", "SSG08" },
	{ "weapon_aug", "AUG" },
	{ "weapon_awp", "AWP" },
	{ "weapon_scar20", "SCAR20" },
	{ "weapon_galilar", "AR-Galil" },
	{ "weapon_ak47", "AK-47" },
	{ "weapon_sg556", "SG553" },
	{ "weapon_g3sg1", "G3SG1" },

	// Heavy
	{ "weapon_nova", "Nova" },
	{ "weapon_xm1014", "XM1014" },
	{ "weapon_sawedoff", "Sawed-Off" },
	{ "weapon_m249", "M249" },
	{ "weapon_negev", "Negev" },
	{ "weapon_mag7", "MAG-7" },

	// Grenades
	{ "weapon_flashbang", "Flash grenade" },
	{ "weapon_smokegrenade", "Smoke grenade" },
	{ "weapon_molotov", "Molotov" },
	{ "weapon_incgrenade", "Incereative grenade" },
	{ "weapon_decoy", "Decoy grenade" },
	{ "weapon_hegrenade", "HE Grenade" },
};

struct notify_t
{
	float life_ime_local;
	float life_ime;
	std::string type;
	std::string message;
	Color c_type;
	Color c_message;
	float x;
	float y = -15;
	float max_x;
};

std::deque<notify_t> notifications;

notify_t find_notify(std::string pre_text, std::string body)
{
	for (size_t i = 0; i < notifications.size(); i++)
		if (notifications[i].type == pre_text && notifications[i].message == body)
			return notifications[i];
	return notify_t();
}

void eventlogs::paint_traverse()
{
	if (cfg::g_cfg.menu.watermark) {
		std::string cheat_name = "LUNA ";
		std::string username = "| " + g_ctx.username;
		ImVec2 cheat_size = CMenu::get().logs_font->CalcTextSizeA(14.f, FLT_MAX, 0.0f, cheat_name.c_str());
		ImVec2 username_size = CMenu::get().logs_font->CalcTextSizeA(14.f, FLT_MAX, 0.0f, username.c_str());
		renderer::get().rect_filled_gradient(0.f, 16, cheat_size.x + username_size.x + 16, 1, Color(cfg::g_cfg.menu.menu_color_col.r(), cfg::g_cfg.menu.menu_color_col.g(), cfg::g_cfg.menu.menu_color_col.b(), 150), Color(cfg::g_cfg.menu.menu_color_col.r(), cfg::g_cfg.menu.menu_color_col.g(), cfg::g_cfg.menu.menu_color_col.b(), 0), GradientType::GRADIENT_HORIZONTAL);
		renderer::get().text(CMenu::get().logs_font, 3, 1, Color(cfg::g_cfg.menu.menu_color_col.r(), cfg::g_cfg.menu.menu_color_col.g(), cfg::g_cfg.menu.menu_color_col.b()), false, false, false, false, cheat_name.c_str());
		renderer::get().text(CMenu::get().logs_font, 3 + cheat_size.x, 1, Color(255, 255, 255), false, false, false, false, username.c_str());
	}

	if (notifications.empty())
		return;

	float last_y = 0;
	int x, y;
	m_engine()->GetScreenSize(x, y);
	for (size_t i = 0; i < notifications.size(); i++)
	{
		auto& notify = notifications.at(i);

		const auto pre = notify.type.c_str();
		const auto text = notify.message.c_str();
		ImVec2 textSize = CMenu::get().logs_font->CalcTextSizeA(14.f, FLT_MAX, 0.0f, "LUNA ");

		std::string all_text;
		all_text += pre;
		all_text += "";
		all_text += text;

		ImVec2 all_textSize = CMenu::get().logs_font->CalcTextSizeA(14.f, FLT_MAX, 0.0f, all_text.c_str());

		notify.y = math::lerp(notify.y, (i * 17.f), 0.05f);

		if (notify.y > y + 17) {
			continue;
		}

		if (util::epoch_time() - notify.life_ime_local > notify.life_ime)
		{
			if ((notify.x + all_textSize.x + 16) < 0) {
				notifications.erase(notifications.begin() + i);
				continue;
			}

			notify.max_x = all_textSize.x + 16;

			notify.x = math::lerp(notify.x, (notify.max_x * -1) - 10, 0.05f);

			int procent_x = (100 * (notify.max_x + notify.x)) / notify.max_x;

			auto opacity = int((255 / 100) * procent_x);

			if (procent_x >= 0 && procent_x <= 100)
			{
				notify.c_message = Color(notify.c_message);
				notify.c_message.SetAlpha(opacity);
				notify.c_type = Color(notify.c_type);
				notify.c_type.SetAlpha(opacity);
			}
			else
			{
				notify.c_message = Color(notify.c_message);
				notify.c_message.SetAlpha(255);
				notify.c_type = Color(notify.c_type);
				notify.c_type.SetAlpha(255);
			}
		}

		int add_watermark = cfg::g_cfg.menu.watermark ? 18 : 0;

		float box_w = (float)fabs(0 - (all_textSize.x + 16));
		auto main_colf = Color(39, 39, 39, 240);
		auto main_coll = Color(39, 39, 39, 0);
		//renderer::get().rect_filled_gradient(0.f, last_y + notify.y - 1, notify.x + all_textSize.x + 16, last_y + notify.y + all_textSize.y + 2, main_colf, main_coll, GradientType::GRADIENT_HORIZONTAL);
		auto main_colf2 = Color(39, 39, 39, 100);
		//renderer::get().rect_filled_gradient(0.f, last_y + notify.y - 1, notify.x + all_textSize.x + 16, last_y + notify.y + all_textSize.y + 2, main_colf2, main_coll, GradientType::GRADIENT_HORIZONTAL);
		//renderer::get().rect_filled_gradient(0.f, last_y + notify.y - 1, notify.x + all_textSize.x + 16, 1, Color(notify.c_type.r(), notify.c_type.g(), notify.c_type.b(), 150), Color(notify.c_type.r(), notify.c_type.g(), notify.c_type.b(), 0), GradientType::GRADIENT_HORIZONTAL);
		renderer::get().rect_filled_gradient(0.f, add_watermark + last_y + notify.y - 1 + all_textSize.y + 2, notify.x + all_textSize.x + 16, 1, Color(notify.c_type.r(), notify.c_type.g(), notify.c_type.b(), 150), Color(notify.c_type.r(), notify.c_type.g(), notify.c_type.b(), 0), GradientType::GRADIENT_HORIZONTAL);
		renderer::get().text(CMenu::get().logs_font, notify.x + 3, add_watermark + last_y + notify.y, Color(notify.c_type.r(), notify.c_type.g(), notify.c_type.b()), false, false, false, false, "LUNA");
		renderer::get().text(CMenu::get().logs_font, notify.x + 3 + textSize.x, add_watermark + last_y + notify.y, Color(notify.c_message.r(), notify.c_message.g(), notify.c_message.b()), false, false, false, false, text);

	}
}

void eventlogs::events(IGameEvent* event)
{
	static auto get_hitgroup_name = [](int hitgroup) -> std::string
	{
		switch (hitgroup)
		{
		case HITGROUP_HEAD:
			return crypt_str("head");
		case HITGROUP_CHEST:
			return crypt_str("chest");
		case HITGROUP_STOMACH:
			return crypt_str("stomach");
		case HITGROUP_LEFTARM:
			return crypt_str("left arm");
		case HITGROUP_RIGHTARM:
			return crypt_str("right arm");
		case HITGROUP_LEFTLEG:
			return crypt_str("left leg");
		case HITGROUP_RIGHTLEG:
			return crypt_str("right leg");
		default:
			return crypt_str("generic");
		}
	};

	if (cfg::g_cfg.visuals.hiteffect && !strcmp(event->GetName(), crypt_str("player_hurt"))) {
		auto userid = event->GetInt(crypt_str("userid")), attacker = event->GetInt(crypt_str("attacker"));

		if (!userid || !attacker)
			return;

		auto userid_id = m_engine()->GetPlayerForUserID(userid), attacker_id = m_engine()->GetPlayerForUserID(attacker); //-V807

		player_info_t userid_info, attacker_info;

		if (!m_engine()->GetPlayerInfo(userid_id, &userid_info))
			return;

		if (!m_engine()->GetPlayerInfo(attacker_id, &attacker_info))
			return;

		if (attacker_id != m_engine()->GetLocalPlayer())
			return;

		for (int i = 0; i < 15; i++) {
			iEffects()->Sparks(otheresp::get().hitmarker.point, 1, 1, &Vector(0, 0, 0));
		}
	}

	if (cfg::g_cfg.misc.events_to_log[EVENTLOG_HIT] && !strcmp(event->GetName(), crypt_str("player_hurt")))
	{
		auto userid = event->GetInt(crypt_str("userid")), attacker = event->GetInt(crypt_str("attacker"));

		if (!userid || !attacker)
			return;

		auto userid_id = m_engine()->GetPlayerForUserID(userid), attacker_id = m_engine()->GetPlayerForUserID(attacker); //-V807

		player_info_t userid_info, attacker_info;

		if (!m_engine()->GetPlayerInfo(userid_id, &userid_info))
			return;

		if (!m_engine()->GetPlayerInfo(attacker_id, &attacker_info))
			return;

		auto m_victim = static_cast<player_t*>(m_entitylist()->GetClientEntity(userid_id));

		std::stringstream ss;

		if (attacker_id == m_engine()->GetLocalPlayer() && userid_id != m_engine()->GetLocalPlayer())
		{
			ss << crypt_str("Hit ") << userid_info.szName << crypt_str(" in the ") << get_hitgroup_name(event->GetInt(crypt_str("hitgroup"))) << crypt_str(" for ") << event->GetInt(crypt_str("dmg_health"));
			event->GetInt("health") == 0 ? ss << " damage (dead)" : ss << " damage (" << event->GetInt("health") << " health remaining)";

			add(ss.str());
		}
		else if (userid_id == m_engine()->GetLocalPlayer() && attacker_id != m_engine()->GetLocalPlayer())
		{
			ss << crypt_str("Hurt by ") << attacker_info.szName << " in " << get_hitgroup_name(event->GetInt(crypt_str("hitgroup"))) << " for " << event->GetInt(crypt_str("dmg_health")) << " damage";

			add(ss.str());
		}
	}

	if (cfg::g_cfg.misc.events_to_log[EVENTLOG_ITEM_PURCHASES] && !strcmp(event->GetName(), crypt_str("item_purchase")))
	{
		auto userid = event->GetInt(crypt_str("userid"));

		if (!userid)
			return;

		auto userid_id = m_engine()->GetPlayerForUserID(userid);

		player_info_t userid_info;

		if (!m_engine()->GetPlayerInfo(userid_id, &userid_info))
			return;

		auto m_player = static_cast<player_t*>(m_entitylist()->GetClientEntity(userid_id));

		if (!g_ctx.local() || !m_player)
			return;

		if (g_ctx.local() == m_player)
			g_ctx.globals.should_buy = 0;

		if (m_player->m_iTeamNum() == g_ctx.local()->m_iTeamNum() && !m_cvar()->FindVar("mp_teammates_are_enemies")->GetInt() == 1)
			return;

		std::string weapon = event->GetString(crypt_str("weapon"));

		std::stringstream ss;
		ss << userid_info.szName << crypt_str(" purchased a ") << event_to_normal[weapon.c_str()];

		add(ss.str());
	}

	if (cfg::g_cfg.misc.events_to_log[EVENTLOG_BOMB] && !strcmp(event->GetName(), crypt_str("bomb_beginplant")))
	{
		auto userid = event->GetInt(crypt_str("userid"));

		if (!userid)
			return;

		auto userid_id = m_engine()->GetPlayerForUserID(userid);

		player_info_t userid_info;

		if (!m_engine()->GetPlayerInfo(userid_id, &userid_info))
			return;

		auto m_player = static_cast<player_t*>(m_entitylist()->GetClientEntity(userid_id));

		if (!m_player)
			return;

		std::stringstream ss;
		ss << userid_info.szName << crypt_str(" has began planting the bomb");

		add(ss.str());
	}

	if (cfg::g_cfg.misc.events_to_log[EVENTLOG_BOMB] && !strcmp(event->GetName(), crypt_str("bomb_begindefuse")))
	{
		auto userid = event->GetInt(crypt_str("userid"));

		if (!userid)
			return;

		auto userid_id = m_engine()->GetPlayerForUserID(userid);

		player_info_t userid_info;

		if (!m_engine()->GetPlayerInfo(userid_id, &userid_info))
			return;

		auto m_player = static_cast<player_t*>(m_entitylist()->GetClientEntity(userid_id));

		if (!m_player)
			return;

		std::stringstream ss;
		ss << userid_info.szName << crypt_str(" has began defusing the bomb ") << (event->GetBool(crypt_str("haskit")) ? crypt_str("with defuse kit") : crypt_str("without defuse kit"));

		add(ss.str());
	}
}

void notify(std::string text, Color color_pre, Color color_text = Color(255, 255, 255, 255), int life_time = 4700) {
	std::string type_buf;
	type_buf += "[";
	type_buf += "LUNA";
	type_buf += "]";

	notifications.push_front(notify_t{ static_cast<float>(util::epoch_time()), (float)life_time, type_buf, text, color_pre, color_text });
}

void eventlogs::add(std::string text, bool full_display)
{
	logs.emplace_front(loginfo_t(absolute_time(), text, cfg::g_cfg.misc.log_color));

	last_log = true;

	m_cvar()->ConsoleColorPrintf(Color(cfg::g_cfg.menu.menu_color_col.r(), cfg::g_cfg.menu.menu_color_col.g(), cfg::g_cfg.menu.menu_color_col.b()), crypt_str("LUNA | "));

	m_cvar()->ConsoleColorPrintf(Color(255, 255, 255), text.c_str());
	m_cvar()->ConsolePrintf(crypt_str("\n"));
	notify(text, Color(cfg::g_cfg.menu.menu_color_col.r(), cfg::g_cfg.menu.menu_color_col.g(), cfg::g_cfg.menu.menu_color_col.b()));
	//L::Print(text);
}

void eventlogs::shot_add(std::string begin_text, std::string missreason, std::string end_text, bool full_display, Color miss_color)
{
	logs.emplace_front(loginfo_t(absolute_time(), begin_text + missreason + end_text, cfg::g_cfg.misc.log_color));
	notify(begin_text + missreason + end_text, Color(cfg::g_cfg.menu.menu_color_col.r(), cfg::g_cfg.menu.menu_color_col.g(), cfg::g_cfg.menu.menu_color_col.b()));

	if (!full_display)
		return;

	last_log = true;

	m_cvar()->ConsoleColorPrintf(Color(cfg::g_cfg.menu.menu_color_col.r(), cfg::g_cfg.menu.menu_color_col.g(), cfg::g_cfg.menu.menu_color_col.b()), crypt_str("LUNA | "));

	m_cvar()->ConsoleColorPrintf(Color(255, 255, 255), begin_text.c_str());
	m_cvar()->ConsoleColorPrintf(miss_color, missreason.c_str());
	m_cvar()->ConsoleColorPrintf(Color(255, 255, 255), end_text.c_str());

	m_cvar()->ConsolePrintf(crypt_str("\n"));
}