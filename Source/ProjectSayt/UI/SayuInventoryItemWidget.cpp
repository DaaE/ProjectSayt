// Copyright (c) 2026 Sungmin JI. All Rights Reserved.


#include "SayuInventoryItemWidget.h"
#include "Components/Image.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Engine/AssetManager.h"
#include "Items/SayuItemDefinition.h"
#include "Items/SayuItemDragDropOperation.h"
#include "Items/SayuItemInstance.h"


void USayuInventoryItemWidget::SetItemInstance(USayuItemInstance* Instance)
{
	if (!Instance || !Instance->ItemDefinition)
	{
		return;
	}
	
	CurrentInstance = Instance;
	
	const USayuItemDefinition* ItemDef = Instance->ItemDefinition;
	
	// 아이콘 위에 마우스 올렸을 때 아이템 이름 표시.
	// UUserWidget(UCommonUserWidget의 부모)에 이미 내장된 함수라 별도 include 불필요.
	SetToolTipText(ItemDef->ItemName);
	
	// 임시 동기 로딩 — 다음 단계(비동기 로딩 정식 구현)에서 FStreamableManager 기반으로
	// 교체할 예정. 지금은 "레이아웃이 의도대로 동작하는지"만 빠르게 확인하는 목적.
	// if (UTexture2D* LoadedIcon = ItemDef->Icon.LoadSynchronous())
	// {
	// 	IconImage->SetBrushFromTexture(LoadedIcon);
	// }
	
	FStreamableManager& Streamable = UAssetManager::GetStreamableManager();
	IconStreamableHandle = Streamable.RequestAsyncLoad(
		ItemDef->Icon.ToSoftObjectPath(), FStreamableDelegate::CreateUObject(this, &USayuInventoryItemWidget::OnIconLoaded));
	
	// 스택 1개면 숫자 숨기고, 2개 이상이면 표시.
	if (Instance->StackCount > 1)
	{
		StackText->SetText(FText::AsNumber(Instance->StackCount));
		StackText->SetVisibility(ESlateVisibility::HitTestInvisible);
	}
	else
	{
		StackText->SetVisibility(ESlateVisibility::Hidden);
	}
}

void USayuInventoryItemWidget::SetValidityTint(bool bValid) const
{
	const FLinearColor TintColor = bValid ? FLinearColor(0.4f, 1.f, 0.4f) : FLinearColor(1.f, 0.4f, 0.4f);
	IconImage->SetColorAndOpacity(TintColor);
}

void USayuInventoryItemWidget::ResetTint() const
{
	IconImage->SetColorAndOpacity(FLinearColor::White);
}

FReply USayuInventoryItemWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	// 좌클릭일 때만 드래그 감지를 등록 — 등록 안 하면 NativeOnDragDetected는 절대 안 불림.
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		return FReply::Handled().DetectDrag(TakeWidget(), EKeys::LeftMouseButton);
	}
	
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

void USayuInventoryItemWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent,
	UDragDropOperation*& OutOperation)
{
	Super::NativeOnDragDetected(InGeometry, InMouseEvent, OutOperation);
	
	if (!CurrentInstance)
	{
		return;
	}

	USayuItemDragDropOperation* DragOperation = NewObject<USayuItemDragDropOperation>();
	DragOperation->DraggedInstance = CurrentInstance;
	DragOperation->bIsRotated = false;
	DragOperation->OriginalTopLeft = CurrentTopLeft;

	// 마우스를 따라다닐 미리보기 위젯 — 원래 슬롯의 위젯 자체를 끌고 가면
	// 그리드에서 사라지는 부작용이 있어서, 같은 클래스로 새 인스턴스를 하나 더 만들어 씀.
	USayuInventoryItemWidget* DragVisual = CreateWidget<USayuInventoryItemWidget>(GetOwningPlayer(), GetClass());
	DragVisual->SetItemInstance(CurrentInstance);
	
	// 드래그 미리보기는 그리드 슬롯 밖이라 크기 기준이 없음 — 지금 이 위젯(원본)이
	// 실제로 차지하고 있는 크기를 그대로 가져와 SizeBox로 못박음.
	const FVector2D OriginalSize = InGeometry.GetLocalSize();
	USizeBox* SizeWrapper = NewObject<USizeBox>(this);
	SizeWrapper->SetWidthOverride(OriginalSize.X);
	SizeWrapper->SetHeightOverride(OriginalSize.Y);
	SizeWrapper->AddChild(DragVisual);
	
	DragOperation->DefaultDragVisual = SizeWrapper;
	DragOperation->Pivot = EDragPivot::MouseDown;

	OutOperation = DragOperation;
}

void USayuInventoryItemWidget::OnIconLoaded()
{
	if (CurrentInstance && CurrentInstance->ItemDefinition)
	{
		if (UTexture2D* LoadedIcon = CurrentInstance->ItemDefinition->Icon.Get())
		{
			IconImage->SetBrushFromTexture(LoadedIcon);
		}
	}
}
