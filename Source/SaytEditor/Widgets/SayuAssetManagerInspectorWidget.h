// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "SayuAssetManagerInspectorWidget.generated.h"

class USayuAssetAuditRowObject;

/**
 * 프로젝트에 등록된 모든 Primary Asset Type/Bundle 현황을 점검하는 범용 EUW.
 * 특정 프로젝트의 클래스(예: Sayu의 아이템 타입)를 하드코딩하지 않고,
 * AssetManager에 "현재 등록된" 모든 타입을 리플렉션으로 그대로 읍는다.
 */
UCLASS()
class SAYTEDITOR_API USayuAssetManagerInspectorWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Sayu|AssetManager")
	TArray<USayuAssetAuditRowObject*> GetAssetAuditReport();
};
