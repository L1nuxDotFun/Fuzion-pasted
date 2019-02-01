#include "logshots.h"

std::deque<Shots> LogShots::shots;
std::deque<LoggedEvent> LogShots::eventList;

std::array<int, 64> LogShots::missedShots;

std::string HitgroupToString(int hitgroup)
{
    switch((HitGroups) hitgroup)
    {
        case HitGroups::HITGROUP_GENERIC:
            return XORSTR("Generic");
        case HitGroups::HITGROUP_CHEST:
            return XORSTR("Body");
        case HitGroups::HITGROUP_HEAD:
            return XORSTR("Head");
        case HitGroups::HITGROUP_LEFTARM:
            return XORSTR("Left Arm");
        case HitGroups::HITGROUP_LEFTLEG:
            return XORSTR("Left Leg");
        case HitGroups::HITGROUP_RIGHTARM:
            return XORSTR("Right Arm");
        case HitGroups::HITGROUP_RIGHTLEG:
            return XORSTR("Right Leg");
        case HitGroups::HITGROUP_STOMACH:
            return XORSTR("Stomach");
        case HitGroups::HITGROUP_GEAR:
            return XORSTR("Gear");
    }
}

void LogShots::Paint()
{
    int offset = 18;
    int i = 0;
    static HFont logFont = Draw::CreateFont(XORSTR("Andale Mono"), 10, (int)FontFlags::FONTFLAG_DROPSHADOW );

    if (!engine->IsInGame())
        return;

    for (auto ev : LogShots::eventList)
    {
        float delta = ev.expTime - globalVars->curtime;
        auto white = Color(255, 255, 255, 180);

        if (ev.expTime > globalVars->curtime && delta < 6.f)
        {
            if (delta < 0.5f)
            {
                auto clamped = std::clamp(delta * 2, 0.f, 1.f);
                white.a = clamped * 180;
                offset -= (1.f - clamped) * 12;
            }

            Draw::Text(8, offset, ev.string.c_str(), logFont, white);
            offset += 12;
            i++;
        } else{
            eventList.erase(eventList.begin() + i);
            if (i > 0) i--;
        }
    }
}

void LogShots::FireGameEvent(IGameEvent *event)
{
    if (!event)
        return;

    C_BasePlayer* localplayer = (C_BasePlayer*) entityList->GetClientEntity(engine->GetLocalPlayer());
    if (!localplayer)
        return;

    if (!strcmp(event->GetName(), XORSTR("player_hurt")))
    {
        int attacker = engine->GetPlayerForUserID(event->GetInt(XORSTR("attacker")));
        int victim = engine->GetPlayerForUserID(event->GetInt(XORSTR("userid")));
        std::string hitgroup = HitgroupToString(event->GetInt(XORSTR("hitgroup")));
        std::string damage = event->GetString(XORSTR("dmg_health"));
        std::string health = event->GetString(XORSTR("health"));

        if (attacker != engine->GetLocalPlayer())
            return;

        if (victim == engine->GetLocalPlayer())
            return;

        auto ent = entityList->GetClientEntity(engine->GetPlayerForUserID(victim));
        IEngineClient::player_info_t info;
        engine->GetPlayerInfo(victim, &info);

        std::string str;
        str += std::string(XORSTR("Hit ")) + info.name + XORSTR(" in the ") + hitgroup + XORSTR(" for ") + damage + XORSTR(" dmg (") + health + XORSTR(" health remaining.)\n");

        cvar->ConsoleColorPrintf(ColorRGBA(39, 106, 219, 255), XORSTR("[Fuzion] "));
        cvar->ConsoleDPrintf(str.c_str());
        eventList.push_back(LoggedEvent(str, globalVars->curtime + 5.f));

        if (!shots.empty())
        {
            shots.front().hit = true;
            shots.front().processed = true;
            shots.front().networked = true;
        }
    }
    if (!strcmp(event->GetName(), XORSTR("bullet_impact")))
    {
        int userid = engine->GetPlayerForUserID(event->GetInt(XORSTR("userid")));

        if (userid != engine->GetLocalPlayer())
            return;

        if (!shots.empty())
        {
            shots.front().impacts.push_back(Vector(event->GetFloat(XORSTR("x")), event->GetFloat(XORSTR("y")), event->GetFloat(XORSTR("z"))));
            shots.front().networked = true;
        }
    }
}

void LogShots::CreateMove(CUserCmd* cmd)
{
    C_BasePlayer* localplayer = (C_BasePlayer*) entityList->GetClientEntity(engine->GetLocalPlayer());
    if (!localplayer)
        return;

    if (shots.empty()) return;
    auto shot = shots.front();
    if (shot.processed || shot.hit || abs(shot.time - globalVars->curtime) > 1.f)
    {
        shots.erase(shots.begin());
        return;
    }
    if (!shot.networked) return;

    Vector impact;
    float bestLength = 0.f;

    while (!shot.impacts.empty())
    {
        if ((shot.src - shot.impacts.front()).Length() > bestLength)
        {
            impact = shot.impacts.front();
            bestLength = (shot.src - shot.impacts.front()).Length();
        }
        shot.impacts.erase(shot.impacts.begin());
    }

    Ray_t ray;
    trace_t tr;
    CTraceFilterEntitiesOnly filter;

    ray.Init(shot.src, impact);
    filter.pSkip = localplayer;
    trace->TraceRay(ray, MASK_SHOT, &filter, &tr);

    if (tr.m_pEntityHit == shot.ent) // resolver error
    {
        std::string str;
        switch ( (int) (globalVars->curtime * 100) % 5 )
        {
            case 0:
                str = XORSTR("Missed shot due to config\n");
                break;
            case 1:
                str = XORSTR("Missed shot due to the cheat refusing to look at gay people\n");
                break;
            case 2:
                str = XORSTR("Missed shot due to Earth's curvature\n");
                break;
            case 3:
                str = XORSTR("Missed shot due to pasted resolver\n");
                break;
            case 4:
                str = XORSTR("Missed shot due to [insert excuses here]\n");
                break;
        }
        eventList.push_back({str, globalVars->curtime + 5.f});
        cvar->ConsoleColorPrintf(ColorRGBA(39, 106, 219, 255), XORSTR("[Fuzion] "));
        cvar->ConsoleDPrintf(str.c_str());
    } else { // spread
        eventList.push_back({"Missed shot due to spread", globalVars->curtime + 5.f});
        cvar->ConsoleColorPrintf(ColorRGBA(39, 106, 219, 255), XORSTR("[Fuzion] "));
        cvar->ConsoleDPrintf(XORSTR("Missed shot due to spread\n"));
    }

    shot.processed = true;
    shots.erase(shots.begin());
}