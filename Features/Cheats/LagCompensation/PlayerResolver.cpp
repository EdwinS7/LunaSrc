#include "LagCompensation.h"
#include "..\ragebot\ragebot.h"
#include <Features\logging.h>

void CResolver::Init(player_t* e, adjust_data* record, const float& goal_feet_yaw, const float& pitch) {
    player = e;
    player_record = record;

    original_goal_feet_yaw = math::normalize_yaw(goal_feet_yaw);
    original_pitch = math::normalize_pitch(pitch);
}

void CResolver::Reset() {
    player = nullptr;
    player_record = nullptr;


    original_goal_feet_yaw = 0.0f;
    original_pitch = 0.0f;
}

//We will only go to this if we don't know where there head is.
//This function just bruteforces where there hitbox will be so we can hit it. (duh)
float CResolver::BruteForce(player_t* player, bool roll) {
    c_baseplayeranimationstate* animstate = player->get_animation_state();

    auto idx = player->EntIndex();
    auto missed_shots = g_ctx.globals.missed_shots[idx];

    float delta = 60.f;
    if (!roll) {
        switch (missed_shots % 6) {
        case 3:
            delta = 0.f;
            break;
        case 4:
            delta = 58.f;
            break;
        case 5:
            delta = -58.f;
            break;
        case 6:
            delta = 29.f;
            break;
        case 7:
            delta = -29.f;
            break;
        default:
            delta = 0.f;
            break;
        }
    }
    else {
        switch (missed_shots % 6) {
        case 3:
            delta = 50.f;
            break;
        case 4:
            delta = -50.f;
            break;
        case 5:
            delta = 25.f;
            break;
        case 6:
            delta = -25.5f;
            break;
        case 7:
            delta = 0.f;
            break;
        default:
            delta = 0.f;
            break;
        }
    }

    return delta;
}

//Thanks weave.su
float CResolver::GetAngle(player_t* player) {
    return math::normalize_yaw(player->m_angEyeAngles().y);
}

float CResolver::GetAwayAngle(player_t* player) {
    return math::calculate_angle(g_ctx.local()->GetEyePos(), player->GetEyePos()).y;
}

float CResolver::GetBackwardYaw(player_t* player) {
    return math::calculate_angle(g_ctx.local()->m_vecOrigin(), player->m_vecOrigin()).y;
}

float CResolver::GetForwardYaw(player_t* player) {
    return math::normalize_yaw(GetBackwardYaw(player) - 180.f);
}

float CResolver::GetLeftYaw(player_t* player) {
    return math::normalize_yaw(math::calculate_angle(g_ctx.local()->m_vecOrigin(), player->m_vecOrigin()).y - 90.f);
}

float CResolver::GetRightYaw(player_t* player) {
    return math::normalize_yaw(math::calculate_angle(g_ctx.local()->m_vecOrigin(), player->m_vecOrigin()).y + 90.f);
}

//Figure out a more viable way for stand resoling instead of IsEvenOrOdd
//This is a mix of animation layer & trace & math is even (for standing) LOL
int CResolver::GetThinkSide(player_t* player)  {
    //vars
    int side;
    player_t* e;

    auto animstate = e->get_animation_state();
    if (!animstate)
        return 0;

    float speed = player->m_vecVelocity().Length2D();

    //Animation layers
    if (player->m_fFlags() & FL_ONGROUND && speed > 0.1f && (player_record->layers[6].m_flWeight * 1000.0f) == (player_record->layers[6].m_flWeight * 1000.0f)) {
        float move_delta;

        if (player_record->layers[3].m_flWeight == 0.f && player_record->layers[3].m_flCycle == 0.f && player_record->layers[6].m_flWeight == 0.f)
        {
            //Thanks llama
            auto move_delta = math::angle_diff_onetap(player->m_angEyeAngles().y, original_goal_feet_yaw);
            side = (2 * (move_delta <= 0.0f) - 1) > 0.0f ? -1 : 1;
        }
        else if (!(player_record->layers[12].m_flWeight * 10000.0f))
        {
            if (player_record->layers[6].m_flWeight * 10000.0f == int(player_record->layers[6].m_flWeight * 10000.0f))
            {
                float delta_eye = abs(resolver_layers[0][6].m_flPlaybackRate - player_record->layers[6].m_flPlaybackRate);
                float delta_negative = abs(resolver_layers[1][6].m_flPlaybackRate - player_record->layers[6].m_flPlaybackRate);
                float delta_positive = abs(resolver_layers[2][6].m_flPlaybackRate - player_record->layers[6].m_flPlaybackRate);

                if ((delta_eye * 10000.0f) != (delta_negative * 10000.0f)) {
                    if (move_delta <= delta_eye)
                        move_delta = delta_eye;
                    else if (move_delta > delta_eye)
                        move_delta = delta_negative;
                    else
                        move_delta = delta_positive;

                    if (!(move_delta * 1000.0f) || (delta_eye * 10000.0f) != (delta_negative * 10000.0f)) {
                        if (move_delta == delta_negative) {
                            side = -1;
                        }
                        else if (move_delta == delta_positive) {
                            side = 1;
                        }
                    }
                }
            }
        }
    }
    else if (m_globals()->m_curtime - lock_side > 2.0f)
    {
        auto delta = math::normalize_yaw(player->m_angEyeAngles().y - original_goal_feet_yaw);
        auto first_visible = util::visible(g_ctx.globals.eye_pos, player->hitbox_position_matrix(CSGOHitboxID::Head, player_record->matrixes_data.first), player, g_ctx.local());
        auto second_visible = util::visible(g_ctx.globals.eye_pos, player->hitbox_position_matrix(CSGOHitboxID::Head, player_record->matrixes_data.second), player, g_ctx.local());

        if (first_visible != second_visible)
        {
            side = second_visible;
            lock_side = m_globals()->m_curtime;
        }
        else
        {
            auto first_position = g_ctx.globals.eye_pos.DistTo(player->hitbox_position_matrix(CSGOHitboxID::Head, player_record->matrixes_data.first));
            auto second_position = g_ctx.globals.eye_pos.DistTo(player->hitbox_position_matrix(CSGOHitboxID::Head, player_record->matrixes_data.second));

            if (fabs(first_position - second_position) > 1.0f)
                side = first_position > second_position;
        }
    }
    else {
        if (g_ctx.globals.missed_shots[player->EntIndex()] % 2 == 0)
            side = 1;
        else
            side = 0;
    }

    return side; //If its 1 then its positive.
}

//Rip legit aa resolving, todo: make a fix for this.
//Gheto but works good if the target does not know you have this LOL.
bool CResolver::DoesHaveFakeAngles(player_t* player) {
    if (player->m_angEyeAngles().x > 0.f && player->m_angEyeAngles().x < 85.f)
        return false;

    if (player->m_angEyeAngles().x < 0.f && player->m_angEyeAngles().x > -85.f)
        return false;
}

//This is not always accurate so make sure to improve it in the future so we get more accurate results.
bool CResolver::GetLowDeltaState(player_t* player) {
    c_baseplayeranimationstate* animstate = player->get_animation_state();

    float fl_eye_yaw = player->m_angEyeAngles().y;
    float fl_desync_delta = remainderf(fl_eye_yaw, animstate->m_flGoalFeetYaw);
    fl_desync_delta = std::clamp(fl_desync_delta, -60.f, 60.f);

    if (fabs(fl_desync_delta) < 30.f)
        return true;

    return false;
}

//Main, (x = pitch, y = yaw, z = roll/body lean)
void CResolver::RunResolve(player_t* player) {
    RegisterCallback("pre_resolve");

    //Setup our primary vars.
    auto idx = player->EntIndex();
    auto missed_shots = g_ctx.globals.missed_shots[idx];
    auto animstate = player->get_animation_state();

    //This returns true then false. Figure out if its the same if they are actually opposite.
    bool Extending = player_record->layers[3].m_flCycle == 0.f
        && player_record->layers[3].m_flWeight == 0.f;

    //This works perfectly. Figure out a way to hit it tho.
    bool FakeLeaning = player->m_angEyeAngles().x > 90;

    //Extract data from out other functions.
    int Side = GetThinkSide(player);
    bool IsLowDelta = GetLowDeltaState(player);

    //Get our angles (Backwards, Sideways (left/right), Forward).
    //These can be extremely helpful in some situations, figure out when and make a fix for it.
    const float Angle = GetAngle(player); //Default angle, We will be using this alot throughout the resolver.
    const bool& Sideways = fabsf(math::normalize_yaw(Angle - GetLeftYaw(player))) < 45.f || fabsf(math::normalize_yaw(Angle - GetRightYaw(player))) < 45.f; //Use this for sideways check.
    const bool& Forward = fabsf(math::normalize_yaw(Angle - GetForwardYaw(player))) < 90.f && !Sideways; //This is just 180 of get angle.

    //Make luna clients connect and share data so we know if they are leaning.
    //Resolve Z angles, these are not networked so we gotta brute.
    if (key_binds::get().get_key_bind_state(23))
        player->m_angEyeAngles().z = BruteForce(player, true);

    //Simple fix to over max brute fix.
    if (missed_shots == 8 || missed_shots > 7) {
        missed_shots = 0;
        return;
    }

    //Resolve desync.
    if (DoesHaveFakeAngles(player)) {
        if (missed_shots > 2 || missed_shots == 2) {
            desync_angle = BruteForce(player, false);
            goto Skip_logic;
        }
        else if (Sideways) {
            should_force_safepoint = true;
            goto Skipped;
        }
        else {
            should_force_safepoint = false;

            if (Side)
                desync_angle = !IsLowDelta ? 58 : 29;
            else
                desync_angle = !IsLowDelta ? -58 : -29;
        }
    }

Skip_logic:

    //Set our goal feet yaw and we now hit p100.
    animstate->m_flGoalFeetYaw = math::normalize_yaw(player->m_angEyeAngles().y + desync_angle);

Skipped:

    //Set globals
    g_ctx.globals.side = Side;
    g_ctx.globals.desync = desync_angle;

    //Set our pitch to the original players.
    //todo: Add a no spread resolver 💀 :skull:
    player->m_angEyeAngles().x = original_pitch;

    //Print our resolved angle, pitch, etc to the external console for debugging purposes.
    //L::Print("Pitch: " + (player->m_angEyeAngles().x > 90 ? "Fake leaning (" + std::to_string(player->m_angEyeAngles().x) + ")" : std::to_string(player->m_angEyeAngles().x)) + ", Deysnc: " + std::to_string(desync_angle) + ", Roll: " + std::to_string(player->m_angEyeAngles().z) + ", Side: " + std::to_string(Side) + ", Opposite: " + (Extending ? "true" : "false"));
    return;
}
