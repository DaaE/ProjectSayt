// Fill out your copyright notice in the Description page of Project Settings.


#include "SayuCharacterBase.h"
#include "AbilitySystemComponent.h"
#include "SayuLogChannels.h"
#include "AbilitySystem/Abilities/SayuGameplayAbility.h"
#include "AbilitySystem/Attributes/SayuAttributeSet_Combat.h"
#include "Components/CapsuleComponent.h"
#include "Subsystems/SayuGameDataSubsystem.h"


// Sets default values
ASayuCharacterBase::ASayuCharacterBase()
{
	// === GAS 컴포넌트 생성 ===
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	// AttributeSet은 여기서 안 만든다 — 자식마다 구체 타입이
	// 다르므로(플레이어용/적용) 각 자식 생성자에서 만든다.
	
	// 모든 캐릭터(플레이어/NPC)가 공통으로 필요하니 여기서 한 번에 생성
	CombatAttributeSet = CreateDefaultSubobject<USayuAttributeSet_Combat>(TEXT("CombatAttributeSet"));
	
	// DataTable에서 가져올 ID
	CombatStatsRowID = TEXT("Player_Default");
	
	// 생성자에 추가 - 틱이 꺼져있으면 디버그도 안 그려지니 켜둠
	PrimaryActorTick.bCanEverTick = true;
}

// GAS 생태계 전체 (어빌리티, GameplayCue, UI 등)가
// 이 함수를 통해 ASC에 접근해요. 단순하지만 핵심적인 함수예요.
UAbilitySystemComponent* ASayuCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

// Called when the game starts or when spawned
void ASayuCharacterBase::BeginPlay()
{
	Super::BeginPlay();
	
	// 컨트롤러가 없는 액터(지금의 더미 Enemy처럼 AI도 안 붙은 경우)는
	// PossessedBy가 호출될 일이 없어서, 여기서도 한 번 시도한다.
	InitializeAbilitySystem();
}

void ASayuCharacterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// 플레이어 Pawn은 보통 이 경로로 초기화된다.
	InitializeAbilitySystem();
}

void ASayuCharacterBase::InitializeAbilitySystem()
{
	if (bAbilitySystemInitialized || !AbilitySystemComponent)
	{
		return;
	}

	// ASC에게 "이 액터가 주인(Owner)이자 본체(Avatar)다" 라고 초기화
	// Owner와 Avatar가 분리된 이유: PlayerState 방식에서는
	// Owner = PlayerState(영속적), Avatar = Character(리스폰마다 교체)
	// 가 다르기 때문이에요. 우리는 둘 다 Character로 동일하게 설정.
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	// ASC 초기화가 끝난 직후에 어빌리티 부여
	// 순서가 중요해요 - InitAbilityActorInfo 전에 부여하면 동작 안 함
	
	// === DataTable 기반 기본 스탯 초기화 (Phase 4) ===
	if (CombatAttributeSet && !CombatStatsRowID.IsNone())
	{
		if (const USayuGameDataSubsystem* GameData = GetGameInstance()->GetSubsystem<USayuGameDataSubsystem>())
		{
			FSayuCombatStatsRow Stats;
			if (GameData->GetCombatStats(CombatStatsRowID, Stats))
			{
				CombatAttributeSet->InitMaxHealth(Stats.MaxHealth);
				CombatAttributeSet->InitHealth(Stats.MaxHealth);
				CombatAttributeSet->InitMaxMana(Stats.MaxMana);
				CombatAttributeSet->InitMana(Stats.MaxMana);
				CombatAttributeSet->InitAttackPower(Stats.AttackPower);
				CombatAttributeSet->InitDefense(Stats.Defense);

				UE_LOG(LogSayuCombat, Verbose, TEXT("%s: CombatStats 적용됨 (Row: %s, MaxHealth: %.0f, AttackPower: %.0f)"),
					*GetName(), *CombatStatsRowID.ToString(), Stats.MaxHealth, Stats.AttackPower);
			}
		}
	}

	
	GiveDefaultAbilities();
	bAbilitySystemInitialized = true;
}

void ASayuCharacterBase::GiveDefaultAbilities()
{
	if (!AbilitySystemComponent)
	{
		return;
	}

	for (TSubclassOf<USayuGameplayAbility> AbilityClass : DefaultAbilities)
	{
		if (AbilityClass)
		{
			// FGameplayAbilitySpec : "이 어빌리티를 레벨 몇으로, 어떤 InputID로
			// 캐릭터에게 부여할지"를 담는 명세 객체
			// GameplayEffectSpec과 비슷한 패턴이에요 - GAS는 뭔가를
			// "적용"하기 전에 항상 Spec(명세)을 먼저 만드는 일관된 패턴을 씁니다
			FGameplayAbilitySpec AbilitySpec(AbilityClass, 1);
			// 1 = 어빌리티 레벨 (스킬 레벨업 시스템 만들 때 쓰일 값)
			
			AbilitySystemComponent->GiveAbility(AbilitySpec);
			// GiveAbility : 실제로 ASC의 "보유 어빌리티 목록"에 등록
			// 이 시점부터 TryActivateAbility로 발동 가능해짐
		}
	}
}

void ASayuCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

#if !UE_BUILD_SHIPPING
	if (bShowCollisionDebug)
	{
		if (UCapsuleComponent* Capsule = GetCapsuleComponent())
		{
			DrawDebugCapsule(GetWorld(), Capsule->GetComponentLocation(),
				Capsule->GetScaledCapsuleHalfHeight(),
				Capsule->GetScaledCapsuleRadius(),
				Capsule->GetComponentQuat(),
				FColor::Green, false, -1.f, 0, 1.5f);
		}
	}
#endif
}
