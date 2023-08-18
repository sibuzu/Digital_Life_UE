#include "MyGlobal.h"

#include "Kismet/GameplayStatics.h"
#include "GameSaveObject.h"
 
int32 MyGlobal::DebugLevel = 0;
void MyGlobal::SaveDebugLevel(int32 Value) {
	MyGlobal::DebugLevel = Value;

	if (UGameplayStatics::DoesSaveGameExist(TEXT("PlayerInfo"), 0)) {
		UGameSaveObject* GameSaveObject = Cast<UGameSaveObject>(UGameplayStatics::LoadGameFromSlot(TEXT("PlayerInfo"), 0));
		GameSaveObject->DebugLevel = Value;
		UGameplayStatics::SaveGameToSlot(GameSaveObject, TEXT("PlayerInfo"), 0);
	}

	if (Value == 0) {
		GEngine->ClearOnScreenDebugMessages();
	}
}
