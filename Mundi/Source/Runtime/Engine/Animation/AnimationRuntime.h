#pragma once
#include "AnimSequenceBase.h"

class FAnimationRuntime
{
	static ETypeAdvanceAnim AdvanceTime(const bool bAllowLooping, const float MoveDelta, float& InOutTime, const float EndTime);

};