#pragma once

#include "CoreMinimal.h"

class MyGlobal {
public:
	static int32 ActorIndex;
	static int32 DebugLevel;
	static void SaveActorIndex(int32 Value);
	static void SaveDebugLevel(int32 Value);
};
