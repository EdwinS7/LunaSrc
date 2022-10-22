#pragma once

#include "..\sdk\interfaces\IInputSystem.hpp"
#include "..\utils\json.hpp"
#include "..\SkinChanger\SkinChanger.h"
#include "..\SkinChanger\item_definitions.hpp"

#include <limits>
#include <unordered_map>
#include <array>
#include <algorithm>
#include <vector>

struct item_setting
{
	void update()
	{
		itemId = game_data::weapon_names[itemIdIndex].definition_index;
		quality = game_data::quality_names[entity_quality_vector_index].index;

		const std::vector <SkinChanger::PaintKit>* kit_names;
		const game_data::weapon_name* defindex_names;

		if (itemId == GLOVE_T_SIDE)
		{
			kit_names = &SkinChanger::gloveKits;
			defindex_names = game_data::glove_names;
		}
		else
		{
			kit_names = &SkinChanger::skinKits;
			defindex_names = game_data::knife_names;
		}

		paintKit = (*kit_names)[paint_kit_vector_index].id;
		definition_override_index = defindex_names[definition_override_vector_index].definition_index;
		skin_name = (*kit_names)[paint_kit_vector_index].skin_name;
	}

	int itemIdIndex = 0;
	int itemId = 1;
	int entity_quality_vector_index = 0;
	int quality = 0;
	int paint_kit_vector_index = 0;
	int paintKit = 0;
	int definition_override_vector_index = 0;
	int definition_override_index = 0;
	int seed = 0;
	bool stat_trak = false;
	float wear = 0.0f;
	char custom_name[24] = "\0";
	std::string skin_name;
};

item_setting* get_by_definition_index(const int definition_index);

struct Player_list_data
{
	int i = -1;
	std::string name;

	Player_list_data()
	{
		i = -1;
		name.clear();
	}

	Player_list_data(int i, std::string name) //-V818
	{
		this->i = i;
		this->name = name; //-V820
	}
};

class Color;
class C_GroupBox;
class C_Tab;

using json = nlohmann::json;

class C_ConfigManager
{
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

	void add_item(void* pointer, const char* name, const std::string& type);
	void setup_item(int*, int, const std::string&);
	void setup_item(bool*, bool, const std::string&);
	void setup_item(float*, float, const std::string&);
	void setup_item(key_bind*, key_bind, const std::string&);
	void setup_item(Color*, Color, const std::string&);
	void setup_item(std::vector< int >*, int, const std::string&);
	void setup_item(std::vector< std::string >*, const std::string&);
	void setup_item(std::string*, const std::string&, const std::string&);

	std::vector <C_ConfigItem*> items;

	C_ConfigManager()
	{
		setup();
	};

	void setup();
	void save(std::string config);
	void load(std::string config, bool load_script_items);
	void remove(std::string config);
	std::vector<std::string> files;
	void config_files();
};

extern C_ConfigManager* cfg_manager;

enum
{
	FLAGS_MONEY,
	FLAGS_ARMOR,
	FLAGS_FAKE,
	FLAGS_HIT,
	FLAGS_KIT,
	FLAGS_SCOPED,
	FLAGS_FAKEDUCKING,
	FLAGS_PING,
	FLAGS_C4,
};

enum
{
	BUY_GRENADES,
	BUY_ARMOR,
	BUY_TASER,
	BUY_DEFUSER
};

enum
{
	WEAPON_ICON,
	WEAPON_TEXT,
	WEAPON_BOX,
	WEAPON_DISTANCE,
	WEAPON_GLOW,
	WEAPON_AMMO
};

enum
{
	GRENADE_ICON,
	GRENADE_TEXT,
	GRENADE_BOX,
	GRENADE_GLOW
};

enum
{
	PLAYER_CHAMS_VISIBLE,
	PLAYER_CHAMS_INVISIBLE
};

enum
{
	ENEMY,
	TEAM,
	LOCAL
};

enum
{
	REMOVALS_SCOPE,
	REMOVALS_ZOOM,
	REMOVALS_SMOKE,
	REMOVALS_FLASH,
	REMOVALS_RECOIL,
	REMOVALS_LANDING_BOB,
	REMOVALS_POSTPROCESSING,
	REMOVALS_FOGS,
	REMOVALS_SHADOWS,
	REMOVALS_DECALS,
	REMOVALS_SLEEVES
};

enum
{
	AUTOSTOP_BETWEEN_SHOTS,
	AUTOSTOP_FORCE_ACCURACY,
	AUTOSTOP_PREDICTIVE
};

enum
{
	AUTOSTOP_LETHAL,
	AUTOSTOP_VISIBLE,
	AUTOSTOP_CENTER
};

enum
{
	EVENTLOG_HIT,
	EVENTLOG_ITEM_PURCHASES,
	EVENTLOG_BOMB
};

enum
{
	FAKELAG_SLOW_WALK,
	FAKELAG_MOVE,
	FAKELAG_AIR,
	FAKELAG_PEEK
};

enum
{
	ANTIAIM_STAND,
	ANTIAIM_SLOW_WALK,
	ANTIAIM_MOVE,
	ANTIAIM_AIR
};

extern std::unordered_map <std::string, float[4]> colors;

namespace cfg {

	struct Config
	{
		struct {
			bool enable;
			bool zeus_bot;
			int zeus_bot_hitchance;
			bool knife_bot;
			bool autoshoot;
			bool extrapolation;
			bool prediction;
			int prediction_ticks;
			bool headshot_only;
			bool accurate_fd;
			bool dormant_aimbot;
			int charge_time;
			bool double_tap;
			key_bind double_tap_key;
			std::vector <int> exploit_modifiers;
			std::vector <int> double_tap_modifiers;
			key_bind safe_point_key;
			key_bind body_aim_key;
			key_bind damage_override_key;
			bool resolver;
			key_bind roll_correction_bind;

			struct {
				bool double_tap_hc;
				int double_tap_hitchance_amount;
				int hitchance_amount;
				int noscope_hitchance_amount;
				int min_dmg;
				int min_override_dmg;
				std::vector <int> hitboxes;
				float head_scale;
				float body_scale;
				std::vector <int> multipoints;
				bool prefer_safe_points;
				bool prefer_body_aim;
				int prefer_body_aim_mode;
				bool lethal_baim;
				bool autostop;
				bool autoscope;
				int autoscope_mode;
		
				std::vector <int> autostop_modifiers;
				std::vector <int> autostop_conditions;

				int selection_type;
			} weapon[9];
		} ragebot;

		struct {
			bool enabled;
			bool friendly_fire;
			bool autopistol;

			bool autoscope;
			bool unscope;
			bool sniper_in_zoom_only;
			bool backtrack;
			bool do_if_local_flashed;
			bool do_if_local_in_air;
			bool do_if_enemy_in_smoke;

			std::vector <int> disasblers;

			int backtrack_ticks;
			int autofire_delay;
			int auto_pistol_delay;
			key_bind autofire_key;
			key_bind key;

			struct {
				int priority;

				bool auto_stop;

				int fov_type;
				float fov;

				int smooth_type;
				float smooth;

				float silent_fov;

				int rcs_type;
				float rcs;
				float custom_rcs_smooth;
				float custom_rcs_fov;

				int awall_dmg;

				float target_switch_delay;
				int autofire_hitchance;
			} weapon[9];
		} legitbot;

		struct {
			bool enable;

			bool hide_shots;
			key_bind hide_shots_key;

			key_bind manual_back;
			key_bind manual_left;
			key_bind manual_right;
			key_bind flip_desync;

			bool fakelag;
			std::vector <int> fakelag_enablers;
			int fakelag_type;
			int fakelag_amount;
			int triggers_fakelag_amount;

			bool freestand;
			key_bind freestand_key;

			int pitch;
			int custom_pitch;
			int base_angle;
			int yaw_direction;
			int yaw;
			int range;
			int speed;
			int yaw_add_left;
			int yaw_add_right;

			int desync;
			int desync_range;
			int roll;
		} antiaim;

		struct {
			bool enable;
			bool arrows;
			Color arrows_color;
			int distance;
			int size;

			bool show_multi_points;
			Color show_multi_points_color;
			bool lag_hitbox;
			Color lag_hitbox_color;
			int player_model_t;
			int player_model_ct;

			int local_chams_type;

			bool fake_chams_enable;
			bool visualize_lag;
			bool layered;

			Color fake_chams_color;
			int fake_chams_type;

			bool fake_double_material;
			int fake_double_material_material;
			Color fake_double_material_color;

			bool backtrack_chams;
			int backtrack_chams_material;
			Color backtrack_chams_color;



			bool transparency_in_scope;
			int transparency_in_scope_amount;

			struct {
				bool flag;
				std::vector <int> flags;
				bool box;
				Color box_color;
				float dormant_time;
				bool name;
				Color name_color;
				bool health;
				bool custom_health_color;
				Color health_color;
				Color health_color_two;
				bool weapons;
				std::vector <int> weapon;
				Color weapon_color;
				bool skeleton;
				Color skeleton_color;
				bool ammo;
				Color ammobar_color;
				bool footsteps;
				Color footsteps_color;
				int thickness;
				int radius;
				bool glow;
				Color glow_color;
				int glow_type;

				// Default chams
				bool enable_chams;
				bool xqz_enable;
				bool shot_enable;
				int chams_type;
				int chams_xqz;
				int chams_shot;
				Color chams_color;
				Color xqz_color;
				Color shot_color;

				// Double normal chams
				bool double_material;
				int double_material_material;
				Color double_material_color;

				// Double xqz chams
				bool double_material_xqz;
				int double_material_material_xqz;
				Color double_material_color_xqz;

				// Double shot chams
				bool double_material_shot;
				int double_material_material_shot;
				Color double_material_color_shot;

				//Shot chams
				float shot_time;

				// Ragdoll chams
				bool ragdoll_chams;
				int ragdoll_chams_material;
				Color ragdoll_chams_color;
			} type[3];
		} player;

		struct {
			bool enable;
			std::vector <int> indicators;
			std::vector <int> removals;
			bool fix_zoom_sensivity;
			bool forcecrosshair;
			bool recoil_crosshair;
			bool penetration_crosshair;

			bool grenade_prediction;
			bool on_click;

			Color grenade_prediction_tracer_color;

			bool grenade_proximity_warning;
			Color grenade_proximity_warning_progress_color;

			bool projectiles;
			Color projectiles_color;
			bool molotov_timer;
			Color molotov_timer_color;
			bool smoke_timer;
			Color smoke_timer_color;
			bool bomb_timer;
			bool nightmode;
			int nightmode_amount;
			bool ambient_lighting;
			Color world_color;
			int prop_alpha;
			bool sunset_mode;
			int skybox;
			Color skybox_color;
			std::string custom_skybox;
			bool client_bullet_impacts;
			Color client_bullet_impacts_color;
			bool server_bullet_impacts;
			Color server_bullet_impacts_color;
			float bullet_impacts_size;
			bool bullet_tracer;
			Color bullet_tracer_color;
			bool enemy_bullet_tracer;
			Color enemy_bullet_tracer_color;
			int bullet_tracers_type;
			float bullet_tracers_width;
			bool preserve_killfeed;
			bool hitmarker;
			bool hiteffect;
			int hitsound;
			bool damage_marker;
			Color damage_marker_color;
			int fov;

			bool viewmodel_enable;
			int viewmodel_fov;
			int viewmodel_x;
			int viewmodel_y;
			int viewmodel_z;
			int viewmodel_roll;


			bool arms_chams;
			int arms_chams_type;
			Color arms_chams_color;

			bool arms_double_material;
			int arms_double_material_material;
			Color arms_double_material_color;

			bool weapon_chams;
			int weapon_chams_type;
			Color weapon_chams_color;

			bool weapon_double_material;
			int weapon_double_material_material;
			Color weapon_double_material_color;

			bool show_spread;
			Color show_spread_color;
			bool world_modulation;
			float bloom;
			float exposure;
			float ambient;
			bool fog;
			int fog_distance;
			int fog_density;
			Color fog_color;
			std::vector <int> weapon;
			Color box_color;
			Color weapon_color;
			Color weapon_glow_color;
			Color weapon_ammo_color;
			std::vector <int> grenade_esp;
			Color grenade_glow_color;
			Color grenade_box_color;

			bool precipitation;
			int precipitation_mode; //Rain & Snow.
		} visuals;

		struct {
			key_bind thirdperson_toggle;
			bool thirdperson_when_spectating;
			int thirdperson_distance;
			bool ragdolls;
			bool bunnyhop;
			int airstrafe;
			bool crouch_in_air;
			key_bind automatic_peek;
			Color automatic_peek_color;
			key_bind edge_jump;
			bool noduck;
			key_bind fakeduck_key;
			bool fast_stop;
			bool slidewalk;
			key_bind slowwalk_key;
			int slowwalk_type;
			float slowwalk_speed;
			bool log_output;
			bool debug_log;
			std::vector <int> events_to_log;
			bool console_filter;
			Color log_color;
			bool inventory_access;
			bool rank_reveal;
			bool auto_accept;
			bool clantag_spammer;
			bool buybot_enable;
			int buybot1;
			int buybot2;
			std::vector <int> buybot3;
			bool aspect_ratio;
			float aspect_ratio_amount;
			bool anti_untrusted;
			bool extended_backtracking;
			int extended_backtracking_value;
		} misc;

		struct {
			bool rare_animations;
			bool show_names;
			std::array <item_setting, 36> skinChanger;
			std::string custom_name_tag[36];
		} skins;

		struct {
			bool menu_color;
			Color menu_color_col;
			bool watermark;
			bool keybind_list;
			bool spectators_list;
		} menu;

		struct {
			bool developer_mode;
			bool allow_http;
			bool allow_file;
			std::vector <std::string> scripts;
		} scripts;

		struct {
			bool refreshing = false;
			std::vector <Player_list_data> players;

			bool white_list[65];
			bool high_priority[65];
			bool force_safe_points[65];
			bool force_body_aim[65];
			bool low_delta[65];
		} player_list;


		int selected_config;
		std::string new_config_name;
		std::string new_script_name;
	};

	extern Config g_cfg;
}

