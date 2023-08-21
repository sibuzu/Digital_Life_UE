#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Components/Slider.h"
#include "Components/ComboBoxString.h"
#include "GameFramework/SaveGame.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameEngine.h"

#include "../Code_Public/GameSaveObject.h"

#include "UserSetWidget.generated.h"

UCLASS()
class T_API UUserSetWidget : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativePreConstruct() override;

	UFUNCTION(BlueprintCallable)
		void InitWidget();
	UFUNCTION(BlueprintImplementableEvent)
		void UpdateBackground(bool IsClick);
	UFUNCTION(BlueprintImplementableEvent)
		void UpdateFOV(float Value);
	UFUNCTION(BlueprintCallable)
		void UpdateMaxFPS(bool IsClick);
	UFUNCTION(BlueprintImplementableEvent)
		void UpdateWindowState(bool IsClick);
	UFUNCTION(BlueprintImplementableEvent)
		void UpdateQualitySetting(int32 Index);
	UFUNCTION(BlueprintImplementableEvent)
		void UpdateActorSetting(int32 Index);
	UFUNCTION(BlueprintImplementableEvent)
		void UpdateDebugSetting(int32 Index);

	UFUNCTION(BlueprintCallable)
		void ApplySaveSet();
	UFUNCTION(BlueprintCallable)
		bool SetGameSaveObjectConfig(int32 Index, int32 Value);
	UFUNCTION(BlueprintCallable, BlueprintPure)
		bool GetGameSaveObjectConfig(int32 Index);

	UFUNCTION(BlueprintCallable)
		void SetFOV(int32 FOV, float Value);
	UFUNCTION(BlueprintCallable)
		void SetQualityIndex(int32 Index);
	UFUNCTION(BlueprintCallable)
		void SetActorIndex(int32 Index);
	UFUNCTION(BlueprintCallable)
		void SetDebugLevel(int32 Index);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (BindWidget))
		USlider* UpdateSize;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (BindWidget))
		UComboBoxString* QualitySelectBox;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (BindWidget))
		UButton* SwitchBackground;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (BindWidget))
		UButton* LimitFPS;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, meta = (BindWidget))
		UButton* FullScreen;

public:
	UPROPERTY(BlueprintReadWrite)
		float FOVValue = 0.f;
	UPROPERTY(BlueprintReadWrite)
		int32 ActorIndex = 0;
	UPROPERTY(BlueprintReadWrite)
		int32 DebugLevel = 2;
};
