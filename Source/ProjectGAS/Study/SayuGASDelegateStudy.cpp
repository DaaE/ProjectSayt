#include "CoreMinimal.h"
#include "EngineUtils.h"                                       // TActorIterator
#include "AbilitySystemComponent.h"
#include "AbilitySystem/Attributes/SayuAttributeSet_Combat.h"  // GetHealthAttribute()
#include "Character/SayuNPCCharacter.h"

// 콘솔 커맨드: 월드의 모든 NPC를 찾아 체력 방송을 구독
static FAutoConsoleCommandWithWorld GHealthWatchCmd(
	TEXT("Sayu.Study.HealthWatch"),
	TEXT("월드의 모든 SayuNPCCharacter의 Health 변경 델리게이트를 구독해 로그 출력"),
	FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* World)
	{
		int32 Count = 0;
		for (TActorIterator<ASayuNPCCharacter> It(World); It; ++It)
		{
			ASayuNPCCharacter* NPC = *It;
			UAbilitySystemComponent* ASC = NPC->GetAbilitySystemComponent();
			if (!ASC) { continue; }

			// 로그에 어느 NPC인지 표시하기 위해 이름을 '값으로 복사' 캡처.
			// [Name] 문법: 람다가 만들어지는 순간 Name 변수를 자기 안에 복사해 들고 다님
			// (단원 3에서 이 캡처 방식의 선택이 왜 중요한지 다룹니다)
			const FString Name = NPC->GetName();

			// ★ 오늘의 핵심 한 줄: GAS 내장 방송국을 꺼내서(Get...) 구독(Add...)
			ASC->GetGameplayAttributeValueChangeDelegate(USayuAttributeSet_Combat::GetHealthAttribute()).AddLambda([Name](const FOnAttributeChangeData& Data)
			{
				UE_LOG(LogTemp, Warning, TEXT("[HealthWatch] %s : %.1f -> %.1f"),
					*Name, Data.OldValue, Data.NewValue);
			});
			
			// ★ 초기 1회 Pull: 구독 '이전'의 세계를 따라잡는 유일한 방법.
			//    방송(Push)은 이 순간 이후의 변화만 알려주기 때문.
			const float CurrentHealth = ASC->GetNumericAttribute(USayuAttributeSet_Combat::GetHealthAttribute());
			UE_LOG(LogTemp, Warning, TEXT("[HealthWatch] %s 구독 시점 현재값(Pull): %.1f"), *Name, CurrentHealth);
			
			++Count;
		}
		UE_LOG(LogTemp, Warning, TEXT("--- HealthWatch: NPC %d마리 구독 완료. 이제 때려보세요 ---"), Count);
	})
);