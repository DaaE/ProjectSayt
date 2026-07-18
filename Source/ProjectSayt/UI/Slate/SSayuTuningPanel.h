// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

// Magnitude는 0~1 범위로 가정합니다.
struct FSayuTuningModel
{
	float Magnitude = 0.5f;
};

// 드래그로 값을 바꾸는 입력 전용 위젯입니다. 자기가 직접 들고 있는 모델이 없고,
// 외부에서 TAttribute로 현재값을 받고, 델리게이트로 변경값을 내보냅니다.
// (SSlider가 실제로 이렇게 만들어져 있어서, 같은 방식을 그대로 따라간 겁니다.)
DECLARE_DELEGATE_OneParam(FOnSayuDragValueChanged, float)

class SSayuDragTrack : public SCompoundWidget
{
	public:
	SLATE_BEGIN_ARGS(SSayuDragTrack)
		: _Value(0.f)
		{}
		SLATE_ATTRIBUTE(float, Value)
		SLATE_EVENT(FOnSayuDragValueChanged, OnValueChanged)
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs);
	
	// Slate가 마우스 입력을 처리하라고 호출해주는 가상 함수들입니다.
	virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseMove(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
	
private:
	void UpdateValueFromMouse(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent);
	FMargin GetHandlePadding() const;
	
	static constexpr float HandleSize = 16.f;

	TAttribute<float> Value;
	FOnSayuDragValueChanged OnValueChanged;
	bool bIsDragging;
};

// 실제로 화면에 띄울 패널입니다. Model을 직접 들고 있고,
// 자식 위젯(ProgressBar/TextBlock)은 전부 그 Model을 TAttribute로 Pull합니다.
class PROJECTSAYT_API SSayuTuningPanel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SSayuTuningPanel) {}
	SLATE_END_ARGS()
	
	void Construct(const FArguments& InArgs);

private:
	void HandleDragValueChanged(float NewValue);
	
	FSayuTuningModel Model;

	// TSharedPtr로 들고 있는 이유: 나중에 값이 바뀔 때 Invalidate()를
	// 직접 호출하려면, 그 위젯을 다시 가리킬 방법이 있어야 합니다.
	TSharedPtr<class SProgressBar> ProgressBarWidget;
	TSharedPtr<class STextBlock> ValueTextWidget;
};
