// Copyright (c) 2026 Sungmin JI. All Rights Reserved.


#include "SayuUIInputModeSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

void USayuUIInputModeSubsystem::PushUIRequest(FName RequesterId)
{
	ActiveRequesters.Add(RequesterId);
	ApplyCurrentMode();
}

void USayuUIInputModeSubsystem::PopUIRequest(FName RequesterId)
{
	ActiveRequesters.Remove(RequesterId);
	ApplyCurrentMode();
}

void USayuUIInputModeSubsystem::ApplyCurrentMode()
{
	APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
	if (!PC)
	{
		return;
	}

	if (ActiveRequesters.Num() > 0)
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}
	else
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetShowMouseCursor(false);
	}
}
