#include "CoreMinimal.h"

DECLARE_MULTICAST_DELEGATE(FOnStudyPing);   // 인자 없는 버전 (접미사 없음)
static FOnStudyPing GPingStudy;

// TSharedFromThis: "내가 어떤 TSharedPtr에 담겨 있는지 스스로 알 수 있게" 해주는 베이스 클래스.
// AddSP가 요구하는 조건이고, 모든 Slate 위젯이 이미 이걸 상속하고 있습니다.
class FStudyListener : public TSharedFromThis<FStudyListener>
{
public:
	void OnPing() { UE_LOG(LogTemp, Warning, TEXT("[Listener] 핑 수신! 나 아직 살아있음")); }
};

static TSharedPtr<FStudyListener> GListener;

static FAutoConsoleCommand GLifetimeCmd(
	TEXT("Sayu.Study.Lifetime"),
	TEXT("AddSP의 자동 무효화(구독자가 죽으면 호출 건너뜀) 관찰"),
	FConsoleCommandDelegate::CreateLambda([]()
	{
		GPingStudy.Clear();   // 재실행 대비 초기화 (단원 1의 교훈)

		// [1] 구독자를 만들어 AddSP로 등록
		GListener = MakeShared<FStudyListener>();
		GPingStudy.AddSP(GListener.ToSharedRef(), &FStudyListener::OnPing);

		UE_LOG(LogTemp, Warning, TEXT("--- 방송 1회차 (구독자 생존 중) ---"));
		GPingStudy.Broadcast();

		// [2] 구독자 파괴. 주목: Remove(해제)는 일부러 안 부릅니다!
		GListener.Reset();
		UE_LOG(LogTemp, Warning, TEXT("--- 구독자 파괴 완료 (Remove는 안 불렀음) ---"));

		// [3] 죽은 구독자를 향해 방송
		UE_LOG(LogTemp, Warning, TEXT("--- 방송 2회차 (구독자 사망 후) ---"));
		GPingStudy.Broadcast();
	})
);