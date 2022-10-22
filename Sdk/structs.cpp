// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <array>
#include "structs.hpp"
#include "..\Features\cheats\misc\misc.h"
#include "..\Features\cheats\misc\logs.h"
#include "..\Features\cheats\lagcompensation\LocalAnimations.h"
#include "..\Features\cheats\lagcompensation\SetupBones.h"

bool entity_t::compute_hitbox_surrounding_box(Vector* mins, Vector* maxs)
{
	using Fn = bool(__thiscall*)(void*, Vector*, Vector*);
	static auto fn = reinterpret_cast<Fn>(util::FindSignature(CLIENT_DLL, crypt_str("55 8B EC 83 E4 F8 81 EC ?? ?? ?? ?? 53 56 8B F1 57 83 BE ?? ?? ?? ?? ??")));

	return fn(this, mins, maxs);
}

std::array<Vector, 5>& player_t::m_vecPlayerPatchEconIndices()
{
	static int _m_vecPlayerPatchEconIndices = netvars::get().get_offset(crypt_str("CCSPlayer"), crypt_str("m_vecPlayerPatchEconIndices"));
	return *(std::array<Vector, 5>*)((uintptr_t)this + _m_vecPlayerPatchEconIndices);
}


datamap_t* entity_t::GetPredDescMap()
{
	typedef datamap_t* (__thiscall* Fn)(void*);
	return call_virtual< Fn >(this, g_ctx.indexes.at(16))(this);
}

bool entity_t::is_player()
{
	if (!this) //-V704
		return false;

	auto client_class = GetClientClass();

	if (!client_class)
		return false;

	return client_class->m_ClassID == CCSPlayer;
}

void entity_t::set_model_index(int index)
{
	if (!this) //-V704
		return;

	using Fn = void(__thiscall*)(PVOID, int);
	return call_virtual<Fn>(this, g_ctx.indexes.at(7))(this, index);
}

void entity_t::set_abs_angles(const Vector& angle)
{
	if (!this) //-V704
		return;

	using Fn = void(__thiscall*)(void*, const Vector&);
	static auto fn = reinterpret_cast<Fn>(util::FindSignature(CLIENT_DLL, crypt_str("55 8B EC 83 E4 F8 83 EC 64 53 56 57 8B F1 E8")));

	return fn(this, angle);
}

void entity_t::set_abs_origin(const Vector& origin)
{
	if (!this) //-V704
		return;

	using Fn = void(__thiscall*)(void*, const Vector&);
	static auto fn = reinterpret_cast<Fn>(util::FindSignature(CLIENT_DLL, crypt_str("55 8B EC 83 E4 F8 51 53 56 57 8B F1 E8")));

	return fn(this, origin);
}

weapon_info_t* weapon_t::get_csweapon_info()
{
	if (!this) //-V704
		return nullptr;

	if (!m_weaponsys())
		return nullptr;

	return m_weaponsys()->GetWpnData(this->m_iItemDefinitionIndex());
}

Vector& player_t::get_abs_angles()
{
	return call_virtual<Vector& (__thiscall*)(void*)>(this, 11)(this);
}

Vector& player_t::get_abs_origin()
{
	if (!this) //-V704
		return ZERO;

	return call_virtual<Vector& (__thiscall*)(void*)>(this, 10)(this);
}

bool player_t::setup_local_bones(int matrix)
{
	if (!this)
		return false;

	if (IsDormant())
		return false;

	static auto r_jiggle_bones = m_cvar()->FindVar("r_jiggle_bones");

	const auto penis = r_jiggle_bones->GetInt();
	const auto old_origin = get_abs_origin();
	const auto clientsideanim = m_bClientSideAnimation();

	r_jiggle_bones->SetValue(0);

	float bk = FLT_MAX;

	auto previous_weapon = get_animation_state() ? get_animation_state()->m_pLastBoneSetupWeapon : nullptr;

	if (previous_weapon)
		get_animation_state()->m_pLastBoneSetupWeapon = get_animation_state()->m_pActiveWeapon;

	if (this != g_ctx.local()) {
		set_abs_origin(m_vecOrigin());
	}
	else
	{
		if (get_animation_state() != nullptr && get_animation_state()->m_velocity < 0.1f && get_animation_state()->m_bOnGround && sequence_activity(get_animlayers()[3].m_nSequence) == 979)
		{
			bk = get_animlayers()[3].m_flWeight;

			get_animlayers()[3].m_flWeight = 0.f;
		}
	}

	const auto v20 = *(int*)(uintptr_t(this) + 0xA28);
	const auto v19 = *(int*)(uintptr_t(this) + 0xA30);
	const auto v22 = *(uint8_t*)(uintptr_t(this) + 0x68);
	const auto v62 = *(uint8_t*)(uintptr_t(this) + 0x274);

	*(uint8_t*)(uintptr_t(this) + 0x274) = 0;

	const auto effects = m_fEffects();
	m_fEffects() |= 8;

	*(int*)(uintptr_t(this) + 0xA68) = 0;
	*(int*)(uintptr_t(this) + 0xA28) &= ~10u;
	*(int*)(uintptr_t(this) + 0xA30) = 0;

	*(unsigned short*)(uintptr_t(this) + 0x68) |= 2;

	auto realtime_backup = m_globals()->m_realtime;
	auto curtime = m_globals()->m_curtime;
	auto frametime = m_globals()->m_frametime;
	auto absoluteframetime = m_globals()->m_absoluteframetime;
	auto framecount = m_globals()->m_framecount;
	auto tickcount = m_globals()->m_tickcount;
	auto interpolation_amount = m_globals()->m_interpolation_amount;

	float time = (this != g_ctx.local() ? m_flSimulationTime() : TICKS_TO_TIME(m_clientstate()->m_iServerTick));
	int ticks = TIME_TO_TICKS(time);

	m_globals()->m_curtime = time;
	m_globals()->m_realtime = time;
	m_globals()->m_frametime = m_globals()->m_intervalpertick;
	m_globals()->m_absoluteframetime = m_globals()->m_intervalpertick;
	m_globals()->m_framecount = ticks;
	m_globals()->m_tickcount = ticks;
	m_globals()->m_interpolation_amount = 0.f;

	const float weight12 = animlayer_count() < 12 ? -999 : get_animlayers()[12].m_flWeight;
	if (g_ctx.local() != this)
		get_animlayers()[12].m_flWeight = 0.f;

	m_globals()->m_framecount = -999;

	const auto v26 = *(int*)(uintptr_t(this) + 0x2670);

	*(int*)(uintptr_t(this) + 0x2670) = 0;
	g_ctx.globals.setuping_bones = true;

	m_bClientSideAnimation() = false;
	const bool result = SetupBones(nullptr, -1, matrix, time);
	m_bClientSideAnimation() = clientsideanim;

	g_ctx.globals.setuping_bones = false;

	*(int*)(uintptr_t(this) + 0x2670) = v26;

	*(int*)(uintptr_t(this) + 0xA28) = v20;
	*(int*)(uintptr_t(this) + 0xA30) = v19;
	*(unsigned short*)(uintptr_t(this) + 0x68) = v22;

	m_fEffects() = effects;

	*(uint8_t*)(uintptr_t(this) + 0x274) = v62;

	r_jiggle_bones->SetValue(penis);

	if (weight12 > -1)
		get_animlayers()[12].m_flWeight = weight12;

	m_globals()->m_realtime = realtime_backup;
	m_globals()->m_curtime = curtime;
	m_globals()->m_frametime = frametime;
	m_globals()->m_absoluteframetime = absoluteframetime;
	m_globals()->m_framecount = framecount;
	m_globals()->m_tickcount = tickcount;
	m_globals()->m_interpolation_amount = interpolation_amount;

	if (this != g_ctx.local())
		set_abs_angles(old_origin);
	else
	{
		if (bk < FLT_MAX)
			get_animlayers()[3].m_flWeight = bk;
	}

	if (previous_weapon)
		get_animation_state()->m_pLastBoneSetupWeapon = previous_weapon;

	return result;
}

void player_t::force_bone_rebuild()
{
	m_BoneAccessor().m_WritableBones = m_BoneAccessor().m_ReadableBones = 0;

	*(int*)(uintptr_t(this) + 0x2924) = 0xFF7FFFFF;
	*(int*)(uintptr_t(this) + 0x2690) = 0;
}

Vector& player_t::get_render_angles()
{
	if (!this)
		return ZERO;

	static auto deadflag = netvars::get().get_offset(crypt_str("CBasePlayer"), crypt_str("deadflag"));
	return *(Vector*)(uintptr_t(this) + (deadflag + 0x4));
}

float weapon_t::get_spread_virtual()
{
	if (!this) //-V704
		return 0.0f;

	return call_virtual<float(__thiscall*)(void*)>(this, g_ctx.indexes.at(10))(this);
}

float weapon_t::get_inaccuracy()
{
	if (!this) //-V704
		return 0.0f;

	return call_virtual<float(__thiscall*)(void*)>(this, g_ctx.indexes.at(9))(this);
}

float weapon_t::get_spread()
{
	if (!this) //-V704
		return 0.0f;

	return call_virtual<float(__thiscall*)(void*)>(this, g_ctx.indexes.at(10))(this);
}

float weapon_t::get_inaccuracy_virtual()
{
	if (!this) //-V704
		return 0.0f;

	return call_virtual<float(__thiscall*)(void*)>(this, g_ctx.indexes.at(9))(this);
}

void weapon_t::update_accuracy_penality()
{
	if (!this) //-V704
		return;

	call_virtual<void(__thiscall*)(void*)>(this, g_ctx.indexes.at(11))(this);
}

bool weapon_t::is_empty()
{
	if (!this) //-V704
		return true;

	return m_iClip1() <= 0;
}

bool weapon_t::can_fire(bool check_revolver)
{
	if (!this) //-V704
		return false;

	if (!is_non_aim() && is_empty())
		return false;

	auto owner = (player_t*)m_entitylist()->GetClientEntityFromHandle(m_hOwnerEntity());

	if (owner == g_ctx.local() && antiaim::get().freeze_check)
		return false;

	if (!owner->valid(false))
		return false;

	if (owner->m_bIsDefusing())
		return false;

	auto server_time = TICKS_TO_TIME(g_ctx.globals.fixed_tickbase);

	if (server_time < m_flNextPrimaryAttack())
		return false;

	if (server_time < owner->m_flNextAttack())
		return false;

	if (check_revolver && m_iItemDefinitionIndex() == WEAPON_REVOLVER && m_flPostponeFireReadyTime() >= server_time)
		return false;

	return true;
}
bool weapon_t::CanShiftShot(int ticks_to_shift)
{
	auto me = g_ctx.local();
	auto wpn = g_ctx.local()->m_hActiveWeapon().Get();

	if (me == nullptr || wpn == nullptr)
		return false;

	auto tickbase = g_ctx.local()->m_nTickBase();
	auto curtime = m_globals()->m_intervalpertick * (tickbase - ticks_to_shift);

	if (curtime < g_ctx.local()->m_flNextAttack())
		return false;

	if (curtime < wpn->m_flNextPrimaryAttack())
		return false;

	return true;
}
bool weapon_t::is_misc_weapon()
{
	if (!this)
		return false;

	int iWeaponID = m_iItemDefinitionIndex();

	return (iWeaponID == WEAPON_HEALTHSHOT || iWeaponID == WEAPON_C4
		|| iWeaponID == WEAPON_HEGRENADE || iWeaponID == WEAPON_DECOY
		|| iWeaponID == WEAPON_FLASHBANG || iWeaponID == WEAPON_MOLOTOV
		|| iWeaponID == WEAPON_SMOKEGRENADE || iWeaponID == WEAPON_INCGRENADE
		|| iWeaponID == 59 || iWeaponID == 57
		|| iWeaponID == 500 || iWeaponID == 505 || iWeaponID == 506
		|| iWeaponID == 507 || iWeaponID == 508 || iWeaponID == 509
		|| iWeaponID == 515);
}
bool weapon_t::IsGun()
{
	if (!this)
		return false;

	int id = this->m_iItemDefinitionIndex();

	if (!id)
		return false;

	if (id == WEAPON_KNIFE && id == WEAPON_HEGRENADE && id == WEAPON_DECOY && id == WEAPON_MOLOTOV && id == WEAPON_C4
		&& id == WEAPON_FLASHBANG && id == WEAPON_SMOKEGRENADE || id == WEAPON_KNIFE_T)
		return false;
	else
		return true;
}

int weapon_t::get_weapon_group(bool rage)
{
	if (!this) //-V704
		return -1;

	if (rage)
	{
		if (is_pistol() && m_iItemDefinitionIndex() != WEAPON_DEAGLE && m_iItemDefinitionIndex() != WEAPON_REVOLVER)
			return 0;//light pistols
		else if (m_iItemDefinitionIndex() == WEAPON_REVOLVER)
			return 1;//rev
		else if (m_iItemDefinitionIndex() == WEAPON_DEAGLE)
			return 2;//deag
		else if (is_smg())
			return 3;//smg
		else if (is_rifle())
			return 4;//rifle
		else if (is_shotgun())
			return 5;//shot
		else if (m_iItemDefinitionIndex() == WEAPON_SSG08)
			return 6;//ssg
		else if (m_iItemDefinitionIndex() == WEAPON_SCAR20 || m_iItemDefinitionIndex() == WEAPON_G3SG1)
			return 7;//scar
		else if (m_iItemDefinitionIndex() == WEAPON_AWP)
			return 8;//awp

	}
	else
	{

		if (is_pistol() && m_iItemDefinitionIndex() != WEAPON_DEAGLE && m_iItemDefinitionIndex() != WEAPON_REVOLVER )
			return 0;//light pistols
		else if (m_iItemDefinitionIndex() == WEAPON_REVOLVER)
			return 1;//rev
		else if (m_iItemDefinitionIndex() == WEAPON_DEAGLE)
			return 2;//deag
		else if (is_smg())
			return 3;//smg
		else if (is_rifle())
			return 4;//rifle
		else if (is_shotgun())
			return 5;//shot
		else if (m_iItemDefinitionIndex() == WEAPON_SSG08)
			return 6;//ssg
		else if (m_iItemDefinitionIndex() == WEAPON_SCAR20 || m_iItemDefinitionIndex() == WEAPON_G3SG1)
			return 7;//scar
		else if (m_iItemDefinitionIndex() == WEAPON_AWP)
			return 8;//awp

	}

	return -1;
}

bool weapon_t::is_rifle()
{
	if (!this) //-V704
		return false;

	int weapon_id = m_iItemDefinitionIndex();

	return weapon_id == WEAPON_AK47 || weapon_id == WEAPON_M4A1 || weapon_id == WEAPON_M4A1_SILENCER || weapon_id == WEAPON_GALILAR ||
		weapon_id == WEAPON_FAMAS || weapon_id == WEAPON_AUG || weapon_id == WEAPON_SG553;
}

bool weapon_t::is_smg()
{
	if (!this) //-V704
		return false;

	int weapon_id = m_iItemDefinitionIndex();

	return weapon_id == WEAPON_MAC10 || weapon_id == WEAPON_MP7 || weapon_id == WEAPON_MP9 || weapon_id == WEAPON_P90 ||
		weapon_id == WEAPON_BIZON || weapon_id == WEAPON_UMP45 || weapon_id == WEAPON_MP5SD;
}

bool weapon_t::is_shotgun()
{
	if (!this) //-V704
		return false;

	int weapon_id = m_iItemDefinitionIndex();

	return weapon_id == WEAPON_XM1014 || weapon_id == WEAPON_NOVA || weapon_id == WEAPON_SAWEDOFF || weapon_id == WEAPON_MAG7 || weapon_id == WEAPON_M249 || weapon_id == WEAPON_NEGEV;
}

bool weapon_t::is_pistol()
{
	if (!this) //-V704
		return false;

	int weapon_id = m_iItemDefinitionIndex();

	return weapon_id == WEAPON_DEAGLE || weapon_id == WEAPON_ELITE || weapon_id == WEAPON_FIVESEVEN || weapon_id == WEAPON_P250 ||
		weapon_id == WEAPON_GLOCK || weapon_id == WEAPON_HKP2000 || weapon_id == WEAPON_CZ75A || weapon_id == WEAPON_USP_SILENCER || weapon_id == WEAPON_TEC9 || weapon_id == WEAPON_REVOLVER;
}

bool weapon_t::is_sniper()
{
	if (!this) //-V704
		return false;

	int weapon_id = m_iItemDefinitionIndex();

	return weapon_id == WEAPON_AWP || weapon_id == WEAPON_SCAR20 || weapon_id == WEAPON_G3SG1 || weapon_id == WEAPON_SSG08;
}

bool weapon_t::is_grenade()
{
	if (!this) //-V704
		return false;

	int idx = m_iItemDefinitionIndex();

	return idx == WEAPON_FLASHBANG || idx == WEAPON_HEGRENADE || idx == WEAPON_SMOKEGRENADE || idx == WEAPON_MOLOTOV || idx == WEAPON_DECOY || idx == WEAPON_INCGRENADE;
}

bool weapon_t::is_knife()
{
	if (!this) //-V704
		return false;

	int idx = m_iItemDefinitionIndex();

	return idx == WEAPON_KNIFE || idx == WEAPON_KNIFE_BAYONET || idx == WEAPON_KNIFE_BUTTERFLY || idx == WEAPON_KNIFE_FALCHION
		|| idx == WEAPON_KNIFE_FLIP || idx == WEAPON_KNIFE_GUT || idx == WEAPON_KNIFE_KARAMBIT || idx == WEAPON_KNIFE_M9_BAYONET
		|| idx == WEAPON_KNIFE_PUSH || idx == WEAPON_KNIFE_SURVIVAL_BOWIE || idx == WEAPON_KNIFE_T || idx == WEAPON_KNIFE_TACTICAL
		|| idx == WEAPON_KNIFEGG || idx == WEAPON_KNIFE_GHOST || idx == WEAPON_KNIFE_GYPSY_JACKKNIFE || idx == WEAPON_KNIFE_STILETTO
		|| idx == WEAPON_KNIFE_URSUS || idx == WEAPON_KNIFE_WIDOWMAKER || idx == WEAPON_KNIFE_CSS || idx == WEAPON_KNIFE_CANIS
		|| idx == WEAPON_KNIFE_CORD || idx == WEAPON_KNIFE_OUTDOOR || idx == WEAPON_KNIFE_SKELETON;
}
bool weapon_t::is_scope_gun()
{
	if (!this) //-V704
		return false;

	int weapon_id = m_iItemDefinitionIndex();

	return weapon_id == WEAPON_AUG || weapon_id == WEAPON_SG553 ||
		weapon_id == WEAPON_AWP || weapon_id == WEAPON_SCAR20 ||
		weapon_id == WEAPON_G3SG1 || weapon_id == WEAPON_SSG08;
}
bool weapon_t::is_non_aim()
{
	if (!this) //-V704
		return true;

	auto idx = m_iItemDefinitionIndex();

	if (idx == WEAPON_C4 || idx == WEAPON_HEALTHSHOT)
		return true;

	if (is_knife())
		return true;

	if (is_grenade())
		return true;

	return false;
}

bool weapon_t::can_double_tap()
{
	if (!this) //-V704
		return false;

	if (is_non_aim())
		return false;

	auto idx = m_iItemDefinitionIndex();

	if (idx == WEAPON_TASER)
		return false;

	return true;
}

int weapon_t::get_max_tickbase_shift()
{
	if (!can_double_tap())
		return m_gamerules()->m_bIsValveDS() ? 6 : 16;

	auto idx = m_iItemDefinitionIndex();
	auto max_tickbase_shift = 0;

	switch (idx)
	{
	case WEAPON_M249:
	case WEAPON_MAC10:
	case WEAPON_P90:
	case WEAPON_MP5SD:
	case WEAPON_NEGEV:
	case WEAPON_MP9:
		max_tickbase_shift = 5;
		break;
	case WEAPON_ELITE:
	case WEAPON_UMP45:
	case WEAPON_BIZON:
	case WEAPON_TEC9:
	case WEAPON_MP7:
		max_tickbase_shift = 6;
		break;
	case WEAPON_AK47:
	case WEAPON_AUG:
	case WEAPON_FAMAS:
	case WEAPON_GALILAR:
	case WEAPON_M4A1:
	case WEAPON_M4A1_SILENCER:
	case WEAPON_CZ75A:
		max_tickbase_shift = 7;
		break;
	case WEAPON_FIVESEVEN:
	case WEAPON_GLOCK:
	case WEAPON_P250:
	case WEAPON_SG553:
		max_tickbase_shift = 8;
		break;
	case WEAPON_HKP2000:
	case WEAPON_USP_SILENCER:
		max_tickbase_shift = 9;
		break;
	case WEAPON_DEAGLE:
		max_tickbase_shift = 13;
		break;
	case WEAPON_G3SG1:
	case WEAPON_SCAR20:
	case WEAPON_SSG08:
		max_tickbase_shift = 14;
		break;
	}

	if (m_gamerules()->m_bIsValveDS())
		max_tickbase_shift = min(max_tickbase_shift, 6);

	return max_tickbase_shift;
}

char* weapon_t::get_icon()
{
	if (!this) //-V704
		return " ";

	switch (m_iItemDefinitionIndex())
	{
	case WEAPON_KNIFE_BAYONET:
		return "1";
	case WEAPON_KNIFE_SURVIVAL_BOWIE:
		return "7";
	case WEAPON_KNIFE_BUTTERFLY:
		return "8";
	case WEAPON_KNIFE_FALCHION:
		return "0";
	case WEAPON_KNIFE_FLIP:
		return "2";
	case WEAPON_KNIFE_GUT:
		return "3";
	case WEAPON_KNIFE_KARAMBIT:
		return "4";
	case WEAPON_KNIFE_M9_BAYONET:
		return "5";
	case WEAPON_KNIFE_TACTICAL:
		return "6";
	case WEAPON_KNIFE_PUSH:
		return "]";
	case WEAPON_DEAGLE:
		return "A";
	case WEAPON_ELITE:
		return "B";
	case WEAPON_FIVESEVEN:
		return "C";
	case WEAPON_GLOCK:
		return "D";
	case WEAPON_HKP2000:
		return "E";
	case WEAPON_P250:
		return "F";
	case WEAPON_USP_SILENCER:
		return "G";
	case WEAPON_TEC9:
		return "H";
	case WEAPON_REVOLVER:
		return "J";
	case WEAPON_MAC10:
		return "K";
	case WEAPON_UMP45:
		return "L";
	case WEAPON_BIZON:
		return "M";
	case WEAPON_MP7:
		return "N";
	case WEAPON_MP9:
		return "O";
	case WEAPON_P90:
		return "P";
	case WEAPON_GALILAR:
		return "Q";
	case WEAPON_FAMAS:
		return "R";
	case WEAPON_M4A1_SILENCER:
		return "T";
	case WEAPON_M4A1:
		return "S";
	case WEAPON_AUG:
		return "U";
	case WEAPON_SG553:
		return "V";
	case WEAPON_AK47:
		return "W";
	case WEAPON_G3SG1:
		return "X";
	case WEAPON_SCAR20:
		return "Y";
	case WEAPON_AWP:
		return "Z";
	case WEAPON_SSG08:
		return "a";
	case WEAPON_XM1014:
		return "b";
	case WEAPON_SAWEDOFF:
		return "c";
	case WEAPON_MAG7:
		return "d";
	case WEAPON_NOVA:
		return "e";
	case WEAPON_NEGEV:
		return "f";
	case WEAPON_M249:
		return "g";
	case WEAPON_TASER:
		return "h";
	case WEAPON_FLASHBANG:
		return "i";
	case WEAPON_HEGRENADE:
		return "j";
	case WEAPON_SMOKEGRENADE:
		return "k";
	case WEAPON_MOLOTOV:
		return "l";
	case WEAPON_DECOY:
		return "m";
	case WEAPON_INCGRENADE:
		return "n";
	case WEAPON_C4:
		return "o";
	case WEAPON_CZ75A:
		return "I";
	default:
		return " ";
	}
}

std::string weapon_t::get_name()
{
	if (!this) //-V704
		return " ";

	switch (m_iItemDefinitionIndex())
	{
	case WEAPON_KNIFE:
		return "KNIFE";
	case WEAPON_KNIFE_T:
		return "KNIFE";
	case WEAPON_KNIFE_BAYONET:
		return "KNIFE";
	case WEAPON_KNIFE_SURVIVAL_BOWIE:
		return "KNIFE";
	case WEAPON_KNIFE_BUTTERFLY:
		return "KNIFE";
	case WEAPON_KNIFE_FALCHION:
		return "KNIFE";
	case WEAPON_KNIFE_FLIP:
		return "KNIFE";
	case WEAPON_KNIFE_GUT:
		return "KNIFE";
	case WEAPON_KNIFE_KARAMBIT:
		return "KNIFE";
	case WEAPON_KNIFE_M9_BAYONET:
		return "KNIFE";
	case WEAPON_KNIFE_TACTICAL:
		return "KNIFE";
	case WEAPON_KNIFE_PUSH:
		return "KNIFE";
	case WEAPON_DEAGLE:
		return "DEAGLE";
	case WEAPON_ELITE:
		return "DUAL BERETTAS";
	case WEAPON_FIVESEVEN:
		return "FIVE-SEVEN";
	case WEAPON_GLOCK:
		return "GLOCK 18";
	case WEAPON_HKP2000:
		return "P2000";
	case WEAPON_P250:
		return "P250";
	case WEAPON_USP_SILENCER:
		return "USP-S";
	case WEAPON_TEC9:
		return "TEC-9";
	case WEAPON_REVOLVER:
		return "REVOLVER";
	case WEAPON_MAC10:
		return "MAC-10";
	case WEAPON_UMP45:
		return "UMP-45";
	case WEAPON_BIZON:
		return "PP-BIZON";
	case WEAPON_MP7:
		return "MP7";
	case WEAPON_MP9:
		return "MP9";
	case WEAPON_P90:
		return "P90";
	case WEAPON_GALILAR:
		return "GALIL AR";
	case WEAPON_FAMAS:
		return "FAMAS";
	case WEAPON_M4A1_SILENCER:
		return "M4A1-S";
	case WEAPON_M4A1:
		return "M4A4";
	case WEAPON_AUG:
		return "AUG";
	case WEAPON_SG553:
		return "SG 553";
	case WEAPON_AK47:
		return "AK-47";
	case WEAPON_G3SG1:
		return "G3SG1";
	case WEAPON_SCAR20:
		return "SCAR-20";
	case WEAPON_AWP:
		return "AWP";
	case WEAPON_SSG08:
		return "SSG 08";
	case WEAPON_XM1014:
		return "XM1014";
	case WEAPON_SAWEDOFF:
		return "SAWED-OFF";
	case WEAPON_MAG7:
		return "MAG-7";
	case WEAPON_NOVA:
		return "NOVA";
	case WEAPON_NEGEV:
		return "NEGEV";
	case WEAPON_M249:
		return "M249";
	case WEAPON_TASER:
		return "ZEUS X27";
	case WEAPON_FLASHBANG:
		return "FLASHBANG";
	case WEAPON_HEGRENADE:
		return "HE GRENADE";
	case WEAPON_SMOKEGRENADE:
		return "SMOKE";
	case WEAPON_MOLOTOV:
		return "MOLOTOV";
	case WEAPON_DECOY:
		return "DECOY";
	case WEAPON_INCGRENADE:
		return "INCENDIARY";
	case WEAPON_C4:
		return "C4";
	case WEAPON_CZ75A:
		return "CZ75-AUTO";
	default:
		return " ";
	}
}

std::array <float, 24>& entity_t::m_flPoseParameter()
{
	static auto _m_flPoseParameter = netvars::get().get_offset(crypt_str("CCSPlayer"), crypt_str("m_flPoseParameter"));//dont change or crash
	return *(std::array <float, 24>*)((uintptr_t)this + _m_flPoseParameter);
}

Vector player_t::GetEyePos() noexcept
{
	return m_vecOrigin() + m_vecViewOffset();
}

Vector player_t::GetHitboxPos(int hitbox)
{
	matrix3x4_t boneMatrix[MAXSTUDIOBONES];

	if (this->SetupBones(boneMatrix, 128, BONE_USED_BY_HITBOX, this->m_flSimulationTime()))
	{
		studiohdr_t* studioHdr = m_modelinfo()->GetStudioModel(this->GetModel());
		if (studioHdr)
		{
			mstudiobbox_t* hitbox_box = studioHdr->pHitboxSet(this->m_nHitboxSet())->pHitbox(hitbox);
			if (hitbox_box)
			{
				Vector
					min = Vector{},
					max = Vector{};

				math::vector_transform(hitbox_box->bbmin, boneMatrix[hitbox_box->bone], min);
				math::vector_transform(hitbox_box->bbmax, boneMatrix[hitbox_box->bone], max);

				return (min + max) / 2.0f;
			}
		}
	}

	return Vector{};
}

Vector player_t::get_shoot_position()
{
	if (!this) //-V704
		return ZERO;

	auto shoot_position = m_vecOrigin() + m_vecViewOffset();

	if (this != g_ctx.local())
		return shoot_position;

	if (g_ctx.globals.fakeducking && EntIndex() == m_engine()->GetLocalPlayer()) {
		Vector origin = m_vecOrigin();

		Vector vDuckHullMin = m_gamemovement()->GetPlayerMins(true);
		Vector vStandHullMin = m_gamemovement()->GetPlayerMins(false);

		float fMore = (vDuckHullMin.z - vStandHullMin.z);

		Vector vecDuckViewOffset = m_gamemovement()->GetPlayerViewOffset(true);
		Vector vecStandViewOffset = m_gamemovement()->GetPlayerViewOffset(false);
		float duckFraction = m_flDuckAmount();

		float tempz = ((vecDuckViewOffset.z - fMore) * duckFraction) +
			(vecStandViewOffset.z * (1 - duckFraction));

		origin.z += tempz;

		return origin;
	}
	else
		modify_eye_position(shoot_position);

	return shoot_position;
}

void player_t::modify_eye_position(Vector& eye_position)
{
	if (!this)
		return;

	if (!local_animations::get().local_data.prediction_animstate)
		return;

	if (!local_animations::get().local_data.prediction_animstate->m_bInHitGroundAnimation && local_animations::get().local_data.prediction_animstate->m_fDuckAmount <= 0.0f)
		return;

	local_animations::get().local_data.prediction_animstate->m_pBaseEntity = this;

	static auto lookup_bone = reinterpret_cast <int(__thiscall*)(void*, const char*)> (util::FindSignature(CLIENT_DLL, crypt_str("55 8B EC 53 56 8B F1 57 83 BE ?? ?? ?? ?? ?? 75 14")));
	auto head_bone = lookup_bone(local_animations::get().local_data.prediction_animstate->m_pBaseEntity, crypt_str("head_0"));

	if (head_bone == -1)
		return;

	auto head_position = Vector(g_ctx.globals.prediction_matrix[head_bone][0][3], g_ctx.globals.prediction_matrix[head_bone][1][3], g_ctx.globals.prediction_matrix[head_bone][2][3] + 1.7f);

	if (head_position.z >= eye_position.z)
		return;

	eye_position.z = math::lerp2(math::simple_spline_remap_val_clamped(fabs(eye_position.z - head_position.z), 4.0f, 10.0f, 0.0f, 1.0f), eye_position.z, head_position.z);
}

bool player_t::is_alive()
{
	if (!this) //-V704
		return false;

	if (m_iTeamNum() != 2 && m_iTeamNum() != 3)
		return false;

	if (m_lifeState() != LIFE_ALIVE)
		return false;

	return true;
}

int	player_t::get_move_type()
{
	if (!this) //-V704
		return 0;

	return *(int*)((uintptr_t)this + 0x25C);
}

int player_t::get_hitbox_bone_id(int hitbox_id)
{
	if (!this) //-V704
		return -1;

	auto hdr = m_modelinfo()->GetStudioModel(GetModel());

	if (!hdr)
		return -1;

	auto hitbox_set = hdr->pHitboxSet(m_nHitboxSet());

	if (!hitbox_set)
		return -1;

	auto hitbox = hitbox_set->pHitbox(hitbox_id);

	if (!hitbox)
		return -1;

	return hitbox->bone;
}

QAngle player_t::m_aimPunchAngleScaled()
{
	static auto weapon_recoil_scale = m_cvar()->FindVar(XorStr("weapon_recoil_scale"));
	const auto m_aim_punch_angle = m_aimPunchAngle2();

	return m_aim_punch_angle * weapon_recoil_scale->GetFloat();
}

bool player_t::CanSeePlayer(player_t* player, const Vector& pos)
{
	CGameTrace tr;
	Ray_t ray;
	CTraceFilter filter;
	filter.pSkip = this;

	ray.Init(get_shoot_position(), pos);
	m_trace()->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, &filter, &tr);

	return tr.hit_entity == player || tr.fraction > 0.97f;
}

Vector player_t::hitbox_position(int hitbox_id)
{
	if (!this) //-V704
		return ZERO;

	auto hdr = m_modelinfo()->GetStudioModel(GetModel());

	if (!hdr)
		return ZERO;

	auto hitbox_set = hdr->pHitboxSet(m_nHitboxSet());

	if (!hitbox_set)
		return ZERO;

	auto hitbox = hitbox_set->pHitbox(hitbox_id);

	if (!hitbox)
		return ZERO;

	Vector min, max;

	math::vector_transform(hitbox->bbmin, m_CachedBoneData().Base()[hitbox->bone], min);
	math::vector_transform(hitbox->bbmax, m_CachedBoneData().Base()[hitbox->bone], max);

	return (min + max) * 0.5f;
}

Vector player_t::hitbox_position_matrix(int hitbox_id, matrix3x4_t matrix[MAXSTUDIOBONES])
{
	if (!this) //-V704
		return ZERO;

	auto hdr = m_modelinfo()->GetStudioModel(GetModel());

	if (!hdr)
		return ZERO;

	auto hitbox_set = hdr->pHitboxSet(m_nHitboxSet());

	if (!hitbox_set)
		return ZERO;

	auto hitbox = hitbox_set->pHitbox(hitbox_id);

	if (!hitbox)
		return ZERO;

	Vector min, max;

	math::vector_transform(hitbox->bbmin, matrix[hitbox->bone], min);
	math::vector_transform(hitbox->bbmax, matrix[hitbox->bone], max);

	return (min + max) * 0.5f;
}

CUtlVector <matrix3x4_t>& player_t::m_CachedBoneData()
{
	static auto m_CachedBoneData = *(DWORD*)(util::FindSignature(CLIENT_DLL, crypt_str("FF B7 ?? ?? ?? ?? 52")) + 0x2) + 0x4;
	return *(CUtlVector <matrix3x4_t>*)(uintptr_t(this) + m_CachedBoneData);
}

CBoneAccessor& player_t::m_BoneAccessor()
{
	static auto m_nForceBone = netvars::get().get_offset(crypt_str("CBaseAnimating"), crypt_str("m_nForceBone"));
	static auto BoneAccessor = m_nForceBone + 0x1C;

	return *(CBoneAccessor*)((uintptr_t)this + BoneAccessor);
}

void player_t::invalidate_bone_cache()
{
	if (!this) //-V704
		return;

	m_flLastBoneSetupTime() = -FLT_MAX;
	m_iMostRecentModelBoneCounter() = UINT_MAX;
}

void player_t::set_abs_velocity(const Vector& velocity)
{
	if (!this) //-V704
		return;

	using Fn = void(__thiscall*)(void*, const Vector&);
	static auto fn = reinterpret_cast<Fn>(util::FindSignature(CLIENT_DLL, crypt_str("55 8B EC 83 E4 F8 83 EC 0C 53 56 57 8B 7D 08 8B F1 F3")));

	return fn(this, velocity);
}

void player_t::update_clientside_animation()
{
	if (!this)
		return;

	auto animstate = get_animation_state();

	if (!animstate)
		return;

	if (animstate->m_iLastClientSideAnimationUpdateFramecount >= m_globals()->m_framecount)
		animstate->m_iLastClientSideAnimationUpdateFramecount = m_globals()->m_framecount - 1;

	using Fn = void(__thiscall*)(void*);
	call_virtual<Fn>(this, g_ctx.indexes.at(13))(this);
}

float player_t::GetMaxPlayerSpeed()
{
	weapon_t* pWeapon = this->m_hActiveWeapon().Get();

	if (pWeapon)
	{
		weapon_info_t* pWeaponData = pWeapon->get_csweapon_info();

		if (pWeaponData)
			return this->m_bIsScoped() ? pWeaponData->flMaxPlayerSpeedAlt : pWeaponData->flMaxPlayerSpeed;
	}

	return 260.0f;
}

uint32_t& player_t::m_iMostRecentModelBoneCounter()
{
	static auto invalidate_bone_cache = util::FindSignature(CLIENT_DLL, crypt_str("80 3D ?? ?? ?? ?? ?? 74 16 A1 ?? ?? ?? ?? 48 C7 81"));
	static auto most_recent_model_bone_counter = *(uintptr_t*)(invalidate_bone_cache + 0x1B);

	return *(uint32_t*)((uintptr_t)this + most_recent_model_bone_counter);
}

float& player_t::m_flLastBoneSetupTime()
{
	static auto invalidate_bone_cache = util::FindSignature(CLIENT_DLL, crypt_str("80 3D ?? ?? ?? ?? ?? 74 16 A1 ?? ?? ?? ?? 48 C7 81"));
	static auto last_bone_setup_time = *(uintptr_t*)(invalidate_bone_cache + 0x11);

	return *(float*)((uintptr_t)this + last_bone_setup_time);
}

void player_t::select_item(const char* string, int sub_type)
{
	static auto select_item_fn = reinterpret_cast <void(__thiscall*)(void*, const char*, int)> (util::FindSignature(CLIENT_DLL, crypt_str("55 8B EC 56 8B F1 ? ? ? 85 C9 74 71 8B 06")));
	select_item_fn(this, string, sub_type);
}

bool player_t::using_standard_weapons_in_vechile()
{
	static auto using_standard_weapons_in_vechile_fn = reinterpret_cast <bool(__thiscall*)(void*)> (util::FindSignature(CLIENT_DLL, crypt_str("56 57 8B F9 8B 97 ? ? ? ? 83 FA FF 74 41")));
	return using_standard_weapons_in_vechile_fn(this);
}

bool player_t::physics_run_think(int index)
{
	static auto physics_run_think_fn = reinterpret_cast <bool(__thiscall*)(void*, int)> (util::FindSignature(CLIENT_DLL, crypt_str("55 8B EC 83 EC 10 53 56 57 8B F9 8B ? ? ? ? ? C1")));
	return physics_run_think_fn(this, index);
}

VarMapping_t* player_t::var_mapping()
{
	return reinterpret_cast<VarMapping_t*>((DWORD)this + 0x24);
}

bool player_t::valid(bool check_team, bool check_dormant)
{
	if (!this) //-V704
		return false;

	if (!g_ctx.local())
		return false;

	if (!is_player())
		return false;

	if (!is_alive())
		return false;

	if (IsDormant() && check_dormant)
		return false;

	if (check_team && g_ctx.local()->m_iTeamNum() == m_iTeamNum())
		return false;

	return true;
}

int player_t::animlayer_count()
{
	if (!this)
		return 0;

	return *(int*)((DWORD)this + 0x299C);
}

AnimationLayer* player_t::get_animlayers()
{
	return *(AnimationLayer**)((DWORD)this + 0x2990);
}

int player_t::sequence_activity(int sequence)
{
	if (!this)
		return -1;

	auto hdr = m_modelinfo()->GetStudioModel(GetModel());

	if (hdr == nullptr)
		return -1;

	if (!hdr)
		return -1;

	static auto get_sequence_activity = reinterpret_cast<int(__fastcall*)(void*, studiohdr_t*, int)>(util::FindSignature(CLIENT_DLL, crypt_str("55 8B EC 53 8B 5D 08 56 8B F1 83")));
	return get_sequence_activity(this, hdr, sequence);
}

c_baseplayeranimationstate* player_t::get_animation_state()
{
	return *reinterpret_cast<c_baseplayeranimationstate**>(reinterpret_cast<void*>(uintptr_t(this) + 0x9960));
}

CStudioHdr* player_t::m_pStudioHdr()
{
	static auto studio_hdr = util::FindSignature(CLIENT_DLL, crypt_str("8B B7 ?? ?? ?? ?? 89 74 24 20"));
	return *(CStudioHdr**)((uintptr_t)this + *(uintptr_t*)(studio_hdr + 0x2) + 0x4);
}

bool player_t::setup_bones_rebuilt(matrix3x4_t* matrix, int mask)
{
	if (!this) //-V704
		return false;

	auto setuped = false;

	auto backup_value = *(uint8_t*)((uintptr_t)this + 0x274);
	*(uint8_t*)((uintptr_t)this + 0x274) = 0;

	auto backup_effects = m_fEffects();
	m_fEffects() |= 8;

	auto animstate = get_animation_state();
	auto previous_weapon = animstate ? animstate->m_pLastBoneSetupWeapon : nullptr;

	if (previous_weapon)
		animstate->m_pLastBoneSetupWeapon = animstate->m_pActiveWeapon;

	auto backup_abs_origin = GetAbsOrigin();

	if (this != g_ctx.local())
		set_abs_origin(m_vecOrigin());

	g_ctx.globals.setuping_bones = true;
	invalidate_bone_cache();

	SetupBones(matrix, matrix ? MAXSTUDIOBONES : -1, mask, m_flSimulationTime());

	g_ctx.globals.setuping_bones = false;

	if (this != g_ctx.local())
		set_abs_origin(backup_abs_origin);

	if (previous_weapon)
		animstate->m_pLastBoneSetupWeapon = previous_weapon;

	m_fEffects() = backup_effects;
	*(uint8_t*)((uintptr_t)this + 0x274) = backup_value;

	return setuped;
}
bool player_t::setup_bones_fixed(matrix3x4_t* matrix, int mask)
{
	if (!this)
		return false;

	auto setuped = false;

	auto backup_value = *(uint8_t*)((uintptr_t)this + 0x274);
	*(uint8_t*)((uintptr_t)this + 0x274) = 0;

	auto backup_effects = m_fEffects();
	m_fEffects() |= 8;

	auto animstate = get_animation_state();
	auto previous_weapon = animstate ? animstate->m_pLastBoneSetupWeapon : nullptr;

	if (previous_weapon)
		animstate->m_pLastBoneSetupWeapon = animstate->m_pActiveWeapon;

	auto backup_abs_origin = GetAbsOrigin();

	if (this != g_ctx.local())
		set_abs_origin(m_vecOrigin());

	g_ctx.globals.setuping_bones = true;

	invalidate_bone_cache();

	SetupBones(matrix, MAXSTUDIOBONES, mask, m_flSimulationTime());

	g_ctx.globals.setuping_bones = false;

	if (this != g_ctx.local())
		set_abs_origin(backup_abs_origin);

	if (previous_weapon)
		animstate->m_pLastBoneSetupWeapon = previous_weapon;

	m_fEffects() = backup_effects;
	*(uint8_t*)((uintptr_t)this + 0x274) = backup_value;

	return setuped;
}

uint32_t& player_t::m_fEffects()
{
	static auto m_fEffects = util::find_in_datamap(GetPredDescMap(), crypt_str("m_fEffects"));
	return *(uint32_t*)(uintptr_t(this) + m_fEffects);
}

uint32_t& player_t::m_iEFlags()
{
	static auto m_iEFlags = util::find_in_datamap(GetPredDescMap(), crypt_str("m_iEFlags"));
	return *(uint32_t*)(uintptr_t(this) + m_iEFlags);
}

float& player_t::m_surfaceFriction()
{
	static auto m_surfaceFriction = util::find_in_datamap(GetPredDescMap(), crypt_str("m_surfaceFriction"));
	return *(float*)(uintptr_t(this) + m_surfaceFriction);
}

Vector& player_t::m_vecAbsVelocity()
{
	if (!this) //-V704
		return ZERO;

	static auto m_vecAbsVelocity = util::find_in_datamap(GetPredDescMap(), crypt_str("m_vecAbsVelocity"));
	return *(Vector*)(uintptr_t(this) + m_vecAbsVelocity);
}

float player_t::get_max_desync_delta()
{
	if (!this) //-V704
		return 0.0f;

	auto animstate = get_animation_state();

	if (!animstate)
		return 0.0f;

	auto speedfactor = math::clamp(animstate->m_flFeetSpeedForwardsOrSideWays, 0.0f, 1.0f);
	auto avg_speedfactor = (animstate->m_flStopToFullRunningFraction * -0.3f - 0.2f) * speedfactor + 1.0f;

	auto duck_amount = animstate->m_fDuckAmount;

	if (duck_amount) //-V550
	{
		auto max_velocity = math::clamp(animstate->m_flFeetSpeedUnknownForwardOrSideways, 0.0f, 1.0f);
		auto duck_speed = duck_amount * max_velocity;

		avg_speedfactor += duck_speed * (0.5f - avg_speedfactor);
	}

	return animstate->yaw_desync_adjustment() * avg_speedfactor;
}

void player_t::invalidate_physics_recursive(int change_flags)
{
	static auto m_uInvalidatePhysics = util::FindSignature(CLIENT_DLL, crypt_str("55 8B EC 83 E4 F8 83 EC 0C 53 8B 5D 08 8B C3"));
	reinterpret_cast <void(__thiscall*)(void*, int)> (m_uInvalidatePhysics)(this, change_flags);
}

float& viewmodel_t::m_flCycle()
{
	static auto m_flCycle = util::find_in_datamap(GetPredDescMap(), crypt_str("m_flCycle"));
	return *(float*)(uintptr_t(this) + m_flCycle);
}

float& viewmodel_t::m_flAnimTime()
{
	static auto m_flAnimTime = util::find_in_datamap(GetPredDescMap(), crypt_str("m_flAnimTime"));
	return *(float*)(uintptr_t(this) + m_flAnimTime);
}

void viewmodel_t::SendViewModelMatchingSequence(int sequence)
{
	using Fn = void(__thiscall*)(void*, int);
	call_virtual <Fn>(this, g_ctx.indexes.at(14))(this, sequence);
}

void CHudChat::chat_print(const char* fmt, ...)
{
	char msg[1024];
	va_list args;
	va_start(args, fmt);
	vsnprintf(msg, 1024, fmt, args);
	call_virtual <void(__cdecl*)(void*, int, int, const char*, ...)>(this, g_ctx.indexes.at(15))(this, 0, 0, fmt);
	va_end(args);
}


float player_t::MaxDesyncDelta()
{
	auto animstate = uintptr_t(g_ctx.local()->get_animation_state());
	float duckammount = *(float*)(animstate + 0xA4);
	float speedfraction = max(0, min(*reinterpret_cast<float*>(animstate + 0xF8), 1));
	float speedfactor = max(0, min(1, *reinterpret_cast<float*> (animstate + 0xFC)));
	float unk1 = ((*reinterpret_cast<float*> (animstate + 0x11C) * -0.30000001) - 0.19999999) * speedfraction;
	float unk2 = unk1 + 1.f;
	float unk3;
	if (duckammount > 0) {
		unk2 += ((duckammount * speedfactor) * (0.5f - unk2));
	}
	unk3 = *(float*)(animstate + 0x338) * unk2;
	return unk3;
}

