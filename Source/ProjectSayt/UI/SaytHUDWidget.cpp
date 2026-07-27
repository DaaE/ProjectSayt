// Copyright (c) 2026 Sungmin JI. All Rights Reserved.


#include "SaytHUDWidget.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/NativeWidgetHost.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/PlayerController.h"
#include "UI/SaytHealthDisplayTypes.h"
#include "UI/SaytIndicatorManagerComponent.h"
#include "UI/Slate/SSaytHealthDisplay.h"
#include "UI/Slate/SSaytWorldPanel.h"
#include "SaytLogChannels.h"

void USaytHUDWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	if (HealthBarHost)
	{
		// enum → 프리셋 조회 → 개별 값 전달. 위젯은 타입을 모른다
		const FSaytHealthDisplayPreset Preset = GetSaytHealthDisplayPreset(ESaytHealthDisplayType::Player);
		HealthBar = SNew(SSaytHealthDisplay)
			.BarStyle(Preset.BarStyle)
			.DesiredBarSize(Preset.BarSize);
		
		HealthBarHost->SetContent(HealthBar.ToSharedRef());
		
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwningPlayerPawn()))
		{
			HealthBar->BindToASC(ASC);
		}
	}
	
	// ── 몬스터 월드 패널 ──
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		return;
	}
	
	WorldPanel = SNew(SSaytWorldPanel, PC);
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->AddViewportWidgetContent(WorldPanel.ToSharedRef(), 0);
	}
	
	USaytIndicatorManagerComponent* Manager = USaytIndicatorManagerComponent::FindOrAdd(PC);
	if (!Manager)
	{
		return;
	}
	
	IndicatorManager = Manager;
	IndicatorAddedHandle = Manager->OnIndicatorAdded.AddUObject(this, &USaytHUDWidget::HandleIndicatorAdded);
	IndicatorRemovedHandle = Manager->OnIndicatorRemoved.AddUObject(this, &USaytHUDWidget::HandleIndicatorRemoved);
	
	// 구독보다 먼저 등록된 대상 따라잡기 — 알림은 지나간 것을 다시 들려주지 않는다
	for (const FSaytIndicatorEntry& Entry : Manager->GetIndicators())
	{
		HandleIndicatorAdded(Entry);
	}
}

void USaytHUDWidget::NativeDestruct()
{
	// 대칭 해제 — Slate 바 소멸자도 스스로 풀지만, 소유자가 명시적으로 정리하는 규율 유지
	if (HealthBar.IsValid())
	{
		HealthBar->UnbindFromASC();
	}
	HealthBar.Reset();
	
	if (USaytIndicatorManagerComponent* Manager = IndicatorManager.Get())
	{
		Manager->OnIndicatorAdded.Remove(IndicatorAddedHandle);
		Manager->OnIndicatorRemoved.Remove(IndicatorRemovedHandle);
	}
	IndicatorManager.Reset();
	
	for (TPair<TWeakObjectPtr<AActor>, TSharedPtr<SSaytHealthDisplay>>& Pair : MobDisplays)
	{
		if (Pair.Value.IsValid())
		{
			Pair.Value->UnbindFromASC();
		}
	}
	MobDisplays.Empty();
	
	// 유휴 위젯도 소유자가 들고 있는 강참조이므로 여기서 놓아야 해제된다
	MobDisplayPool.Empty();
	
	if (WorldPanel.IsValid() && GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->RemoveViewportWidgetContent(WorldPanel.ToSharedRef());
	}
	WorldPanel.Reset();
	
	Super::NativeDestruct();
}

void USaytHUDWidget::HandleIndicatorAdded(const FSaytIndicatorEntry& Entry)
{
	AActor* Actor = Entry.Actor.Get();
	if (!Actor || !WorldPanel.IsValid())
	{
		return;
	}

	// 지금은 몹만 이 패널이 담당한다 (플레이어는 HUD 고정 위치, 보스는 파트 A 결과물)
	if (Entry.DisplayType != ESaytHealthDisplayType::Mob)
	{
		return;
	}

	if (MobDisplays.Contains(Entry.Actor))
	{
		return;
	}

	TSharedRef<SSaytHealthDisplay> Display = AcquireMobDisplay();

	// 재사용 위젯의 잔여 표시도 여기서 전부 새로 세팅된다 —
	// BindToASC가 구독 등록 + 현재 값 직접 읽기를 함께 하기 때문
	if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Actor))
	{
		Display->BindToASC(ASC);
	}

	WorldPanel->AddSlot()
		.TrackedActor(Actor)
		[
			Display
		];

	MobDisplays.Add(Entry.Actor, Display);
}

void USaytHUDWidget::HandleIndicatorRemoved(const FSaytIndicatorEntry& Entry)
{
	if (TSharedPtr<SSaytHealthDisplay>* Found = MobDisplays.Find(Entry.Actor))
	{
		ReleaseMobDisplay(*Found);
		MobDisplays.Remove(Entry.Actor);
	}

	if (WorldPanel.IsValid())
	{
		WorldPanel->RemoveSlotForActor(Entry.Actor);
	}
}

TSharedRef<SSaytHealthDisplay> USaytHUDWidget::AcquireMobDisplay()
{
	while (MobDisplayPool.Num() > 0)
	{
		TSharedPtr<SSaytHealthDisplay> Reused = MobDisplayPool.Pop();
		if (Reused.IsValid())
		{
			UE_LOG(LogSaytUI, Verbose, TEXT("[풀] 재사용 (유휴 %d개 남음)"), MobDisplayPool.Num());
			return Reused.ToSharedRef();
		}
	}

	const FSaytHealthDisplayPreset Preset = GetSaytHealthDisplayPreset(ESaytHealthDisplayType::Mob);
	UE_LOG(LogSaytUI, Verbose, TEXT("[풀] 신규 생성 (유휴 없음)"));

	return SNew(SSaytHealthDisplay)
		.SegmentCount(1)
		.BarStyle(Preset.BarStyle)
		.DesiredBarSize(Preset.BarSize);
}

void USaytHUDWidget::ReleaseMobDisplay(const TSharedPtr<SSaytHealthDisplay>& InDisplay)
{
	if (!InDisplay.IsValid())
	{
		return;
	}

	// 구독을 반드시 끊는다 — 남겨 두면 재사용 위젯이 죽은 대상의 신호를 계속 받는다
	InDisplay->UnbindFromASC();

	if (MobDisplayPool.Num() < MaxPooledMobDisplays)
	{
		MobDisplayPool.Add(InDisplay);
		UE_LOG(LogSaytUI, Verbose, TEXT("[풀] 반납 (유휴 %d개)"), MobDisplayPool.Num());
	}
	else
	{
		UE_LOG(LogSaytUI, Verbose, TEXT("[풀] 상한 초과로 폐기"));
	}
}