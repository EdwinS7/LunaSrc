// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "penetration.h""

bool autowall::is_breakable_entity(IClientEntity* e) {
	if (!e || !e->EntIndex())
		return false;

	//Sig may be outdated?
	static auto is_breakable = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 51 56 8B F1 85 F6 74 68"));

	//Crash may occur here
	auto take_damage = *(uintptr_t*)((uintptr_t)is_breakable + 0x26);
	auto backup = *(uint8_t*)((uintptr_t)e + take_damage);

	auto client_class = e->GetClientClass();
	auto network_name = client_class->m_pNetworkName;

	if (!strcmp(network_name, "CBreakableSurface"))
		*(uint8_t*)((uintptr_t)e + take_damage) = DAMAGE_YES;
	else if (!strcmp(network_name, "CBaseDoor") || !strcmp(network_name, "CDynamicProp"))
		*(uint8_t*)((uintptr_t)e + take_damage) = DAMAGE_NO;

	using Fn = bool(__thiscall*)(IClientEntity*);
	auto result = ((Fn)is_breakable)(e);

	*(uint8_t*)((uintptr_t)e + take_damage) = backup;
	return result;
}

void autowall::scale_damage(player_t* e, CGameTrace& enterTrace, weapon_info_t* weaponData, float& flDamage)
{
	if (!e->is_player())
		return;

	/* https://totalcsgo.com/command/mpdamagescalecthead */
	static auto mp_damage_scale_ct_head = m_cvar()->FindVar(crypt_str("mp_damage_scale_ct_head"));
	static auto mp_damage_scale_t_head = m_cvar()->FindVar(crypt_str("mp_damage_scale_t_head"));
	static auto mp_damage_scale_ct_body = m_cvar()->FindVar(crypt_str("mp_damage_scale_ct_body"));
	static auto mp_damage_scale_t_body = m_cvar()->FindVar(crypt_str("mp_damage_scale_t_body"));
	auto flHeadDamageScale = e->m_iTeamNum() == 3 ? mp_damage_scale_ct_head->GetFloat() : mp_damage_scale_t_head->GetFloat();
	auto flBodyDamageScale = e->m_iTeamNum() == 3 ? mp_damage_scale_ct_body->GetFloat() : mp_damage_scale_t_body->GetFloat();

	auto HasHeavyArmor = e->m_bHasHeavyArmor();
	auto ArmorValue = (float)e->m_ArmorValue();
	if (HasHeavyArmor)
		flHeadDamageScale *= 0.5f;

	float flHeadShotMultiplier = 3.5f;
	float flBodyShotMultiplier = 1.25f;
	float flLegsShotMultiplier = 0.75f;

	//if (weaponData)
	//	flHeadShotMultiplier = weaponData->headshotmultyplrier;

	int nTraceHitgroup = enterTrace.hitgroup;
	switch (nTraceHitgroup)
	{
	case HITGROUP_HEAD:
		flDamage = (flDamage * flHeadShotMultiplier) * flHeadDamageScale;
		break;
	case HITGROUP_STOMACH:
		flDamage = (flDamage + flBodyShotMultiplier) * flBodyDamageScale;
		break;
	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		flDamage = (flDamage * flLegsShotMultiplier) * flBodyDamageScale;
		break;
	default:
		break;
	}

	auto IsArmored = [&]()->bool
	{
		switch (enterTrace.hitgroup)
		{
		case HITGROUP_HEAD:
			return e->m_bHasHelmet();
		case HITGROUP_GENERIC:
		case HITGROUP_CHEST:
		case HITGROUP_STOMACH:
		case HITGROUP_LEFTARM:
		case HITGROUP_RIGHTARM:
			return true;
		default:
			return false;
		}
	};

	if (ArmorValue > 0.f && IsArmored())
	{
		auto armorRatio = weaponData->flArmorRatio * 0.5f;
		auto armorBonusRatio = 0.5f;
		auto bonusValue = 1.0f;

		auto NewDamage = 0.f;

		/* https://counterstrike.fandom.com/wiki/Heavy_Assault_Suit */
		if (HasHeavyArmor)
		{
			armorBonusRatio = 0.33f;
			armorRatio = armorRatio * 0.25f;
			bonusValue = 0.33f;

			NewDamage = (flDamage * armorRatio) * 0.85f;
		}
		else
			NewDamage = flDamage * armorRatio;
		
		if ( ( flDamage - ( flDamage * armorRatio ) ) * bonusValue * armorBonusRatio > ArmorValue )
			NewDamage = flDamage - ArmorValue / armorBonusRatio;

		flDamage = NewDamage;
	}
}

void TraceLine(Vector& absStart, const Vector& absEnd, unsigned int mask, player_t* ignore, CGameTrace* ptr)
{
	Ray_t ray;
	ray.Init(absStart, absEnd);
	CTraceFilter filter;
	filter.pSkip = ignore;

	m_trace()->TraceRay(ray, mask, &filter, ptr);
}

bool autowall::trace_to_exit(CGameTrace& enterTrace, CGameTrace& exitTrace, Vector startPosition, const Vector& direction)
{
	static ConVar* sv_clip_penetration_traces_to_players = m_cvar()->FindVar(crypt_str("sv_clip_penetration_traces_to_players"));

	Vector          new_end, out;
	float           dist = 0.0f;
	int                iterations = 23;
	int                first_contents = 0;
	int             contents;
	Ray_t r{};

	while (1)
	{
		iterations--;

		if (iterations <= 0 || dist > 90.f)
			break;

		dist += 4.0f;
		out = startPosition + (direction * dist);

		contents = m_trace()->GetPointContents(out, 0x4600400B, nullptr);

		if (first_contents == -1)
			first_contents = contents;

		if (contents & 0x600400B && (!(contents & CONTENTS_HITBOX) || first_contents == contents))
			continue;

		new_end = out - (direction * 4.f);

		TraceLine(out, new_end, 0x4600400B, nullptr, &exitTrace);

		if (exitTrace.startsolid && (exitTrace.surface.flags & SURF_HITBOX) != 0)
		{
			TraceLine(out, startPosition, MASK_SHOT_HULL, (player_t*)exitTrace.hit_entity, &exitTrace);

			if (exitTrace.DidHit() && !exitTrace.startsolid)
			{
				out = exitTrace.endpos;
				return true;
			}
			continue;
		}

		if (!exitTrace.DidHit() || exitTrace.startsolid)
		{
			if (enterTrace.hit_entity != m_entitylist()->GetClientEntity(0))
			{
				if (exitTrace.hit_entity && is_breakable_entity(exitTrace.hit_entity))
				{
					exitTrace.surface.surfaceProps = enterTrace.surface.surfaceProps;
					exitTrace.endpos = startPosition + direction;
					return true;
				}
			}

			continue;
		}

		if ((exitTrace.surface.flags & 0x80u) != 0)
		{
			if (enterTrace.hit_entity && is_breakable_entity(enterTrace.hit_entity) && exitTrace.hit_entity && is_breakable_entity(exitTrace.hit_entity))
			{
				out = exitTrace.endpos;
				return true;
			}

			if (!(enterTrace.surface.flags & 0x80u))
				continue;
		}

		if (exitTrace.plane.normal.Dot(direction) <= 1.f)
		{
			out -= direction * (exitTrace.fraction * 4.0f);
			return true;
		}
	}

	return false;
}
bool autowall::handle_bullet_penetration(weapon_info_t* weaponData, CGameTrace& enterTrace, Vector& eyePosition, const Vector& direction, int& possibleHitsRemaining, float& currentDamage, float penetrationPower, float ff_damage_reduction_bullets, float ff_damage_bullet_penetration, bool draw_impact)
{
	if (weaponData->flPenetration <= 0.0f)
		return false;

	if (possibleHitsRemaining <= 0)
		return false;

	auto contents_grate = enterTrace.contents & CONTENTS_GRATE;
	auto surf_nodraw = enterTrace.surface.flags & SURF_NODRAW;

	auto enterSurfaceData = m_physsurface()->GetSurfaceData(enterTrace.surface.surfaceProps);
	auto enter_material = enterSurfaceData->game.material;

	auto is_solid_surf = enterTrace.contents >> 3 & CONTENTS_SOLID;
	auto is_light_surf = enterTrace.surface.flags >> 7 & SURF_LIGHT;

	trace_t exit_trace;

	if (!trace_to_exit(enterTrace, exit_trace, enterTrace.endpos, direction) && !(m_trace()->GetPointContents(enterTrace.endpos, MASK_SHOT_HULL) & MASK_SHOT_HULL))
		return false;

	auto enter_penetration_modifier = enterSurfaceData->game.flPenetrationModifier;
	auto exit_surface_data = m_physsurface()->GetSurfaceData(exit_trace.surface.surfaceProps);

	if (!exit_surface_data)
		return false;

	auto exit_material = exit_surface_data->game.material;
	auto exit_penetration_modifier = exit_surface_data->game.flPenetrationModifier;

	auto combined_damage_modifier = 0.16f;
	auto combined_penetration_modifier = (enter_penetration_modifier + exit_penetration_modifier) * 0.5f;

	if (enter_material == CHAR_TEX_GLASS || enter_material == CHAR_TEX_GRATE)
	{
		combined_penetration_modifier = 3.0f;
		combined_damage_modifier = 0.05f;
	}
	else if (contents_grate || surf_nodraw)
		combined_penetration_modifier = 1.0f;
	else if (enter_material == CHAR_TEX_FLESH && ((player_t*)enterTrace.hit_entity)->m_iTeamNum() == g_ctx.local()->m_iTeamNum() && !ff_damage_reduction_bullets && m_cvar()->FindVar("mp_teammates_are_enemies")->GetInt() == 0)
	{
		if (!ff_damage_bullet_penetration) //-V550
			return false;

		combined_penetration_modifier = ff_damage_bullet_penetration;
		combined_damage_modifier = 0.16f;
	}

	if (enter_material == exit_material)
	{
		if (exit_material == CHAR_TEX_WOOD || exit_material == CHAR_TEX_CARDBOARD)
			combined_penetration_modifier = 3.0f;
		else if (exit_material == CHAR_TEX_PLASTIC)
			combined_penetration_modifier = 2.0f;
	}

	auto penetration_modifier = std::fmaxf(0.0f, 1.0f / combined_penetration_modifier);
	auto penetration_distance = (exit_trace.endpos - enterTrace.endpos).Length();

	penetration_distance = penetration_distance * penetration_distance * penetration_modifier * 0.041666668f;

	auto damage_modifier = max(0.0f, 3.0f / weaponData->flPenetration * 1.25f) * penetration_modifier * 3.0f + currentDamage * combined_damage_modifier + penetration_distance;
	auto damage_lost = max(0.0f, damage_modifier);

	if (damage_lost > currentDamage)
		return false;

	currentDamage -= damage_lost;

	if (currentDamage < 1.0f)
		return false;

	eyePosition = exit_trace.endpos;
	--possibleHitsRemaining;

	return true;
}

bool autowall::fire_bullet(weapon_t* pWeapon, Vector& direction, bool& visible, float& currentDamage, int& hitbox, IClientEntity* e, float length, const Vector& pos)
{
	if (!pWeapon)
		return false;

	auto weaponData = pWeapon->get_csweapon_info();

	if (!weaponData)
		return false;

	CGameTrace enterTrace;
	CTraceFilter filter;

	filter.pSkip = g_ctx.local();
	currentDamage = weaponData->iDamage;

	auto eyePosition = pos;
	auto currentDistance = 0.0f;
	auto maxRange = weaponData->flRange;
	auto penetrationDistance = 3000.0f;
	auto penetrationPower = weaponData->flPenetration;
	auto possibleHitsRemaining = 4;

	while (currentDamage >= 1.0f)
	{
		maxRange -= currentDistance;
		auto end = eyePosition + direction * maxRange;

		CTraceFilter filter;
		filter.pSkip = g_ctx.local();

		util::trace_line(eyePosition, end, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &enterTrace);
		util::clip_trace_to_players(e, eyePosition, end + direction * 40.0f, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &enterTrace);

		auto enterSurfaceData = m_physsurface()->GetSurfaceData(enterTrace.surface.surfaceProps);
		auto enterSurfPenetrationModifier = enterSurfaceData->game.flPenetrationModifier;
		auto enterMaterial = enterSurfaceData->game.material;

		if (enterTrace.fraction == 1.0f)  //-V550
			break;

		currentDistance += enterTrace.fraction * maxRange;
		currentDamage *= pow(weaponData->flRangeModifier, currentDistance / 500.0f);

		if (currentDistance > penetrationDistance && weaponData->flPenetration || enterSurfPenetrationModifier < 0.1f)  //-V1051
			break;

		auto canDoDamage = enterTrace.hitgroup != HITGROUP_GEAR && enterTrace.hitgroup != HITGROUP_GENERIC;
		auto isPlayer = ((player_t*)enterTrace.hit_entity)->is_player();
		auto isEnemy = ((player_t*)enterTrace.hit_entity)->m_iTeamNum() != g_ctx.local()->m_iTeamNum() || m_cvar()->FindVar("mp_teammates_are_enemies")->GetInt() == 1;

		if (canDoDamage && isPlayer && isEnemy)
		{
			scale_damage((player_t*)enterTrace.hit_entity, enterTrace, weaponData, currentDamage);
			hitbox = enterTrace.hitbox;
			return true;
		}

		if (!possibleHitsRemaining)
			break;

		static auto damageReductionBullets = m_cvar()->FindVar(crypt_str("ff_damage_reduction_bullets"));
		static auto damageBulletPenetration = m_cvar()->FindVar(crypt_str("ff_damage_bullet_penetration"));

		if (!handle_bullet_penetration(weaponData, enterTrace, eyePosition, direction, possibleHitsRemaining, currentDamage, penetrationPower, damageReductionBullets->GetFloat(), damageBulletPenetration->GetFloat(), !e))
			break;

		visible = false;
	}

	return false;
}

autowall::returninfo_t autowall::wall_penetration(const Vector& eye_pos, Vector& point, IClientEntity* e)
{
	g_ctx.globals.autowalling = true;
	auto tmp = point - eye_pos;

	auto angles = ZERO;
	math::vector_angles(tmp, angles);

	auto direction = ZERO;
	math::angle_vectors(angles, direction);

	direction.NormalizeInPlace();

	auto visible = true;
	auto damage = -1.0f;
	auto hitbox = -1;

	auto weapon = g_ctx.local()->m_hActiveWeapon().Get();

	if (fire_bullet(weapon, direction, visible, damage, hitbox, e, 0.0f, eye_pos))
	{
		g_ctx.globals.autowalling = false;
		return returninfo_t(visible, (int)damage, hitbox); //-V2003
	}
	else
	{
		g_ctx.globals.autowalling = false;
		return returninfo_t(false, -1, -1);
	}
}

float autowall::can_hit_floating_point(Vector& vecEyePos, Vector& point)
{
	Vector angles, direction;
	Vector tmp = point = g_ctx.local()->get_shoot_position();
	float currentDamage = 0;

	math::vector_angles(tmp, angles);
	math::AngleVectors(angles, &direction);
	auto visible = true;
	auto hitbox = -1;
	direction.Normalize();

	auto local_weapon = (weapon_t*)(m_entitylist()->GetClientEntityFromHandle(g_ctx.local()->m_hActiveWeapon()));

	if (local_weapon != nullptr)
	{
		if (local_weapon->IsGun() && fire_bullet(local_weapon, vecEyePos, visible, currentDamage, hitbox, g_ctx.local()))
			return currentDamage;
		else
			return -1;
	}

	return -1;
}