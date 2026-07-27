// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SaytHUDWidget.generated.h"

class UNativeWidgetHost;
class SSaytHealthDisplay;
class SSaytWorldPanel;
class USaytIndicatorManagerComponent;
struct FSaytIndicatorEntry;

UCLASS()
class PROJECTSAYT_API USaytHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	// 위젯 생성 직후 1회 호출 - 리스너 등록 + 초기값 동기화 자리
	virtual void NativeOnInitialized() override;
	
	// 위젯이 파괴될 때 호출 - 리스너 해제 필수
	virtual void NativeDestruct() override;
	
	// WBP_HUD 디자이너에서 동명(HealthBarHost) 위젯이 자동 주입됨 (BindWidget 계약)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNativeWidgetHost> HealthBarHost;
	
private:
	void HandleIndicatorAdded(const FSaytIndicatorEntry& Entry);
	void HandleIndicatorRemoved(const FSaytIndicatorEntry& Entry);
	
	/** 유휴 위젯이 있으면 꺼내 쓰고, 없으면 새로 만든다 */
	TSharedRef<SSaytHealthDisplay> AcquireMobDisplay();
	/** 사용이 끝난 위젯을 구독 해제 후 유휴 보관소로 되돌린다 */
	void ReleaseMobDisplay(const TSharedPtr<SSaytHealthDisplay>& InDisplay);
	
	// UObject(HUD 위젯)가 Slate 위젯을 '소유'하는 표준 형태 — TSharedPtr 강참조
	// 2-3부터 범용 표시 단위로 이관 (SegmentCount 1 = Stage 1과 동일 형태)
	TSharedPtr<SSaytHealthDisplay> HealthBar;
	
	// ── 몬스터 월드 패널 (2-8) ──
	TSharedPtr<SSaytWorldPanel> WorldPanel;
	TWeakObjectPtr<USaytIndicatorManagerComponent> IndicatorManager;
	FDelegateHandle IndicatorAddedHandle;
	FDelegateHandle IndicatorRemovedHandle;
	/** 액터 → 그 액터의 바. 해제 시 대칭 정리를 위해 소유자가 들고 있는다 */
	TMap<TWeakObjectPtr<AActor>, TSharedPtr<SSaytHealthDisplay>> MobDisplays;
	
	// ── 위젯 풀 (2-8b) ──
	// 유휴 위젯 보관소. 공유 포인터 참조가 0이 되면 즉시 해제되므로
	// 풀이 강참조로 들고 있어야 재사용할 대상이 살아남는다
	TArray<TSharedPtr<SSaytHealthDisplay>> MobDisplayPool;
	
	/** 보관 상한 — 한 번 부풀어 오른 최대치를 영구히 붙잡고 있지 않기 위해 */
	static constexpr int32 MaxPooledMobDisplays = 16;
};
