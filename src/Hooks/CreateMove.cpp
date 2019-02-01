#include "hooks.h"

bool CreateMove::sendPacket = true;
QAngle CreateMove::real;

bool Hooks::CreateMove(void* thisptr, float flInputSampleTime, CUserCmd* cmd)
{
	clientModeVMT->GetOriginalMethod<CreateMoveFn>(25)(thisptr, flInputSampleTime, cmd);

	if (cmd && cmd->command_number)
	{
        // Special thanks to Gre-- I mean Heep ( https://www.unknowncheats.me/forum/counterstrike-global-offensive/290258-updating-bsendpacket-linux.html )
        uintptr_t rbp;
        asm volatile("mov %%rbp, %0" : "=r" (rbp));
        bool *sendPacket = ((*(bool **)rbp) - 0x18);
        *sendPacket = CreateMove::sendPacket;
        CreateMove::sendPacket = true;

		/* run code that affects movement before prediction */
		Aimbot::PrePredictionCreateMove(cmd);
		BHop::CreateMove(cmd);
		AutoStrafe::CreateMove(cmd);
		ShowRanks::CreateMove(cmd);
		AutoDefuse::CreateMove(cmd);
		JumpThrow::CreateMove(cmd);
		EdgeJump::PrePredictionCreateMove(cmd);
		Autoblock::CreateMove(cmd);

		PredictionSystem::StartPrediction(cmd);
		{
			Aimbot::CreateMove(cmd);
			Triggerbot::CreateMove(cmd);
			AutoKnife::CreateMove(cmd);
			GrenadePrediction::CreateMove(cmd);
			AntiAim::CreateMove(cmd);
			FakeLag::CreateMove(cmd);
			ESP::CreateMove(cmd);
			LogShots::CreateMove(cmd);
			TracerEffect::CreateMove(cmd);
		}
		PredictionSystem::EndPrediction();

		EdgeJump::PostPredictionCreateMove(cmd);

		if (CreateMove::sendPacket)
		{
			CreateMove::real = cmd->viewangles;
			ClanTagChanger::CreateMove();
		}
	}

	return false;
}
