// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Styling/SlateWidgetStyle.h"
#include "Styling/SlateWidgetStyleContainerBase.h"

#include "SayuHealthBarWidgetStyle.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct PROJECTGAS_API FSayuHealthBarStyle : public FSlateWidgetStyle
{
	GENERATED_USTRUCT_BODY()

	FSayuHealthBarStyle();
	virtual ~FSayuHealthBarStyle();

	// FSlateWidgetStyle
	virtual void GetResources(TArray<const FSlateBrush*>& OutBrushes) const override;
	static const FName TypeName;
	virtual const FName GetTypeName() const override { return TypeName; };
	static const FSayuHealthBarStyle& GetDefault();
	
	// ── 비주얼 멤버 ──
	UPROPERTY(EditAnywhere, Category = Appearance)
	FSlateBrush BackgroundBrush;
	FSayuHealthBarStyle& SetBackgroundBrush(const FSlateBrush& InBrush) { BackgroundBrush = InBrush; return *this; }

	UPROPERTY(EditAnywhere, Category = Appearance)
	FSlateBrush FillBrush;
	FSayuHealthBarStyle& SetFillBrush(const FSlateBrush& InBrush) { FillBrush = InBrush; return *this; }

	UPROPERTY(EditAnywhere, Category = Appearance)
	FSlateBrush GhostBrush;
	FSayuHealthBarStyle& SetGhostBrush(const FSlateBrush& InBrush) { GhostBrush = InBrush; return *this; }

	// ── 고스트 연출 파라미터 (연출 튜닝 값은 스타일 소관 — EnableFillAnimation 선례) ──
	UPROPERTY(EditAnywhere, Category = Appearance)
	float GhostDelay = 0.25f;   // 피격 후 잔상이 줄기 시작할 때까지 대기(초)
	FSayuHealthBarStyle& SetGhostDelay(float InValue) { GhostDelay = InValue; return *this; }
	
	UPROPERTY(EditAnywhere, Category = Appearance)
	float GhostDrainSpeed = 1.2f;   // 잔상 감소 속도(초당 전체 대비 비율)
	FSayuHealthBarStyle& SetGhostDrainSpeed(float InValue) { GhostDrainSpeed = InValue; return *this; }
	
	// 기본 틴트 — 런타임 세터(줄 색 스왑)가 덮어쓰기 전의 기본 룩.
	// 색은 브러시 원색(흰색)에 곱셈으로 입힘 — 원색에 색이 있으면 곱셈이 오염되므로
	UPROPERTY(EditAnywhere, Category = Appearance)
	FSlateColor DefaultFillTint = FLinearColor(0.7f, 0.1f, 0.1f);
	FSayuHealthBarStyle& SetDefaultFillTint(const FSlateColor& InTint) { DefaultFillTint = InTint; return *this; }
	
	UPROPERTY(EditAnywhere, Category = Appearance)
	FSlateColor DefaultBackgroundTint = FLinearColor(0.02f, 0.02f, 0.025f, 0.9f);
	FSayuHealthBarStyle& SetDefaultBackgroundTint(const FSlateColor& InTint) { DefaultBackgroundTint = InTint; return *this; }
};

// 에디터 에셋 컨테이너 — 이 스타일을 에디터에서 에셋으로 만들 수 있게 하는 포장.
// 지금은 사용하지 않고 존재만 시킴 (UMG 래핑 Stage 3/5에서 존재 이유를 다룸).
UCLASS(hidecategories=Object, MinimalAPI)		
class USayuHealthBarWidgetStyle : public USlateWidgetStyleContainerBase
{
	GENERATED_BODY()

public:
	/** The actual data describing the widget appearance. */
	UPROPERTY(Category=Appearance, EditAnywhere, meta=(ShowOnlyInnerProperties))
	FSayuHealthBarStyle WidgetStyle;

	virtual const struct FSlateWidgetStyle* const GetStyle() const override
	{
		return static_cast<const struct FSlateWidgetStyle*>(&WidgetStyle);
	}
};
