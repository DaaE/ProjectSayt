// Fill out your copyright notice in the Description page of Project Settings.


#include "SayuInventoryComponent.h"

#include "SayuLogChannels.h"
#include "Items/SayuItemDefinition.h"
#include "Items/SayuItemIdSubsystem.h"
#include "Items/SayuItemInstance.h"


// Sets default values for this component's properties
USayuInventoryComponent::USayuInventoryComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

// Called when the game starts
void USayuInventoryComponent::BeginPlay()
{
	Super::BeginPlay();

	CellOccupancy.Init(nullptr, GridWidth * GridHeight);
	Entries.Reserve(GridWidth * GridHeight);
}

bool USayuInventoryComponent::GetUniformFootprintOccupant(FIntPoint TopLeft, FIntPoint Size, USayuItemInstance*& OutOccupant, USayuItemInstance* IgnoreInstance) const
{
	OutOccupant = GetEffectiveOccupant(TopLeft.X, TopLeft.Y, IgnoreInstance);

	for (int32 Y = TopLeft.Y; Y < TopLeft.Y + Size.Y; ++Y)
	{
		for (int32 X = TopLeft.X; X < TopLeft.X + Size.X; ++X)
		{
			if (GetEffectiveOccupant(X, Y, IgnoreInstance) != OutOccupant)
			{
				return false; // 풋프린트 안에 섞인 점유 상태 존재
			}
		}
	}

	return true;
}

USayuItemInstance* USayuInventoryComponent::GetEffectiveOccupant(int32 X, int32 Y, USayuItemInstance* IgnoreInstance) const
{
	USayuItemInstance* Cell = CellOccupancy[Y * GridWidth + X];
	return (Cell == IgnoreInstance) ? nullptr : Cell;
}

bool USayuInventoryComponent::CanPlaceItemAt(USayuItemDefinition* ItemDef, FIntPoint TopLeft, USayuItemInstance* IgnoreInstance) const
{
	if (!ItemDef)
	{
		return false;
	}

	const FIntPoint Size = ItemDef->GridSize;

	if (TopLeft.X < 0 || TopLeft.Y < 0 ||
		TopLeft.X + Size.X > GridWidth ||
		TopLeft.Y + Size.Y > GridHeight)
	{
		return false; // 그리드 경계를 벗어남
	}
	
	USayuItemInstance* Occupant = nullptr;
	if (!GetUniformFootprintOccupant(TopLeft, Size, Occupant, IgnoreInstance))
	{
		return false; // 풋프린트 안에 점유 상태가 섞여 있음
	}
	
	if (Occupant == nullptr)
	{
		return true; // 완전히 비어있음 → 새로 배치 가능
	}

	// 완전히 같은 기존 아이템으로 차있음 → 병합 가능한지 확인
	return ItemDef->bIsStackable
		&& Occupant->ItemDefinition == ItemDef
		&& Occupant->StackCount < ItemDef->MaxStackSize;
}

bool USayuInventoryComponent::TryAddInstanceAt(USayuItemInstance* Instance, FIntPoint TopLeft)
{
	if (!Instance || !Instance->ItemDefinition)
	{
		return false;
	}
	
	USayuItemDefinition* ItemDef = Instance->ItemDefinition;
	const FIntPoint Size = ItemDef->GridSize;
	
	if (TopLeft.X < 0 || TopLeft.Y < 0 ||
		TopLeft.X + Size.X > GridWidth ||
		TopLeft.Y + Size.Y > GridHeight)
	{
		return false;
	}
	
	USayuItemInstance* Occupant = nullptr;
	if (!GetUniformFootprintOccupant(TopLeft, Size, Occupant))
	{
		return false;
	}
	
	if (Occupant != nullptr)
	{
		if (!ItemDef->bIsStackable || Occupant->ItemDefinition != ItemDef ||
			Occupant->StackCount >= ItemDef->MaxStackSize)
		{
			return false;
		}

		// CellOccupancy와 Entries가 항상 같이 갱신된다는 전제(불변식)가 맞는지
		// 확인하는 방어적 체크 — 이론상 항상 찾아져야 정상.
		const int32 EntryIndex = Entries.IndexOfByPredicate(
			[Occupant](const FSayuInventoryEntry& Entry) { return Entry.Instance == Occupant; });

		if (EntryIndex == INDEX_NONE)
		{
			return false;
		}

		const int32 Room = ItemDef->MaxStackSize - Occupant->StackCount;
		const int32 Transfer = FMath::Min(Room, Instance->StackCount);

		Occupant->StackCount += Transfer;     // 기존 슬롯: 4 → 5
		Instance->StackCount -= Transfer;     // 들고 온 것: 3 → 2

		return true;
	}
	
	// 빈 자리 → 새 엔트리로 배치
	FSayuInventoryEntry NewEntry;
	NewEntry.Instance = Instance;
	NewEntry.TopLeft = TopLeft;
	Entries.Add(NewEntry);

	for (int32 Y = TopLeft.Y; Y < TopLeft.Y + Size.Y; ++Y)
	{
		for (int32 X = TopLeft.X; X < TopLeft.X + Size.X; ++X)
		{
			CellOccupancy[Y * GridWidth + X] = Instance;
		}
	}

	return true;
}

bool USayuInventoryComponent::TryAddInstance(USayuItemInstance* Instance)
{
	if (!Instance || !Instance->ItemDefinition)
	{
		return false;
	}
	
	USayuItemDefinition* ItemDef = Instance->ItemDefinition;
	
	// 1단계 — 스택 가능한 아이템이면, 인벤토리 전체에서 합칠 수 있는 기존 슬롯부터 찾음.
	// (TryAddInstanceAt의 병합은 "정확히 그 좌표"에 한정되지만, 여긴 "어디든" 찾음.)
	if (ItemDef->bIsStackable)
	{
		for (FSayuInventoryEntry& Entry : Entries)
		{
			if (Instance->StackCount <= 0)
			{
				break; // 들고 온 수량을 다 합쳤으면 더 볼 필요 없음
			}
			
			if (Entry.Instance && Entry.Instance != Instance &&
				Entry.Instance->ItemDefinition == ItemDef &&
				Entry.Instance->StackCount < ItemDef->MaxStackSize)
			{
				const int32 Room = ItemDef->MaxStackSize - Entry.Instance->StackCount;
				const int32 Transfer = FMath::Min(Room, Instance->StackCount);

				Entry.Instance->StackCount += Transfer; // 기존 슬롯: 4 → 5
				Instance->StackCount -= Transfer;        // 들고 온 것: 3 → 2
			}
		}
		
		if (Instance->StackCount <= 0)
		{
			return true; // 전부 기존 스택에 합쳐짐. 이 Instance 자체는 더 이상 필요 없음(호출자가 폐기).
		}
	}
	
	// 합치고 남은 수량(또는 원래 스택 불가 아이템)은 빈 자리에 새로 배치.
	for (int32 Y = 0; Y <= GridHeight - ItemDef->GridSize.Y; ++Y)
	{
		for (int32 X = 0; X <= GridWidth - ItemDef->GridSize.X; ++X)
		{
			if (TryAddInstanceAt(Instance, FIntPoint(X, Y)))
			{
				return true;
			}
		}
	}

	return false; // 자리 없음. Instance->StackCount엔 아직 못 넣은 수량이 남아있음.
}

void USayuInventoryComponent::RemoveItem(USayuItemInstance* Instance)
{
	const int32 EntryIndex = Entries.IndexOfByPredicate(
		[Instance](const FSayuInventoryEntry& Entry) { return Entry.Instance == Instance; });

	if (EntryIndex == INDEX_NONE || !Instance->ItemDefinition)
	{
		return;
	}

	const FIntPoint TopLeft = Entries[EntryIndex].TopLeft;
	const FIntPoint Size = Instance->ItemDefinition->GridSize;

	for (int32 Y = TopLeft.Y; Y < TopLeft.Y + Size.Y; ++Y)
	{
		for (int32 X = TopLeft.X; X < TopLeft.X + Size.X; ++X)
		{
			CellOccupancy[Y * GridWidth + X] = nullptr;
		}
	}

	Entries.RemoveAt(EntryIndex);
}

bool USayuInventoryComponent::ExpandGrid(int32 NewWidth, int32 NewHeight)
{
	if (NewWidth < GridWidth || NewHeight < GridHeight)
	{
		return false; // 축소 시도는 거부
	}

	GridWidth = NewWidth;
	GridHeight = NewHeight;

	// 평탄화 공식이 GridWidth에 의존하므로, 점유 데이터를 새 크기 기준으로
	// 전부 다시 채워야 함 — 기존 Entries의 TopLeft/Size 자체는 그대로 유효함
	// (확장만 허용했기 때문에 범위를 벗어날 일이 없음).
	CellOccupancy.Init(nullptr, GridWidth * GridHeight);

	for (const FSayuInventoryEntry& Entry : Entries)
	{
		const FIntPoint Size = Entry.Instance->ItemDefinition->GridSize;
		for (int32 Y = Entry.TopLeft.Y; Y < Entry.TopLeft.Y + Size.Y; ++Y)
		{
			for (int32 X = Entry.TopLeft.X; X < Entry.TopLeft.X + Size.X; ++X)
			{
				CellOccupancy[Y * GridWidth + X] = Entry.Instance;
			}
		}
	}

	// 새 최대 칸 수만큼 Entries 용량도 다시 확보 — 이 호출 자체는 재할당을
	// 일으킬 수 있지만, 게임플레이 루프 중이 아니라 "확장 이벤트"라는 명확히
	// 통제된 시점에만 일어나므로 댕글링 위험이 없음.
	Entries.Reserve(GridWidth * GridHeight);

	return true;
}

void USayuInventoryComponent::DebugPrintGrid() const
{
	FString GridString = TEXT("\n");
	for (int32 Y = 0; Y < GridHeight; ++Y)
	{
		FString Row;
		for (int32 X = 0; X < GridWidth; ++X)
		{
			Row += CellOccupancy[Y * GridWidth + X] ? FString::Printf(TEXT("[%d]"), CellOccupancy[Y * GridWidth + X]->StackCount) : TEXT("[ ]");
		}
		GridString += Row + TEXT("\n");
	}

	UE_LOG(LogSayuData, Verbose, TEXT("%s"), *GridString);
}



