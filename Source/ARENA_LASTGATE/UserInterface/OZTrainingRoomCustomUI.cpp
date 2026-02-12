// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/OZTrainingRoomCustomUI.h"
#include "Subsystem/OZConvertSubsystem.h"
#include "Components/ScrollBox.h"
#include "Components/ScrollBoxSlot.h"
#include "Widget/OZTrainingCustomModuelWidget.h"
#include "Widget/OZConverterListSlotWidget.h"

void UOZTrainingRoomCustomUI::NativeConstruct()
{
	Super::NativeConstruct();

	converterSubsystem = GetGameInstance()->GetSubsystem<UOZConvertSubsystem>();
	if (converterSubsystem == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Fail to Get converterSubsystem"));
		return;
	}

	moduleSubsystem = GetGameInstance()->GetSubsystem<UOZModuleSubsystem>();
	if (moduleSubsystem == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Fail to Get moduleSubsystem"));
		return;
	}

	InitModuleWidget();
	InitListLayout();
}

TArray<FOZConvertData> UOZTrainingRoomCustomUI::GetEquipSlotConvertData()
{
	TArray<FOZConvertData> equipSlotDats;
	for (UOZConverterEquipSlotWidget* currEquipSlot : equipSlotArray)
	{
		if (currEquipSlot->assingedConvertID == -1)
		{
			equipSlotDats.Add(FOZConvertData());
			continue;
		}
			
		equipSlotDats.Add(currEquipSlot->GetConvertData());
	}
	return equipSlotDats;
}

void UOZTrainingRoomCustomUI::SetOwnerHUD(AOZPracticeHUD* owningHUD)
{
	if (owningHUD != nullptr)
		OZPracticeHUD = owningHUD;
}

void UOZTrainingRoomCustomUI::InitModuleWidget()
{
	const TArray<FOZModuleData>& moduleDatas = moduleSubsystem->GetAllModules();

	for (FOZModuleData moduleData : moduleDatas)
	{
		int moduleID = moduleData.Module_ID;

		switch (moduleID)
		{

		case 40101: //HP
			ApplyCustomModule(moduleData, Module_HP);
			break;

		case 40102: //Stanima
			ApplyCustomModule(moduleData, Module_Stamina);
			break;

		case 40103: //WalkSpeed
			ApplyCustomModule(moduleData, Module_WalkSpeed);
			break;

		case 40104: //SprintSpeed
			ApplyCustomModule(moduleData, Module_SprintSpeed);
			break;

		case 40105: //EvLDistance
			ApplyCustomModule(moduleData, Module_EvLDistance);
			break;

		case 40106: //Sheird
			ApplyCustomModule(moduleData, Module_Shield);
			break;

		case 40107: //Armor
			ApplyCustomModule(moduleData, Module_Armor);
			break;
		default:
			break;
		}
	}
}

void UOZTrainingRoomCustomUI::InitListLayout()
{
	totalConverterMap.Empty();

	OnNativeConstructEnd();
}

void UOZTrainingRoomCustomUI::PrepareSlots()
{
	MakeListSlot();
	MakeEquipSlot();
	PrepareCustomModule();
}

void UOZTrainingRoomCustomUI::PrepareCustomModule()
{
	CustomModuleWidget_HP->SetData(Module_HP);
	CustomModuleWidget_Stamina->SetData(Module_Stamina);
	CustomModuleWidget_WalkSpeed->SetData(Module_WalkSpeed);
	CustomModuleWidget_SprintSpeed->SetData(Module_SprintSpeed);
	CustomModuleWidget_EvLDistance->SetData(Module_EvLDistance);
	CustomModuleWidget_Shield->SetData(Module_Shield);
	CustomModuleWidget_Armor->SetData(Module_Armor);
}

//void UOZTrainingRoomCustomUI::PrepareCustomModule()
//{
//
//	/*CustomModuleWidget_HP->SetData(Module_HP);
//	CustomModuleWidget_Stamina->SetData(Module_Stamina);
//	CustomModuleWidget_WalkSpeed->SetData(Module_WalkSpeed);
//	CustomModuleWidget_SprintSpeed->SetData(Module_SprintSpeed);
//	CustomModuleWidget_EvLDistance->SetData(Module_EvLDistance);
//	CustomModuleWidget_Shield->SetData(Module_Shield);
//	CustomModuleWidget_Armor->SetData(Module_Armor);*/
//}

void UOZTrainingRoomCustomUI::MakeListSlot()
{
	const TArray<FOZConvertData>& converterList = converterSubsystem->GetAllConverts();

	for (int i = 0; i < converterList.Num(); i++)
	{
		FOZConvertData currConverterData = converterList[i];

		if (TObjectPtr<UOZTrainingConverterListWidget>* slotPtr = TierSlot_Map.Find(currConverterData.Grade))
		{
			UOZTrainingConverterListWidget* targetSlotList = slotPtr->Get();

			if (targetSlotList == nullptr)
				continue;

			UOZConverterListSlotWidget* listSlotWidget = CreateWidget<UOZConverterListSlotWidget>(this, ListSlotWidget);

			if (targetSlotList->slotScrollBox->HasAnyChildren())
			{

				targetSlotList->slotScrollBox->AddChild(listSlotWidget);

				if (UScrollBoxSlot* ScrollSlot = Cast<UScrollBoxSlot>(listSlotWidget->Slot))
				{
					ScrollSlot->SetPadding(FMargin(ListSlotPadd, 0.0f, 0.0f, 0.0f));
				}
			}

			else
			{
				targetSlotList->slotScrollBox->AddChild(listSlotWidget);
			}

			listSlotWidget->InitConverWidget(currConverterData);
			listSlotWidget->OnConverterSelectedDeligate.AddDynamic(this, &UOZTrainingRoomCustomUI::OnListSlotSelected);

			totalConverterMap.Add(currConverterData.ID, listSlotWidget);
		}
	}
}

void UOZTrainingRoomCustomUI::MakeEquipSlot()
{
	if (equipSlotArray.IsEmpty())
		return;

	for (int i = 0; i < equipSlotArray.Num(); i++)
	{
		equipSlotArray[i]->OnEquipSlotSelected.AddDynamic(this, &UOZTrainingRoomCustomUI::OnEquipSlotSelected);
	}
}

void UOZTrainingRoomCustomUI::OnListSlotSelected(UOZConverterListSlotWidget* clickedWidget, bool bIsLeftMouseClick)
{
	if (clickedWidget == nullptr)
		return;

	FOZConvertData convertData = clickedWidget->converterData;

	//좌클릭 일 경우
	if (bIsLeftMouseClick == true)
	{
		int convertID = convertData.ID;
		EConvertGrade convertGrad = convertData.Grade;

		if (convertGrad != EConvertGrade::Rare)
		{
			if (CheckOverlap(convertData.ID) == true)
				return;
		}

		if (AddEquip(convertData) == true)
		{
			totalConverterMap[convertData.ID]->CallBoundActive(true);
			totalConverterMap[convertData.ID]->equipCount += 1;
		}
	}

	//우클릭 일 경우
	else
	{
		for (int i = 0; i < equipSlotArray.Num(); i++)
		{
			if (equipSlotArray[i]->assingedConvertID != convertData.ID)
				continue;

			equipSlotArray[i]->SetSlotDefault();
			totalConverterMap[convertData.ID]->equipCount -= 1;

			if (totalConverterMap[convertData.ID]->equipCount == 0)
				totalConverterMap[convertData.ID]->CallBoundActive(false);

			break;
		}
	}

}

void UOZTrainingRoomCustomUI::OnEquipSlotSelected(FOZConvertData assingedConvertData, int equipIndex)
{
	if (equipSlotArray[equipIndex]->assingedConvertID != assingedConvertData.ID)
		return;

	if (totalConverterMap.Find(assingedConvertData.ID) != nullptr)
	{
		equipSlotArray[equipIndex]->SetSlotDefault();

		totalConverterMap[assingedConvertData.ID]->equipCount -= 1;

		if (totalConverterMap[assingedConvertData.ID]->equipCount == 0)
			totalConverterMap[assingedConvertData.ID]->CallBoundActive(false);

		return;
	}
}

bool UOZTrainingRoomCustomUI::CheckOverlap(int convertID)
{
	if (equipSlotArray.IsEmpty())
		return true;

	for (int i = 0; i < equipSlotArray.Num(); i++)
	{
		if (equipSlotArray[i]->assingedConvertID == convertID)
			return true;
	}

	return false;
}

bool UOZTrainingRoomCustomUI::AddEquip(FOZConvertData addedConvertData)
{
	for (int i = 0; i < equipSlotArray.Num(); i++)
	{
		if (equipSlotArray[i]->assingedConvertID != -1)
			continue;

		equipSlotArray[i]->SetEquipSlotConverter(addedConvertData);
		return true;
	}

	return false;
}

void UOZTrainingRoomCustomUI::ApplyCustomModule(const FOZModuleData& moduleData, FOZCustomModuleData& cumtomModuleData)
{
	cumtomModuleData.ModuleID = moduleData.Module_ID;
	cumtomModuleData.ModuleIcon = moduleData.Icon.Get();
	cumtomModuleData.Module_PerValue = moduleData.ValueMin;
	cumtomModuleData.Module_Name = moduleData.name;
}