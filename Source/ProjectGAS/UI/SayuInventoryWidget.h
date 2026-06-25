// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "SayuInventoryWidget.generated.h"

class USizeBox;
class UCanvasPanel;
class UUniformGridPanel;
class USayuInventoryItemWidget;
class USayuInventorySlotWidget;
class USayuInventoryComponent;


/**
 * 
 */
UCLASS()
class PROJECTGAS_API USayuInventoryWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	void SetInventoryComponent(USayuInventoryComponent* InInventoryComponent);
	
protected:
	// 한 칸의 픽셀 크기. 배경 그리드(UniformGridPanel)와 아이콘 오버레이(CanvasPanel)
	// 좌표 계산이 반드시 이 값을 같이 써야 정렬이 맞음.
	UPROPERTY(EditDefaultsOnly, Category = "Sayu|Inventory")
	float CellSize = 64.f;

	UPROPERTY(EditDefaultsOnly, Category = "Sayu|Inventory")
	TSubclassOf<USayuInventorySlotWidget> SlotWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "Sayu|Inventory")
	TSubclassOf<USayuInventoryItemWidget> ItemWidgetClass;

	//grid의 크기를 정함.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<USizeBox> SizeContainer;
	
	// 디자이너에서 두 레이어가 정확히 겹치게 배치(Overlay 등으로) 필요.
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UUniformGridPanel> GridPanel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> ItemCanvas;

private:
	void BuildGrid();
	void RefreshItems();

	UPROPERTY()
	TObjectPtr<USayuInventoryComponent> InventoryComponent;

	// CellOccupancy랑 같은 평탄화 공식(Y*Width+X) — 나중에 드래그 하이라이트 갱신할 때
	// 좌표로 바로 해당 SlotWidget을 찾기 위한 캐시.
	TArray<TObjectPtr<USayuInventorySlotWidget>> SlotWidgets;
};
