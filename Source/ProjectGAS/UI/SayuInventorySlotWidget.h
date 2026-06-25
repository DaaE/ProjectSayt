// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "SayuInventorySlotWidget.generated.h"

class UTextBlock;
class UImage;

// 드래그 중 이 칸에 배치 가능한지를 나타내는 시각적 상태.
UENUM(BlueprintType)
enum class ESayuPlacementHighlight : uint8
{
	None,    // 평상시 — 하이라이트 없음
	Valid,   // 배치 가능 (녹색)
	Invalid  // 배치 불가 (빨강)
};

/**
 * 그리드 한 칸(셀)을 표현하는 위젯.
 * 점유 상태 표시(View)만 담당 — 배치 가능 여부 판정은 절대 여기서 하지 않음
 * (Model-View 분리 — 판정은 항상 USayuInventoryComponent::CanPlaceItemAt이 함).
 * USayuInventoryItemWidget(별도 오버레이 레이어, Option B)이 전담함 —
 * SlotWidget은 순수하게 빈 배경 + 드래그 중 하이라이트만 책임짐.
 */
UCLASS()
class PROJECTGAS_API USayuInventorySlotWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	// 드래그 중 이 칸의 하이라이트 상태를 외부(그리드 컨테이너)에서 지정.
	void SetHighlight(ESayuPlacementHighlight NewState);

protected:
	// 평상시엔 숨겨져 있다가, 드래그 중에만 녹/빨로 보여지는 오버레이.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> HighlightOverlay;
};
