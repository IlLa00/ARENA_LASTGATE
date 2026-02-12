#include "UserInterface/OZBuffProgressEntry.h"
#include "Components/ProgressBar.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UOZBuffProgressEntry::InitBuff(UTexture2D* Icon, float InTotalDuration, FGameplayTag InBuffTag, const FText& BuffName)
{
	TotalDuration = InTotalDuration;
	RemainingTime = InTotalDuration;
	BuffTag = InBuffTag;

	if (Img_BuffIcon && Icon)
	{
		Img_BuffIcon->SetBrushFromTexture(Icon);
		Img_BuffIcon->SetVisibility(ESlateVisibility::Visible);
	}

	if (Text_BuffName)
	{
		Text_BuffName->SetText(BuffName);
	}

	UTexture2D* ChosenFillTex = DefaultFillTex;

	if (const TObjectPtr<UTexture2D>* Found = FillTexByBuffTag.Find(BuffTag))
	{
		ChosenFillTex = Found->Get();
	}

	if (Img_ProgressFill && ProgressFillMaterial)
	{
		ProgressFillMID = UMaterialInstanceDynamic::Create(ProgressFillMaterial, this);
		Img_ProgressFill->SetBrushFromMaterial(ProgressFillMID);

		// �ʱⰪ
		ProgressFillMID->SetScalarParameterValue(FName("Percent"), 1.f);
	}


	if (ProgressFillMID && ChosenFillTex)
	{
		ProgressFillMID->SetTextureParameterValue(FName("FillTex"), ChosenFillTex);
	}


	UpdateProgressUI();
}

void UOZBuffProgressEntry::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (TotalDuration <= 0.f)
		return;

	RemainingTime = FMath::Max(0.f, RemainingTime - InDeltaTime);
	UpdateProgressUI();

	if (RemainingTime <= 0.f)
	{
		OnBuffExpired.ExecuteIfBound(this);
		RemoveFromParent();
	}
}

void UOZBuffProgressEntry::NativeConstruct()
{
	Super::NativeConstruct();

	
}

void UOZBuffProgressEntry::UpdateProgressUI()
{
	if (TotalDuration <= 0.f)
		return;

	if (Text_RemainingTime)
	{
		const FString TimeText = FString::Printf(TEXT("%.1f"), FMath::Max(0.f, RemainingTime));
		Text_RemainingTime->SetText(FText::FromString(TimeText));
	}

	const float ClampedRemaining = FMath::Max(0.f, RemainingTime);
	const float Percent = FMath::Clamp(ClampedRemaining / TotalDuration, 0.f, 1.f);

	if (ProgressFillMID)
	{
		ProgressFillMID->SetScalarParameterValue(FName("Percent"), Percent);
	}
}
