#include "MyGlobal.h"

#include "Kismet/GameplayStatics.h"
#include "GameSaveObject.h"
 
int32 MyGlobal::ActorIndex = 0;
int32 MyGlobal::DebugLevel = 0;
void MyGlobal::SaveActorIndex(int32 Value) {
	MyGlobal::ActorIndex = Value;

	if (MyGlobal::DebugLevel==2) {
		GEngine->AddOnScreenDebugMessage(-1, 1000.f, FColor::Red, FString("SaveActorIndex: ActorIndex=") + FString::FromInt(Value));
	}
	if (UGameplayStatics::DoesSaveGameExist(TEXT("PlayerInfo"), 0)) {
		UGameSaveObject* GameSaveObject = Cast<UGameSaveObject>(UGameplayStatics::LoadGameFromSlot(TEXT("PlayerInfo"), 0));
		GameSaveObject->ActorIndex = Value;
		UGameplayStatics::SaveGameToSlot(GameSaveObject, TEXT("PlayerInfo"), 0);
	}
}
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
