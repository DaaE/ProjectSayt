# 엔진 동작 노트 — GAS 활성 효과 / 게임플레이 이펙트

> **이 파일이 있는 이유.** Phase 8 Stage 3 파트 A를 진행하던 대화창이 세 번 폐기됐다.
> 원인은 매번 같았다 — 미확인 사실을 「필요 시점에 조회」로 미뤄 두고, 그 시점이 학습 단원
> 한가운데였다. 새 창은 조사부터 다시 시작했고 DaaE에게 엔진 grep 요청이 반복됐다.
>
> **파트 A 단원 진행 중에는 조회하지 않는다. 답은 이 파일에 있다.**
> 단원 중간에 조회가 필요해지면 그것은 이 파일이 부실하다는 신호이므로, 단원을 물리고
> 이 파일을 채우는 작업으로 돌아간다.
>
> `CONVENTIONS.md`는 코드 주석과 Docs에 엔진 동작 주장을 남기지 말라고 규정한다.
> 그 규정의 목적은 검증 안 된 추측이 문서에 굳는 것을 막는 데 있다. **이 파일은 그 규정의
> 지정된 예외이자 유일한 수용처다** — 여기 적힌 것은 전부 로컬 엔진 소스나 공식 문서로
> 확인된 것이고, 다른 문서와 코드 주석에는 계속 남기지 않는다.

**기준 버전: UE 5.7** (로컬 `C:\Program Files\Epic Games\UE_5.7\Engine`)
엔진 업그레이드 시 이 파일 전체를 재검증 대상으로 취급한다. 근거는 파일명까지만 적는다 —
행 번호는 버전이 바뀌면 어긋나므로 적지 않는다.

출처 표기: 표기 없음 = 로컬 5.7 소스 직접 확인. `[문서]` = Epic 공식 5.7 문서.
`[커뮤니티 5.3]` = tranek/GASDocumentation(Epic이 공식 문서에서 링크하는 레퍼런스, 5.3 기준).
`[이전 세션]` = 이전 대화창에서 로컬 소스로 확인했으나 이번 세션에 재확인하지 않음.

---

## 1. 지속시간 센티넬 값

`GameplayEffect.h` / `GameplayEffect.cpp`

```
FGameplayEffectConstants::INFINITE_DURATION   = -1.f
FGameplayEffectConstants::INSTANT_APPLICATION =  0.f
FGameplayEffectConstants::NO_PERIOD           =  0.f
FGameplayEffectConstants::INVALID_LEVEL       = -1.f
```

`UGameplayEffect::INFINITE_DURATION` 등 같은 이름의 정적 멤버는 **폐기된 별칭**이다
(주석: please use FGameplayEffectConstants:: instead). 신규 코드는 `FGameplayEffectConstants::`를 쓴다.

센티넬 비교는 엔진 자신이 `==`로 한다. `CONVENTIONS.md`의 float 규칙(`IsNearlyEqual`)은
산술 결과의 동등 판정을 대상으로 하며, 센티넬 값 비교는 그 범주가 아니다.

## 2. 잔여 시간을 읽는 방법 — 일괄 조회는 무한 지속에서 깨진다

무한 지속 여부를 분기해 `-1`을 반환하는 처리는 `GetTimeRemaining` / `GetEndTime` 계열에
들어 있다. 반면 **일괄 조회 함수 `GetActiveEffectsTimeRemainingAndDuration`의 계산부에는
그 분기가 없다** — `Duration - Elapsed`를 그대로 내므로 무한 지속 효과에서는 반환된
`TPair`의 Key(잔여)와 Value(전체)가 **둘 다 음수**가 된다.

따라서 표시 모델은 잔여 시간을 **핸들 단위로 읽는다.**
`GetActiveGameplayEffect(Handle)`로 활성 효과 구조체를 받아 그쪽 함수를 부른다.

일괄 조회 함수를 쓰지 않는 이유는 둘이다.
1. 무한 지속에서 음수가 나온다 (위)
2. 핸들 목록과 시간 목록이 **두 번의 별도 순회**로 만들어진다. 인덱스는 대응하지만
   두 호출 사이에 목록이 바뀌면 어긋난다 — 같은 프레임 내 연속 호출에서만 유효하다

`TPair` 자체의 의미는 Key = 잔여시간, Value = 전체 지속시간이다 (함수 이름 순서와 일치).

## 3. 시간 변경 통지는 감소 통지가 아니다

`OnTimeChanged`가 넘기는 `NewStartTime` / `NewDuration`은 **지속시간이 갱신됐다는 사건**이다.
매 프레임 잔여 시간이 줄어드는 것을 알려주지 않는다.

→ 라디얼 쿨다운 와이프는 **ActiveTimer + 조회** 방식으로 그린다
(Stage 1 고스트 바에서 쓴 ActiveTimer 패턴 재사용).

## 4. 활성 효과 목록 조회

`AbilitySystemComponent.h`

```cpp
TArray<FActiveGameplayEffectHandle> GetActiveEffects(const FGameplayEffectQuery&) const;
const FActiveGameplayEffect*       GetActiveGameplayEffect(FActiveGameplayEffectHandle) const;
const UGameplayEffect*             GetGameplayEffectCDO(FActiveGameplayEffectHandle) const;
const UGameplayEffect*             GetGameplayEffectDefForHandle(FActiveGameplayEffectHandle);  // 비-const
```

**빈 `FGameplayEffectQuery`는 전체 매치다.** `Matches`는 무시 핸들 목록과 커스텀 델리게이트를
먼저 보고, 이후 모든 태그 조건을 「비어 있지 않을 때만」 검사한다. 유효한 `Def`를 가진 활성
효과는 전부 통과한다 (`Def == nullptr`만 탈락).

읽기 전용 용도에는 `GetGameplayEffectCDO`(const)를 쓴다.

## 5. 즉발 효과는 활성 목록에 들어오지 않는다

`[문서]` 즉발 효과는 "Executed"로 처리되며 Active Gameplay Effects Container에 들어가지
않는다. 지속형과 무한 효과만 들어간다.

→ **트레이 필터의 1단계는 코드가 필요 없다.** `GE_Damage`, `GE_ComboIncrement` 같은 즉발
효과는 우리 코드가 관여하지 않고 걸러진다. 걸러야 할 것은 지속형인데 표시 대상이 아닌 것
(`GE_ComboWindow`, `GE_Cooldown_BasicAttack`)뿐이다.

## 6. GameplayEffectComponent — 데이터 컴포넌트의 계약

`GameplayEffectComponent.h`

`UCLASS(Abstract, Const, DefaultToInstanced, EditInlineNew, CollapseCategories, Within=GameplayEffect, MinimalAPI)`

**GE 하나당 컴포넌트 인스턴스는 하나뿐이고, 그 GE가 적용된 모든 대상이 그것을 공유한다.**
`[문서]`에도 같은 서술이 있다. 클래스 주석은 런타임에 조작되는/인스턴스별 데이터를 담아서는
안 된다고 명시하며, `Const` 지정자가 이를 문법으로 강제한다.

→ **스택 수·잔여 시간·진행 중인 연출을 컴포넌트에 저장하는 것은 계약 위반이다.**
정의(모든 인스턴스 공유 불변 데이터)와 인스턴스(적용 개체별 값)를 나누는 근거가 여기다.

| | 사는 곳 | 예 |
|---|---|---|
| 정의 | UI Data 컴포넌트 (GE 애셋) | 아이콘, 표시명, 버프/디버프 분류 |
| 인스턴스 | 활성 효과 (대상 ASC) | 스택 수, 잔여 시간, 억제 여부 |

**게임플레이 훅은 전부 기본 구현이 「허용 / 무동작」이다.**
`CanGameplayEffectApply` → `true`, `OnActiveGameplayEffectAdded` → `true`,
`OnGameplayEffectExecuted` → 빈 본문, `OnGameplayEffectApplied` → 빈 본문,
`OnGameplayEffectChanged` → 빈 본문.
아무것도 override하지 않는 순수 데이터 컴포넌트는 **게임플레이에 부작용이 없다.**

`WITH_EDITOR` `IsDataValid` 훅이 열려 있고, 기본 구현이 **같은 클래스의 중복 부착을 막는다**
(UI Data가 두 개 붙는 사고는 구조적으로 없다). override하면 애셋 저장 시점에 자체 검증
경고를 띄울 수 있다.

## 7. UGameplayEffectUIData — 비어 있는 것이 계약이다

`GameplayEffectUIData.h` / `GameplayEffectUIData_TextOnly.h`

- `UGameplayEffectUIData`는 `UCLASS(Blueprintable, Abstract, ...)`이고 **멤버가 0개다.**
  클래스 주석이 게임별 데이터를 넣어 파생시키라고 직접 지시한다. 5.3부터
  `UGameplayEffectComponent` 파생이다.
- 엔진 제공 파생 클래스 `UGameplayEffectUIData_TextOnly`는 `FText Description` 하나뿐이며,
  주석이 스스로를 파생 예시로 규정한다. 아이콘·분류가 없어 트레이에는 부족하다.
- 낡은 `UGameplayEffect::UIData` 프로퍼티는 5.3에서 폐기됐고, 폐기 메시지가 대체 경로를
  그대로 알려준다 — 컴포넌트로 추가하고 `FindComponent<UGameplayEffectUIData>()`로 접근.
- **주의:** `[문서]`의 GEComponent 목록 표에는 UI Data가 들어 있지 않다(9종만 나열).
  파생해서 쓰라는 근거는 문서 표가 아니라 클래스 주석과 폐기 메시지다.

## 8. FindComponent는 파생 클래스를 잡는다

`GameplayEffect.h` / `GameplayEffect.cpp`

```cpp
template<typename GEComponentClass> const GEComponentClass* FindComponent() const;
const UGameplayEffectComponent* FindComponent(TSubclassOf<UGameplayEffectComponent>) const;
```

템플릿판은 컴포넌트 배열을 순회하며 `Cast<GEComponentClass>`를 시도해 첫 성공을 반환한다.
비-템플릿판은 `IsA`를 쓰고 주석도 "the first component that derives from"이라고 명시한다.
`static_assert`로 `UGameplayEffectComponent` 파생이 아닌 타입은 컴파일이 막힌다.

→ **우리 파생 타입으로 직접 질의하면 필터 판정과 데이터 획득이 한 번에 끝난다.**
기반 타입으로 질의한 뒤 캐스트하는 2단 구조는 `_TextOnly`가 붙은 경우 「필터는 통과하는데
데이터는 못 읽는」 틈을 만든다.

## 9. 추가 / 제거 전역 통지 — 이름이 비슷한 델리게이트 주의

`AbilitySystemComponent.h`

**UI가 쓸 것 (지속형·무한만 발화):**

```cpp
// 추가
OnActiveGameplayEffectAddedDelegateToSelf
  → void Callback(UAbilitySystemComponent* Target,
                  const FGameplayEffectSpec& SpecApplied,
                  FActiveGameplayEffectHandle ActiveHandle);

// 제거
FOnGivenActiveGameplayEffectRemoved& OnAnyGameplayEffectRemovedDelegate();
  → void Callback(const FActiveGameplayEffect& EffectRemoved);
```

**리플리케이션 조건 `[커뮤니티 5.3]`** — Phase 10에서 「몹의 디버프 아이콘이 클라이언트에
보이는가」를 결정한다. 싱글플레이에서는 차이가 드러나지 않는다.

| 역할 | 발화 조건 |
|---|---|
| 서버 | 리플리케이션 모드와 무관하게 항상 |
| autonomous proxy | Full·Mixed 모드에서 리플리케이트된 효과에 대해서만 |
| simulated proxy | Full 모드에서만 |

**쓰면 안 되는 유사 델리게이트 `[이전 세션]`** — 이름이 한 단어 차이라 잘못 고르기 쉽다.
아래 셋은 **서버에서만** 발화하므로 UI 구독에 쓰면 클라이언트에서 조용히 죽는다.

- `OnGameplayEffectAppliedDelegateToSelf` — 즉발 포함
- `OnGameplayEffectAppliedDelegateToTarget` — 즉발 포함
- `OnPeriodicGameplayEffectExecuteDelegateOnSelf` — 주기 실행

**초기 따라잡기 필요.** 델리게이트는 구독 이후의 변화만 통지한다. 구독 직후 현재 목록을
1회 순회해 이미 걸려 있는 효과를 반영해야 한다(Stage 1 「초기 1회 Pull + 이후 Push」와 동일,
Stage 2 몹 패널의 리스폰 따라잡기와 동일한 문제).

## 10. 항목 단위 구독

`AbilitySystemComponent.h` / `GameplayEffectTypes.h`

```cpp
FActiveGameplayEffectEvents* GetActiveEffectEventSet(FActiveGameplayEffectHandle);  // const 아님

struct FActiveGameplayEffectEvents          // USTRUCT 아닌 일반 구조체
{
    FOnActiveGameplayEffectRemoved_Info      OnEffectRemoved;
    FOnActiveGameplayEffectStackChange      OnStackChanged;
    FOnActiveGameplayEffectTimeChange       OnTimeChanged;
    FOnActiveGameplayEffectInhibitionChanged OnInhibitionChanged;
};
```

개별 접근자도 넷 있으나(`OnGameplayEffectRemoved_InfoDelegate`,
`OnGameplayEffectStackChangeDelegate`, `OnGameplayEffectTimeChangeDelegate`,
`OnGameplayEffectInhibitionChangedDelegate`, 모두 포인터 반환)
**묶음 포인터를 한 번 받아 넷에 바인드하는 쪽을 쓴다** — 호출이 하나로 끝나고 해제 지점도
한 곳이 된다.

델리게이트 시그니처 `[이전 세션]`:

```
OnStackChanged      (Handle, int32 NewStackCount, int32 PreviousStackCount)
OnTimeChanged       (Handle, float NewStartTime, float NewDuration)
OnInhibitionChanged (Handle, bool bIsInhibited)
OnEffectRemoved     (const FGameplayEffectRemovalInfo&)
```

**항목 단위 생명주기 관리가 필요하다** — 항목이 생길 때 개별 구독, 사라질 때 개별 해제.
Stage 1의 `SSaytHealthBar`는 ASC 하나만 구독하면 됐다는 점과 대비된다. 이것이 파트 A의
실질 난이도다.

## 11. 스택 수 조회

`AbilitySystemComponent.h`

```cpp
int32 GetCurrentStackCount(FActiveGameplayEffectHandle) const;
int32 GetCurrentStackCount(FGameplayAbilitySpecHandle) const;   // GE가 부여한 어빌리티 기준
```

`[커뮤니티 5.3]` 스택 수는 GameplayEffectSpec에 있고 상한은 GameplayEffect에 있다.
스택은 **지속형과 무한 효과에서만** 동작한다.

## 12. 스택 정책 5종

`GameplayEffectTypes.h` / `GameplayEffect.h`, 기본값은 `UGameplayEffect` 생성자

| 프로퍼티 | 값 | 기본값 |
|---|---|---|
| `StackingType` | `None`("No Stacking") / `AggregateBySource`("Stack Per Source") / `AggregateByTarget`("Stack Per Target") | `None` |
| `StackLimitCount` | 정수. **0이면 상한 없음** | `0` |
| `StackDurationRefreshPolicy` | `RefreshOnSuccessfulApplication` / `NeverRefresh` / `ExtendDuration` | `RefreshOnSuccessfulApplication` |
| `StackPeriodResetPolicy` | `ResetOnSuccessfulApplication` / `NeverReset` | `ResetOnSuccessfulApplication` |
| `StackExpirationPolicy` | `ClearEntireStack` / `RemoveSingleStackAndRefreshDuration` / `RefreshDuration` | — |

`bFactorInStackCount`의 기본값은 `true`(스택 수가 수정자 크기에 곱해진다).

**스택 종류 선택 기준 `[커뮤니티 5.3]`** — `AggregateBySource`는 소스 ASC별로 별개의 스택
묶음이 생기고, `AggregateByTarget`은 소스가 누구든 대상에 묶음이 하나만 있어 공유 상한을
나눠 쓴다. 냉기 게이지는 누가 걸었든 하나여야 하므로 `AggregateByTarget`이다.

**만료 정책이 트레이 표시에 직결된다.**

| 정책 | 화면에서 보이는 것 |
|---|---|
| `ClearEntireStack` | 시간이 다 되면 5스택이 통째로 사라진다 |
| `RemoveSingleStackAndRefreshDuration` | 1스택씩 깎이며 지속시간이 재충전 — 라디얼 와이프가 한 스택 주기를 표현한다 |
| `RefreshDuration` | 사실상 무한. 스택 감소를 코드가 직접 관리한다 |

## 13. 오버플로 발동 시점 — 상한 도달 후 한 번 더

`GameplayEffect.cpp`

기본 동작의 판정은 **적용 전 스택 수 == 상한**이다. 상한을 초과하는 순간이 아니라
**이미 상한에 도달한 상태에서 한 번 더 적용될 때** `HandleActiveGameplayEffectStackOverflow`가
불린다. 소스 주석도 이것을 "Legacy behavior"로 표기한다.

→ 상한 5로 두면 5회로 5스택을 채우고 **6번째 적용에서** 오버플로 효과가 나간다.
5번째 적중에 전이시키려면 상한을 4로 잡는다.

새 스택 수는 `FMath::Min(NewStackCount, StackLimitCount)`로 잘린다(상한 > 0일 때).

**오버플로 효과는 넘친 스펙의 컨텍스트를 이어받는다** — `InitializeFromLinkedSpec`으로
스펙을 만들어 적용하므로 가해자가 마지막 스택을 넣은 대상으로 유지된다. Phase 10에서
의미가 생긴다.

`bDenyOverflowApplication`이 켜져 있으면 오버플로 처리가 실패를 반환해 적용 자체가 거부되고
(`Application of ... denied (StackLimit)` 로그), 지속시간과 컨텍스트가 갱신되지 않는다.

**표시 모델 요구사항: 스택 변경 통지는 값이 그대로일 때도 발화할 수 있다.**
상한에서 적용이 허용된 경우 이전 값과 새 값이 같은 채로 `OnStackCountChange`가 불린다.
값을 비교해 무변화를 걸러야 불필요한 무효화가 안 생긴다.

## 14. 같은 GE가 복수 인스턴스로 공존한다 — 스택과 다른 메커니즘

`StackingType`이 `None`이면 매 적용이 별개 인스턴스가 된다. `showdebug abilitysystem`에서
`combowindow(1)`, `combowindow(2)`로 관찰됨 `[이전 세션, PIE 실측]`.

→ **트레이는 「활성 효과 1개 = 아이콘 1개」가 아니라 「효과 종류로 묶어 아이콘 1개 + 숫자」여야
한다.** 그리고 그 숫자가 스택 카운터인지 인스턴스 개수인지를 표시 모델이 구분해 들고 있어야
한다. 화면상 구분이 안 되므로 코드에서 섞이면 발견이 늦다.

## 15. GMS는 이 용도에 쓰지 않는다

Phase 6의 Gameplay Message Subsystem은 `FSaytDamageMessage`용이고 효과 적용/제거를
브로드캐스트하지 않는다. 활성 효과 구독은 ASC 직접 구독이 맞다.

이것은 Stage 2에서 세운 판별 기준의 연장이다 — 전달하려는 것이 **사건**이면 방송(GMS),
**상태**면 목록 소유 + 델리게이트. 「현재 걸려 있는 효과 목록」은 상태이고, 늦게 참여한
구독자가 그 전의 것을 알아야 하므로 방송으로는 성립하지 않는다.

---

## 추적하지 않는 것

의도적으로 답을 찾지 않은 항목. 필요해지는 시점이 오면 그때 조사한다.

| 항목 | 왜 추적하지 않는가 |
|---|---|
| `EActiveGameplayEffectOverflowBehavior`를 켜는 방법 | 상한 도달 대신 초과 시점에 발동시키는 비-기본 동작. Stage 3은 기본 동작을 쓴다 |
| Gameplay Debugger 사용법 | `showdebug abilitysystem` 화면 하단이 안내하는 상위 도구. Phase 9~11에서 다룰 여지 |
| `UWidgetComponent` Screen Space 내부 동작 | Stage 2 이연 항목. Phase 11 |
