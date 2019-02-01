#pragma once

#include "../settings.h"
#include "../SDK/SDK.h"
#include "../Hooks/hooks.h"
#include "../interfaces.h"
#include "aimbot.h"

namespace AntiAim
{
	//Hooks
	void CreateMove(CUserCmd* cmd);
	void Paint();

	float GetMaxDelta( CCSGOAnimState *animState );
	extern QAngle real;
}
