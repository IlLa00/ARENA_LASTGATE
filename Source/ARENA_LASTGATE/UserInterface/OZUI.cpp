// Fill out your copyright notice in the Description page of Project Settings.


#include "UserInterface/OZUI.h"

void UOZUI::OnDelayedInitialize()
{
	OnUIConstructed.Broadcast();
}