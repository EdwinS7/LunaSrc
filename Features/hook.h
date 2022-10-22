#include <ShlObj.h>
#include <ShlObj_core.h>
#include "includes.hpp"
#include "utils\ctx.hpp"
#include "utils\recv.h"
#include "utils\imports.h"
#include "SkinChanger\SkinChanger.h"
#include "logging.h"
#include "Resources/custom_sounds.hpp"

class hook : public singleton <hook>
{

public:
    void setup_netvars();
	void setup_skins();
    void setup_hooks();
	void setup_files();
	void setup_sigs();

	void Setup() {
        setup_sigs();
        setup_files();
        setup_sounds();
        setup_skins();
        setup_netvars();
        cfg_manager->setup();
        c_lua::get().initialize();
        key_binds::get().initialize_key_binds();
        setup_hooks();

        //Set default commands
        m_cvar()->FindVar("sv_cheats")->SetValue(1);
        m_cvar()->FindVar("fps_max")->SetValue(0);
        m_cvar()->FindVar("fps_max_menu")->SetValue(0);
	}
};