// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#include "UI/ViewModel/SaytEffectTrayEntryViewModel.h"
#include "UI/SaytEffectTrayComponent.h"
#include "GameplayEffect.h"

void USaytEffectTrayEntryViewModel::Initialize(const FSaytEffectTrayEntry& Entry)
{
	EffectClass = Entry.EffectClass;
	Apply(Entry);
}

void USaytEffectTrayEntryViewModel::Apply(const FSaytEffectTrayEntry& Entry)
{
	// 값이 같으면 통지하지 않는다 — 개수만 바뀌면 개수 바인딩만 갱신된다.
	UE_MVVM_SET_PROPERTY_VALUE(DisplayName, Entry.DisplayName);
	UE_MVVM_SET_PROPERTY_VALUE(Count, Entry.Count);
	UE_MVVM_SET_PROPERTY_VALUE(Category, Entry.Category);
}

const UClass* USaytEffectTrayEntryViewModel::GetEffectClass() const
{
	return EffectClass.Get();
}
