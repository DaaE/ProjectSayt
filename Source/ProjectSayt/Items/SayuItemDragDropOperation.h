// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/DragDropOperation.h"
#include "SayuItemDragDropOperation.generated.h"

class USayuItemInstance;

/**
 * 인벤토리 아이템을 드래그하는 동안 한시적으로 살아있는 데이터 컨테이너.
 * UDragDropOperation 자체가 이미 드래그 아이콘 위젯(DefaultDragVisual) 등을
 * 내장하고 있어서, 우리는 거기에 "어떤 아이템인지" + "회전 상태"만 얹음.
 */
UCLASS()
class PROJECTSAYT_API USayuItemDragDropOperation : public UDragDropOperation
{
	GENERATED_BODY()
	
public:
	// 지금 드래그 중인 아이템.
	UPROPERTY(BlueprintReadWrite)
	TObjectPtr<USayuItemInstance> DraggedInstance;

	// 드래그 중 회전된 상태인지. true면 GridSize의 가로/세로를 바꿔서 취급.
	UPROPERTY(BlueprintReadWrite)
	bool bIsRotated = false;
	
	UPROPERTY(BlueprintReadWrite)
	FIntPoint OriginalTopLeft = FIntPoint::ZeroValue;
};
