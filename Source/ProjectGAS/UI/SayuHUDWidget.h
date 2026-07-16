// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SayuHUDWidget.generated.h"

class UNativeWidgetHost;
class SSayuHealthBar;

UCLASS()
class PROJECTGAS_API USayuHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	// 위젯 생성 직후 1회 호출 - 리스너 등록 + 초기값 동기화 자리
	virtual void NativeOnInitialized() override;
	
	// 위젯이 파괴될 때 호출 - 리스너 해제 필수
	virtual void NativeDestruct() override;
	
	// WBP_HUD 디자이너에서 동명(HealthBarHost) 위젯이 자동 주입됨 (BindWidget 계약)
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UNativeWidgetHost> HealthBarHost;
	
private:
	// UObject(HUD 위젯)가 Slate 위젯을 '소유'하는 표준 형태 — TSharedPtr 강참조
	TSharedPtr<SSayuHealthBar> HealthBar;
};
