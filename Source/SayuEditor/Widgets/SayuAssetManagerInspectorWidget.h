// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"
#include "SayuAssetManagerInspectorWidget.generated.h"

USTRUCT(BlueprintType)
struct FSayuAssetAuditRow
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadOnly, Category = "Sayu|AssetManager")
	FName AssetType;

	UPROPERTY(BlueprintReadOnly, Category = "Sayu|AssetManager")
	FName AssetName;

	UPROPERTY(BlueprintReadOnly, Category = "Sayu|AssetManager")
	TArray<FName> Bundles;

	/** Bundle이 하나도 없으면 true — 태깅 누락 신호 */
	UPROPERTY(BlueprintReadOnly, Category = "Sayu|AssetManager")
	bool bHasNoBundles = false;
};


/**
 * 프로젝트에 등록된 모든 Primary Asset Type/Bundle 현황을 점검하는 범용 EUW.
 * 특정 프로젝트의 클래스(예: Sayu의 아이템 타입)를 하드코딩하지 않고,
 * AssetManager에 "현재 등록된" 모든 타입을 리플렉션으로 그대로 읍는다.
 */
UCLASS()
class SAYUEDITOR_API USayuAssetManagerInspectorWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "Sayu|AssetManager")
	TArray<FSayuAssetAuditRow> GetAssetAuditReport() const;
};
