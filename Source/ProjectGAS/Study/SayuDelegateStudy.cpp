#include "CoreMinimal.h"

// [1] 선언 — "float 하나를 인자로 받는 함수들"을 등록할 수 있는 방송국 '타입'을 정의.
//     C#으로 치면: public delegate void FOnHealthChangedStudy(float NewHealth);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnHealthChangedStudy, float /*NewHealth*/);

// 방송국 실물(인스턴스). C#으로 치면: public static event FOnHealthChangedStudy ...;
static FOnHealthChangedStudy GHealthChangedStudy;

// 구독자가 될 평범한 함수 두 개
static void ListenerA(float NewHealth)
{
	UE_LOG(LogTemp, Warning, TEXT("[ListenerA] 체력이 %.1f 로 바뀌었다는 방송 수신"), NewHealth);
}

static void ListenerB(float NewHealth)
{
	UE_LOG(LogTemp, Warning, TEXT("[ListenerB] 나도 같은 방송 수신: %.1f"), NewHealth);
}

// 콘솔 커맨드 등록 (Stage 0의 Sayu.ToggleTuningDemo와 같은 방식)
static FAutoConsoleCommand GDelegateStudyCmd(
	TEXT("Sayu.Study.Delegate"),
	TEXT("델리게이트 선언-구독-발신 3단계를 로그로 확인"),
	FConsoleCommandDelegate::CreateLambda([]()   // 람다: Stage 0에서 나온 그 익명 함수 문법
	{
		// [2] 구독 — 방송국에 수신자 등록.
		//     AddStatic은 '일반(자유) 함수' 전용 등록 방법입니다.
		//     (구독자의 종류에 따라 Add 계열이 여러 개 있는데, 단원 3에서 다룹니다)
		GHealthChangedStudy.AddStatic(&ListenerA);
		GHealthChangedStudy.AddStatic(&ListenerB);

		UE_LOG(LogTemp, Warning, TEXT("--- 구독자 2명 등록 완료. 방송 시작 ---"));

		// [3] 발신 — 등록된 전원을 한 번에 호출.
		//     주목: 이 줄은 ListenerA/B의 존재를 전혀 모릅니다.
		GHealthChangedStudy.Broadcast(75.f);

		// 정리 — 안 하면 커맨드를 다시 실행할 때 같은 구독자가 '또' 등록되어
		// 방송 한 번에 두 번씩 호출됩니다. (해제가 왜 중요한지의 첫 맛보기)
		GHealthChangedStudy.Clear();
	})
);