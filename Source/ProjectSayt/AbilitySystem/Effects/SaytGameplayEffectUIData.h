// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectUIData.h"

#include "SaytGameplayEffectUIData.generated.h"

class FDataValidationContext;
class UTexture2D;

/** 트레이 표시 분류 — 테두리 색만 가른다. 게임플레이 판정에는 쓰지 않는다. */
UENUM(BlueprintType)
enum class ESaytEffectCategory : uint8
{
	Buff,
	Debuff
};

/**
 * 버프/디버프 트레이에 노출할 표시 데이터.
 *
 * 이 컴포넌트가 붙어 있다는 것 자체가 '트레이에 표시한다'는 선언이다.
 * 별도의 표시 여부 플래그를 두지 않는 이유는, 아이콘 없이는 표시가 불가능하므로
 * 표시 의도와 표시 재료가 분리될 수 없기 때문이다.
 *
 * 런타임에 변하는 값(스택 수, 잔여 시간, 진행 중인 연출)은 여기 두지 않는다.
 * GE 하나당 이 컴포넌트는 하나뿐이고 모든 적용 인스턴스가 이것을 공유하므로,
 * 인스턴스별 상태를 담으면 서로 덮어쓴다.
 */
UCLASS(DisplayName = "UI Data (Sayt Tray)")
class PROJECTSAYT_API USaytGameplayEffectUIData : public UGameplayEffectUIData
{
	GENERATED_BODY()

public:
	/** 툴팁과 접근성 텍스트가 쓴다. 유저 노출 문자열이므로 FText. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Display)
	FText DisplayName;

	/** 트레이에 그릴 아이콘 텍스처. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Display)
	TObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Display)
	ESaytEffectCategory Category = ESaytEffectCategory::Buff;

#if WITH_EDITOR
	virtual EDataValidationResult IsDataValid(FDataValidationContext& Context) const override;
#endif
};