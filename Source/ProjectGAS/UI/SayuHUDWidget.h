// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/GameplayMessageSubsystem.h"
#include "SayuHUDWidget.generated.h"

struct FSayuDamageMessage;
/**
 * 
 */
UCLASS()
class PROJECTGAS_API USayuHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	// 위젯 생성 직후 1회 호출 - 리스너 등록 + 초기값 동기화 자리
	virtual void NativeOnInitialized() override;
	
	// 위젯이 파괴될 때 호출 - 리스너 해제 필수
	virtual void NativeDestruct() override;
	
	// GMS 콜백. RegisterListener가 기대하는 시그니처: (FGameplayTag, const T&)
	void OnDamageMessage(FGameplayTag Channel, const FSayuDamageMessage& Message);
	
	// 위젯 생성 시점엔 아직 데미지 이벤트가 없었으니, 최초 1회는 직접 ASC를 읽어와야 함
	void SyncInitialHealthFromASC();
	
	// BlueprintReadOnly : 블루프린트에서 이 값을 읽어서 TextBlock에 표시할 거예요
	// C++ 에서는 값을 계산만 하고, 실제 화면 배치는 블루프린트가 담당
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float CurrentHealth;

	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	float MaxHealth;
	
	
private:
	FGameplayMessageListenerHandle DamageListenerHandle;
};
