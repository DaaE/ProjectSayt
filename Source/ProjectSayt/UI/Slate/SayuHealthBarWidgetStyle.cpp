// Copyright (c) 2026 Sungmin JI. All Rights Reserved.


#include "SayuHealthBarWidgetStyle.h"

const FName FSayuHealthBarStyle::TypeName(TEXT("FSayuHealthBarStyle"));

FSayuHealthBarStyle::FSayuHealthBarStyle()
{
}

FSayuHealthBarStyle::~FSayuHealthBarStyle()
{
}

const FSayuHealthBarStyle& FSayuHealthBarStyle::GetDefault()
{
	static FSayuHealthBarStyle Default;
	return Default;
}

void FSayuHealthBarStyle::GetResources(TArray<const FSlateBrush*>& OutBrushes) const
{
	// Add any brush resources here so that Slate can correctly atlas and reference them
	OutBrushes.Add(&BackgroundBrush);
	OutBrushes.Add(&FillBrush);
	OutBrushes.Add(&GhostBrush);
	OutBrushes.Add(&FrameBrush);
	OutBrushes.Add(&HighlightBrush);
}
