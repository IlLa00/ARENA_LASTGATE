// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/OZInGameTutorialPopupSubSystem.h"
#include "UserInterface/OZInGameFloorUI.h"
#include "Widget/OZTutorialPopupTextWidget.h"
#include "OZInGameTutorialPopupSubSystem.h"

void UOZInGameTutorialPopupSubSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (TutorialMsgTableRef.IsNull())
		TutorialMsgTableRef = FSoftObjectPath(TEXT("/Game/Data/DT_TutorialMsg.DT_TutorialMsg"));

	TutorialMsgTable = TutorialMsgTableRef.LoadSynchronous();

	if (!TutorialMsgTable)
		UE_LOG(LogTemp, Error, TEXT("[TutorialMsgSubsystem] ? Failed to load FogDataTable!"));
}

FOZTutoMessageData* UOZInGameTutorialPopupSubSystem::GetTutorialMsgData(int MsgID)
{
	if (!TutorialMsgTable)
	{
		UE_LOG(LogTemp, Error, TEXT("[FogSubsystem] FogDataTable is null!"));
		return nullptr;
	}

	TArray<FName> RowNames = TutorialMsgTable->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		FOZTutoMessageData* Data = TutorialMsgTable->FindRow<FOZTutoMessageData>(RowName, TEXT(""));
		if (Data && Data->Msg_ID == MsgID)
		{
			return Data;
		}
	}

	return nullptr;
}

void UOZInGameTutorialPopupSubSystem::BindPopupEndEvent()
{
	if (inGameFloorUI == nullptr)
		return;

	if (inGameFloorUI->TutorialPopupMsg == nullptr)
		return;

	inGameFloorUI->TutorialPopupMsg->OnPopupAnimationEnd.AddDynamic(this, &UOZInGameTutorialPopupSubSystem::OnTutorialPopupEnd);
}

void UOZInGameTutorialPopupSubSystem::CallTutorialPopupByID(int MsgID, float LifeTime)
{
	/*FOZTutoMessageData* Data = GetTutorialMsgData(MsgID);

	if (Data == nullptr)
		return;

	if (inGameFloorUI == nullptr)
		return;

	inGameFloorUI->WBP_TutorialPopupMsg->SetText(Data->Title, Data->MsgText);
	inGameFloorUI->WBP_TutorialPopupMsg->OnPlayTutorialText(LifeTime);*/

	PopupRequestQueue.Enqueue({ MsgID, LifeTime });

	TryPlayNextPopup();
}

void UOZInGameTutorialPopupSubSystem::TryPlayNextPopup()
{
	if (bIsPopupPlaying)
		return;

	if (inGameFloorUI == nullptr || inGameFloorUI->TutorialPopupMsg == nullptr)
		return;

	FTutorialPopupRequest Request;
	if (!PopupRequestQueue.Dequeue(Request))
		return;

	FOZTutoMessageData* Data = GetTutorialMsgData(Request.MsgID);
	if (Data == nullptr)
	{
		TryPlayNextPopup();
		return;
	}

	bIsPopupPlaying = true;

	inGameFloorUI->PlayPopup(Data->Title, Data->MsgText, Request.LifeTime);
}

void UOZInGameTutorialPopupSubSystem::OnTutorialPopupEnd()
{
	bIsPopupPlaying = false;

	TryPlayNextPopup();
}
