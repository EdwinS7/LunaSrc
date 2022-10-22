// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "KeyBinds.h"
#include "..\..\includes.hpp"
#include "misc.h"
#include "..\exploits\TickBase.h"

//This is retarded, fix it in the future. aka recode it
void key_binds::update_key_bind(key_bind* key_bind, int key_bind_id)
{
	auto is_button_down = util::is_button_down(key_bind->key);

	switch (key_bind->mode)
	{
	case HOLD:
		switch (key_bind_id)
		{
		case 2:
			if (exploit::get().recharging_double_tap)
				break;

			exploit::get().double_tap_key = is_button_down;

			if (exploit::get().double_tap_key && cfg::g_cfg.ragebot.double_tap_key.key != cfg::g_cfg.antiaim.hide_shots_key.key)
				exploit::get().hide_shots_key = false;

			break;
		case 12:
			exploit::get().hide_shots_key = is_button_down;

			if (exploit::get().hide_shots_key && cfg::g_cfg.antiaim.hide_shots_key.key != cfg::g_cfg.ragebot.double_tap_key.key)
				exploit::get().double_tap_key = false;

			break;
		case 13:
			if (is_button_down)
				antiaim::get().manual_side = SIDE_BACK;
			else if (antiaim::get().manual_side == SIDE_BACK)
				antiaim::get().manual_side = SIDE_NONE;

			break;
		case 14:
			if (is_button_down)
				antiaim::get().manual_side = SIDE_LEFT;
			else if (antiaim::get().manual_side == SIDE_LEFT)
				antiaim::get().manual_side = SIDE_NONE;

			break;
		case 15:
			if (is_button_down)
				antiaim::get().manual_side = SIDE_RIGHT;
			else if (antiaim::get().manual_side == SIDE_RIGHT)
				antiaim::get().manual_side = SIDE_NONE;

			break;
		default:
			keys[key_bind_id] = is_button_down;
			break;
		}
		
		break;
	case TOGGLE:
		if (!key_bind->holding && is_button_down)
		{
			switch (key_bind_id)
			{
			case 2:
				if (exploit::get().recharging_double_tap)
					break;

				exploit::get().double_tap_key = !exploit::get().double_tap_key;

				if (exploit::get().double_tap_key && cfg::g_cfg.ragebot.double_tap_key.key != cfg::g_cfg.antiaim.hide_shots_key.key)
					exploit::get().hide_shots_key = false;

				break;
			case 12:
				exploit::get().hide_shots_key = !exploit::get().hide_shots_key;

				if (exploit::get().hide_shots_key && cfg::g_cfg.antiaim.hide_shots_key.key != cfg::g_cfg.ragebot.double_tap_key.key)
					exploit::get().double_tap_key = false;

				break;
			case 13:
				if (antiaim::get().manual_side == SIDE_BACK)
					antiaim::get().manual_side = SIDE_NONE;
				else
					antiaim::get().manual_side = SIDE_BACK;

				break;
			case 14:
				if (antiaim::get().manual_side == SIDE_LEFT)
					antiaim::get().manual_side = SIDE_NONE;
				else
					antiaim::get().manual_side = SIDE_LEFT;

				break;
			case 15:
				if (antiaim::get().manual_side == SIDE_RIGHT)
					antiaim::get().manual_side = SIDE_NONE;
				else
					antiaim::get().manual_side = SIDE_RIGHT;

				break;
			default:
				keys[key_bind_id] = !keys[key_bind_id];
				break;
			}

			key_bind->holding = true;
		}
		else if (key_bind->holding && !is_button_down)
			key_bind->holding = false;

		break;
	case ALWAYS:
		switch (key_bind_id)
		{
		case 2:
			if (exploit::get().recharging_double_tap)
				break;

			exploit::get().double_tap_key = true;

			if (exploit::get().double_tap_key && cfg::g_cfg.ragebot.double_tap_key.key != cfg::g_cfg.antiaim.hide_shots_key.key)
				exploit::get().hide_shots_key = false;

			break;
		case 12:
			if (exploit::get().double_tap_key || exploit::get().recharging_double_tap) {
				exploit::get().hide_shots_key = false;
				break;
			}

			exploit::get().hide_shots_key = true;

			if (exploit::get().hide_shots_key && cfg::g_cfg.antiaim.hide_shots_key.key != cfg::g_cfg.ragebot.double_tap_key.key)
				exploit::get().double_tap_key = false;

			break;
		case 13:
			antiaim::get().manual_side = SIDE_BACK;

			break;
		case 14:
			antiaim::get().manual_side = SIDE_LEFT;

			break;
		case 15:
			antiaim::get().manual_side = SIDE_RIGHT;

			break;
		default:
			keys[key_bind_id] = true;
			break;
		}

		break;
	}

	mode[key_bind_id] = key_bind->mode;
}

//Set default bind states.
void key_binds::initialize_key_binds()
{
	for (auto i = 0; i < 23; i++)
	{
		keys[i] = false;

		if (i == 0 || i == 1)
			mode[i] = ALWAYS;
		else if (i == 2 || i > 12 || i != 21)
			mode[i] = TOGGLE;
		else
			mode[i] = HOLD;
	}
}

void key_binds::update_key_binds()
{
	update_key_bind(&cfg::g_cfg.legitbot.autofire_key, 0);
	update_key_bind(&cfg::g_cfg.legitbot.key, 1);
	update_key_bind(&cfg::g_cfg.ragebot.double_tap_key, 2);
	update_key_bind(&cfg::g_cfg.ragebot.safe_point_key, 3);
	update_key_bind(&cfg::g_cfg.ragebot.damage_override_key, 4);
	update_key_bind(&cfg::g_cfg.antiaim.hide_shots_key, 12);
	update_key_bind(&cfg::g_cfg.antiaim.manual_back, 13);
	update_key_bind(&cfg::g_cfg.antiaim.manual_left, 14);
	update_key_bind(&cfg::g_cfg.antiaim.manual_right, 15);
	update_key_bind(&cfg::g_cfg.antiaim.flip_desync, 16);
	update_key_bind(&cfg::g_cfg.misc.thirdperson_toggle, 17);
	update_key_bind(&cfg::g_cfg.misc.automatic_peek, 18);
	update_key_bind(&cfg::g_cfg.misc.edge_jump, 19);
	update_key_bind(&cfg::g_cfg.misc.fakeduck_key, 20);
	update_key_bind(&cfg::g_cfg.misc.slowwalk_key, 21);
	update_key_bind(&cfg::g_cfg.ragebot.body_aim_key, 22);
	update_key_bind(&cfg::g_cfg.ragebot.roll_correction_bind, 23);
	update_key_bind(&cfg::g_cfg.antiaim.freestand_key, 24);
}

bool key_binds::get_key_bind_state(int key_bind_id)
{
	return keys[key_bind_id];
}

bool key_binds::get_key_bind_state_lua(int key_bind_id)
{
	if (key_bind_id < 0 || key_bind_id > 22)
		return false;

	switch (key_bind_id)
	{
	case 2:
		return exploit::get().double_tap_key;
	case 4:
		if (g_ctx.globals.current_weapon < 0)
			return false;

		return keys[4 + g_ctx.globals.current_weapon];
	case 12:
		return exploit::get().hide_shots_key;
	case 13:
		return antiaim::get().manual_side == SIDE_BACK;
	case 14:
		return antiaim::get().manual_side == SIDE_LEFT;
	case 15:
		return antiaim::get().manual_side == SIDE_RIGHT;
	default:
		return keys[key_bind_id];
	}
}

bool key_binds::get_key_bind_mode(int key_bind_id)
{
	return mode[key_bind_id];
}