#include "includes.hpp"

void RegisterCallback(std::string name) {
    if (g_ctx.globals.loaded_script)
        for (auto current : c_lua::get().hooks.getHooks(name))
            current.func();
}