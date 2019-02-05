#include "grenadeprediction.h"

bool Settings::ESP::GrenadePrediction::enabled = false;
ColorVar Settings::ESP::GrenadePrediction::color = ImColor(26, 104, 173, 255);

std::vector<Vector> grenadePath;
std::vector<std::pair<Vector, QAngle>> otherCollisions;

int grenadeType = 0;
int grenadeAct = 0;

void GrenadePrediction::CreateMove(CUserCmd *cmd) {
    if (!Settings::ESP::enabled && !Settings::ESP::GrenadePrediction::enabled)
        return;

    bool in_attack = cmd->buttons & IN_ATTACK;
    bool in_attack2 = cmd->buttons & IN_ATTACK2;

    grenadeAct = (in_attack && in_attack2) ? ACT_LOB :
                 (in_attack2) ? ACT_DROP :
                 (in_attack) ? ACT_THROW :
                 ACT_NONE;
}

void GrenadePrediction::OverrideView(CViewSetup *pSetup) {
    if (!Settings::ESP::GrenadePrediction::enabled)
        return;

    C_BasePlayer *pLocal = (C_BasePlayer *) entityList->GetClientEntity(engine->GetLocalPlayer());
    if (!pLocal || !pLocal->GetAlive())
        return;

    if (pLocal->GetMoveType() == MOVETYPE_NOCLIP)
        return;

    C_BaseCombatWeapon *activeWeapon = (C_BaseCombatWeapon *) entityList->GetClientEntityFromHandle(
            pLocal->GetActiveWeapon());

    if (!activeWeapon)
        return;

    if (activeWeapon->GetCSWpnData()->GetWeaponType() == CSWeaponType::WEAPONTYPE_GRENADE &&
        grenadeAct != ACT_NONE) {
        ItemDefinitionIndex itemDefinitionIndex = *activeWeapon->GetItemDefinitionIndex();

        if (itemDefinitionIndex == ItemDefinitionIndex::WEAPON_TAGRENADE) {
            grenadeType = 0;
            return;
        }

        grenadeType = (int) itemDefinitionIndex;
        Simulate(pSetup);
    } else {
        grenadeType = 0;
    }

}

void GrenadePrediction::Paint() {
    if (!Settings::ESP::enabled && !Settings::ESP::GrenadePrediction::enabled)
        return;

    if ((grenadeType) && (grenadePath.size() > 1)) {
        Vector nadeStart, nadeEnd;
        Vector prev = grenadePath[0];

        for (auto it = grenadePath.begin(), end = grenadePath.end(); it != end; ++it) {
            if (!debugOverlay->ScreenPosition(prev, nadeStart) && !debugOverlay->ScreenPosition(*it, nadeEnd))
                Draw::Line(Vector2D(nadeStart.x, nadeStart.y), Vector2D(nadeEnd.x, nadeEnd.y),
                           Color::FromImColor(Settings::ESP::GrenadePrediction::color.Color()));
            prev = *it;
        }

        for (auto it = otherCollisions.begin(), end = otherCollisions.end(); it != end; ++it) {
            Draw::Cube3D(1.f, it->second, it->first, Color(255, 255, 255, 200));
        }

        Draw::Cube3D(1.f, otherCollisions.rbegin()->second, otherCollisions.rbegin()->first, Color(255, 255, 255, 200));

        if (!debugOverlay->ScreenPosition(prev, nadeEnd)) {
            Draw::Line(Vector2D(nadeStart.x, nadeStart.y), Vector2D(nadeEnd.x, nadeEnd.y),
                       Color::FromImColor(Settings::ESP::GrenadePrediction::color.Color()));
        }
    }
}

void GrenadePrediction::Setup(Vector &vecSrc, Vector &vecThrow, Vector viewangles) {
    if (!Settings::ESP::GrenadePrediction::enabled)
        return;

    C_BasePlayer *local = (C_BasePlayer *) entityList->GetClientEntity(engine->GetLocalPlayer());
    if (!local || !local->GetAlive())
        return;

    Vector angThrow = viewangles;
    float pitch = angThrow.x;

    if (pitch <= 90.0f) {
        if (pitch < -90.0f) {
            pitch += 360.0f;
        }
    } else {
        pitch -= 360.0f;
    }
    float a = pitch - (90.0f - fabs(pitch)) * 10.0f / 90.0f;
    angThrow.x = a;

    float flVel = 750.0f * 0.9f;

    // Do magic on member of grenade object [esi+9E4h]
    // m1=1  m1+m2=0.5  m2=0
    static const float power[] = {1.0f, 1.0f, 0.5f, 0.0f};
    float b = power[grenadeAct];
    // Clamped to [0,1]
    b = b * 0.7f;
    b = b + 0.3f;
    flVel *= b;

    Vector vForward, vRight, vUp;
    QAngle realThrow = QAngle(angThrow.x, angThrow.y, angThrow.z);

    Math::AngleVectors(realThrow, vForward, vRight, vUp);

    vecSrc = local->GetEyePosition();
    float off = (power[grenadeAct] * 12.0f) - 12.0f;
    vecSrc.z += off;

    trace_t tr;
    Vector vecDest = vecSrc;
    vecDest += vForward * 22.0f;

    TraceHull(vecSrc, vecDest, tr);

    Vector vecBack = vForward;
    vecBack *= 6.0f;
    vecSrc = tr.endpos;
    vecSrc -= vecBack;

    vecThrow = local->GetVelocity();
    vecThrow *= 1.25f;
    vecThrow += vForward * flVel;
}

void GrenadePrediction::Simulate(CViewSetup *setup) {
    if (!Settings::ESP::enabled && !Settings::ESP::GrenadePrediction::enabled)
        return;

    Vector vecSrc, vecThrow;
    Setup(vecSrc, vecThrow, setup->angles);

    float interval = globalVars->interval_per_tick;

    // Log positions 20 times per sec
    int logstep = static_cast<int>(0.05f / interval);
    int logtimer = 0;

    grenadePath.clear();
    otherCollisions.clear();

    for (unsigned int i = 0; i < grenadePath.max_size() - 1; ++i) {
        if (!logtimer) {
            if (!grenadePath.empty() &&
                vecSrc.DistTo(grenadePath.back()) < std::max(0.00001f, vecThrow.Length() / 25.f))
                break;    //getto fix to smoke infinite loop anti pasta, should work fine enough
            grenadePath.push_back(vecSrc);
        }

        int s = Step(vecSrc, vecThrow, i, interval);
        if ((s & 1) || vecThrow == Vector(0, 0, 0)) break;

        // Reset the log timer every logstep OR we bounced
        if ((s & 2) || logtimer >= logstep) logtimer = 0;
        else ++logtimer;
    }
    grenadePath.push_back(vecSrc);
}

int GrenadePrediction::Step(Vector &vecSrc, Vector &vecThrow, int tick, float interval) {
    Vector move;
    AddGravityMove(move, vecThrow, interval, false);

    // Push entity
    trace_t tr;
    PushEntity(vecSrc, move, tr);

    int result = 0;
    // Check ending conditions
    if (CheckDetonate(vecThrow, tr, tick, interval)) {
        result |= 1;
    }

    // Resolve collisions
    if (tr.fraction != 1.0f) {
        result |= 2; // Collision!
        ResolveFlyCollisionCustom(tr, vecThrow, interval);
    }

    if ((result & 1) || vecThrow == Vector(0, 0, 0) || tr.fraction != 1.0f) {
        QAngle angles;
        Math::VectorAngles((tr.endpos - tr.startpos).Normalize(), angles);
        otherCollisions.push_back(std::make_pair(tr.endpos, angles));
    }

    // Set new position
    vecSrc = tr.endpos;

    return result;
}

bool GrenadePrediction::CheckDetonate(const Vector &vecThrow, const trace_t &tr, int tick, float interval) {
    if (grenadeType == 0)
        return false;

    switch ((ItemDefinitionIndex) grenadeType) {
        case ItemDefinitionIndex::WEAPON_SMOKEGRENADE:
        case ItemDefinitionIndex::WEAPON_DECOY:
            // Velocity must be <0.1, this is only checked every 0.2s
            if (vecThrow.Length() < 0.1f) {
                int det_tick_mod = static_cast<int>(0.2f / interval);
                return !(tick % det_tick_mod);
            }
            return false;

        case ItemDefinitionIndex::WEAPON_MOLOTOV:
        case ItemDefinitionIndex::WEAPON_INCGRENADE:
            // Detonate when hitting the floor
            if (tr.fraction != 1.0f && tr.plane.normal.z > 0.7f)
                return true;
            // OR we've been flying for too long

        case ItemDefinitionIndex::WEAPON_FLASHBANG:
        case ItemDefinitionIndex::WEAPON_HEGRENADE:
            // Pure timer based, detonate at 1.5s, checked every 0.2s
            return static_cast<float>(tick) * interval > 1.5f && !(tick % static_cast<int>(0.2f / interval));

        default:
            return false;
    }
}

void GrenadePrediction::TraceHull(Vector &src, Vector &end, trace_t &tr) {
    if (!Settings::ESP::enabled && !Settings::ESP::GrenadePrediction::enabled)
        return;

    C_BasePlayer *localplayer = (C_BasePlayer *) entityList->GetClientEntity(engine->GetLocalPlayer());

    if (!localplayer || !localplayer->GetAlive())
        return;

    Ray_t ray;
    ray.Init(src, end, Vector(-2.0f, -2.0f, -2.0f), Vector(2.0f, 2.0f, 2.0f));

    CTraceFilter filter;
    filter.pSkip = localplayer;

    trace->TraceRay(ray, MASK_SHOT_HULL_NOWIN, &filter, &tr);
}

void GrenadePrediction::AddGravityMove(Vector &move, Vector &vel, float frametime, bool onground) {
    if (!Settings::ESP::enabled && !Settings::ESP::GrenadePrediction::enabled)
        return;

    Vector basevel(0.f, 0.f, 0.f);

    move.x = (vel.x + basevel.x) * frametime;
    move.y = (vel.y + basevel.y) * frametime;

    if (onground) {
        move.z = (vel.z + basevel.z) * frametime;
    } else {
        // Game calls GetActualGravity( this );
        float gravity = 800.0f * 0.4f;

        float newZ = vel.z - (gravity * frametime);
        move.z = ((vel.z + newZ) / 2.0f + basevel.z) * frametime;

        vel.z = newZ;
    }
}

void GrenadePrediction::PushEntity(Vector &src, const Vector &move, trace_t &tr) {
    if (!Settings::ESP::enabled && !Settings::ESP::GrenadePrediction::enabled)
        return;

    Vector vecAbsEnd = src;
    vecAbsEnd += move;

    // Trace through world
    TraceHull(src, vecAbsEnd, tr);
}

void GrenadePrediction::ResolveFlyCollisionCustom(trace_t &tr, Vector &vecVelocity, float interval) {
    if (!Settings::ESP::enabled && !Settings::ESP::GrenadePrediction::enabled)
        return;

    // Calculate elasticity
    float flSurfaceElasticity = 1.0;  // Assume all surfaces have the same elasticity
    float flGrenadeElasticity = 0.45f; // GetGrenadeElasticity()
    float flTotalElasticity = std::clamp(flGrenadeElasticity * flSurfaceElasticity, 0.0f, 0.9f);

    if (tr.m_pEntityHit) {
        // TODO : resolve player collision
    }

    // Calculate bounce
    Vector vecAbsVelocity;
    PhysicsClipVelocity(vecVelocity, tr.plane.normal, vecAbsVelocity, 2.0f);
    vecAbsVelocity *= flTotalElasticity;

    // Stop completely once we move too slow
    float flSpeedSqr = vecAbsVelocity.LengthSqr();

    // Stop if on ground
    if (tr.plane.normal.z > 0.7f) {
        if (flSpeedSqr > 96000.f) {
            auto l = vecAbsVelocity.Normalize().Dot(tr.plane.normal);
            if (l > 0.5f)
                vecAbsVelocity *= 1.f - l + 0.5f;
        }
        static const float flMinSpeedSqr = 20.0f * 20.0f; // 30.0f * 30.0f in CSS
        if (flSpeedSqr < flMinSpeedSqr) {
            vecAbsVelocity.Zero();
        }
        vecVelocity = vecAbsVelocity;
        vecAbsVelocity *= ((1.0f - tr.fraction) * interval);
        PushEntity(tr.endpos, vecAbsVelocity, tr);
    } else {
        vecVelocity = vecAbsVelocity;
    }
}

int GrenadePrediction::PhysicsClipVelocity(const Vector &in, const Vector &normal, Vector &out, float overbounce) {
    static const float STOP_EPSILON = 0.1f;

    float backoff;
    float change;
    float angle;
    int i, blocked;

    blocked = 0;

    angle = normal[2];

    if (angle > 0) {
        blocked |= 1;        // floor
    }
    if (!angle) {
        blocked |= 2;        // step
    }

    backoff = in.Dot(normal) * overbounce;

    for (i = 0; i < 3; i++) {
        change = normal[i] * backoff;
        out[i] = in[i] - change;
        if (out[i] > -STOP_EPSILON && out[i] < STOP_EPSILON) {
            out[i] = 0;
        }
    }

    return blocked;
}
