// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#include "UI/ViewModel/SaytEffectTrayViewModel.h"
#include "UI/SaytEffectTrayComponent.h"
#include "UI/ViewModel/SaytEffectTrayReconcile.h"
#include "GameplayEffect.h"
#include "SaytLogChannels.h"

void USaytEffectTrayViewModel::BindToComponent(USaytEffectTrayComponent* InComponent)
{
	UnbindFromComponent();

	if (InComponent)
	{
		WatchedComponent = InComponent;
		ChangedHandle = InComponent->OnTrayChanged.AddUObject(this, &USaytEffectTrayViewModel::HandleTrayChanged);
	}

	// 구독 이전에 쌓인 상태를 따라잡는다 — 방송은 지나간 것을 다시 들려주지 않는다.
	// nullptr이면 빈 Entries와 대조되어 목록이 비워진다.
	Refresh();
}

void USaytEffectTrayViewModel::UnbindFromComponent()
{
	if (USaytEffectTrayComponent* Component = WatchedComponent.Get())
	{
		Component->OnTrayChanged.Remove(ChangedHandle);
	}
	ChangedHandle.Reset();
	WatchedComponent.Reset();
}

void USaytEffectTrayViewModel::HandleTrayChanged()
{
	Refresh();
}

void USaytEffectTrayViewModel::Refresh()
{
	static const TArray<FSaytEffectTrayEntry> NoEntries;
	const USaytEffectTrayComponent* Component = WatchedComponent.Get();
	const TArray<FSaytEffectTrayEntry>& Entries = Component ? Component->GetEntries() : NoEntries;

	TArray<const UClass*> OldKeys;
	OldKeys.Reserve(Items.Num());
	for (const USaytEffectTrayEntryViewModel* Item : Items)
	{
		OldKeys.Add(Item->GetEffectClass());
	}

	// 컴포넌트가 GE당 한 항목만 만들고 EffectClass를 항상 채운다는 전제.
	TArray<const UClass*> NewKeys;
	NewKeys.Reserve(Entries.Num());
	for (const FSaytEffectTrayEntry& Entry : Entries)
	{
		if (!Entry.EffectClass)
		{
			UE_LOG(LogSaytUI, Error, TEXT("[트레이VM] 항목의 효과 클래스 키가 비어 있다: %s"), *Entry.DisplayName.ToString());
		}

		NewKeys.Add(Entry.EffectClass.Get());
	}

	const SaytEffectTray::FReconcileResult Result = SaytEffectTray::Reconcile(OldKeys, NewKeys);

	TArray<TObjectPtr<USaytEffectTrayEntryViewModel>> NewItems;
	NewItems.Reserve(Entries.Num());
	for (int32 Index = 0; Index < Entries.Num(); ++Index)
	{
		const int32 OldIndex = Result.OldIndexForNew[Index];
		if (OldIndex != INDEX_NONE)
		{
			USaytEffectTrayEntryViewModel* Existing = Items[OldIndex];
			Existing->Apply(Entries[Index]);
			NewItems.Add(Existing);
		}
		else
		{
			USaytEffectTrayEntryViewModel* Created = NewObject<USaytEffectTrayEntryViewModel>(this);
			Created->Initialize(Entries[Index]);
			NewItems.Add(Created);
		}
	}

	UE_LOG(LogSaytUI, Verbose, TEXT("[트레이VM] 항목 %d개 / 구성 변화 %s"),
		NewItems.Num(), Result.bCompositionChanged ? TEXT("있음") : TEXT("없음"));

	if (Result.bCompositionChanged)
	{
		Items = MoveTemp(NewItems);
		UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(Items);
	}
}