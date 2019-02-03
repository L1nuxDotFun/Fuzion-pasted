#pragma once

#include "../Utils/draw.h"
#include "../interfaces.h"
#include "../Utils/entity.h"

struct LoggedEvent
{
    LoggedEvent(std::string stringi, float expTimei)
    {
        string = stringi;
        expTime = expTimei;
    }

    std::string string;
    float expTime;
};

struct Shots
{
    Shots()
    {
        ent = nullptr;
        src = Vector(0, 0, 0);
        time = 0.f;
        hit = false;
        networked = false;
        processed = true;
    }

    Shots(C_BasePlayer* enti, Vector srci, float timei)
    {
        ent = enti;
        src = srci;
        time = timei;
        hit = false;
        networked = false;
        processed = false;
    }

    C_BasePlayer* ent;
    Vector src;
    float time;
    bool hit, networked, processed;
    std::vector<Vector> impacts;
};

namespace LogShots
{
    void FireGameEvent(IGameEvent* event);
    void Paint();
    void CreateMove(CUserCmd* cmd);

    extern std::deque<Shots> shots;
    extern std::deque<LoggedEvent> eventList;

    extern std::array<int, 64> missedShots;
};