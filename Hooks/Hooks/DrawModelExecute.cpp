// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "..\hooks.hpp"
#include "..\..\Features\cheats\misc\fakelag.h"
#include "..\..\Features\cheats\lagcompensation\LocalAnimations.h"
#include "..\..\Features\cheats\visuals\PlayerEsp.h"

using DrawOthModel = int(__thiscall*)(ConVar*);
using DrawModelExecute_t = void(__thiscall*)(IVModelRender*, IMatRenderContext*, const DrawModelState_t&, const ModelRenderInfo_t&, matrix3x4_t*);

IMaterial* CreateMaterial(bool lit, const std::string& material_data)
{
	static auto created = 0;
	std::string type = lit ? crypt_str("VertexLitGeneric") : crypt_str("UnlitGeneric");

	auto matname = crypt_str("SL") + std::to_string(created);
	++created;

	auto keyValues = new KeyValues(matname.c_str());
	static auto key_values_address = util::FindSignature(CLIENT_DLL, crypt_str("55 8B EC 51 33 C0 C7 45"));

	using KeyValuesFn = void(__thiscall*)(void*, const char*, int, int);
	reinterpret_cast <KeyValuesFn> (key_values_address)(keyValues, type.c_str(), 0, 0);

	static auto load_from_buffer_address = util::FindSignature(CLIENT_DLL, crypt_str("55 8B EC 83 E4 F8 83 EC 34 53 8B 5D 0C 89"));
	using LoadFromBufferFn = void(__thiscall*)(void*, const char*, const char*, void*, const char*, void*);

	reinterpret_cast <LoadFromBufferFn> (load_from_buffer_address)(keyValues, matname.c_str(), material_data.c_str(), nullptr, nullptr, nullptr);

	auto material = m_materialsystem()->CreateMaterial(matname.c_str(), keyValues);
	material->IncrementReferenceCount();

	return material;
}


// this is to fix some entities are not being drawn in drawmodelexecute, as an example: ragdolls.
int __fastcall hooks::hkDrawModelStatsOverlay(ConVar* thisptr, void* edx)
{
	static auto original = r_drawmodelstatsoverlay_hook->get_func_address<DrawOthModel>(13);

	static int* return_check = (int*)util::FindSignature(CLIENT_DLL, crypt_str("85 C0 75 54 8B 0D ? ? ? ?"));

	if (_ReturnAddress() == return_check)
		return true;

	return original(thisptr);
}

static IMaterial* materials[] =
{
	// Default(Regular) [ 0 ]
	CreateMaterial(true, crypt_str(R"#("VertexLitGeneric"
			{
				"$basetexture"				"vgui/white"
				"$ignorez"					"0"
				"$envmap"					" "
				"$nofog"					"1"
				"$model"					"1"
				"$nocull"					"0"
				"$selfillum"				"1"
				"$halflambert"				"1"
				"$znearer"					"0"
				"$flat"						"0"
				"$wireframe"				"0"
			}
		)#")),

	// Flat [ 1 ]
	CreateMaterial(false, crypt_str(R"#("UnlitGeneric"
			{
				"$basetexture"				"vgui/white"
				"$ignorez"					"0"
				"$envmap"					" "
				"$nofog"					"1"
				"$model"					"1"
				"$nocull"					"0"
				"$selfillum"				"1"
				"$halflambert"				"1"
				"$znearer"					"0"
				"$flat"						"1"
				"$wireframe"				"0"
			}
		)#")),

	// Glass [ 2 ]
	m_materialsystem()->FindMaterial(XorStr("models/inventory_items/cologne_prediction/cologne_prediction_glass"), nullptr),

	// Velvet [ 3 ]
	m_materialsystem()->FindMaterial(crypt_str("models/inventory_items/trophy_majors/velvet"), nullptr),

	// Circuit [ 4 ]
	m_materialsystem()->FindMaterial(crypt_str("dev/glow_armsrace.vmt"), nullptr),

	// Glow [ 5 ]
	CreateMaterial(true, crypt_str(R"#("VertexLitGeneric" 
			{ 
				"$additive"					"1" 
				"$envmap"					"models/effects/cube_white" 
				"$envmaptint"				"[1 1 1]" 
				"$envmapfresnel"			"1" 
				"$envmapfresnelminmaxexp" 	"[0 1 2]" 
				"$alpha" 					"1" 
			}
		)#"))
};

void DrawChams(Color color, const ModelRenderInfo_t& info, int material_i, float alpha_modifier, bool xqz)
{
	auto model_entity = static_cast<player_t*>(m_entitylist()->GetClientEntity(info.entity_index));
	auto material = materials[material_i];
	auto alpha = (float)color.a() / 255.0f;

	float material_color[3] =
	{
		color[0] / 255.0f,
		color[1] / 255.0f,
		color[2] / 255.0f
	};

	m_renderview()->SetBlend(alpha * alpha_modifier);
	util::color_modulate(material_color, material);

	material->IncrementReferenceCount();
	material->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, xqz);

	m_modelrender()->ForcedMaterialOverride(material);
}

void __stdcall hooks::hkDrawModelExecute(IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* bone_to_world)
{
	static auto original_fn = modelrender_hook->get_func_address <DrawModelExecute_t>(21);
	g_ctx.local((player_t*)m_entitylist()->GetClientEntity(m_engine()->GetLocalPlayer()), true);

	if (!cfg::g_cfg.player.enable)
		return original_fn(m_modelrender(), ctx, state, info, bone_to_world);

	if (!info.pModel)
		return original_fn(m_modelrender(), ctx, state, info, bone_to_world);

	if (!info.pRenderable)
		return original_fn(m_modelrender(), ctx, state, info, bone_to_world);

	auto model_entity = static_cast<player_t*>(m_entitylist()->GetClientEntity(info.entity_index));
	auto name = m_modelinfo()->GetModelName(info.pModel);

	if (strstr(name, "sleeve") && cfg::g_cfg.visuals.removals[REMOVALS_SLEEVES])
		return;

	auto is_player = model_entity->is_player() && model_entity->is_alive() && (cfg::g_cfg.player.type[LOCAL].enable_chams || cfg::g_cfg.player.type[ENEMY].enable_chams || cfg::g_cfg.player.type[ENEMY].xqz_enable || cfg::g_cfg.player.type[TEAM].enable_chams || cfg::g_cfg.player.type[TEAM].xqz_enable || cfg::g_cfg.player.fake_chams_enable || cfg::g_cfg.player.backtrack_chams);
	auto is_weapon = strstr(name, "weapons/v_") && !strstr(name, "arms") && !strstr(name, "sleeve") && cfg::g_cfg.visuals.weapon_chams;
	auto is_arms = strstr(name, "arms") && cfg::g_cfg.visuals.arms_chams;
	auto is_sleeve = strstr(name, "sleeve") && cfg::g_cfg.visuals.arms_chams;

	//todo: fix this crash.
	if (m_modelrender()->IsForcedMaterialOverride() && !is_weapon && !is_arms && !is_sleeve)
		return original_fn(m_modelrender(), ctx, state, info, bone_to_world);

	m_renderview()->SetColorModulation(1.0f, 1.0f, 1.0f);

	if (!is_player && !is_weapon && !is_arms && !is_sleeve)
		return original_fn(m_modelrender(), ctx, state, info, bone_to_world);

	auto called_original = false;

	if (is_player)
	{
		auto type = ENEMY;

		if (model_entity == g_ctx.local())
			type = LOCAL;
		else if (model_entity->m_iTeamNum() == g_ctx.local()->m_iTeamNum() && !m_cvar()->FindVar("mp_teammates_are_enemies")->GetInt() == 1)
			type = TEAM;

		if (type == ENEMY)
		{
			auto alpha_modifier = playeresp::get().esp_alpha_fade[model_entity->EntIndex()];

			auto material = materials[cfg::g_cfg.player.type[ENEMY].chams_type];
			auto double_material = materials[5];

			if (cfg::g_cfg.player.type[ENEMY].enable_chams || cfg::g_cfg.player.type[ENEMY].xqz_enable)
			{
				auto alpha = (float)cfg::g_cfg.player.backtrack_chams_color.a() / 255.0f;;

				if (cfg::g_cfg.player.backtrack_chams)
				{
					auto backtrack_material = materials[cfg::g_cfg.player.backtrack_chams_material];

					if (backtrack_material && !backtrack_material->IsErrorMaterial())
					{
						matrix3x4_t matrix[MAXSTUDIOBONES];

						if (util::get_backtrack_matrix(model_entity, matrix))
						{
							DrawChams(cfg::g_cfg.player.backtrack_chams_color, info, cfg::g_cfg.player.backtrack_chams_material, alpha_modifier, true);

							original_fn(m_modelrender(), ctx, state, info, matrix);
							m_modelrender()->ForcedMaterialOverride(nullptr);
						}
					}
				}

				// Invisible chams
				if (cfg::g_cfg.player.type[ENEMY].xqz_enable)
				{
					DrawChams(cfg::g_cfg.player.type[ENEMY].xqz_color, info, cfg::g_cfg.player.type[ENEMY].chams_xqz, alpha_modifier, true);

					original_fn(m_modelrender(), ctx, state, info, bone_to_world);
					m_modelrender()->ForcedMaterialOverride(nullptr);

					if (cfg::g_cfg.player.type[ENEMY].double_material_xqz)
					{
						DrawChams(cfg::g_cfg.player.type[ENEMY].double_material_color_xqz, info, cfg::g_cfg.player.type[ENEMY].double_material_material_xqz, alpha_modifier, true);

						original_fn(m_modelrender(), ctx, state, info, bone_to_world);
						m_modelrender()->ForcedMaterialOverride(nullptr);
					}
				}

				// Visible chams
				if (cfg::g_cfg.player.type[ENEMY].enable_chams)
				{
					DrawChams(cfg::g_cfg.player.type[ENEMY].chams_color, info, cfg::g_cfg.player.type[ENEMY].chams_type, alpha_modifier, false);

					original_fn(m_modelrender(), ctx, state, info, bone_to_world);
					m_modelrender()->ForcedMaterialOverride(nullptr);

					if (cfg::g_cfg.player.type[ENEMY].double_material)
					{
						DrawChams(cfg::g_cfg.player.type[ENEMY].double_material_color, info, cfg::g_cfg.player.type[ENEMY].double_material_material, alpha_modifier, false);

						original_fn(m_modelrender(), ctx, state, info, bone_to_world);
						m_modelrender()->ForcedMaterialOverride(nullptr);
					}
				}

				called_original = true;
			}

			if (!called_original)
				return original_fn(m_modelrender(), ctx, state, info, bone_to_world);
		}
		else if (type == TEAM)
		{
			auto alpha_modifier = playeresp::get().esp_alpha_fade[model_entity->EntIndex()];
			auto material = materials[cfg::g_cfg.player.type[TEAM].chams_type];
			auto double_material = materials[5];

			if (!material->IsErrorMaterial() && !double_material->IsErrorMaterial())
			{
				// Invisible chams
				if (cfg::g_cfg.player.type[TEAM].enable_chams && cfg::g_cfg.player.type[TEAM].xqz_enable)
				{
					if (cfg::g_cfg.player.type[TEAM].xqz_enable)
					{
						DrawChams(cfg::g_cfg.player.type[TEAM].xqz_color, info, cfg::g_cfg.player.type[TEAM].chams_xqz, alpha_modifier, true);

						original_fn(m_modelrender(), ctx, state, info, bone_to_world);
						m_modelrender()->ForcedMaterialOverride(nullptr);

						if (cfg::g_cfg.player.type[TEAM].double_material_xqz)
						{
							DrawChams(cfg::g_cfg.player.type[TEAM].double_material_color_xqz, info, cfg::g_cfg.player.type[TEAM].double_material_material_xqz, alpha_modifier, true);

							original_fn(m_modelrender(), ctx, state, info, bone_to_world);
							m_modelrender()->ForcedMaterialOverride(nullptr);
						}
					}

					called_original = true;
				}

				// Visible chams
				if (cfg::g_cfg.player.type[TEAM].enable_chams)
				{
					DrawChams(cfg::g_cfg.player.type[TEAM].chams_color, info, cfg::g_cfg.player.type[TEAM].chams_type, alpha_modifier, false);

					original_fn(m_modelrender(), ctx, state, info, bone_to_world);
					m_modelrender()->ForcedMaterialOverride(nullptr);

					if (cfg::g_cfg.player.type[TEAM].double_material)
					{
						DrawChams(cfg::g_cfg.player.type[TEAM].double_material_color, info, cfg::g_cfg.player.type[TEAM].double_material_material, alpha_modifier, false);

						original_fn(m_modelrender(), ctx, state, info, bone_to_world);
						m_modelrender()->ForcedMaterialOverride(nullptr);
					}
				}
			}

			if (!called_original)
				return original_fn(m_modelrender(), ctx, state, info, bone_to_world);
		}
		else if (m_input()->m_fCameraInThirdPerson)
		{
			auto alpha_modifier = 1.0f;

			if (cfg::g_cfg.player.transparency_in_scope && g_ctx.globals.scoped)
				alpha_modifier = cfg::g_cfg.player.transparency_in_scope_amount / 100.f;

			auto material = materials[cfg::g_cfg.player.type[LOCAL].chams_type];
			auto double_material = materials[5];

			if (!material->IsErrorMaterial() && !double_material->IsErrorMaterial())
			{
				if (cfg::g_cfg.player.type[LOCAL].enable_chams)
				{
					if (cfg::g_cfg.player.type[LOCAL].enable_chams)
					{
						DrawChams(cfg::g_cfg.player.type[LOCAL].chams_color, info, cfg::g_cfg.player.type[LOCAL].chams_type, alpha_modifier, false);

						original_fn(m_modelrender(), ctx, state, info, bone_to_world);
						m_modelrender()->ForcedMaterialOverride(nullptr);
					}

					if (cfg::g_cfg.player.type[LOCAL].double_material)
					{
						DrawChams(cfg::g_cfg.player.type[LOCAL].double_material_color, info, cfg::g_cfg.player.type[LOCAL].double_material_material, alpha_modifier, false);

						original_fn(m_modelrender(), ctx, state, info, bone_to_world);
						m_modelrender()->ForcedMaterialOverride(nullptr);
					}

					called_original = true;
				}
			}

			if (!called_original && cfg::g_cfg.player.layered)
			{
				m_renderview()->SetBlend(alpha_modifier);
				m_renderview()->SetColorModulation(1.0f, 1.0f, 1.0f);

				original_fn(m_modelrender(), ctx, state, info, bone_to_world);
			}

			if (cfg::g_cfg.player.fake_chams_enable)
			{
				if (!local_animations::get().local_data.visualize_lag)
				{
					for (auto& i : g_ctx.globals.fake_matrix)
					{
						i[0][3] += info.origin.x;
						i[1][3] += info.origin.y;
						i[2][3] += info.origin.z;
					}
				}

				DrawChams(cfg::g_cfg.player.fake_chams_color, info, cfg::g_cfg.player.fake_chams_type, alpha_modifier, false);

				original_fn(m_modelrender(), ctx, state, info, g_ctx.globals.fake_matrix);
				m_modelrender()->ForcedMaterialOverride(nullptr);

				if (cfg::g_cfg.player.fake_double_material)
				{
					DrawChams(cfg::g_cfg.player.fake_double_material_color, info, cfg::g_cfg.player.fake_double_material_material, alpha_modifier, false);

					original_fn(m_modelrender(), ctx, state, info, g_ctx.globals.fake_matrix);
					m_modelrender()->ForcedMaterialOverride(nullptr);
				}

				if (!local_animations::get().local_data.visualize_lag)
				{
					for (auto& i : g_ctx.globals.fake_matrix)
					{
						i[0][3] -= info.origin.x;
						i[1][3] -= info.origin.y;
						i[2][3] -= info.origin.z;
					}
				}
			}

			if (!called_original && !cfg::g_cfg.player.layered)
			{
				m_renderview()->SetBlend(alpha_modifier);
				m_renderview()->SetColorModulation(1.0f, 1.0f, 1.0f);

				original_fn(m_modelrender(), ctx, state, info, bone_to_world);
			}
		}
	}
	else if (is_weapon)
	{
		auto alpha = (float)cfg::g_cfg.visuals.weapon_chams_color.a() / 255.0f;

		auto material = materials[cfg::g_cfg.visuals.weapon_chams_type];
		auto double_material = materials[5];

		if (!material->IsErrorMaterial() && !double_material->IsErrorMaterial())
		{
			if (cfg::g_cfg.visuals.weapon_chams)
			{
				DrawChams(cfg::g_cfg.visuals.weapon_chams_color, info, cfg::g_cfg.visuals.weapon_chams_type, alpha, false);

				original_fn(m_modelrender(), ctx, state, info, bone_to_world);
				m_modelrender()->ForcedMaterialOverride(nullptr);

				if (cfg::g_cfg.visuals.weapon_double_material)
				{
					DrawChams(cfg::g_cfg.visuals.weapon_double_material_color, info, cfg::g_cfg.visuals.weapon_double_material_material, alpha, false);

					original_fn(m_modelrender(), ctx, state, info, bone_to_world);
					m_modelrender()->ForcedMaterialOverride(nullptr);
				}
			}

			called_original = true;
		}

		if (!called_original)
			return original_fn(m_modelrender(), ctx, state, info, bone_to_world);
	}
	else if (is_arms || is_sleeve)
	{
		auto alpha = (float)cfg::g_cfg.visuals.arms_chams_color.a() / 255.0f;

		auto material = materials[cfg::g_cfg.visuals.arms_chams_type];
		auto double_material = materials[5];

		if (!material->IsErrorMaterial() && !double_material->IsErrorMaterial())
		{
			DrawChams(cfg::g_cfg.visuals.arms_chams_color, info, cfg::g_cfg.visuals.arms_chams_type, alpha, false);

			original_fn(m_modelrender(), ctx, state, info, bone_to_world);
			m_modelrender()->ForcedMaterialOverride(nullptr);

			if (cfg::g_cfg.visuals.arms_double_material && cfg::g_cfg.visuals.arms_chams_type != 4)
			{
				DrawChams(cfg::g_cfg.visuals.arms_double_material_color, info, cfg::g_cfg.visuals.arms_double_material_material, alpha, false);

				original_fn(m_modelrender(), ctx, state, info, bone_to_world);
				m_modelrender()->ForcedMaterialOverride(nullptr);
			}

			called_original = true;
		}

		if (!called_original)
			return original_fn(m_modelrender(), ctx, state, info, bone_to_world);
	}
}