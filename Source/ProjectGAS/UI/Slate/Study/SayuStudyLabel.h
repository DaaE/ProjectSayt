// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * 
 */
class PROJECTGAS_API SayuStudyLabel : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SayuStudyLabel){}
	SLATE_END_ARGS()

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);
	// 주목: 그리기 관련 함수가 하나도 없음. Compound는 조립만 하면 그리기는 공짜.
};
