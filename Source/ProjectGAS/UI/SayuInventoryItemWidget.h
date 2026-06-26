// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "SayuInventoryItemWidget.generated.h"

class UTextBlock;
class UImage;
class USayuItemInstance;
struct FStreamableHandle;

/**
 * 아이템 하나(=Entry 하나)를 표현하는 위젯. 그리드 배경(SlotWidget)과 별도 레이어에
 * 절대 좌표로 배치됨 — 여러 칸을 차지하는 아이템도 큰 아이콘 하나로 자연스럽게 표현.
 */
UCLASS()
class PROJECTGAS_API USayuInventoryItemWidget : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	void SetItemInstance(USayuItemInstance* Instance);
	void SetGridPosition(FIntPoint InGridPosition) { CurrentTopLeft = InGridPosition; }
	void SetValidityTint(bool bValid) const;
	void ResetTint() const;
	
protected:
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UImage> IconImage;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> StackText;
	
	// 이 위젯이 지금 표현 중인 아이템. SetItemInstance() 호출 시점 값을 들고 있어야
	// 나중에 NativeOnDragDetected에서 "뭘 드래그하는지" 알 수 있음 (기존엔 저장 안 하고 있었음).
	UPROPERTY()
	TObjectPtr<USayuItemInstance> CurrentInstance;
	
private:
	FIntPoint CurrentTopLeft = FIntPoint::ZeroValue;
	
	void OnIconLoaded();
	
	// 이 핸들이 살아있는 동안만 로드된 텍스처가 메모리에 유지됨
	TSharedPtr<FStreamableHandle> IconStreamableHandle;
};
