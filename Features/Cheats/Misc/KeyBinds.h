#pragma once

#include <unordered_map>
#include "..\..\sdk\interfaces\IInputSystem.hpp"
#include "..\..\utils\singleton.h"

enum key_bind_mode
{
	HOLD,
	TOGGLE,
	ALWAYS
};

struct key_bind
{
	ButtonCode_t key = KEY_NONE;
	key_bind_mode mode = HOLD;
	bool holding = false;

	key_bind(key_bind_mode mode = HOLD)
	{
		this->mode = mode;
	}

	key_bind(ButtonCode_t key, key_bind_mode mode)
	{
		this->key = key;
		this->mode = mode;
	}
};

class key_binds : public singleton <key_binds>
{
	std::unordered_map <int, bool> keys;
	std::unordered_map <int, key_bind_mode> mode;

	void update_key_bind(key_bind* key_bind, int key_bind_id);
public:
	void initialize_key_binds();
	void update_key_binds(); 
	bool get_key_bind_state(int key_bind_id);
	bool get_key_bind_state_lua(int key_bind_id);
	bool get_key_bind_mode(int key_bind_id);
};

enum KeyBinds {
	LEGIT_BOT_AUTOFIRE,
	LEGIT_BOT,
	DOUBLE_TAP,
	FORCE_SAFE,
	DAMAGE_OVERRIDE,
	HIDE_SHOTS,
	MANUAL_BACK,
	MANUAL_LEFT,
	MANUAL_RIGHT,
	FLIP_DESYNC,
	THIRDPERSON,
	AUTO_PEEK,
	EDGE_JUMP,
	FAKEDUCK,
	SLOWWALK,
	FORCE_BODY,
	ROLL_RESOLVER,
	FREESTANDING,
};