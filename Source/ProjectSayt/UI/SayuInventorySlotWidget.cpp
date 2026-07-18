// Copyright (c) 2026 Sungmin JI. All Rights Reserved.


#include "SayuInventorySlotWidget.h"

#include "SayuInventoryItemWidget.h"
#include "SayuInventoryWidget.h"
#include "Components/Image.h"
#include "Inventory/SayuInventoryComponent.h"
#include "Items/SayuItemInstance.h"
#include "Items/SayuItemDragDropOperation.h"


void USayuInventorySlotWidget::SetHighlight(ESayuPlacementHighlight NewState) const
{
	switch (NewState)
	{
	case ESayuPlacementHighlight::None:
		HighlightOverlay->SetVisibility(ESlateVisibility::Hidden);
		break;

	case ESayuPlacementHighlight::Valid:
		HighlightOverlay->SetVisibility(ESlateVisibility::HitTestInvisible);
		HighlightOverlay->SetColorAndOpacity(FLinearColor(0.f, 1.f, 0.f, 0.4f));
		break;

	case ESayuPlacementHighlight::Invalid:
		HighlightOverlay->SetVisibility(ESlateVisibility::HitTestInvisible);
		HighlightOverlay->SetColorAndOpacity(FLinearColor(1.f, 0.f, 0.f, 0.4f));
		break;
	}
}

void USayuInventorySlotWidget::NativeOnDragEnter(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragEnter(InGeometry, InDragDropEvent, InOperation);
	
	const USayuItemDragDropOperation* DragOp = Cast<USayuItemDragDropOperation>(InOperation);
	if (!DragOp || !DragOp->DraggedInstance || !OwningInventoryWidget)
	{
		return;
	}

	USayuInventoryComponent* InventoryComponent = OwningInventoryWidget->GetInventoryComponent();
	if (!InventoryComponent)
	{
		return;
	}

	// 의도적 단순화: 지금은 호버 중인 한 칸만 하이라이트. 풋프린트 전체 하이라이트는
	// 다듬기 단계 후보로 남겨둠.
	const bool bCanPlace = InventoryComponent->CanPlaceItemAt(DragOp->DraggedInstance->ItemDefinition, GridPosition, DragOp->DraggedInstance);
	SetHighlight(bCanPlace ? ESayuPlacementHighlight::Valid : ESayuPlacementHighlight::Invalid);
	
	if (USayuInventoryItemWidget* DragVisual = Cast<USayuInventoryItemWidget>(DragOp->DefaultDragVisual))
	{
		DragVisual->SetValidityTint(bCanPlace);
	}
}

void USayuInventorySlotWidget::NativeOnDragLeave(const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
	Super::NativeOnDragLeave(InDragDropEvent, InOperation);
	SetHighlight(ESayuPlacementHighlight::None);
	
	const USayuItemDragDropOperation* DragOp = Cast<USayuItemDragDropOperation>(InOperation);
	if (USayuInventoryItemWidget* DragVisual = DragOp ? Cast<USayuInventoryItemWidget>(DragOp->DefaultDragVisual) : nullptr)
	{
		DragVisual->ResetTint();
	}
}

bool USayuInventorySlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent,
	UDragDropOperation* InOperation)
{
	Super::NativeOnDrop(InGeometry, InDragDropEvent, InOperation);
	SetHighlight(ESayuPlacementHighlight::None);

	USayuItemDragDropOperation* DragOp = Cast<USayuItemDragDropOperation>(InOperation);
	if (!DragOp || !DragOp->DraggedInstance || !OwningInventoryWidget)
	{
		return false;
	}

	OwningInventoryWidget->HandleItemDropped(DragOp->DraggedInstance, GridPosition, DragOp->OriginalTopLeft);
	return true;
}
