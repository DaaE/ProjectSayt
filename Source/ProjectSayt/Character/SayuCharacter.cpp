// Copyright (c) 2026 Sungmin JI. All Rights Reserved.


#include "SayuCharacter.h"
#include "AbilitySystemComponent.h"

// Enhanced Input 관련 헤더
// 구현 파일(.cpp)에서는 전방 선언 대신 실제 헤더를 include 해야 함
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

// 카메라 관련
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"

// 이동 관련
#include "GameFramework/CharacterMovementComponent.h"

#include "../Subsystems/SayuGameDataSubsystem.h"

#include "Blueprint/UserWidget.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "SayuLogChannels.h"
#include "AbilitySystem/Attributes/SayuAttributeSet_Combat.h"
#include "Items/SayuItemDefinition.h"
#include "Items/SayuItemFragment.h"
#include "items/SayuItemInstance.h"
#include "Inventory/SayuInventoryComponent.h"
#include "UI/SayuInventoryWidget.h"
#include "UI/SayuUIInputModeSubsystem.h"


// Sets default values
ASayuCharacter::ASayuCharacter()
// 생성자 - C#의 Awake() 또는 생성자와 같음
// 컴포넌트 생성과 기본값 설정은 여기서
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	// Tick(Update)를 활성화할지 여부
	// C#에서는 Update()가 항상 호출됐지만
	// UE에서는 성능을 위해 필요한 것만 true로 설정
	
	// === 카메라 붐 생성 ===
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	// CreateDefaultSubobject : 생성자에서만 쓰는 컴포넌트 생성 함수
	// C#의 AddComponent<T>() 와 비슷하지만 생성자 전용
	// TEXT() : FString/FName용 문자열 매크로, 유니코드 처리해줌
	
	CameraBoom->SetupAttachment(RootComponent);
	// RootComponent에 붙임 (캡슐 콜라이더가 루트)
	// C#의 transform.SetParent() 와 비슷

	CameraBoom->TargetArmLength = 400.f;
	// 카메라와 캐릭터 사이 거리 (cm 단위, UE는 cm 기준)

	CameraBoom->bUsePawnControlRotation = true;
	// 마우스 회전에 따라 카메라 붐도 회전
	
	// === 카메라 생성 ===
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// 카메라 붐 끝에 붙임
	
	FollowCamera->bUsePawnControlRotation = false;
	// 카메라 자체는 회전 안 함 (붐이 회전하니까)
	
	// === 캐릭터 회전 설정 ===
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	// 컨트롤러(마우스) 회전이 캐릭터 몸통에 직접 영향 안 주게
	
	GetCharacterMovement()->bOrientRotationToMovement = true;
	// 이동 방향으로 캐릭터가 자동으로 회전 (RPG 스타일)
	GetCharacterMovement()->RotationRate = FRotator(0.f, 500.f, 0.f);
	// 회전 속도
	
	// SayuCharacter.cpp (생성자에 추가 — ASC/AttributeSet 만들 때와 같은 패턴)
	InventoryComponent = CreateDefaultSubobject<USayuInventoryComponent>(TEXT("InventoryComponent"));
}

void ASayuCharacter::OnBasicAttackInput(const struct FInputActionValue& Value)
{
	if (!AbilitySystemComponent)
	{
		return;;
	}
	
	const FGameplayTag LightAttackTag = FGameplayTag::RequestGameplayTag(FName("Ability.Attack.Light"));
	
	if (AbilitySystemComponent->HasMatchingGameplayTag(LightAttackTag))
	{
		// 어빌리티가 이미 활성 중 → 재발동 시도 안 함, 신호만 전달
		FGameplayEventData EventData;
		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(
			this, FGameplayTag::RequestGameplayTag(FName("Event.Combo.Input")), EventData);
	}
	else
	{
		FGameplayTagContainer TagContainer;
		// FGameplayTagContainer : 태그를 "여러 개" 담는 컨테이너
		// 지금은 1개만 넣지만, 컨테이너로 받는 이유는 나중에
		// "이 중 하나라도 매칭되면 발동" 같은 OR 조건 확장이 가능해서예요

		TagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Attack.Light")));
				
		// "Try"가 붙은 이유: CanActivateAbility 체크를 통과해야만
		// 실제 발동되기 때문이에요 (쿨타임/코스트 등 자동 체크)
		AbilitySystemComponent->TryActivateAbilitiesByTag(TagContainer);
		// 클래스를 직접 안 적었죠? 태그만 보고 ASC가 보유한 어빌리티 중에서
		// 해당 태그를 가진 걸 찾아서 발동시켜요
	}
}

void ASayuCharacter::OnBuffInput(const struct FInputActionValue& Value)
{
	if (AbilitySystemComponent)
	{
		FGameplayTagContainer TagContainer;
		TagContainer.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Skill.Dash")));

		AbilitySystemComponent->TryActivateAbilitiesByTag(TagContainer);
	}
}

void ASayuCharacter::OnQuickSaveInput(const struct FInputActionValue& Value)
{
	if (USayuGameDataSubsystem* GameData = GetGameInstance()->GetSubsystem<USayuGameDataSubsystem>())
	{
		// CombatAttributeSet은 ASayuCharacterBase의 protected 멤버라 자식 클래스에서 바로 접근 가능
		GameData->SaveCombatState(CombatAttributeSet, TEXT("QuickSave"));
	}
}

void ASayuCharacter::OnQuickLoadInput(const FInputActionValue& Value)
{
	if (USayuGameDataSubsystem* GameData = GetGameInstance()->GetSubsystem<USayuGameDataSubsystem>())
	{
		GameData->LoadCombatState(CombatAttributeSet, TEXT("QuickSave"));
	}
}

void ASayuCharacter::OnDebugDamageInput(const struct FInputActionValue& Value)
{
	// InitHealth() 직접 호출은 GameplayEffect 파이프라인을 안 거쳐서
	// PostGameplayEffectExecute(=GMS Broadcast)가 안 탑니다.
	// 그래서 실제 GameplayEffect를 자기 자신에게 적용하는 방식으로 교체했어요.
	if (AbilitySystemComponent && DebugSelfDamageEffectClass)
	{
		FGameplayEffectContextHandle Context = AbilitySystemComponent->MakeEffectContext();
		Context.AddSourceObject(this);
		
		const FGameplayEffectSpecHandle Spec =
			AbilitySystemComponent->MakeOutgoingSpec(DebugSelfDamageEffectClass, 1.f, Context);
		
		if (Spec.IsValid())
		{
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
			UE_LOG(LogSayuCombat, Log, TEXT("[Debug] Self-damage GE applied"));
		}
	}
}

void ASayuCharacter::DebugEquipTestItem()
{
	if (!DebugTestItem)
	{
		return;
	}

	for (const TObjectPtr<USayuItemFragment>& Fragment : DebugTestItem->Fragments)
	{
		if (Fragment)
		{
			Fragment->OnEquipped(this);
		}
	}
	
	// Phase 5 임시 디버그 — 인벤토리 추가 테스트.
	// 정식 루팅/UI 플로우가 생기면 이 디버그 경로는 제거할 예정 (Phase 11 정리 대상).
	if (InventoryComponent)
	{
		if (USayuItemInstance* NewInstance = USayuItemInstance::CreateInstance(this, DebugTestItem))
		{
			InventoryComponent->TryAddInstance(NewInstance);
			InventoryComponent->DebugPrintGrid();
		}
	}
}

void ASayuCharacter::DebugToggleInventory()
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	
	if (ActiveInventoryWidget)
	{
		ActiveInventoryWidget->RemoveFromParent();
		ActiveInventoryWidget = nullptr;
		
		if (UGameInstance* GI = GetGameInstance())
		{
			GI->GetSubsystem<USayuUIInputModeSubsystem>()->PopUIRequest(TEXT("Inventory"));
		}
		
		return;
	}

	if (InventoryWidgetClass)
	{
		ActiveInventoryWidget = CreateWidget<USayuInventoryWidget>(GetWorld(), InventoryWidgetClass);
		ActiveInventoryWidget->SetInventoryComponent(InventoryComponent);
		ActiveInventoryWidget->AddToViewport();
		
		// if (PC)
		// {
		// 	PC->bShowMouseCursor = true;
		//
		// 	// GameAndUI: UI가 안 쓰는 입력(=토글 키)은 그대로 게임에 전달됨 —
		// 	// 그래서 인벤토리 연 채로도 "I" 키로 다시 닫을 수 있음.
		// 	FInputModeGameAndUI InputMode;
		// 	InputMode.SetWidgetToFocus(ActiveInventoryWidget->TakeWidget());
		// 	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		// 	PC->SetInputMode(InputMode);
		// }
		
		if (UGameInstance* GI = GetGameInstance())
		{
			GI->GetSubsystem<USayuUIInputModeSubsystem>()->PushUIRequest(TEXT("Inventory"));
		}
	}
}

void ASayuCharacter::OnUIMousePressed()
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock); // 화면 밖으로도 나갈 수 있게(드래그 자유도)
		InputMode.SetHideCursorDuringCapture(false);
		PC->SetInputMode(InputMode);
		PC->SetShowMouseCursor(true);
	}
}

void ASayuCharacter::OnUIMouseReleased()
{
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->SetShowMouseCursor(false);
	}
}

// Called when the game starts or when spawned
void ASayuCharacter::BeginPlay()
{
	Super::BeginPlay();
	// C#의 base.Start() - 부모 클래스 BeginPlay 먼저 호출
	// UE에서는 Super:: 호출을 빠뜨리면 부모 초기화가 안 돼서
	// 예측 불가한 버그가 생길 수 있음. 습관적으로 항상 호출할 것
	
	// === Enhanced Input 컨텍스트 등록 ===
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
		// Cast<T> : C#의 as 연산자와 같음
		// APlayerController* PC = GetController() as APlayerController; 과 동일
		// 실패 시 nullptr 반환 (C#의 null 반환과 같음)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
				PC->GetLocalPlayer()))
			// Subsystem에서 Enhanced Input 가져오는 방법
				// Subsystem 개념은 Phase 4에서 자세히 다룰 예정
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
			// 0 = 우선순위. 숫자 낮을수록 우선순위 높음
			// 여러 컨텍스트를 상황에 따라 교체하는 게 Enhanced Input의 핵심
		}
		
		if (HUDWidgetClass)
		{
			HUDWidgetInstance = CreateWidget<UUserWidget>(PC, HUDWidgetClass);
			if (HUDWidgetInstance)
			{
				HUDWidgetInstance->AddToViewport();
				// AddToViewport : 이 위젯을 실제 화면에 그리기 시작
			}
		}
	}
	
}

// Called to bind functionality to input
void ASayuCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// UInputComponent를 UEnhancedInputComponent로 캐스팅
	if (UEnhancedInputComponent* EIC =
		Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// IA를 함수에 바인딩
		// ETriggerEvent::Triggered : 키 누르는 동안 매 프레임 호출
		// ETriggerEvent::Started   : 키 누른 순간 한 번
		// ETriggerEvent::Completed : 키 뗀 순간 한 번
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered,
			this, &ASayuCharacter::Move);

		EIC->BindAction(LookAction, ETriggerEvent::Triggered,
			this, &ASayuCharacter::Look);

		EIC->BindAction(JumpAction, ETriggerEvent::Started,
			this, &ACharacter::Jump);
		// Jump는 ACharacter에 이미 구현되어 있어서 직접 바인딩 가능

		EIC->BindAction(JumpAction, ETriggerEvent::Started,
			this, &ACharacter::StopJumping);
		
		EIC->BindAction(BasicAttackAction, ETriggerEvent::Started,
			this, &ASayuCharacter::OnBasicAttackInput);
		
		EIC->BindAction(BuffAction, ETriggerEvent::Started,
			this, &ASayuCharacter::OnBuffInput);

		EIC->BindAction(QuickSaveAction, ETriggerEvent::Started,
			this, &ASayuCharacter::OnQuickSaveInput);

		EIC->BindAction(QuickLoadAction, ETriggerEvent::Started,
			this, &ASayuCharacter::OnQuickLoadInput);
		
		// save load 디버그용 확인 후 삭제
		EIC->BindAction(DebugDamageAction, ETriggerEvent::Started,
			this, &ASayuCharacter::OnDebugDamageInput);
		
		EIC->BindAction(IA_DebugEquip, ETriggerEvent::Started,
			this, &ASayuCharacter::DebugEquipTestItem);
		
		EIC->BindAction(IA_DebugToggleInventory, ETriggerEvent::Started,
			this, &ASayuCharacter::DebugToggleInventory);
		
		EIC->BindAction(IA_ToggleUIMouse, ETriggerEvent::Started,
			this, &ASayuCharacter::OnUIMousePressed);
		
		EIC->BindAction(IA_ToggleUIMouse, ETriggerEvent::Completed,
			this, &ASayuCharacter::OnUIMouseReleased);
	}
}

void ASayuCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();
	// FInputActionValue에서 Vector2D 타입으로 꺼냄
	// IA 에셋에서 Value Type을 Axis2D로 설정했을 때 사용

	if (Controller != nullptr)
	{
		// 컨트롤러(카메라)가 바라보는 방향 기준으로 이동
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		// Yaw = 좌우 회전값만 추출 (위아래 기울기 제거)

		const FVector ForwardDirection =
			FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection =
			FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// 카메라 기준 앞/옆 방향 벡터 계산

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
		// AddMovementInput : CharacterMovementComponent에 이동 의도를 전달
		// 실제 이동은 CharacterMovementComponent가 처리
	}
}

void ASayuCharacter::Look(const FInputActionValue& Value)
{
	// 인벤토리 열려있는 동안 시점 회전 정지 — 마우스가 이제 UI 조작용으로 쓰이니까.
	if (ActiveInventoryWidget)
	{
		return;
	}
	
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);    // 좌우 회전
		AddControllerPitchInput(LookAxisVector.Y);  // 상하 회전
	}
}

// Called every frame
void ASayuCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	// 지금은 비어있음. 나중에 필요하면 채울 것
}
