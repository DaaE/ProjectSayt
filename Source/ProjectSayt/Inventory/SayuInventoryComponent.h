// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SayuInventoryComponent.generated.h"

class USayuItemInstance;
class USayuItemDefinition;

// 그리드에 배치된 아이템 한 개를 나타냄.
USTRUCT(BlueprintType)
struct FSayuInventoryEntry
{
	GENERATED_BODY()
	
	UPROPERTY()
	TObjectPtr<USayuItemInstance> Instance = nullptr;
	
	// 차지하는 영역의 좌상단 좌표 (그리드 셀 단위).
	UPROPERTY()
	FIntPoint TopLeft = FIntPoint::ZeroValue;
};


/**
 * 그리드 인벤토리의 점유 판정/추가/제거 로직(Model)을 담당.
 * UI(Common UI)는 이 컴포넌트를 들여다보고 함수를 호출만 함.
 */
UCLASS()
class PROJECTSAYT_API USayuInventoryComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	USayuInventoryComponent();

	// 지정 위치에 놓을 수 있는지 판정만 함 (실제로 놓지 않음).
	// 풋프린트 전체가 비어있거나, 동일한 스택 가능 아이템과 정확히 겹치면 true.
	bool CanPlaceItemAt(USayuItemDefinition* ItemDef, FIntPoint TopLeft, USayuItemInstance* IgnoreInstance = nullptr) const;

	// 지정 위치에 Instance 배치 시도 (드래그&드롭처럼 좌표가 확정된 경우용).
	// 호환되는 기존 스택과 정확히 겹치면 병합, 비어있으면 새로 배치.
	bool TryAddInstanceAt(USayuItemInstance* Instance, FIntPoint TopLeft);
	
	// 빈 자리를 좌상단부터 자동으로 찾아서 추가 시도.
	// 이미 존재하는 Instance를 추가 시도. 같은 종류의 스택 가능한 슬롯이 있으면
	// 먼저 그쪽에 (부분적으로라도) 합치고, 남는 수량은 빈 자리에 새로 배치.
	bool TryAddInstance(USayuItemInstance* Instance);

	void RemoveItem(USayuItemInstance* Instance);
	
	int32 GetGridWidth() const { return GridWidth; }
	int32 GetGridHeight() const { return GridHeight; }

	// UI가 그리드를 그릴 때 순회할 대상. 읽기 전용 참조로 반환 — 호출하는 쪽이
	// 이 배열을 직접 수정할 수 없게(const&), 그리고 복사 비용 없이 넘겨줌.
	const TArray<FSayuInventoryEntry>& GetEntries() const { return Entries; }

	// 그리드 크기를 확장 (가방 업그레이드 등 외부 신호로 호출됨).
	// 확장만 지원 — 축소는 기존 배치된 아이템이 경계 밖으로 밀려날 수 있어 허용하지 않음.
	bool ExpandGrid(int32 NewWidth, int32 NewHeight);
	
	// 디버그용 — 그리드 점유 상태를 Output Log에 텍스트로 출력.
	void DebugPrintGrid() const;
	

protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, Category = "Sayu|Inventory")
	int32 GridWidth = 10;

	UPROPERTY(EditDefaultsOnly, Category = "Sayu|Inventory")
	int32 GridHeight = 6;

private:
	// 풋프린트 안의 점유 상태를 읽기만 함 (읽기 전용 헬퍼라 const).
	// 풋프린트 전체가 일관되면(전부 비었거나 전부 같은 인스턴스) true를 반환하고
	// OutOccupant에 그 결과를 채움(nullptr = 비어있음). 섞여 있으면 false.
	bool GetUniformFootprintOccupant(FIntPoint TopLeft, FIntPoint Size, USayuItemInstance*& OutOccupant, USayuItemInstance* IgnoreInstance = nullptr) const;
	
	// 특정 칸의 "사실상" 점유자. IgnoreInstance와 같으면 빈 칸(nullptr)인 것처럼 취급 —
	// 드래그 호버 판정에서 "자기 자신이 차지한 칸"을 무시하기 위한 용도.
	USayuItemInstance* GetEffectiveOccupant(int32 X, int32 Y, USayuItemInstance* IgnoreInstance) const;
	
	// 각 셀을 점유 중인 인스턴스. nullptr이면 빈 칸. 크기 = GridWidth * GridHeight.
	// (Entries가 이미 UPROPERTY로 소유권을 갖고 있어서, 이 배열은 GC 추적이
	// 따로 필요 없는 "빠른 조회용 캐시"일 뿐임.)
	TArray<TObjectPtr<USayuItemInstance>> CellOccupancy;

	UPROPERTY()
	TArray<FSayuInventoryEntry> Entries;
};
