// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "nightmode.h"
#include "WorldEsp.h"

std::vector <MaterialBackup> materials;

void nightmode::clear_stored_materials() 
{
	materials.clear();
}

void nightmode::modulate(MaterialHandle_t i, IMaterial* material, bool backup = false) 
{
	auto name = material->GetTextureGroupName();

	if (strstr(name, crypt_str("World")))
	{
		if (backup) 
			materials.emplace_back(MaterialBackup(i, material));

		if (cfg::g_cfg.visuals.ambient_lighting) {
			material->AlphaModulate((float)cfg::g_cfg.visuals.world_color.a() / 255.0f);
			material->ColorModulate((float)cfg::g_cfg.visuals.world_color.r() / 255.0f, (float)cfg::g_cfg.visuals.world_color.g() / 255.0f, (float)cfg::g_cfg.visuals.world_color.b() / 255.0f);
			return;
		}
		else if (cfg::g_cfg.visuals.nightmode) {
			int new_col = (100.f - (float)cfg::g_cfg.visuals.nightmode_amount) * 2.55f;

			material->AlphaModulate(1.f);
			material->ColorModulate((255.f - new_col) / 255.f, (255.f - new_col) / 255.f, (255.f - new_col) / 255.f);
			return;
		}
	}
	else if (strstr(name, crypt_str("StaticProp")))
	{
		if (backup) 
			materials.emplace_back(MaterialBackup(i, material));

		if (cfg::g_cfg.visuals.ambient_lighting) {
			material->AlphaModulate((float)cfg::g_cfg.visuals.prop_alpha / 255.0f);
			material->ColorModulate((float)cfg::g_cfg.visuals.world_color.r() / 255.0f, (float)cfg::g_cfg.visuals.world_color.g() / 255.0f, (float)cfg::g_cfg.visuals.world_color.b() / 255.0f);
			return;
		}
		else if (cfg::g_cfg.visuals.nightmode) {
			int new_col = (100 - cfg::g_cfg.visuals.nightmode_amount) * 2.55f;

			material->AlphaModulate((float)cfg::g_cfg.visuals.prop_alpha / 255.0f);
			material->ColorModulate((255.f - new_col) / 255.f, (255.f - new_col) / 255.f, (255.f - new_col) / 255.f);
			return;
		}
	}
}

void nightmode::apply()
{
	if (!materials.empty())
	{
		for (auto i = 0; i < (int)materials.size(); i++) //-V202
			modulate(materials[i].handle, materials[i].material);

		return;
	}

	materials.clear();
	auto materialsystem = m_materialsystem();

	for (auto i = materialsystem->FirstMaterial(); i != materialsystem->InvalidMaterial(); i = materialsystem->NextMaterial(i))
	{
		auto material = materialsystem->GetMaterial(i);

		if (!material)
			continue;

		if (material->IsErrorMaterial())
			continue;

		modulate(i, material, true);
	}
}

void nightmode::remove() 
{
	for (auto i = 0; i < materials.size(); i++)
	{
		if (!materials[i].material)
			continue;

		if (materials[i].material->IsErrorMaterial())
			continue;

		materials[i].restore();
		materials[i].material->Refresh();
	}

	materials.clear();
}

//Holy retard but whatever.
void nightmode::nightmode_fix()
{
	static auto in_game = false;

	if (m_engine()->IsInGame() && !in_game)
	{
		in_game = true;

		g_ctx.globals.change_materials = true;
		worldesp::get().changed = true;

		static auto skybox = m_cvar()->FindVar(crypt_str("sv_skyname"));
		worldesp::get().backup_skybox = skybox->GetString();
		return;
	}
	else if (!m_engine()->IsInGame() && in_game)
		in_game = false;

	static auto player_enable = cfg::g_cfg.visuals.enable;

	if (player_enable != cfg::g_cfg.visuals.enable)
	{
		player_enable = cfg::g_cfg.visuals.enable;
		g_ctx.globals.change_materials = true;
		return;
	}

	static auto setting = cfg::g_cfg.visuals.nightmode;

	if (setting != cfg::g_cfg.visuals.nightmode)
	{
		setting = cfg::g_cfg.visuals.nightmode;
		g_ctx.globals.change_materials = true;
		return;
	}

	static auto setting2 = cfg::g_cfg.visuals.ambient_lighting;

	if (setting2 != cfg::g_cfg.visuals.ambient_lighting)
	{
		setting2 = cfg::g_cfg.visuals.ambient_lighting;
		g_ctx.globals.change_materials = true;
		return;
	}


	static auto setting_world = cfg::g_cfg.visuals.nightmode_amount;

	if (setting_world != cfg::g_cfg.visuals.nightmode_amount)
	{
		setting_world = cfg::g_cfg.visuals.nightmode_amount;
		g_ctx.globals.change_materials = true;
		return;
	}

	static auto setting_world2 = cfg::g_cfg.visuals.world_color;

	if (setting_world2 != cfg::g_cfg.visuals.world_color)
	{
		setting_world2 = cfg::g_cfg.visuals.world_color;
		g_ctx.globals.change_materials = true;
		return;
	}

	static auto setting_props = cfg::g_cfg.visuals.prop_alpha;

	if (setting_props != cfg::g_cfg.visuals.prop_alpha)
	{
		setting_props = cfg::g_cfg.visuals.prop_alpha;
		g_ctx.globals.change_materials = true;
		return;
	}
}