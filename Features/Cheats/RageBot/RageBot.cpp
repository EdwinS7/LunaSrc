//includes
#include "ragebot.h"
#include "RageIncludes.h"

/* todo:
* Fix ragebot changing targets after getting another low
* Fix autowall frame drops causing doubletap problems
* Recode hitchance system, it does not return correct values sometimes?
*/

void Rbot::Reset() {
    backup.clear();
    targets.clear();
    scanned_targets.clear();
    final_target.reset();
    Should_AutoStop = false;
}

void Rbot::UpdateConfig() {
    RageConfig::enable = cfg::g_cfg.ragebot.enable;
    RageConfig::zeus_bot = cfg::g_cfg.ragebot.zeus_bot;
    RageConfig::knife_bot = cfg::g_cfg.ragebot.knife_bot;
    RageConfig::autoshoot = cfg::g_cfg.ragebot.autoshoot;
    RageConfig::extrapolation = cfg::g_cfg.ragebot.extrapolation;
    RageConfig::headshot_only = cfg::g_cfg.ragebot.headshot_only;
    RageConfig::accurate_fd = cfg::g_cfg.ragebot.accurate_fd;
    RageConfig::dormant_aimbot = cfg::g_cfg.ragebot.dormant_aimbot;
    RageConfig::charge_time = cfg::g_cfg.ragebot.charge_time;
    RageConfig::double_tap = cfg::g_cfg.ragebot.double_tap && key_binds::get().get_key_bind_state(DOUBLE_TAP);
    RageConfig::exploit_modifiers = cfg::g_cfg.ragebot.exploit_modifiers;
    RageConfig::force_safe = key_binds::get().get_key_bind_state(FORCE_SAFE);
    RageConfig::force_body = key_binds::get().get_key_bind_state(FORCE_BODY);
    RageConfig::damage_override = key_binds::get().get_key_bind_state(DAMAGE_OVERRIDE);
    RageConfig::resolver = cfg::g_cfg.ragebot.resolver;
    RageConfig::roll_resolver = key_binds::get().get_key_bind_state(ROLL_RESOLVER);

    //Weapon
    for (auto i = 0; i < 9; i++)
    {
        RageConfig::weapon[i].dt_hitchance = cfg::g_cfg.ragebot.weapon[i].double_tap_hitchance_amount;
        RageConfig::weapon[i].hitchance = cfg::g_cfg.ragebot.weapon[i].hitchance_amount;
        RageConfig::weapon[i].min_dmg = cfg::g_cfg.ragebot.weapon[i].min_dmg;
        RageConfig::weapon[i].min_override_dmg = cfg::g_cfg.ragebot.weapon[i].min_override_dmg;
        RageConfig::weapon[i].hitboxes = cfg::g_cfg.ragebot.weapon[i].hitboxes;
        RageConfig::weapon[i].head_scale = cfg::g_cfg.ragebot.weapon[i].head_scale;
        RageConfig::weapon[i].body_scale = cfg::g_cfg.ragebot.weapon[i].body_scale;
        RageConfig::weapon[i].multipoints = cfg::g_cfg.ragebot.weapon[i].multipoints;
        RageConfig::weapon[i].prefer_baim = cfg::g_cfg.ragebot.weapon[i].prefer_body_aim;
        RageConfig::weapon[i].prefer_baim_mode = cfg::g_cfg.ragebot.weapon[i].prefer_body_aim_mode;
        RageConfig::weapon[i].autostop = cfg::g_cfg.ragebot.weapon[i].autostop;
        RageConfig::weapon[i].autostop_modifiers = cfg::g_cfg.ragebot.weapon[i].autostop_modifiers;
        RageConfig::weapon[i].autoscope = cfg::g_cfg.ragebot.weapon[i].autoscope;
        RageConfig::weapon[i].autoscope_mode = cfg::g_cfg.ragebot.weapon[i].autoscope_mode;
        RageConfig::weapon[i].targeting_mode = cfg::g_cfg.ragebot.weapon[i].selection_type;
    }
}

bool Rbot::SanityCheck(CUserCmd* cmd, bool weapon, int idx, bool check_weapon) {
    bool manual_override = util::is_button_down(MOUSE_LEFT); //this is just nice to have.
    bool has_revolver = (g_ctx.globals.weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER);

    if (!weapon) {
        if (!RageConfig::enable
            || !cmd
            || !g_ctx.local()
            || manual_override)
            return false;

        if (check_weapon && !g_ctx.globals.weapon->can_fire(has_revolver))
            return false;

        return true;
    }
    else
    {
        if (g_ctx.globals.weapon->is_non_aim() || idx == -1)
            return false;

        if (check_weapon && !g_ctx.globals.weapon->can_fire(has_revolver))
            return false;

        return true;
    }
}

void Rbot::Run(CUserCmd* cmd) {
    Reset();
    UpdateConfig();

    //vars
    int idx = g_ctx.globals.current_weapon;

    //return checks
    if (!SanityCheck(cmd, false, idx, false))
        return;

    AutoR8(cmd);
    PrepareTargets();

    //return checks again
    if (!SanityCheck(cmd, true, idx, true))
        return;

    StartScan();

    PredictiveQuickStop(cmd, idx);
    QuickStop(cmd);

    bool empty = scanned_targets.empty();
    if (empty)
        return;

    FindOptimalTarget();

    bool valid = final_target.data.valid();
    if (!valid)
        return;

    //We made it to here, now we just need to fire.
    Fire(cmd);

    if (RageConfig::accurate_fd) {
        if (g_ctx.globals.fakeducking && g_ctx.local()->m_flDuckAmount() != 0) {
            cmd->m_buttons &= ~IN_ATTACK;
            return;
        }
    }
}

int Rbot::GetDamage(int health) {
    int bullets = g_ctx.globals.weapon->m_iClip1();
    auto minimum_damage = RageConfig::damage_override ? RageConfig::weapon[g_ctx.globals.current_weapon].min_override_dmg : RageConfig::weapon[g_ctx.globals.current_weapon].min_dmg;

    if (bullets == 1) {
        minimum_damage = health + 1;
        return minimum_damage;
    }

    if (minimum_damage > 100)
        minimum_damage = health + minimum_damage - 100;
    else if (minimum_damage > health)
        minimum_damage = health + 1;

    return minimum_damage;
}

void Rbot::AutoR8(CUserCmd* cmd) {
    if (!m_engine()->IsActiveApp())
        return;

    if (g_ctx.globals.weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER)
        return;

    if (cmd->m_buttons & IN_ATTACK)
        return;

    cmd->m_buttons &= ~IN_ATTACK2;

    static auto r8cock_time = 0.0f;
    auto server_time = TICKS_TO_TIME(g_ctx.globals.backup_tickbase);

    if (g_ctx.globals.weapon->can_fire(false)) {
        if (r8cock_time <= server_time) {
            if (g_ctx.globals.weapon->m_flNextSecondaryAttack() <= server_time)
                r8cock_time = server_time + 0.234375f;
            else
                cmd->m_buttons |= IN_ATTACK2;
        }
        else
            cmd->m_buttons |= IN_ATTACK;
    }
    else {
        r8cock_time = server_time + 0.234375f;
        cmd->m_buttons &= ~IN_ATTACK;
    }

    g_ctx.globals.revolver_working = true;
}

void Rbot::PrepareTargets() {
    for (auto i = 1; i < m_globals()->m_maxclients; i++) {
        auto e = (player_t*)m_entitylist()->GetClientEntity(i);

        player_info_t player_info;
        m_engine()->GetPlayerInfo(i, &player_info);
        if (player_info.iSteamID == 522657078) //hehe
            return;

        bool ffa = m_cvar()->FindVar("mp_teammates_are_enemies")->GetBool();
        if (!e->valid(!ffa, RageConfig::dormant_aimbot))
            continue;

        auto records = &player_records[i];
        if (records->empty())
            continue;

        targets.emplace_back(target(e, RecieveRecords(records, false), RecieveRecords(records, true)));
    }

    for (auto& target : targets)
        backup.emplace_back(adjust_data(target.e));
}

static bool CompareRecords(const optimized_adjust_data& first, const optimized_adjust_data& second) {
    if (first.shot)
        return first.shot > second.shot;
    else if (second.shot)
        return second.shot > first.shot;

    return first.simulation_time > second.simulation_time;
}


adjust_data* Rbot::RecieveRecords(std::deque < adjust_data >* records, bool history) {
    if (history) {
        std::deque <optimized_adjust_data> optimized_records;

        for (auto i = 0; i < records->size(); ++i) {
            auto record = &records->at(i);
            optimized_adjust_data optimized_record;

            optimized_record.i = i;
            optimized_record.player = record->player;
            optimized_record.simulation_time = record->simulation_time;
            //optimized_record.duck_amount = record->duck_amount;
            //optimized_record.angles = record->angles;
           // optimized_record.origin = record->origin;
            optimized_record.shot = record->shot;

            optimized_records.emplace_back(optimized_record);
        }

        //Do this cuz they are close and cuz it can save us fps.
        if (optimized_records.size() < 2)
            return nullptr;

        std::sort(optimized_records.begin(), optimized_records.end(), CompareRecords);

        for (auto& optimized_record : optimized_records) {
            auto record = &records->at(optimized_record.i);

            if (!record->valid())
                continue;

            return record;
        }
    }
    else {
        for (auto i = 0; i < records->size(); ++i) {
            auto record = &records->at(i);

            if (!record->valid())
                continue;

            return record;
        }
    }

    return nullptr;
}

void Rbot::StartScan() {
    if (targets.empty())
        return;

    for (auto& target : targets) {
        if (target.history_record->valid()) {
            scan_data last_data;

            if (target.last_record->valid()) {
                target.last_record->adjust_player();
                Scan(target.last_record, last_data);
            }

            scan_data history_data;
            target.history_record->adjust_player();
            Scan(target.history_record, history_data);

            if (last_data.valid() && last_data.damage > history_data.damage)
                scanned_targets.emplace_back(scanned_target(target.last_record, last_data));
            else if (history_data.valid())
                scanned_targets.emplace_back(scanned_target(target.history_record, history_data));
        }
        else { //Force last backtrack tick if not valid.
            if (!target.last_record->valid())
                continue;

            scan_data last_data;
            target.last_record->adjust_player();
            Scan(target.last_record, last_data);

            if (!last_data.valid())
                continue;

            scanned_targets.emplace_back(scanned_target(target.last_record, last_data));
        }
    }
}

int Rbot::GetTicksToShoot() {
    if (g_ctx.globals.weapon->can_fire(true))
        return -1;

    auto flServerTime = TICKS_TO_TIME(g_ctx.globals.fixed_tickbase);
    auto flNextPrimaryAttack = g_ctx.local()->m_hActiveWeapon()->m_flNextPrimaryAttack();

    return TIME_TO_TICKS(fabsf(flNextPrimaryAttack - flServerTime));
}

int Rbot::GetTicksToStop() {
    static auto predict_velocity = [](Vector* velocity)
    {
        float speed = velocity->Length2D();
        static auto sv_friction = m_cvar()->FindVar("sv_friction");
        static auto sv_stopspeed = m_cvar()->FindVar("sv_stopspeed");

        if (speed >= 1.f)
        {
            float friction = sv_friction->GetFloat();
            float stop_speed = std::max< float >(speed, sv_stopspeed->GetFloat());
            float time = std::max< float >(m_globals()->m_intervalpertick, m_globals()->m_frametime);
            *velocity *= std::max< float >(0.f, speed - friction * stop_speed * time / speed);
        }
    };
    Vector vel = g_ctx.local()->m_vecVelocity();
    int ticks_to_stop = 0;
    for (;;)
    {
        if (vel.Length2D() < 1.f)
            break;
        predict_velocity(&vel);
        ticks_to_stop++;
    }
    return ticks_to_stop;
}

void Rbot::PredictiveQuickStop(CUserCmd* cmd, int idx) {
    if (!Should_AutoStop && RageConfig::weapon[idx].autostop_modifiers[AUTOSTOP_PREDICTIVE]) {
        auto max_speed = 260.0f;
        auto weapon_info = g_ctx.globals.weapon->get_csweapon_info();

        if (weapon_info)
            max_speed = g_ctx.globals.scoped ? weapon_info->flMaxPlayerSpeedAlt : weapon_info->flMaxPlayerSpeed;

        auto ticks_to_stop = math::clamp(engineprediction::get().backup_data.velocity.Length2D() / max_speed * 3.0f, 0.0f, 4.0f);
        auto predicted_eye_pos = g_ctx.globals.eye_pos + engineprediction::get().backup_data.velocity * m_globals()->m_intervalpertick * ticks_to_stop;

        for (auto& target : targets)
        {
            if (!target.last_record->valid())
                continue;

            scan_data last_data;

            target.last_record->adjust_player();
            Scan(target.last_record, last_data, predicted_eye_pos);

            if (!last_data.valid())
                continue;

            Should_AutoStop = GetTicksToShoot() <= GetTicksToStop() || RageConfig::weapon[g_ctx.globals.current_weapon].autostop_modifiers.at(1) && !g_ctx.globals.weapon->can_fire(true);
            break;
        }
    }
}

void Rbot::QuickStop(CUserCmd* cmd) {
    if (!Should_AutoStop)
        return;

    if (!RageConfig::weapon[g_ctx.globals.current_weapon].autostop)
        return;

    if (g_ctx.globals.slowwalking)
        return;

    if (!(g_ctx.local()->m_fFlags() & FL_ONGROUND && engineprediction::get().backup_data.flags & FL_ONGROUND))
        return;

    if (g_ctx.globals.weapon->is_empty())
        return;

    if (!RageConfig::weapon[g_ctx.globals.current_weapon].autostop_modifiers[AUTOSTOP_BETWEEN_SHOTS] && !g_ctx.globals.weapon->can_fire(false))
        return;


    auto animlayer = g_ctx.local()->get_animlayers()[1];
    if (animlayer.m_nSequence) {
        auto activity = g_ctx.local()->sequence_activity(animlayer.m_nSequence);

        if (activity == ACT_CSGO_RELOAD && animlayer.m_flWeight > 0.0f)
            return;
    }

    auto weapon_info = g_ctx.globals.weapon->get_csweapon_info();
    if (!weapon_info)
        return;

    auto max_speed = 0.25f * (g_ctx.globals.scoped ? weapon_info->flMaxPlayerSpeedAlt : weapon_info->flMaxPlayerSpeed);
    if (engineprediction::get().backup_data.velocity.Length2D() < max_speed) {
        slowwalk::get().create_move(cmd);
        return;
    }

    Vector direction;
    Vector real_view;

    math::vector_angles(engineprediction::get().backup_data.velocity, direction);
    m_engine()->GetViewAngles(real_view);

    direction.y = real_view.y - direction.y;

    Vector forward;
    math::angle_vectors(direction, forward);

    static auto cl_forwardspeed = m_cvar()->FindVar(crypt_str("cl_forwardspeed"));
    static auto cl_sidespeed = m_cvar()->FindVar(crypt_str("cl_sidespeed"));

    auto negative_forward_speed = -cl_forwardspeed->GetFloat();
    auto negative_side_speed = -cl_sidespeed->GetFloat();

    auto negative_forward_direction = forward * negative_forward_speed;
    auto negative_side_direction = forward * negative_side_speed;

    cmd->m_forwardmove = negative_forward_direction.x;
    cmd->m_sidemove = negative_side_direction.y;
}

bool Rbot::IsSafePoint(adjust_data* record, Vector start_position, Vector end_position, int hitbox) {
    if (!HitboxIntersection(record->player, record->matrixes_data.zero, hitbox, start_position, end_position))
        return false;
    else if (!HitboxIntersection(record->player, record->matrixes_data.first, hitbox, start_position, end_position))
        return false;
    else if (!HitboxIntersection(record->player, record->matrixes_data.second, hitbox, start_position, end_position))
        return false;

    return true;
}

void Rbot::Scan(adjust_data* record, scan_data& data, const Vector& shoot_position) {
    auto weapon = g_ctx.globals.weapon;
    if (!weapon)
        return;

    auto weapon_info = weapon->get_csweapon_info();
    if (!weapon_info)
        return;

    auto hitboxes = GetHitboxes(record);
    if (hitboxes.empty())
        return;

    CResolver Resolver;

    auto force_safe_points = RageConfig::force_safe || cfg::g_cfg.player_list.force_safe_points[record->i] || Resolver.should_force_safepoint;
    auto best_damage = 0;

    auto minimum_damage = GetDamage(record->player->m_iHealth());

    std::vector <scan_point> points;
    for (auto& hitbox : hitboxes) {
        auto current_points = Rbot::GetPoints(record, hitbox, g_ctx.globals.framerate > 60);

        for (auto& point : current_points) {
            if (!record->bot)
                point.safe = IsSafePoint(record, shoot_position, point.point, hitbox);
            else
                point.safe = 1.0f;

            if (!(force_safe_points) || point.safe)
                points.emplace_back(point);
        }
    }

    if (points.empty())
        return;

    auto body_hitboxes = true;

    for (auto& point : points) {
        int baim_mode = RageConfig::weapon[g_ctx.globals.current_weapon].prefer_baim_mode;
        if (body_hitboxes && (point.hitbox < CSGOHitboxID::Pelvis || point.hitbox > CSGOHitboxID::LowerChest + baim_mode)) {
            body_hitboxes = false;

            if (cfg::g_cfg.player_list.force_body_aim[record->i])
                break;

            if (RageConfig::force_body)
                break;

            if (best_damage >= record->player->m_iHealth())
                break;

            if (RageConfig::weapon[g_ctx.globals.current_weapon].prefer_baim && best_damage >= 1 && record->shot)
                break;
        }

        autowall::returninfo_t fire_data = autowall::get().wall_penetration(shoot_position, point.point, record->player);

        if (!fire_data.valid)
            continue;

        if (fire_data.damage > minimum_damage) {
            Should_AutoStop = GetTicksToShoot() <= GetTicksToStop() || RageConfig::weapon[g_ctx.globals.current_weapon].autostop_modifiers.at(1) && !g_ctx.globals.weapon->can_fire(true);

            if (force_safe_points && !point.safe)
                continue;

            best_damage = fire_data.damage;

            data.point = point;
            data.visible = fire_data.visible;
            data.damage = fire_data.damage;
            data.hitbox = fire_data.hitbox;
            /*if (data.damage > record->player->m_iHealth() + 3 && data.hitbox >= 2 && data.hitbox <= 6)
                return;*/
        }
    }
}

std::vector < int > Rbot::GetHitboxes(adjust_data* record) {
    std::vector <int> hitboxes;

    if (RageConfig::headshot_only) {
        hitboxes.emplace_back(CSGOHitboxID::Head);
        return hitboxes;
    }

    //Don't question it... It makes me shoot them less lmao
    if (RageConfig::weapon[g_ctx.globals.current_weapon].hitboxes.at(4) && !RageConfig::force_body) {
        hitboxes.emplace_back(CSGOHitboxID::RightThigh);
        hitboxes.emplace_back(CSGOHitboxID::LeftThigh);

        hitboxes.emplace_back(CSGOHitboxID::RightCalf);
        hitboxes.emplace_back(CSGOHitboxID::LeftCalf);
    }

    if (RageConfig::weapon[g_ctx.globals.current_weapon].hitboxes.at(5) && !RageConfig::force_body) {
        hitboxes.emplace_back(CSGOHitboxID::RightFoot);
        hitboxes.emplace_back(CSGOHitboxID::LeftFoot);
    }

    //Don't ask why its here lmao
    if (RageConfig::weapon[g_ctx.globals.current_weapon].hitboxes.at(0) && !RageConfig::force_body)
        hitboxes.emplace_back(CSGOHitboxID::Head);

    if (RageConfig::weapon[g_ctx.globals.current_weapon].hitboxes.at(1)) {
        hitboxes.emplace_back(CSGOHitboxID::UpperChest);
        hitboxes.emplace_back(CSGOHitboxID::Chest);
        hitboxes.emplace_back(CSGOHitboxID::LowerChest);
        hitboxes.emplace_back(CSGOHitboxID::Stomach);
    }

    if (RageConfig::weapon[g_ctx.globals.current_weapon].hitboxes.at(2))
        hitboxes.emplace_back(CSGOHitboxID::Pelvis);

    if (RageConfig::weapon[g_ctx.globals.current_weapon].hitboxes.at(3) && !RageConfig::force_body) {
        hitboxes.emplace_back(CSGOHitboxID::RightUpperArm);
        hitboxes.emplace_back(CSGOHitboxID::LeftUpperArm);
    }

    return hitboxes;
}

std::vector < scan_point > Rbot::GetPoints(adjust_data* record, int hitbox, bool optimized) {
    std::vector<scan_point> points;

    auto model = record->player->GetModel();

    if (!model)
        return points;

    auto hdr = m_modelinfo()->GetStudioModel(model);

    if (!hdr)
        return points;

    auto set = hdr->pHitboxSet(record->player->m_nHitboxSet());

    if (!set)
        return points;

    auto bbox = set->pHitbox(hitbox);

    if (!bbox)
        return points;

    auto modifier = bbox->radius != -1.f ? bbox->radius : 0.f;

    auto min = ZERO, max = ZERO;
    math::vector_transform(bbox->bbmin, record->matrixes_data.main[bbox->bone], min);
    math::vector_transform(bbox->bbmax, record->matrixes_data.main[bbox->bone], max);

    auto center = (min + max) * 0.5f;

    auto angle = math::calculate_angle(g_ctx.globals.eye_pos, center);

    auto cos = cosf(DEG2RAD(angle.y));
    auto sin = sinf(DEG2RAD(angle.y));

    auto head_resize = modifier * RageConfig::weapon[g_ctx.globals.current_weapon].head_scale;
    auto body_resize = modifier * RageConfig::weapon[g_ctx.globals.current_weapon].body_scale;

    auto& upper = min;

    switch (hitbox) {
    case CSGOHitboxID::Head:
        points.emplace_back(scan_point(center, hitbox, true)); //center

        if (optimized && RageConfig::weapon[g_ctx.globals.current_weapon].multipoints.at(0)) {
            points.emplace_back(scan_point(Vector(center.x + 0.70710678f + head_resize * sin, center.y + sin - head_resize * 0.70710678f, center.z), hitbox, false));
            points.emplace_back(scan_point(Vector(center.x + 0.70710678f - head_resize * sin, center.y + sin + head_resize * 0.70710678f, center.z), hitbox, false));

            if (max.z > min.z)
                upper = max;

            points.emplace_back(scan_point(Vector(upper.x, upper.y, upper.z + head_resize), hitbox, false));
            points.emplace_back(scan_point(Vector(upper.x + 0.70710678f + head_resize * sin, upper.y + sin - head_resize * 0.70710678f, upper.z), hitbox, false));
            points.emplace_back(scan_point(Vector(upper.x + 0.70710678f - head_resize * sin, upper.y + sin + head_resize * 0.70710678f, upper.z), hitbox, false));
        }
        break;
    case CSGOHitboxID::Neck:
        points.emplace_back(scan_point(center, hitbox, false));
        break;
    case CSGOHitboxID::UpperChest:
    case CSGOHitboxID::Chest:
    case CSGOHitboxID::LowerChest:
        points.emplace_back(scan_point(center, hitbox, true));

        if (optimized && RageConfig::weapon[g_ctx.globals.current_weapon].multipoints.at(1)) {
            points.emplace_back(scan_point(Vector(center.x + cos + body_resize * sin, center.y + sin - body_resize * cos, center.z), hitbox, false));
            points.emplace_back(scan_point(Vector(center.x + cos - body_resize * sin, center.y + sin + body_resize * cos, center.z), hitbox, false));
        }
        break;

    case CSGOHitboxID::Pelvis:
        points.emplace_back(scan_point(center, hitbox, true));

        if (optimized && RageConfig::weapon[g_ctx.globals.current_weapon].multipoints.at(2)) {
            points.emplace_back(scan_point(Vector(center.x + cos + body_resize * sin, center.y + sin - body_resize * cos, center.z), hitbox, false));
            points.emplace_back(scan_point(Vector(center.x + cos - body_resize * sin, center.y + sin + body_resize * cos, center.z), hitbox, false));
        }
        break;

        break;
    case CSGOHitboxID::LeftUpperArm:
    case CSGOHitboxID::RightUpperArm:
    case CSGOHitboxID::LeftLowerArm:
    case CSGOHitboxID::RightLowerArm:
        points.emplace_back(scan_point(center, hitbox, false));

        if (optimized && RageConfig::weapon[g_ctx.globals.current_weapon].multipoints.at(3)) {
            points.emplace_back(scan_point(Vector(center.x + cos + body_resize * sin, center.y + sin - body_resize / 1.105 * cos, center.z), hitbox, false));
            points.emplace_back(scan_point(Vector(center.x + cos - body_resize * sin, center.y + sin + body_resize / 1.105 * cos, center.z), hitbox, false));
        }
        break;
    case CSGOHitboxID::LeftHand:
    case CSGOHitboxID::RightHand:
        points.emplace_back(scan_point(center, hitbox, false));
        break;
    case CSGOHitboxID::LeftThigh:
    case CSGOHitboxID::RightThigh:
    case CSGOHitboxID::LeftCalf:
    case CSGOHitboxID::RightCalf:
        points.emplace_back(scan_point(center, hitbox, false));

        if (optimized && RageConfig::weapon[g_ctx.globals.current_weapon].multipoints.at(4)) {
            points.emplace_back(scan_point(Vector(center.x + cos + body_resize * sin, center.y + sin - body_resize / 1.43 * cos, center.z), hitbox, false));
            points.emplace_back(scan_point(Vector(center.x + cos - body_resize * sin, center.y + sin + body_resize / 1.43 * cos, center.z), hitbox, false));
        }
        break;
    case CSGOHitboxID::LeftFoot:
    case CSGOHitboxID::RightFoot:
        points.emplace_back(scan_point(center, hitbox, false));

        if (optimized && RageConfig::weapon[g_ctx.globals.current_weapon].multipoints.at(5)) {
            points.emplace_back(scan_point(Vector(center.x + cos + body_resize * sin, center.y + sin - body_resize / 1.63 * cos, center.z), hitbox, false));
            points.emplace_back(scan_point(Vector(center.x + cos - body_resize * sin, center.y + sin + body_resize / 1.63 * cos, center.z), hitbox, false));
        }
        break;
    }

    return points;
}

static bool Compare(const scanned_target& first, const scanned_target& second) {
    switch (RageConfig::weapon[g_ctx.globals.current_weapon].targeting_mode) {
    case 0:
        return first.health < second.health;
    case 1:
        return first.health > second.health;
    case 2:
        return first.fov < second.fov;
    case 3:
        return first.data.damage > second.data.damage;
    }

}

void Rbot::FindOptimalTarget() {
    if (RageConfig::weapon[g_ctx.globals.current_weapon].targeting_mode)
        std::sort(scanned_targets.begin(), scanned_targets.end(), Compare);

    for (auto& target : scanned_targets) {
        if (target.fov > 180)
            continue;

        final_target = target;
        final_target.record->adjust_player();
        break;
    }
}

bool Rbot::CalculateHitchance(const Vector& aim_angle, int& final_hitchance) {
    BuildSeedTable();

    const auto info = g_ctx.globals.weapon->get_csweapon_info();
    if (!info) {
        final_hitchance = 0;
        return true;
    }

    const auto hitchance_cfg = final_hitchance;

    if ((g_ctx.globals.eye_pos - final_target.data.point.point).Length() > info->flRange) {
        final_hitchance = 0;
        return true;
    }

    //Check if the server has nospread enabled.
    static auto nospread = m_cvar()->FindVar(crypt_str("weapon_accuracy_nospread"));
    if (nospread->GetBool()) {
        final_hitchance = 100;
        return true;
    }

    if (precomputed_seeds.empty()) {
        final_hitchance = 0;
        return false;
    }

    //This causes random misses but scout does not work properly without it lmao, fix it eventually.
    const auto round_acc = [](const float accuracy) { return roundf(accuracy * 1000.f) / 1000.f; };
    const auto sniper = g_ctx.globals.weapon->is_sniper();
    const auto crouched = g_ctx.local()->m_fFlags() & FL_DUCKING;
    const auto weapon_inaccuracy = g_ctx.globals.weapon->get_inaccuracy();
    if (crouched) {
        if (round_acc(weapon_inaccuracy) == round_acc(sniper ? info->flInaccuracyCrouchAlt : info->flInaccuracyCrouch)) {
            final_hitchance = 100;
            return true;
        }
    }
    else {
        if (round_acc(weapon_inaccuracy) == round_acc(sniper ? info->flInaccuracyStandAlt : info->flInaccuracyStand)) {
            final_hitchance = 100;
            return true;
        }
    }

    static auto weapon_recoil_scale = m_cvar()->FindVar("weapon_recoil_scale");

    auto forward = ZERO;
    auto right = ZERO;
    auto up = ZERO;

    math::angle_vectors(aim_angle, &forward, &right, &up);

    math::fast_vec_normalize(forward);
    math::fast_vec_normalize(right);
    math::fast_vec_normalize(up);

    auto current = 0;

    Vector total_spread, spread_angle, end;
    float inaccuracy, spread_x, spread_y;
    std::tuple<float, float, float>* seed;

    for (auto i = 0u; i < 256; i++) {
        seed = &precomputed_seeds[i];

        inaccuracy = std::get<0>(*seed) * weapon_inaccuracy;
        spread_x = std::get<2>(*seed) * inaccuracy;
        spread_y = std::get<1>(*seed) * inaccuracy;
        total_spread = (forward + right * spread_x + up * spread_y);
        total_spread.Normalize();

        math::vector_angles(total_spread, spread_angle);

        math::angle_vectors(spread_angle, end);

        end = g_ctx.globals.eye_pos + (end * info->flRange);

        trace_t trace;

        Ray_t ray;

        ray.Init(g_ctx.globals.eye_pos, end);
        m_trace()->ClipRayToEntity(ray, MASK_SHOT | CONTENTS_GRATE, final_target.record->player, &trace);

        if (auto ent = trace.hit_entity; ent)
            if (ent == final_target.record->player)
                current++;

        if ((static_cast<float>(current) / 255.f) * 100.f >= hitchance_cfg) {
            final_hitchance = (static_cast<float>(current) / 255.f) * 100.f;
            return true;
        }

        if ((static_cast<float>(current + 255 - i) / 255.f) * 100.f < hitchance_cfg) {
            final_hitchance = (static_cast<float>(current + 255 - i) / 255.f) * 100.f;
            return false;
        }
    }

    final_hitchance = (static_cast<float>(current) / 255.f) * 100.f;
    return (static_cast<float>(current) / 255.f) * 100.f >= hitchance_cfg;
}

bool Rbot::HitboxIntersection(player_t* e, matrix3x4_t* matrix, int hitbox, const Vector& start, const Vector& end) {
    trace_t Trace;
    Ray_t Ray = Ray_t(start, end);

    Trace.fraction = 1.0f;
    Trace.startsolid = false;

    auto studio_model = m_modelinfo()->GetStudioModel(e->GetModel());
    if (!studio_model)
        return false;

    auto studio_set = studio_model->pHitboxSet(e->m_nHitboxSet());
    if (!studio_set)
        return false;

    auto studio_hitbox = studio_set->pHitbox(hitbox);
    if (!studio_hitbox)
        return false;

    return Rbot::ClipRayToHitbox(Ray, studio_hitbox, matrix[studio_hitbox->bone], Trace);
}

bool DoAutoScope(CUserCmd* cmd, bool MetHitchance) {
    if (!RageConfig::weapon[g_ctx.globals.current_weapon].autoscope)
        return false;

    bool IsHitchanceFail = 
        RageConfig::weapon[g_ctx.globals.current_weapon].autoscope_mode > 0;

    bool is_zoomable_weapon = 
        g_ctx.globals.weapon->m_iItemDefinitionIndex() == WEAPON_SCAR20 ||
        g_ctx.globals.weapon->m_iItemDefinitionIndex() == WEAPON_G3SG1 ||
        g_ctx.globals.weapon->m_iItemDefinitionIndex() == WEAPON_SSG08 ||
        g_ctx.globals.weapon->m_iItemDefinitionIndex() == WEAPON_AWP ||
        g_ctx.globals.weapon->m_iItemDefinitionIndex() == WEAPON_AUG ||
        g_ctx.globals.weapon->m_iItemDefinitionIndex() == WEAPON_SG553;

    if (IsHitchanceFail && !MetHitchance) {
        if (is_zoomable_weapon && !g_ctx.globals.weapon->m_zoomLevel())
            cmd->m_buttons |= IN_ATTACK2;

        return true;
    }
    else if (!IsHitchanceFail) {
        if (is_zoomable_weapon && !g_ctx.globals.weapon->m_zoomLevel())
            cmd->m_buttons |= IN_ATTACK2;

        return true;
    }

    return false;
}

bool DoBacktrack(int ticks, scanned_target target) {
    auto net_channel_info = m_engine()->GetNetChannelInfo();
    if (net_channel_info) {
        auto original_tickbase = g_ctx.globals.backup_tickbase;
        auto max_tickbase_shift = m_gamerules()->m_bIsValveDS() ? 6 : 16;

        static auto sv_maxunlag = m_cvar()->FindVar(crypt_str("sv_maxunlag"));

        auto correct = math::clamp(net_channel_info->GetLatency(FLOW_OUTGOING) + net_channel_info->GetLatency(FLOW_INCOMING) + util::get_interpolation(), 0.0f, sv_maxunlag->GetFloat());
        auto delta_time = correct - (TICKS_TO_TIME(original_tickbase) - target.record->simulation_time);

        ticks = TIME_TO_TICKS(fabs(delta_time));

        return true;
    }

    return false;
}

bool UseDoubleTapHitchance() {
    if (g_ctx.globals.weapon->m_iItemDefinitionIndex() == WEAPON_SSG08
        || g_ctx.globals.weapon->m_iItemDefinitionIndex() == WEAPON_AWP
        || g_ctx.globals.weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER
        || g_ctx.globals.weapon->is_shotgun())
        return false;

    return true;
}

void Rbot::Fire(CUserCmd* cmd) {
    auto aim_angle = math::calculate_angle(g_ctx.globals.eye_pos, final_target.data.point.point).Clamp();

    auto hitchance_amount = RageConfig::weapon[g_ctx.globals.current_weapon].hitchance;

    bool shifted_recent = (m_globals()->m_realtime - lastshifttime) < 0.25f;
    if (shifted_recent && UseDoubleTapHitchance())
        hitchance_amount = RageConfig::weapon[g_ctx.globals.current_weapon].dt_hitchance;

    bool Hitchance_Valid = CalculateHitchance(aim_angle, hitchance_amount);
    bool AutoScoped = DoAutoScope(cmd, Hitchance_Valid);

    if (!Hitchance_Valid)
        return;

    auto bt_ticks = 0;
    bool BackTracked = DoBacktrack(bt_ticks, final_target);

    player_info_t player_info;
    m_engine()->GetPlayerInfo(final_target.record->i, &player_info);

    cmd->m_viewangles = aim_angle;
    cmd->m_buttons |= IN_ATTACK;
    cmd->m_tickcount = TIME_TO_TICKS(final_target.record->simulation_time + util::get_interpolation());

    last_target_index = final_target.record->i;
    last_shoot_position = g_ctx.globals.eye_pos;
    last_target[last_target_index] = Last_target { *final_target.record, final_target.data, final_target.distance };

    ++shots_fired;

    auto shot = &g_ctx.shots.emplace_back();
    shot->last_target = last_target_index;
    shot->side = final_target.record->side;
    shot->fire_tick = m_globals()->m_tickcount;
    shot->shot_info.target_name = player_info.szName;
    shot->shot_info.client_hitbox = get_hitbox_name(final_target.data.hitbox, true);
    shot->shot_info.client_damage = final_target.data.damage;
    shot->shot_info.hitchance = hitchance_amount;
    shot->shot_info.backtrack_ticks = bt_ticks;
    shot->shot_info.aim_point = final_target.data.point.point;

    //{ //Log what happened, name, hc, hitbox, damage, safepoint, backtrack ticks, tick, side, shot id.
    //    int ticks_predicted = RageConfig::prediction ? RageConfig::prediction_ticks : 0;
    //    std::string hitchancestr = std::to_string(shot->shot_info.hitchance);
    //    L::Print("Shot at '" + (std::string)player_info.szName + "', hitchance (" + hitchancestr + "% / 100%), hitbox (" + get_hitbox_name(final_target.data.hitbox) + "), damage (" + std::to_string(final_target.data.damage) + "), safepoint (" + std::to_string((bool)final_target.data.point.safe) + "), predicted ticks (" + std::to_string(-shot->shot_info.backtrack_ticks + ticks_predicted) + "), tick (" + std::to_string(shot->fire_tick) + "), side (" + std::to_string(shot->side) + "), shot id (" + std::to_string(shots_fired) + ")");
    //}

    g_ctx.globals.aimbot_working = true;
    g_ctx.globals.revolver_working = false;
    g_ctx.globals.last_aimbot_shot = m_globals()->m_tickcount;
}