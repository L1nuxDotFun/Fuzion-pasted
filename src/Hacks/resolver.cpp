#include "resolver.h"

bool Settings::Resolver::resolveAll = false;

void Resolver::FrameStageNotify(ClientFrameStage_t stage)
{
	if (!engine->IsInGame())
		return;

	C_BasePlayer* localplayer = (C_BasePlayer*) entityList->GetClientEntity(engine->GetLocalPlayer());
	if (!localplayer)
		return;

	if (stage == ClientFrameStage_t::FRAME_NET_UPDATE_POSTDATAUPDATE_START)
	{
		for (int i = 1; i < engine->GetMaxClients(); ++i)
		{
			C_BasePlayer* player = (C_BasePlayer*) entityList->GetClientEntity(i);

			if (!player
				|| player == localplayer
				|| player->GetDormant()
				|| !player->GetAlive()
				|| player->GetImmune()
				|| player->GetTeam() == localplayer->GetTeam())
				continue;

			CCSGOAnimState* animState = player->GetAnimState();
			if (!animState)
				continue;

			float maxDelta = AntiAim::GetMaxDelta(animState);

			int missedShot = LogShots::missedShots[player->GetIndex() - 1];

			switch (missedShot % 5)
			{
                case 0:
                    player->GetEyeAngles()->y -= 0;
                    break;
				case 1:
					player->GetEyeAngles()->y -= maxDelta;
					break;
				case 2:
					player->GetEyeAngles()->y += maxDelta;
					break;
				case 3:
					player->GetEyeAngles()->y -= maxDelta / 2;
					break;
				case 4:
					player->GetEyeAngles()->y += maxDelta / 2;
					break;
			}

		}
	}
}

void Resolver::PostFrameStageNotify(ClientFrameStage_t stage)
{
}