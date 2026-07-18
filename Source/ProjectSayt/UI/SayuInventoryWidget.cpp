// Copyright (c) 2026 Sungmin JI. All Rights Reserved.


#include "SayuInventoryWidget.h"

#include "SayuInventoryItemWidget.h"
#include "SayuInventorySlotWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/UniformGridPanel.h"
#include "Inventory/SayuInventoryComponent.h"
#include "Items/SayuItemDefinition.h"
#include "Items/SayuItemInstance.h"
#include "Components/SizeBox.h"
#include "Components/UniformGridSlot.h"

void USayuInventoryWidget::SetInventoryComponent(USayuInventoryComponent* InInventoryComponent)
{
	InventoryComponent = InInventoryComponent;
	BuildGrid();
	RefreshItems();
}

void USayuInventoryWidget::BuildGrid()
{
	if (!GridPanel || !InventoryComponent || !SlotWidgetClass)
	{
		return;
	}

	GridPanel->ClearChildren();
	GridPanel->SetMinDesiredSlotWidth(CellSize);
	GridPanel->SetMinDesiredSlotHeight(CellSize);

	const int32 Width = InventoryComponent->GetGridWidth();
	const int32 Height = InventoryComponent->GetGridHeight();
	SlotWidgets.SetNum(Width * Height);
	
	// 루트는 항상 화면 전체를 채우니, 그 안의 실제 그리드 영역만 고정 픽셀
	// 크기로 못박아둠 — UniformGridPanel이 화면 크기에 맞춰 늘어나는 걸 방지.
	if (SizeContainer)
	{
		SizeContainer->SetWidthOverride(Width * CellSize);
		SizeContainer->SetHeightOverride(Height * CellSize);
	}

	for (int32 Y = 0; Y < Height; ++Y)
	{
		for (int32 X = 0; X < Width; ++X)
		{
			USayuInventorySlotWidget* NewSlot = CreateWidget<USayuInventorySlotWidget>(this, SlotWidgetClass);
			if (!NewSlot)
			{
				continue;
			}

			if (UUniformGridSlot* GridSlot = GridPanel->AddChildToUniformGrid(NewSlot, Y, X))
			{
				// UniformGridSlot의 기본 정렬은 "내용 크기에 맞춤"이라, Stretch Anchor만 쓰는
				// SlotWidget 내부 구조에서는 부모 크기를 못 받아 거의 0 크기로 쪼그라듦.
				// 명시적으로 칸 전체를 채우도록 지정해야 함.
				GridSlot->SetHorizontalAlignment(HAlign_Fill);
				GridSlot->SetVerticalAlignment(VAlign_Fill);
			}
			
			NewSlot->SetGridPosition(FIntPoint(X, Y));
			NewSlot->SetOwningInventoryWidget(this);

			SlotWidgets[Y * Width + X] = NewSlot;
		}
	}
}

void USayuInventoryWidget::RefreshItems()
{
	if (!ItemCanvas || !InventoryComponent || !ItemWidgetClass)
	{
		return;
	}

	ItemCanvas->ClearChildren();

	for (const FSayuInventoryEntry& Entry : InventoryComponent->GetEntries())
	{
		if (!Entry.Instance || !Entry.Instance->ItemDefinition)
		{
			continue;
		}

		USayuInventoryItemWidget* NewItemWidget = CreateWidget<USayuInventoryItemWidget>(this, ItemWidgetClass);
		if (!NewItemWidget)
		{
			continue;
		}

		NewItemWidget->SetItemInstance(Entry.Instance);
		NewItemWidget->SetGridPosition(Entry.TopLeft);

		if (UCanvasPanelSlot* CanvasSlot = ItemCanvas->AddChildToCanvas(NewItemWidget))
		{
			const FIntPoint Size = Entry.Instance->ItemDefinition->GridSize;
			CanvasSlot->SetPosition(FVector2D(Entry.TopLeft.X * CellSize, Entry.TopLeft.Y * CellSize));
			CanvasSlot->SetSize(FVector2D(Size.X * CellSize, Size.Y * CellSize));
		}
	}
}

void USayuInventoryWidget::HandleItemDropped(USayuItemInstance* Instance, FIntPoint NewTopLeft, FIntPoint OriginalTopLeft)
{
	if (!Instance || !InventoryComponent)
	{
		return;
	}

	USayuItemDefinition* ItemDef = Instance->ItemDefinition;

	// 일단 원래 자리에서 빼고 검사 — 자기 자신의 옛 칸이 "이미 점유 중"으로 잡혀서
	// 근처로 이동하는 것 자체가 막히는 걸 방지.
	InventoryComponent->RemoveItem(Instance);

	if (InventoryComponent->CanPlaceItemAt(ItemDef, NewTopLeft, nullptr))
	{
		InventoryComponent->TryAddInstanceAt(Instance, NewTopLeft);
	}
	else
	{
		// 배치 불가 — 원래 자리로 롤백.
		InventoryComponent->TryAddInstanceAt(Instance, OriginalTopLeft);
	}

	RefreshItems();
}