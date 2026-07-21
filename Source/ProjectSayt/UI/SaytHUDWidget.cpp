// Copyright (c) 2026 Sungmin JI. All Rights Reserved.


#include "SaytHUDWidget.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Components/NativeWidgetHost.h"
#include "UI/Slate/SSaytHealthDisplay.h"

void USaytHUDWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	if (HealthBarHost)
	{
		// 범용 표시 단위 생성(기본 SegmentCount 1 → 트레이 없음, Stage 1과 동일 룩)
		// → UMG 트리에 안착 → 플레이어 ASC에 바인딩 (초기 Pull은 BindToASC 내부 담당)
		HealthBar = SNew(SSaytHealthDisplay);
		HealthBarHost->SetContent(HealthBar.ToSharedRef());
		
		if (UAbilitySystemComponent* ASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(GetOwningPlayerPawn()))
		{
			HealthBar->BindToASC(ASC);
		}
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
	
	Super::NativeDestruct();
}