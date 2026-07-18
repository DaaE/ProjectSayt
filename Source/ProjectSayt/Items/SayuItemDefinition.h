// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "SayuItemDefinition.generated.h"

class USayuItemFragment;
class UTexture2D;

/**
 * 아이템 하나를 정의하는 데이터 에셋.
 * "이름"과 "기능 모듈(Fragment) 배열"만 들고 있는 최소 구조로 시작.
 * 아이콘, ID 등은 실제로 필요해지는 단계(UI/비동기 로딩 작업)에서 추가할 예정.
 */
UCLASS()
class PROJECTSAYT_API USayuItemDefinition : public UPrimaryDataAsset
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
	
	// 인벤토리에 표시할 아이콘. TSoftObjectPtr라 디스크에 있는지만 가벼운 메타데이터로
	// 들고 있고, 실제 텍스처 데이터는 필요한 시점에 로딩됨.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Item", meta = (AssetBundles = "UI"))
	TSoftObjectPtr<UTexture2D> Icon;
	
	// 같은 종류끼리 한 칸에 겹쳐 쌓일 수 있는지.
	UPROPERTY(EditDefaultsOnly, Category = "Sayu|Item")
	bool bIsStackable = false;

	// 한 칸에 최대 몇 개까지 쌓일 수 있는지 (bIsStackable이 false면 의미 없음).
	UPROPERTY(EditDefaultsOnly, Category = "Sayu|Item", meta = (EditCondition = "bIsStackable"))
	int32 MaxStackSize = 1;
	// meta = (EditCondition = "bIsStackable")
	// bIsStackable 체크박스가 꺼져 있으면 MaxStackSize 필드를 에디터에서 회색으로 비활성화
	// "스택 안 되는 아이템인데 MaxStackSize를 의미 없이 만질 수 있는" 혼란을 막아줌
	
	// Primary Asset Type 이름을 클래스 이름(SayuItemDefinition)이 아니라
	// 안정적인 게임 데이터 식별자("Item")로 직접 고정.
	// 클래스 이름이 나중에 바뀌어도(과거 ProjectGAS→Sayu 리네임 사례처럼) 영향받지 않음.
	virtual FPrimaryAssetId GetPrimaryAssetId() const override
	{
		return FPrimaryAssetId(TEXT("Item"), GetFName());
	}
};
