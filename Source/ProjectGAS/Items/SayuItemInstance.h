// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SayuItemInstance.generated.h"

class USayuItemDefinition;

/**
 * 
 */
UCLASS()
class PROJECTGAS_API USayuItemInstance : public UObject
{
	GENERATED_BODY()
	
public:
	// Definition으로부터 새 Instance를 생성하고 ID까지 발급해서 돌려주는 팩토리 메서드.
	// NewObject 호출 + ID 발급이라는 두 단계 절차를 캡슐화함 — 호출하는 쪽은 절차를 몰라도 됨.
	// Outer는 GC 소유 관계상 부모로 쓰일 객체(보통 호출하는 Actor/Component 자신).
	static USayuItemInstance* CreateInstance(UObject* Outer, USayuItemDefinition* Definition);
	
	// 이 인스턴스가 "어떤 종류"의 아이템인지 가리킴.
	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<USayuItemDefinition> ItemDefinition;

	// 이 인스턴스의 고유 식별자. USayuItemIdSubsystem이 발급.
	// 같은 ItemDefinition을 가리키는 인스턴스가 여러 개 있어도, 이 값으로 서로 구분됨.
	// 기본값은 INDEX_NONE(-1) — 0은 "정상적으로 발급된 첫 번째 ID"라서 구분이 안 되니,
	// "아직 발급 안 됨"을 나타내는 별도의 값이 필요해서 언리얼이 관례적으로 쓰는 INDEX_NONE을 씀.
	UPROPERTY(BlueprintReadOnly)
	int32 ItemInstanceId = INDEX_NONE;
	
	// 같은 슬롯에 겹쳐 쌓인 개수. 예: 포션 1개면 1, 3개 겹쳐 쌓이면 3.
	UPROPERTY(BlueprintReadOnly)
	int32 StackCount = 1;
};
