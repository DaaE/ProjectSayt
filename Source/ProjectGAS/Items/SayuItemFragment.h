// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SayuItemFragment.generated.h"

class ASayuCharacterBase;

/**
 * 아이템 정의(USayuItemDefinition)에 배열로 붙는 기능 모듈의 베이스.
 * 합성(composition) 패턴 — 아이템이 가질 수 있는 개별 동작/데이터를
 * 상속이 아니라 여러 Fragment를 조합해서 표현함 (Phase 3 AttributeSet과 같은 원리).
 *
 * 지금은 가상 함수 없는 빈 뼈대. Definition의 배열에 잘 박히는지(인스턴싱 확인)부터
 * 검증하고, 실제 구체 Fragment가 필요해지는 시점에 인터페이스를 그때 끌어낼 예정.
 */
UCLASS(EditInlineNew, DefaultToInstanced)
class PROJECTGAS_API USayuItemFragment : public UObject
{
	GENERATED_BODY()
	
public:
	// 이 아이템이 장착됐을 때 호출됨. 기본 구현은 아무것도 안 함(no-op) —
	// 모든 Fragment가 장착 동작에 반응할 필요는 없으니까 (예: 아이콘 표시용 Fragment는 무시해도 됨).
	// 장착 시 뭔가 해야 하는 Fragment만 이 함수를 오버라이드해서 자기 동작을 채움.
	virtual void OnEquipped(ASayuCharacterBase* OwningCharacter) const {}
};
