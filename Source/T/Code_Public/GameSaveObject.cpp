#include "GameSaveObject.h"

UGameSaveObject::UGameSaveObject() {
#if PLATFORM_ANDROID
	FOVValue = 0.121429f;
	DebugLevel = 2;
#endif
}