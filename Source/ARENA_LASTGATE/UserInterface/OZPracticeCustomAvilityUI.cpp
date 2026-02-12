// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/OZPracticeCustomAvilityUI.h"
#include "Components/ScrollBox.h"
#include "Components/SizeBox.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Border.h"
#include "Subsystem/OZConvertSubsystem.h"
#include "Widget/OZTraningConverterWidget.h"

#include "Data/OZConvertData.h"

void UOZPracticeCustomAvilityUI::NativeConstruct()
{
	Super::NativeConstruct();

	converterSubsystem = GetGameInstance()->GetSubsystem<UOZConvertSubsystem>();
	if (converterSubsystem == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Fail to Get converterSubsystem"));
		return;
	}

	InitConverterScrollBox();

}

void UOZPracticeCustomAvilityUI::ShowCustomAvilityUI()
{
	OnShowCustomAvilityUI();
}

void UOZPracticeCustomAvilityUI::HideCustomAvilityUI()
{
	OnHideCustomAvilityUI();
}

void UOZPracticeCustomAvilityUI::InitConverterScrollBox()
{
	ConverterWidgetMap.Empty();

	const TArray<FOZConvertData>& converterList = converterSubsystem->GetAllConverts();

	const int32 totalConvertNum = converterList.Num();
	const int32 NumSizeBox = (totalConvertNum + ConverterNumperLine - 1) / ConverterNumperLine;

	int32 convertCount = 0;

	for (int32 boxIndex = 0; boxIndex < NumSizeBox; boxIndex++)
	{
		USizeBox* sizeBox = NewObject<USizeBox>(this);
		sizeBox->SetHeightOverride(300.f);

		UHorizontalBox* horizontalBox = NewObject<UHorizontalBox>(this);

		int32 addedThisLine = 0;

		for (int32 i = 0; i < ConverterNumperLine; i++)
		{
			if (convertCount >= totalConvertNum)
				break;

			UOZTraningConverterWidget* converterWidget = CreateWidget<UOZTraningConverterWidget>(this, ConverterWidget);

			UHorizontalBoxSlot* slot = horizontalBox->AddChildToHorizontalBox(converterWidget);

			slot->SetPadding(FMargin(10.f, 5.f, 5.f, 5.f));
			slot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));

            const FOZConvertData& ConvertData = converterList[convertCount];
            converterWidget->Text_ConverterName->SetText(ConvertData.Name);
            converterWidget->Image_ConverterIcon->SetBrushFromSoftTexture(ConvertData.Icon.Get());
			converterWidget->ID = ConvertData.ID;
			converterWidget->Grade = ConvertData.Grade;

			ConverterWidgetMap.Add(ConvertData.ID , converterWidget);

			convertCount++;
			addedThisLine++;
		}

		const int32 EmptySlotCount = ConverterNumperLine - addedThisLine;

		for (int32 i = 0; i < EmptySlotCount; i++)
		{
			UBorder* EmptyBorder = NewObject<UBorder>(this);
			EmptyBorder->SetBrushColor(FLinearColor::Transparent);

			UHorizontalBoxSlot* emptySlot = horizontalBox->AddChildToHorizontalBox(EmptyBorder);

			emptySlot->SetPadding(FMargin(10.f, 5.f, 5.f, 5.f));
			emptySlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		}

		sizeBox->AddChild(horizontalBox);
		ConverterListScrollBox->AddChild(sizeBox);
	}


	OnInitializeEnd();

}
