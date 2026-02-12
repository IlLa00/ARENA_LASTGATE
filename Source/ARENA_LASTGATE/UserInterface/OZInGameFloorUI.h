#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Utils/Util.h"
#include "GameplayEffectTypes.h"
#include "Data/OZConvertData.h"
#include "ActiveGameplayEffectHandle.h"
#include "OZInGameFloorUI.generated.h"

class UAbilitySystemComponent;
struct FOnAttributeChangeData;
class UOZConvertSubsystem;
class UOZBuffProgressEntry;
class UVerticalBox;
class UHorizontalBox;
class UOverlay;
struct FActiveGameplayEffect;

UCLASS()
class ARENA_LASTGATE_API UOZInGameFloorUI : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	void SetCurrentRound(uint8 currRound);
	void SetCurrentGameState(EGameStateType currGameState, int numRound = 0);
	void SetCurrentRemainTime(int32 remainTime);
	void SetCombatTimer(float remainTime);

	void SetIsOnCutScene(bool isOnCutScene);

	UFUNCTION()
	void OnInventoryUpdated();

	void RefreshInventory();

	void SetActiveInvenEntries(bool state);

	UFUNCTION()
	void ChangeFloorUI(bool bIsWin);

	void ChangeSwitcher(bool bisAlive);

	UFUNCTION(BlueprintImplementableEvent, Category = "Stealth")
	void OnStealthStateChanged(bool bIsInBush);


public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UWidgetSwitcher> Switcher;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UCanvasPanel> AlivePannel;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UCanvasPanel> DeadPannel;


	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_Upper;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_Timer;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UOZInvenEntry> WBP_InvenEntry_1;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UOZInvenEntry> WBP_InvenEntry_2;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UOZInvenEntry> WBP_InvenEntry_3;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UOZInvenEntry> WBP_InvenEntry_4;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_CurrentHP;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_CurrentEvldistance;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_CurrentShield;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_CurrentStamina;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_CurrentSpeed;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UTextBlock> Text_CurrentArmor;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UImage> Img_Stealth;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UImage> img_BG_Convert_1;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UImage> img_Icon_Convert_1;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UImage> img_BG_Convert_2;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UImage> img_Icon_Convert_2;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UImage> img_BG_Convert_3;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UImage> img_Icon_Convert_3;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UImage> img_BG_Convert_4;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UImage> img_Icon_Convert_4;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UImage> img_BG_Convert_5;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UImage> img_Icon_Convert_5;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UImage> img_BG_Convert_6;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<class UImage> img_Icon_Convert_6;

	// 등급별 BG 이미지 (블루프린트에서 설정)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Convert UI")
	TObjectPtr<UTexture2D> ConvertBG_Rare;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Convert UI")
	TObjectPtr<UTexture2D> ConvertBG_Unique;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Convert UI")
	TObjectPtr<UTexture2D> ConvertBG_Legendary;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UVerticalBox> VBox_BuffContainer;

	UPROPERTY(EditDefaultsOnly, Category = "Buff UI")
	TSubclassOf<UOZBuffProgressEntry> BuffProgressEntryClass;

	//튜토피얼 팝업 위젯

	void PlayPopup(FString text_Title, FString text_Msg, float lifeTime);

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class UOZTutorialPopupTextWidget> TutorialPopupMsg = nullptr;

	//컨테이너 위젯
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UHorizontalBox> HorizontalBox_TimeDisplaySlot;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UHorizontalBox> HorizontalBox_BuffSlotUpper;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UHorizontalBox> HorizontalBox_BuffSlotLower;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UOverlay> Overlay_UserInfoSlot;

	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UOverlay> Overlay_InventorySlot;


	

private:
	UPROPERTY()
	class UOZInventoryComponent* InventoryComp = nullptr;

	UPROPERTY()
	class AOZPlayerState* PlayerState = nullptr;

	UPROPERTY()
	UAbilitySystemComponent* ASC = nullptr;

	bool bPlayerStateInitialized = false;

	void BindAttributeDelegates();
	void UnbindAttributeDelegates();

	void OnMaxHealthChanged(const FOnAttributeChangeData& Data);
	void OnMaxStaminaChanged(const FOnAttributeChangeData& Data);
	void OnMaxShieldChanged(const FOnAttributeChangeData& Data);
	void OnArmorChanged(const FOnAttributeChangeData& Data);
	void OnEvLDistanceChanged(const FOnAttributeChangeData& Data);
	void OnMoveSpeedChanged(const FOnAttributeChangeData& Data);

	void UpdateHealthUI(float MaxHP);
	void UpdateStaminaUI(float MaxStamina);
	void UpdateShieldUI(float MaxShield);
	void UpdateArmorUI(float Armor);
	void UpdateEvLDistanceUI(float EvLDistance);
	void UpdateSpeedUI(float MoveSpeed);

	void InitializeAttributeUI();

	void UpdateStealthVisibility(bool bInBush);

	UFUNCTION()
	void OnConvertAcquired(int32 ConvertID, EConvertGrade Grade);

	void UpdateConvertUI();
	void SetConvertSlot(int32 SlotIndex, int32 ConvertID);
	UTexture2D* GetBGTextureForGrade(EConvertGrade Grade) const;

	UPROPERTY()
	TObjectPtr<UOZConvertSubsystem> ConvertSubsystem;

	TArray<TObjectPtr<class UImage>> ConvertBGSlots;
	TArray<TObjectPtr<class UImage>> ConvertIconSlots;

	bool bAttributeDelegatesBound = false;
	bool bPreviousInBush = false;

	UPROPERTY()
	TArray<TObjectPtr<UOZBuffProgressEntry>> ActiveBuffEntries;

	void UpdateBuffTimers();
	void OnBuffEntryExpired(UOZBuffProgressEntry* ExpiredEntry);

	int32 GetBuffItemIDFromTag(const FGameplayTag& BuffTag) const;
};
