#pragma once
#include "json.h"
#include <sdk/misc/Color.hpp>
#include "..\includes.hpp"


//std::unordered_map <std::string, float[4]> colorz;
using namespace std;

enum removals
{
	vis_recoil = 0,
	vis_smoke,
	flash,
	scope,
	zoom,
	post_processing,
	fog,
	shadow,
};

enum weap_type {
	def,
	scar,
	scout,
	_awp,
	rifles,
	pistols,
	heavy_pistols
};


enum chams_type
{
	enemy_visible,
	enemy_xqz,
	enemy_history,
	enemy_ragebot_shot,
	local_default,
	local_desync,
	local_arms,
	local_weapon,
	chams_max,
};

struct CWeaponConfig {



};

struct CGlobalVariables
{
	struct {


	} legitbot;


	struct
	{
		bool one;

	} ragebot;


	struct
	{

	} antiaim;
	struct
	{


	} visuals;

	struct
	{
		Color auto_peek_color;


	} misc;

	struct
	{


	} menu;

	struct
	{



	} menu_color;
};


extern CGlobalVariables vars;

enum binds_enum_t
{
	bind_override_dmg,
	bind_force_safepoint,
	bind_baim,
	bind_double_tap,
	bind_hide_shots,
	bind_aa_inverter,
	bind_manual_left,
	bind_manual_right,
	bind_manual_back,
	bind_manual_forward,
	bind_fake_duck,
	bind_slow_walk,
	bind_third_person,
	bind_peek_assist,

	bind_max
};

#pragma pack(push, 1)
struct c_bind {
	c_bind() {
		static_assert(sizeof(c_bind) <= 4, "sizeof c_bind > 4");

		active = false;
		type = 0;
		key = 0;
	}
	uint8_t type = 0;
	uint16_t key = 0;
	bool active = false;
};
#pragma pack(pop)

extern c_bind g_Binds[bind_max];

extern void CreateConfigFolder();


template<std::size_t strLen>
class _hiddenString
{
protected:
	static __forceinline constexpr std::uint64_t hash(std::uint64_t x, std::uint64_t sol)
	{
		x ^= 948274649985346773LLU ^ sol;
		x ^= x << 13;
		x ^= x >> 7;
		x ^= x << 17;
		return x;
	}
	mutable bool m_isDecrypted;
	mutable char m_str[strLen];
	std::uint64_t m_hashingSol;
public:
	__forceinline constexpr _hiddenString(const char(&str)[strLen], std::uint64_t hashingSol) : m_isDecrypted(false), m_str{ 0 }, m_hashingSol(hashingSol)
	{
		for (std::size_t i = 0; i < strLen; ++i)
			this->m_str[i] = str[i] ^ _hiddenString<strLen>::hash(i, this->m_hashingSol);
	}
	__forceinline constexpr operator std::string() const
	{
		if (!this->m_isDecrypted)
		{
			this->m_isDecrypted = true;
			for (std::size_t i = 0; i < strLen; ++i)
				this->m_str[i] ^= _hiddenString<strLen>::hash(i, this->m_hashingSol);
		}
		return { this->m_str, this->m_str + strLen - 1 };
	}
};

template<std::size_t strLen>
class _hiddenWString
{
protected:
	static __forceinline constexpr std::uint64_t hash(std::uint64_t x, std::uint64_t sol)
	{
		x ^= 948274649985346773LLU ^ sol;
		x ^= x << 13;
		x ^= x >> 7;
		x ^= x << 17;
		return x;
	}
	mutable bool m_isDecrypted;
	mutable wchar_t m_str[strLen];
	std::uint64_t m_hashingSol;
public:
	__forceinline constexpr _hiddenWString(const wchar_t(&str)[strLen], std::uint64_t hashingSol) : m_isDecrypted(false), m_str{ 0 }, m_hashingSol(hashingSol)
	{
		for (std::size_t i = 0; i < strLen; ++i)
			this->m_str[i] = str[i] ^ _hiddenWString<strLen>::hash(i, this->m_hashingSol);
	}
	__forceinline constexpr operator std::wstring() const
	{
		if (!this->m_isDecrypted)
		{
			this->m_isDecrypted = true;
			for (std::size_t i = 0; i < strLen; ++i)
				this->m_str[i] ^= _hiddenWString<strLen>::hash(i, this->m_hashingSol);
		}
		return { this->m_str, this->m_str + strLen - 1 };
	}
};


typedef Json::Value json_t;

#define sexstr(s) ([]() -> std::string \
{ \
	static constexpr _hiddenString hiddenStr { s, __COUNTER__ }; \
	return hiddenStr; \
})().c_str()

#define sstr(s) ([]() -> std::string \
{ \
	static constexpr _hiddenString hiddenStr { s, __COUNTER__ }; \
	return hiddenStr; \
})()

#define strw(ws) ([]() -> std::wstring \
{ \
	static constexpr _hiddenWString hiddenStr { ws, __COUNTER__ }; \
	return hiddenStr; \
})().c_str()



class CConfig
{
private:

	string GetModuleFilePath(HMODULE hModule);
	string GetModuleBaseDir(HMODULE hModule);

public:
	class C_ConfigItem
	{
	public:
		std::string name;
		void* pointer;
		std::string type;

		C_ConfigItem(std::string name, void* pointer, std::string type)  //-V818
		{
			this->name = name; //-V820
			this->pointer = pointer;
			this->type = type; //-V820
		}
	};


	struct preload_cfg {
		std::string created_by;
		std::string created_at;
		std::string last_modified_user;
		std::string last_modified_date;
		bool can_be_loaded;
	};
	std::vector <C_ConfigItem*> items;


	bool init = false;

	CConfig()
	{
		ConfigVarsSetup();
	}


	//std::string name;
	//void* pointer;
	//std::string type;

	//Color* pointer, const char* name, json_t json
	//C_ConfigItem(Color* pointer, const char* name, json_t json)  //-V818
	//{
		//this->name = name; //-V820
		//this->pointer = pointer;
		//this->type = type; //-V820
	//}

	void SaveColor(Color color, const char* name, json_t* json) {
		auto& j = *json;
		//j[name][0] = color.r();
		//j[name][1] = color.g();
		//j[name][2] = color.b();
		//j[name][3] = color.a();
		j[name][sexstr("red")] = color.r();
		j[name][sexstr("green")] = color.g();
		j[name][sexstr("blue")] = color.b();
		j[name][sexstr("alpha")] = color.a();

		/*
			colors[name][0] = (float)value.r() / 255.0f;
			colors[name][1] = (float)value.g() / 255.0f;
			colors[name][2] = (float)value.b() / 255.0f;
			colors[name][3] = (float)value.a() / 255.0f;

			add_item(pointer, name.c_str(), crypt_str("Color"));
			*pointer = value;
		*/
	}


	void SaveBind(c_bind* bind, const char* name, json_t* json) {
		auto& j = *json;
		j[name][sexstr("key")] = bind->key;
		j[name][sexstr("type")] = bind->type;
		j[name][sexstr("active")] = bind->active;
	}
	void LoadBool(bool* pointer, const char* name, json_t json) {
		if (json.isMember(name)) *pointer = json[name].asBool();
		//pointer = pointer;
	}
	void LoadInt(int* pointer, const char* name, json_t json) {
		if (json.isMember(name)) *pointer = json[name].asInt();
		//pointer = pointer;
	}
	void LoadUInt(unsigned int* pointer, const char* name, json_t json) {
		if (json.isMember(name)) *pointer = json[name].asUInt();
		//pointer = pointer;
	}
	void LoadFloat(float* pointer, const char* name, json_t json) {
		if (json.isMember(name)) *pointer = json[name].asFloat();
		//pointer = pointer;
	}

	void LoadColor2(Color* pointer, Color value, const char* name, json_t json)
	{
		const auto& location = json[name];

		colors[name][location[sexstr("red")].asInt()] = (float)value.r() / 255.0f;
		colors[name][location[sexstr("green")].asInt()] = (float)value.g() / 255.0f;
		colors[name][location[sexstr("blue")].asInt()] = (float)value.b() / 255.0f;
		colors[name][location[sexstr("alpha")].asInt()] = (float)value.a() / 255.0f;

		//add_item(pointer, name.c_str(), crypt_str("Color"));
		*pointer = value;
	}

	void LoadColor(Color* pointer, const char* name, json_t json)
	{
		if (!json.isMember(name))
			return;
		const auto& location = json[name];
		if (location.isMember(sexstr("red")) && location.isMember(sexstr("green"))
			&& location.isMember(sexstr("blue")) && location.isMember(sexstr("alpha"))) {

			//if (json.isMember(name))
			//	pointer = color.r();

			//pointer[0] = json[0].asInt();
			pointer->set_red(location[sexstr("red")].asInt());
			pointer->set_green(location[sexstr("green")].asInt());
			pointer->set_blue(location[sexstr("blue")].asInt());
			pointer->set_alpha(location[sexstr("alpha")].asInt());
			//pointer[0] = (float)value.r() / 255.0f;

			/*
				colors[name][0] = (float)value.r() / 255.0f;
				colors[name][1] = (float)value.g() / 255.0f;
				colors[name][2] = (float)value.b() / 255.0f;
				colors[name][3] = (float)value.a() / 255.0f;

				add_item(pointer, name.c_str(), crypt_str("Color"));
				*pointer = value;
			*/
			//pointer[0] = json[0].asInt();



	   //void SaveColor(Color color, const char* name, json_t* json) {
			//j[name][0] = color.r();
			//j[name][1] = color.g();
			//j[name][2] = color.b();
			//j[name][3] = color.a();
	
			//pointer->set_red(location[sexstr("r")].asInt());
			//pointer->set_green(location[sexstr("g")].asInt());
			//pointer->set_blue(location[sexstr("b")].asInt());
			//pointer->set_alpha(location[sexstr("a")].asInt());


		}


	}
	void LoadBind(c_bind* pointer, const char* name, json_t json) {
		if (!json.isMember(name))
			return;
		const auto& location = json[name];
		if (location.isMember(sexstr("key")) && location.isMember(sexstr("type"))) {
			pointer->key = location[sexstr("key")].asUInt();
			pointer->type = location[sexstr("type")].asUInt();
			pointer->active = location[sexstr("active")].asBool();
			pointer = pointer;
		}

	}


	std::vector <CConfig*> ConfigList;

	void add_item(Color* pointer, const char* name, json_t json);
	void ConfigVarsSetup();
	void PreLoad(string cfg_name, preload_cfg* p);
	void Save(string cfg_name, bool create = false);
	//void Delete(string cfg_name);
	void Load(string cfg_name);
	void Remove(string cfg_name);
};

extern CConfig* Config;

class c_style {
public:
	bool debug_mode;
	enum e_style_id : uint16_t {
		accented_color,

		window_background,
		window_background_hovered,

		child_background,
		child_background_hovered,

		button_color,
		button_hovered_color,
		button_inactive_color,
		button_holding_color,

		borders_color,
		borders_color_hovered,

		text_color,
		text_color_hovered,
		text_color_active,
		text_color_inactive,

		e_style_id_last
	};
	void init();
	void set_color(e_style_id id, const Color clr);
	Color& get_color(e_style_id id);


private:
	Color style[e_style_id_last];
};

extern c_style style;
