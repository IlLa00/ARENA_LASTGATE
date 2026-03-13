#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Utils/Util.h"
#include "Data/OZConvertData.h"
#include "OZInGameFloorUI.generated.h"

class UOZFloorViewModel;
class UOZBuffProgressEntry;
class UVerticalBox;
class UHorizontalBox;
class UOverlay;
struct FOZInventorySlotDisplayData;
struct FOZConvertSlotDisplayData;
struct FOZBuffDisplayData;

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

	void PlayPopup(FString text_Title, FString text_Msg, float lifeTime);

	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<class UOZTutorialPopupTextWidget> TutorialPopupMsg = nullptr;

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
	TObjectPtr<UOZFloorViewModel> ViewModel;

	void InitializeViewModel();

	void OnFloatPropertyUpdated(FName PropertyName, float NewValue);
	void OnInventoryDisplayUpdated(const TArray<FOZInventorySlotDisplayData>& SlotData);
	void OnConvertDisplayUpdated(const TArray<FOZConvertSlotDisplayData>& SlotData);
	void OnNewBuffDisplayed(const FOZBuffDisplayData& BuffData);
	void OnStealthStateUpdated(bool bInBush);

	void UpdateAttributeText(FName PropertyName, float Value);
	void SetConvertSlotVisual(int32 SlotIndex, EConvertGrade Grade, UTexture2D* IconTexture);
	UTexture2D* GetBGTextureForGrade(EConvertGrade Grade) const;

	TArray<TObjectPtr<class UImage>> ConvertBGSlots;
	TArray<TObjectPtr<class UImage>> ConvertIconSlots;

	UPROPERTY()
	TArray<TObjectPtr<UOZBuffProgressEntry>> ActiveBuffEntries;

	void OnBuffEntryExpired(UOZBuffProgressEntry* ExpiredEntry);
};
