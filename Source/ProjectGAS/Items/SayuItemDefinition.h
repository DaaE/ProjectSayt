// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SayuItemDefinition.generated.h"

class USayuItemFragment;

/**
 * 아이템 하나를 정의하는 데이터 에셋.
 * "이름"과 "기능 모듈(Fragment) 배열"만 들고 있는 최소 구조로 시작.
 * 아이콘, ID 등은 실제로 필요해지는 단계(UI/비동기 로딩 작업)에서 추가할 예정.
 */
UCLASS()
class PROJECTGAS_API USayuItemDefinition : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	// 플레이어에게 보여줄 이름.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	FText ItemName;

	// 이 아이템이 가지는 기능 모듈들.
	UPROPERTY(EditDefaultsOnly, Instanced)
	TArray<TObjectPtr<USayuItemFragment>> Fragments;
	
	// 그리드에서 차지하는 칸 수. X = 가로, Y = 세로.
	UPROPERTY(EditDefaultsOnly, Category = "Sayu|Item")
	FIntPoint GridSize = FIntPoint(1, 1);
	
	// 같은 종류끼리 한 칸에 겹쳐 쌓일 수 있는지.
	UPROPERTY(EditDefaultsOnly, Category = "Sayu|Item")
	bool bIsStackable = false;

	// 한 칸에 최대 몇 개까지 쌓일 수 있는지 (bIsStackable이 false면 의미 없음).
	UPROPERTY(EditDefaultsOnly, Category = "Sayu|Item", meta = (EditCondition = "bIsStackable"))
	int32 MaxStackSize = 1;
	// meta = (EditCondition = "bIsStackable")
	// bIsStackable 체크박스가 꺼져 있으면 MaxStackSize 필드를 에디터에서 회색으로 비활성화
	// "스택 안 되는 아이템인데 MaxStackSize를 의미 없이 만질 수 있는" 혼란을 막아줌
};
