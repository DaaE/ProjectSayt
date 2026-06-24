// Fill out your copyright notice in the Description page of Project Settings.


#include "SayuItemInstance.h"
#include "SayuItemIdSubsystem.h"

USayuItemInstance* USayuItemInstance::CreateInstance(UObject* Outer, USayuItemDefinition* Definition)
{
	if (!Outer || !Definition)
	{
		return nullptr;
	}

	USayuItemInstance* NewInstance = NewObject<USayuItemInstance>(Outer);
	NewInstance->ItemDefinition = Definition;

	if (UWorld* World = Outer->GetWorld())
	{
		if (UGameInstance* GameInstance = World->GetGameInstance())
		{
			if (USayuItemIdSubsystem* IdSubsystem = GameInstance->GetSubsystem<USayuItemIdSubsystem>())
			{
				NewInstance->ItemInstanceId = IdSubsystem->IssueNextItemInstanceId();
			}
		}
	}

	return NewInstance;
}
