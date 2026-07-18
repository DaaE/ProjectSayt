// Copyright (c) 2026 Sungmin JI. All Rights Reserved.

#pragma once
// #pragma once : 이 헤더가 여러 번 include 되어도 한 번만 처리하라는 지시어
// C#에는 이런 게 필요 없었는데, C++은 헤더를 여러 파일에서 include하다보면
// 중복 선언 오류가 날 수 있어서 반드시 필요함

#include "CoreMinimal.h"
#include "SayuCharacterBase.h"
#include "SayuCharacter.generated.h"
// UE 리플렉션 시스템이 자동 생성하는 코드를 포함
// 반드시 마지막 #include 여야 함 - 순서 틀리면 컴파일 오류


class UGameplayEffect;
class USayuInventoryWidget;
class USpringArmComponent;   // 카메라 붐 (카메라와 캐릭터 사이 거리 조절)
class UCameraComponent;      // 실제 카메라
class UInputMappingContext;  // Enhanced Input - 키 배열표
class UInputAction;          // Enhanced Input - 개별 입력 행동
class USayuItemDefinition;
class USayuInventoryComponent;


UCLASS()

class PROJECTSAYT_API ASayuCharacter : public ASayuCharacterBase


{
	GENERATED_BODY()
	

public:
	// Sets default values for this character's properties
	ASayuCharacter();

	// Called every frame
	virtual void Tick(float DeltaTime) override;
	// C#의 Update() 와 같음. DeltaTime = Time.deltaTime
	
	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// 입력 바인딩 설정 함수 - Enhanced Input 연결 여기서 함
		
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
		
	// === 카메라 컴포넌트 ===
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> CameraBoom;
	// UPROPERTY() : UE GC에 이 포인터를 알려줌 + 에디터에 노출
	// VisibleAnywhere : 에디터에서 보이지만 수정 불가
	// BlueprintReadOnly : 블루프린트에서 읽기만 가능
	// Category : 에디터 디테일 패널에서 그룹 이름
	// TObjectPtr<T> : UE5에서 권장하는 UObject 포인터 타입
	//                 C#의 일반 참조형처럼 쓰면 됨, 내부적으로 안전성 강화됨
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FollowCamera;
	
	// === Enhanced Input 에셋 참조 ===
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
	// EditAnywhere : 에디터에서 읽기/쓰기 모두 가능 (에셋 드래그앤드롭 가능)
	
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> JumpAction;
	
	
	// Enhanced Input용 어빌리티 발동 액션
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> BasicAttackAction;

	void OnBasicAttackInput(const struct FInputActionValue& Value);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> BuffAction;

	void OnBuffInput(const struct FInputActionValue& Value);
	
	// 추가 - Phase 4 Save/Load 동작 검증용
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> QuickSaveAction;

	void OnQuickSaveInput(const struct FInputActionValue& Value);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> QuickLoadAction;

	void OnQuickLoadInput(const struct FInputActionValue& Value);
	
	// 임시 디버그용 - Save/Load 검증 끝나면 삭제
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input")
	TObjectPtr<UInputAction> DebugDamageAction;

	// Phase 6 GMS 테스트용 - BasicAttack의 DamageEffectClass와 같은 BP 에셋을 지정하면 됨
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	TSubclassOf<UGameplayEffect> DebugSelfDamageEffectClass;
	
	void OnDebugDamageInput(const struct FInputActionValue& Value);
	////////////////////////////////////////

	// Phase 5 임시 디버그용 — Fragment 시스템 자체를 테스트하기 위한 입력 트리거.
	// 정식 인벤토리 UI가 생기면 이 디버그 경로는 제거할 예정 (Phase 11 정리 대상).
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	TObjectPtr<UInputAction> IA_DebugEquip;

	// 테스트용으로 장착해볼 아이템. 에디터에서 DA_Item_Test를 직접 지정.
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	TObjectPtr<USayuItemDefinition> DebugTestItem;

	void DebugEquipTestItem();
	
	//Inventory
	UPROPERTY(VisibleAnywhere, Category = "Sayu|Inventory")
	TObjectPtr<USayuInventoryComponent> InventoryComponent;
		
	// 임시 UI 디버그용
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<class UUserWidget> HUDWidgetClass;
	
	// Inventory 디버그용
	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	TObjectPtr<UInputAction> IA_DebugToggleInventory;

	UPROPERTY(EditDefaultsOnly, Category = "Debug")
	TSubclassOf<USayuInventoryWidget> InventoryWidgetClass;

	UPROPERTY()
	TObjectPtr<USayuInventoryWidget> ActiveInventoryWidget;
		
	void DebugToggleInventory();
	
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UInputAction> IA_ToggleUIMouse;
	
	void OnUIMousePressed();
	void OnUIMouseReleased();


private:
	UPROPERTY()
	TObjectPtr<class UUserWidget> HUDWidgetInstance;
	

	// === 입력 처리 함수 ===
	void Move(const struct FInputActionValue& Value);
	void Look(const struct FInputActionValue& Value);
	// FInputActionValue : Enhanced Input이 넘겨주는 입력 값 컨테이너
	// bool / float / FVector2D / FVector 중 하나를 담고 있음
};
