// Fill out your copyright notice in the Description page of Project Settings.


#include "SayuInventorySlotWidget.h"
#include "Components/Image.h"


void USayuInventorySlotWidget::SetHighlight(ESayuPlacementHighlight NewState)
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