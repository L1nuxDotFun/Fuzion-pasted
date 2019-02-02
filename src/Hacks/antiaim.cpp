#include "antiaim.h"
#include "autowall.h"

bool Settings::AntiAim::Yaw::enabled = false;
bool Settings::AntiAim::Pitch::enabled = false;
AntiAimType_Y Settings::AntiAim::Yaw::type = AntiAimType_Y::SPIN_FAST;
bool Settings::AntiAim::Yaw::antiResolver = false;
AntiAimType_X Settings::AntiAim::Pitch::type = AntiAimType_X::STATIC_DOWN;
bool Settings::AntiAim::FreeStanding::enabled = false;
bool Settings::AntiAim::AutoDisable::noEnemy = false;
bool Settings::AntiAim::AutoDisable::knifeHeld = false;

static bool GetBestHeadAngle(QAngle& angle)
{
    C_BasePlayer* localplayer = (C_BasePlayer*) entityList->GetClientEntity(engine->GetLocalPlayer());

    QAngle viewAngles;
    engine->GetViewAngles(viewAngles);

    Vector pVecTarget = localplayer->GetEyePosition();

    float yaw = 180.f;

    float step = M_PI * 2.0 / 8;

    auto GetTargetEntity = [ & ] ( void )
    {
        float bestFov = FLT_MAX;
        C_BasePlayer* bestTarget = NULL;

        for( int i = 0; i < engine->GetMaxClients(); ++i )
        {
            C_BasePlayer* player = (C_BasePlayer*) entityList->GetClientEntity(i);

            if (!player
                || player == localplayer
                || player->GetDormant()
                || !player->GetAlive()
                || player->GetImmune()
                || player->GetTeam() == localplayer->GetTeam())
                continue;

            float fov = Math::GetFov(viewAngles, Math::CalcAngle(pVecTarget, player->GetEyePosition()));

            if( fov < bestFov )
            {
                bestFov = fov;
                bestTarget = player;
            }
        }

        return bestTarget;
    };

    auto target = GetTargetEntity();

    if (!target)
        return false;

    C_BaseCombatWeapon* activeWeapon = (C_BaseCombatWeapon*) entityList->GetClientEntityFromHandle(target->GetActiveWeapon());
    if (!activeWeapon)
        return false;

    Vector eVecTarget = target->GetEyePosition();

    float bestDamage = 1337.f;

    cvar->ConsoleDPrintf("----------- Damage calculation start -----------\n");

    for (float a = 0; a < (M_PI * 2.0); a += step)
    {
        Vector location(cos(a) + pVecTarget.x, sin(a) + pVecTarget.y, pVecTarget.z);
        Autowall::FireBulletData data;

        Autowall::GetDamageCustom(eVecTarget, location, target->GetIndex(), Settings::Aimbot::friendly, data);

        if (data.current_damage < bestDamage)
        {
            bestDamage = data.current_damage;
            yaw = RAD2DEG(a);
        }

        cvar->ConsoleDPrintf("Angle: %f , Damage: %f\n", RAD2DEG(a), data.current_damage);
    }

    cvar->ConsoleDPrintf("Damage calculation done, angle: %f, damage: %f\n\n", angle.y, bestDamage);

    angle.y = yaw;

    return true;
}

float AntiAim::GetMaxDelta( CCSGOAnimState *animState ) {

    float speedFraction = std::max(0.0f, std::min(animState->feetShuffleSpeed, 1.0f));

    float speedFactor = std::max(0.0f, std::min(1.0f, animState->feetShuffleSpeed2));

    float unk1 = ((animState->runningAccelProgress * -0.30000001) - 0.19999999) * speedFraction;
    float unk2 = unk1 + 1.0f;
    float delta;

    if (animState->duckProgress > 0)
    {
        unk2 += ((animState->duckProgress * speedFactor) * (0.5f - unk2));// - 1.f
    }

    delta = *(float*)((uintptr_t)animState + 0x3A4) * unk2;

    return abs(delta - 0.5f);
}


static bool HasViableEnemy()
{
	C_BasePlayer* localplayer = (C_BasePlayer*) entityList->GetClientEntity(engine->GetLocalPlayer());

	for (int i = 1; i < engine->GetMaxClients(); ++i)
	{
		C_BasePlayer* entity = (C_BasePlayer*) entityList->GetClientEntity(i);

		if (!entity
			|| entity == localplayer
			|| entity->GetDormant()
			|| !entity->GetAlive()
			|| entity->GetImmune())
			continue;

		if( !Aimbot::friends.empty() ) // check for friends, if any
		{
			IEngineClient::player_info_t entityInformation;
			engine->GetPlayerInfo(i, &entityInformation);

			if (std::find(Aimbot::friends.begin(), Aimbot::friends.end(), entityInformation.xuid) != Aimbot::friends.end())
				continue;
		}

		if (Settings::Aimbot::friendly || entity->GetTeam() != localplayer->GetTeam())
			return true;
	}

	return false;
}
static void DoAntiAimY(QAngle& angle, int command_number, bool& clamp)
{
	AntiAimType_Y aa_type = Settings::AntiAim::Yaw::type;
	AntiAimType_Y aa_manual = AntiAimType_Y::BACKWARDS;

	static bool yFlip;
	float temp;
	double factor;
	static float trigger;
	QAngle temp_qangle;
	int random;
	int maxJitter;

	switch (aa_type)
	{
		case AntiAimType_Y::SPIN_FAST:
			factor =  360.0 / M_PHI;
			factor *= 25;
			angle.y = fmodf(globalVars->curtime * factor, 360.0);
			break;
		case AntiAimType_Y::SPIN_SLOW:
			factor =  360.0 / M_PHI;
			angle.y = fmodf(globalVars->curtime * factor, 360.0);
			break;
		case AntiAimType_Y::JITTER:
			yFlip ? angle.y -= 90.0f : angle.y -= 270.0f;
			break;
		case AntiAimType_Y::BACKJITTER:
			angle.y -= 180;
			random = rand() % 100;
			maxJitter = rand() % (85 - 70 + 1) + 70;
			temp = maxJitter - (rand() % maxJitter);
			if (random < 35 + (rand() % 15))
				angle.y -= temp;
			else if (random < 85 + (rand() % 15 ))
				angle.y += temp;
			break;
		case AntiAimType_Y::SIDE:
			yFlip ? angle.y += 90.f : angle.y -= 90.0f;
			break;
		case AntiAimType_Y::BACKWARDS:
			angle.y -= 180.0f;
			break;
		case AntiAimType_Y::FORWARDS:
			angle.y -= 0.0f;
			break;
		case AntiAimType_Y::LEFT:
			angle.y += 90.0f;
			break;
		case AntiAimType_Y::RIGHT:
			angle.y -= 90.0f;
			break;
		case AntiAimType_Y::STATICAA:
			angle.y = 0.0f;
			break;
		case AntiAimType_Y::STATICJITTER:
			trigger += 15.0f;
			angle.y = trigger > 50.0f ? 150.0f : -150.0f;

			if (trigger > 100.0f)
				trigger = 0.0f;
			break;
		case AntiAimType_Y::STATICSMALLJITTER:
			trigger += 15.0f;
			angle.y = trigger > 50.0f ? -30.0f : 30.0f;

			if (trigger > 100.0f)
				trigger = 0.0f;
			break;
		case AntiAimType_Y::LISP:
			clamp = false;
			yFlip ? angle.y += 323210000.0f : angle.y -= 323210000.0f;
			break;
		case AntiAimType_Y::LISP_SIDE:
			clamp = false;
			temp = angle.y + 90.0f;
			temp_qangle = QAngle(0.0f, temp, 0.0f);
			Math::NormalizeAngles(temp_qangle);
			temp = temp_qangle.y;

			if (temp > -45.0f)
				temp < 0.0f ? temp = -90.0f : temp < 45.0f ? temp = 90.0f : temp = temp;

			temp += 1800000.0f;
			angle.y = temp;
			break;
		case AntiAimType_Y::LISP_JITTER:
			clamp = false;
			temp = angle.y - 155.0f;

			if (globalVars->tickcount % 2)
			{
				temp = angle.y + 58.0f;
				temp += 1800000.0f;
			}

			angle.y = temp;
			break;
		case AntiAimType_Y::ANGEL_BACKWARD:
			clamp = false;
			angle.y += 36000180.0f;
			break;
		case AntiAimType_Y::ANGEL_INVERSE:
			clamp = false;
			angle.y = 36000180.0f;
			break;
		case AntiAimType_Y::LOWERBODY:
			angle.y = *((C_BasePlayer*)entityList->GetClientEntity(engine->GetLocalPlayer()))->GetLowerBodyYawTarget() + rand()%35 + 165;
			break;
		case AntiAimType_Y::ANGEL_SPIN:
			clamp = false;
			factor = (globalVars->curtime * 5000.0f);
			angle.y = factor + 36000000.0f;
			break;
		case AntiAimType_Y::CASUAL:
			yFlip ? angle.y -= 35.0f : angle.y += 35.0f;
			break;
		case AntiAimType_Y::LBYONGROUND:
			static C_BasePlayer* player = ((C_BasePlayer*) entityList->GetClientEntity(engine->GetLocalPlayer()));
			if (player->GetFlags() & FL_ONGROUND)
				angle.y = *((C_BasePlayer*)entityList->GetClientEntity(engine->GetLocalPlayer()))->GetLowerBodyYawTarget() + rand()%35 + 165;
			else
			{
				static int aimType = rand() % 4;
				switch (aimType)
				{
					case 1:
						yFlip ? angle.y += 90.f : angle.y -= 90.0f;
						break;
					case 2:
						yFlip ? angle.y -= 120.0f : angle.y -= 210.0f;
						break;
					case 3:
						factor =  360.0 / M_PHI;
						factor *= 25;
						angle.y = fmodf(globalVars->curtime * factor, 360.0);
						break;
					default:
						angle.y -= 180.0f;
				}
			}
			break;
		default:
			angle.y -= 0.0f;
			break;
	}
}

static void DoAntiAimX(QAngle& angle, bool& clamp)
{
	static float pDance = 0.0f;
	AntiAimType_X aa_type = Settings::AntiAim::Pitch::type;

	switch (aa_type)
	{
		case AntiAimType_X::STATIC_UP:
			angle.x = -89.0f;
			break;
		case AntiAimType_X::STATIC_DOWN:
			angle.x = 89.0f;
			break;
		case AntiAimType_X::DANCE:
			pDance += 45.0f;
			if (pDance > 100)
				pDance = 0.0f;
			else if (pDance > 75.f)
				angle.x = -89.f;
			else if (pDance < 75.f)
				angle.x = 89.f;
			break;
		case AntiAimType_X::FRONT:
			angle.x = 0.0f;
			break;
		case AntiAimType_X::LISP_DOWN:
			clamp = false;
			angle.x = 1800089.0f;
			break;
		case AntiAimType_X::ANGEL_DOWN:
			clamp = false;
			angle.x = 36000088.0f;
			break;
		case AntiAimType_X::ANGEL_UP:
			clamp = false;
			angle.x = 35999912.0f;
			break;
		default:
			angle.x -= 0.0f;
			break;
	}
}

void AntiAim::CreateMove(CUserCmd* cmd)
{
	if (!Settings::AntiAim::Yaw::enabled && !Settings::AntiAim::Pitch::enabled)
		return;

	if (Settings::Aimbot::AimStep::enabled && Aimbot::aimStepInProgress)
		return;

	QAngle oldAngle = cmd->viewangles;
	float oldForward = cmd->forwardmove;
	float oldSideMove = cmd->sidemove;

	QAngle angle = cmd->viewangles;

	C_BasePlayer* localplayer = (C_BasePlayer*) entityList->GetClientEntity(engine->GetLocalPlayer());
	if (!localplayer)
		return;

	C_BaseCombatWeapon* activeWeapon = (C_BaseCombatWeapon*) entityList->GetClientEntityFromHandle(localplayer->GetActiveWeapon());
	if (!activeWeapon)
		return;

	if (activeWeapon->GetCSWpnData()->GetWeaponType() == CSWeaponType::WEAPONTYPE_GRENADE)
	{
		C_BaseCSGrenade* csGrenade = (C_BaseCSGrenade*) activeWeapon;

		if (csGrenade->GetThrowTime() > 0.f)
			return;
	}

	if (cmd->buttons & IN_USE || cmd->buttons & IN_ATTACK || (cmd->buttons & IN_ATTACK2 && (*activeWeapon->GetItemDefinitionIndex() == ItemDefinitionIndex::WEAPON_REVOLVER || activeWeapon->GetCSWpnData()->GetWeaponType() == CSWeaponType::WEAPONTYPE_KNIFE)))
		return;

	if (localplayer->GetMoveType() == MOVETYPE_LADDER || localplayer->GetMoveType() == MOVETYPE_NOCLIP)
		return;

	// AutoDisable checks

	// Knife
	if (Settings::AntiAim::AutoDisable::knifeHeld && localplayer->GetAlive() && activeWeapon->GetCSWpnData()->GetWeaponType() == CSWeaponType::WEAPONTYPE_KNIFE)
		return;

	if (Settings::AntiAim::AutoDisable::noEnemy && localplayer->GetAlive() && !HasViableEnemy())
		return;

	QAngle edge_angle = angle;
	bool freestanding = Settings::AntiAim::FreeStanding::enabled && GetBestHeadAngle(edge_angle);

	bool should_clamp = true;

	if (!ValveDSCheck::forceUT && (*csGameRules) && (*csGameRules)->IsValveDS())
	{
		if (Settings::AntiAim::Yaw::type >= AntiAimType_Y::LISP)
			Settings::AntiAim::Yaw::type = AntiAimType_Y::SPIN_SLOW;

		if (Settings::AntiAim::Pitch::type >= AntiAimType_X::LISP_DOWN)
			Settings::AntiAim::Pitch::type = AntiAimType_X::STATIC_UP;
	}

	if (Settings::AntiAim::Yaw::enabled)
	{
		DoAntiAimY(angle, cmd->command_number, should_clamp);
		Math::NormalizeAngles(angle);
		if (freestanding)
			angle.y = edge_angle.y;
	}

	if (Settings::AntiAim::Pitch::enabled)
		DoAntiAimX(angle, should_clamp);

	if (should_clamp)
	{
		Math::NormalizeAngles(angle);
		Math::ClampAngles(angle);
	}

	cmd->viewangles = angle;

	if (Settings::AntiAim::Yaw::antiResolver)
	{
		static bool antiResolverFlip = false;
		if (cmd->viewangles.y == *localplayer->GetLowerBodyYawTarget())
		{
			if (antiResolverFlip)
				cmd->viewangles.y += 60.f;
			else
				cmd->viewangles.y -= 60.f;

			antiResolverFlip = !antiResolverFlip;

			if (should_clamp)
			{
				Math::NormalizeAngles(cmd->viewangles);
				Math::ClampAngles(cmd->viewangles);
			}
		}
	}

	Math::CorrectMovement(oldAngle, cmd, oldForward, oldSideMove);
}