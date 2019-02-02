#pragma once

#include "../settings.h"
#include "../interfaces.h"

namespace Resolver
{
	//Hooks
	void FrameStageNotify(ClientFrameStage_t stage);
	void PostFrameStageNotify(ClientFrameStage_t stage);
}
