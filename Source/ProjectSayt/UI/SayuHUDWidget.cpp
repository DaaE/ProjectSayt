// Copyright (c) 2026 Sungmin JI. All Rights Reserved.


#include "SayuHUDWidget.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/NativeWidgetHost.h"
#include "UI/Slate/SSayuHealthBar.h"

void USayuHUDWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	if (HealthBarHost)
	{
		// Slate 바 생성 → UMG 트리에 안착 → 플레이어 ASC에 바인딩.
		// (초기 1회 Pull은 BindToASC 내부가 담당 — 구 SyncInitialHealthFromASC의 역할 승계)
		HealthBar = SNew(SSayuHealthBar);
		HealthBarHost->SetContent(HealthBar.ToSharedRef());
		
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwningPlayerPawn()))
		{
			HealthBar->BindToASC(ASC);
		}
	}
}

void USayuHUDWidget::NativeDestruct()
{
	// 대칭 해제 — Slate 바 소멸자도 스스로 풀지만, 소유자가 명시적으로 정리하는 규율 유지
	if (HealthBar.IsValid())
	{
		HealthBar->UnbindFromASC();
	}
	HealthBar.Reset();
	
	Super::NativeDestruct();
}