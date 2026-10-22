// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "AbilitySystem/Effects/SaytGameplayEffectUIData.h"   // ESaytEffectCategory
#include "SaytEffectTrayEntryViewModel.generated.h"

class UGameplayEffect;
struct FSaytEffectTrayEntry;

/**
 * 트레이 항목 하나의 ViewModel (Phase 8 Stage 3 파트 B).
 * 실행 위치: 클라이언트 (로컬 표시 계층).
 *
 * FSaytEffectTrayEntry를 FieldNotify 인터페이스로 맞추는 얇은 어댑터다(Adapter 패턴).
 * 표시에 쓰는 값만 옮긴다. 아이콘과 잔여 시간 조회용 핸들은 파트 C에서 필요해질 때 추가한다.
 *
 * 생성 방식은 Manual 하나만 허용한다 — 항목 위젯에는 패널 확장이 객체를 넣어 준다.
 */
UCLASS(meta = (MVVMAllowedContextCreationType = "Manual"))
class PROJECTSAYT_API USaytEffectTrayEntryViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	/** 생성 직후 한 번 호출한다. 대조 키를 정하고 표시 값을 채운다. */
	void Initialize(const FSaytEffectTrayEntry& Entry);

	/** 같은 키의 최신 Entry로 표시 값을 갱신한다. 값이 바뀐 필드만 통지한다. */
	void Apply(const FSaytEffectTrayEntry& Entry);

	/** 대조 키. 생성 때 정해지고 바뀌지 않으므로 FieldNotify 대상이 아니다. */
	const UClass* GetEffectClass() const;

	FText GetDisplayName() const { return DisplayName; }
	int32 GetCount() const { return Count; }
	ESaytEffectCategory GetCategory() const { return Category; }

private:
	UPROPERTY()
	TSubclassOf<UGameplayEffect> EffectClass;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Category = "Sayt|Tray", meta = (AllowPrivateAccess = "true"))
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Category = "Sayt|Tray", meta = (AllowPrivateAccess = "true"))
	int32 Count = 0;

	UPROPERTY(BlueprintReadOnly, FieldNotify, Getter, Category = "Sayt|Tray", meta = (AllowPrivateAccess = "true"))
	ESaytEffectCategory Category = ESaytEffectCategory::Buff;
};