#pragma once
#include "CoreMinimal.h"
#include "Widgets/SLeafWidget.h"
#include "AbilitySystem/Attributes/SayuAttributeSet_Combat.h"


class SSayuStudyHealthBar : public SLeafWidget
{
public:
	SLATE_BEGIN_ARGS(SSayuStudyHealthBar) {}
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArg);
	
	// 데이터 입구: 이 ASC의 체력 방송을 구독 + 초기 1회 Pull
	void BindToASC(UAbilitySystemComponent* InASC);
	
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry,
		const FSlateRect& MyCullingRect, FSlateWindowElementList& OutDrawElements,
		int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;
	
private:
	void HandleHealthChanged(const FOnAttributeChangeData& Data);

	// 콜백(쓰기)과 OnPaint(읽기)를 잇는 다리
	float CurrentHealth = 0.f;
	float MaxHealth = 1.f;   // 0 나눗셈 방지용 기본값
};
