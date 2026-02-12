// Fill out your copyright notice in the Description page of Project Settings.


#include "Widget/OZTrainingCustomModuelWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UOZTrainingCustomModuelWidget::SetData(FOZCustomModuleData moduleData)
{
	Image_ModuelIcon->SetBrushFromTexture(moduleData.ModuleIcon);
	//Text_ModuleName->SetText(moduleData.Module_Name);

	Module_ID = moduleData.ModuleID;

	Module_Value = moduleData.Module_PerValue;
	Module_Max = moduleData.Module_Max;
	Module_Min = moduleData.Module_Min;

	//사실 인자는 필요 없음
	SetName(moduleData.Module_Name);
}

int UOZTrainingCustomModuelWidget::Call_GetAmount()
{
	return GetAmount();
}
