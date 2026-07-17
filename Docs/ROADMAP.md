# Sayu — 개발 로드맵

UE 5.7.4 / C++ 기반, GAS(Gameplay Ability System) 중심의 액션 RPG 포트폴리오.
엔진 내장 시스템을 우선 검토하고, Lyra 등 공식 레퍼런스를 비판적으로 수용하며,
설계 결정마다 trade-off를 기록하는 것을 원칙으로 한다.

## Phase 개요

| Phase | 주제 | 상태 |
|---|---|---|
| 0 | 프로젝트 세팅 (UE 5.7.4, Rider, Git+LFS, GAS 플러그인) | ✅ |
| 1 | Enhanced Input 기반 입력 체계 | ✅ |
| 2 | GAS 코어 — AttributeSet 컴포지션, Ability 기초 | ✅ |
| 3 | 콤보 전투 — 단일 어빌리티 생명주기 + GameplayEvent 콤보, ExecCalc 데미지, 오브젝트 풀링 | ✅ |
| 4 | 데이터 파이프라인 — GameInstanceSubsystem, DeveloperSettings, DataTable(CSV), SaveGame | ✅ |
| 5 | 인벤토리 — Lyra Fragment/Definition/Instance 컴포지션, Tarkov식 그리드(1D 점유 배열), CommonUI | ✅ |
| 6 | Gameplay Message Subsystem — 전투 이벤트 브로드캐스트, 폴링 제거 | ✅ |
| 7 | Editor Utility Widget — AssetManager C++ API 학습용 점검 도구 | ✅ |
| **8** | **Slate UI 심화 + 전투 HUD** (하단 상세) | 🔧 Stage 1 완료 |
| 9 | 프로파일링/최적화 — Unreal Insights, Slate Insights, 메모리 측정 | 대기 |
| 10 | Replicated GAS — Listen Server, 클라이언트 예측 | 대기 |
| 11 | AI/보스 — StateTree, Dynamic NavMesh, 보스 페이즈, 사망 상태 처리 | 대기 |
| 12 | 포트폴리오 마무리 — README/아키텍처 다이어그램, 영상, 디버그 코드 정리, 설계 Q&A 문서 | 대기 |
| 13 (선택) | Mass Entity 또는 CI/CD | 선택 |

> 로드맵 재편 예정: Phase 8 종료 시점에 "프레젠테이션 코어"(로코모션,
> Aurora 스킬킷+FX, 보스전, HUD 폴리시)의 스코프와 Phase 9~12 배치를
> 재조정하는 세션이 예약되어 있다. Phase 0~7 코드베이스 감사(시스템별
> 유지/리팩터/재작성 판정)도 이때 수행한다.

## Phase 8 상세 — Slate UI 심화

| Stage | 내용 | 상태 |
|---|---|---|
| 0 | Live-tuning 워밍업 — 직접 마우스 입력(FReply), TAttribute Pull | ✅ |
| 1 | SHealthBar 코어 — GAS Push 델리게이트, TSlateAttribute, 고스트 바(ActiveTimer), 스타일셋 | ✅ |
| 2 | Boss/Monster HP — 줄(라인) 체력 보스 바, SActorCanvas형 월드 패널 | 대기 |
| 3 | 버프/디버프 트레이 — STileView, OnPaint 라디얼 쿨다운 (UMG 래핑 필수) | 대기 |
| 4 | 데미지 폰트 — SWidget 오브젝트 풀 + FCurveSequence | 대기 |
| 5 | 스킬 슬롯 바 — 데이터 기반 슬롯 구성 (UMG 래핑 필수) | 대기 |

**전 Stage 공통 체크리스트**
1. 데이터 바인딩 경계 명시 — 소스가 통지를 제공하면 Push(델리게이트), 아니면 Pull(TAttribute)
2. Widget Role 선택 근거 — Leaf / Compound / Panel
3. Invalidation 전략 — 신고 등급 명시, SlateDebugger/stat slate로 검증
4. 비주얼 폴리시 패스 필수 — 회색 박스 상태로 종료 금지
5. 측정 증거 — before/after 캡처 또는 stat 수치를 Docs/Media에 보존

## Stage 1 산출물 — SSayuHealthBar

### 구현 요약
GAS Attribute에 직결된 커스텀 Slate 체력바(`SLeafWidget`). 텍스처 없이
draw element만으로 7개 레이어(배경 / 피격 잔상 / 채움 / 음영 그라디언트 /
하이라이트 / 눈금 / 외곽 프레임)를 구성하며, 스타일(`FSlateWidgetStyle`)과
런타임 상태를 분리했다.

| | |
|---|---|
| 폴리시 전/후 | `Docs/Media/healthbar_before.png` → `healthbar_after.png` |
| 피격 잔상 동작 | `Docs/Media/healthbar_ghost.gif` |
| 유휴 시 재페인트 측정 | `Docs/Media/stat_slate_idle.png` |

### 설계 결정과 trade-off

- **범용 `SProgressBar` 재사용 대신 특화 위젯 제작.** SProgressBar는 소스를
  모르는 범용 위젯이라 Pull(TAttribute) 인터페이스를 앞세우고, 그 대가로
  Volatility(캐싱 면제) 비용을 진다. 이 위젯은 소스가 GAS로 특정되어 있고
  GAS가 변경 통지(`GetGameplayAttributeValueChangeDelegate`)를 이미 제공하므로,
  이벤트 구동 + 최소 등급(`Paint`) 무효화로 캐싱(Global Invalidation)의 수혜를
  온전히 받는다. 검증: 유휴 시 재페인트 없음, 피격 순간 해당 위젯 1개만
  재페인트(SlateDebugger 플래시 + `SWidget::Paint (Count)` 스파이크로 확인).
- **바인딩 전략: 초기 1회 Pull + 이후 Push.** 델리게이트는 구독 이후의 변화만
  통지하므로, 구독 직후 현재값을 1회 직접 조회해 "위젯 생성 시점 ≠ 대상 생성
  시점" 문제를 해소했다. Health와 MaxHealth를 각각 구독한다(최대체력 변동 대응).
- **Widget Role = Leaf.** 채움 표현이 이 위젯의 최빈 변화인데, 자식 위젯
  조립(Compound)으로 구현하면 크기 변경(Layout급 무효화)이 되는 반면 직접
  그리기는 Paint급으로 끝난다. 가장 빈번한 변화를 가장 싼 무효화 등급에
  고정하는 선택이며, 엔진의 `SProgressBar`가 Leaf인 것과 같은 근거다.
- **런타임 틴트는 `TSlateAttribute`.** 값 변경 시 등록된 등급(Paint)의
  무효화를 자동 발행하고, 동일 값 재설정은 자동 무시된다(수동
  `Invalidate` 호출 및 무변화 필터를 프레임워크 계약으로 대체). 채움/배경
  틴트를 외부에서 스왑할 수 있어, 후속 기획(보스 줄 체력 — 줄마다 채움/배경
  색 페어 교체)에 위젯 수정 없이 대응한다. 위젯은 "퍼센트 1개 + 색 2개"만
  알며, 체력→줄 매핑은 위젯 밖 게임 로직 소관으로 분리했다.
- **피격 잔상은 ActiveTimer.** 상시 Tick 대신, 피격 시 등록 → 잔상이 현재
  체력에 도달하면 `Stop` 반환으로 해제되는 유한 수명 타이머를 사용했다.
  애니메이션 구간에만 프레임 갱신이 존재하고 평시에는 완전 휴면한다.
  지연/감쇠 속도는 스타일 파라미터로 노출해 위젯 코드 무접촉 튜닝이 가능하다.
- **생명주기 안전.** SWidget은 GC 추적 밖이므로 ASC 참조는
  `TWeakObjectPtr`로 보관하고, 구독은 `AddSP`(약참조 자동 무효화) +
  `FDelegateHandle` 보관 + 소멸자/`UnbindFromASC()` 대칭 해제로 관리한다.
  Unbind→재바인딩이 멱등이라 위젯 풀에서의 대상 교체(Stage 2 몬스터 패널)에
  그대로 재사용된다.
- **HUD의 체력 표시 채널 교정.** 기존 표시는 전투 이벤트 채널(Gameplay
  Message)로 상태를 갱신해, 데미지 외 경로(힐·버프)의 변경이 누락되는 틈이
  있었다. 상태는 상태 채널(Attribute 델리게이트), 사건 연출은 이벤트
  채널(GMS)로 역할을 재배정했다 — 후자는 Stage 4 데미지 폰트가 사용한다.