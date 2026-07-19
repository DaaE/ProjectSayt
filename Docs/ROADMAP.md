# Sayt — 개발 로드맵

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
| 9 | 프로파일링/최적화 — Unreal Insights, Slate Insights, 메모리 측정(Phase 7 EUW 재사용), **렌더링 축: Nanite/Lumen 동작 이해·측정·적용 판단 문서화** | 대기 |
| 10 | Replicated GAS — Listen Server, 클라이언트 예측(PredictionKey 검증), **넷코드 이론 비교(딜레이/롤백/데드레커닝), lag compensation, 서버 권위·안티치트 원칙, ASC 위치(PlayerState 이전) 검토** | 대기 |
| 11 | AI/보스 — StateTree, Dynamic NavMesh(NavModifier), 보스 페이즈, **캐릭터 상태 모델링(태그 상태 계층·사망·피격 경직·힐 차단, 세 층위 이론: 태그/StateTree/AnimBP SM), MaxHealth 변동 시 Health 재클램프**, DT 확장·CSV→JSON 재평가, FDataTableRowHandle 재평가, **로코모션·타격감(프레젠테이션 코어 당김분)** | 대기 |
| 12 | 포트폴리오 마무리 — README/아키텍처 다이어그램, 영상, 디버그 코드 정리(콘솔 커맨드·디버그 입력 일괄), 설계 Q&A 문서, **전체 모의 면접 세션**, **CI/CD 소형 편입(GitHub Actions + self-hosted runner 빌드 자동화, 선택이나 기본 포함)** | 대기 |
| 13 (선택) | Mass Entity(ECS 대규모 군중) — 착수 시점에 당시 엔진 버전 기준 공식 문서 재벤치마크 후 커리큘럼 확정. 지원 시작선 이후, 심화 백로그와 병렬 | 선택 |

## 로드맵 재편 세션 결과 (2026-07-18 확정)

### 코드베이스 감사 판정 (Phase 0~7 전 소스)

재작성·삭제 0건. 아키텍처는 현행 표준(Lyra·공식 문서) 대조를 통과.

| 시스템 | 판정 | 비고 |
|---|---|---|
| 캐릭터/GAS 코어 | 유지 | 합성 AttributeSet, 멱등 초기화, 단일 인스턴스 콤보, 표준 ExecCalc 패턴. Replicated props/OnRep 기반 기설치 |
| 이펙트 풀링 | 리팩터(중) | 레벨 배치 액터 → `UWorldSubsystem`화. 실행 시점: **Stage 4**(풀링 재방문 = 재학습 트리거) |
| GMS 연동 | 유지 | 리스너의 풀 수동 배선만 풀링 리팩터에 편승 정리 |
| 데이터 계층 | 유지 | 중앙화 FName 조회 유지(Phase 11 재평가). SaveGame은 소비자 생길 때 확장 |
| 인벤토리 | 유지 | Model/View 분리·footprint 판정 견고 |
| UI | 유지 + 리팩터(소) | 입력 모드 권한 이원화 해소: Alt 홀드 경로를 `PushUIRequest("AltHold")` 경유로 통일 |
| 에디터 모듈 | 유지 | Phase 9 측정 도구로 재사용 |
| 인풋 | 유지 | 디버그 액션 정리는 Phase 12 |

- 소액 수리: CharacterBase 상시 틱 → 디버그 활성 시에만 틱 온 (Stage 2 전)
- 크로스커팅: **학습 주석 스윕** — 설계 의도 주석은 유지, 언어 비교/튜토리얼 주석은 제거.
  리네이밍과 동일 작업 창구에서 실행 (`Docs/RENAME_PLAN.md` 참조)
- repo 전략: **단일 repo 유지 + 이력 보존(리셋 안 함)** — 촘촘한 실커밋 이력이
  AI 시대에 "완성본 투척"과 구분되는 프로세스 증명이라는 판단

### 프레젠테이션 코어 확정 스코프

지원 시작선 = **Phase 12 완료 + 아래 4묶음**. 미흡하면 지원보다 폴리싱이 우선.

1. **Aurora 스킬킷 4종 + FX 풀연동** — 원 키트 재현(기본 공격 + 액티브 4).
   시간 딜레이션 연출(빙결/슬로우 필드 = `CustomTimeDilation`+GE, 스피드업 자기 버프) 내장
2. **타격감 최소 패스 3종** — 히트스톱(시간 딜레이션 공유 기술) + 카메라 셰이크 + 히트 플래시.
   넉백/랙돌 등 확장은 백로그
3. **로코모션 + 팔로우 캠** — AnimBP State Machine(Idle/Run 블렌드, 점프),
   이동 방향 회전 정렬, 캠 감도 튜닝. 대시는 스킬킷 이동기로 대체. 고급 트래버설 스코프 밖
4. **HUD 폴리시** — Phase 8 산출물 통합 마감

실행 창구: 1은 전용 구간(스킬킷), 2·3은 Phase 11~12에 편성.

### 기간 산정 (v2 — Stage 1은 특수 일정으로 예외 처리, 스테이지당 1주 기준)

| 구간 | 견적 |
|---|---|
| Phase 8 잔여 (Stage 2~5) | 3~4주 |
| Phase 9 | 1주 |
| Phase 10 | 2주 |
| Phase 11 (+상태 모델링·로코모션·타격감) | 2~3주 |
| Phase 12 (+CI/CD 소형) | 1~2주 |
| 프레젠테이션 코어 잔여 (스킬킷 4종+FX, HUD) | 2~3주 |
| (병렬) 리네이밍+주석 스윕 | 1일, 크리티컬 패스 밖 |

**합계 11~15주 → 지원 시작선 2026-10초~11초.**
재보정 규칙: Stage 2 종료 시 실측으로 1차 재보정, 이후 매 Phase 종료마다 갱신.
일정 압박을 이유로 학습 루프(개념 이론→실습 검증→문답)를 압축하지 않는다.

## Phase 8 상세 — Slate UI 심화

| Stage | 내용 | 상태 |
|---|---|---|
| 0 | Live-tuning 워밍업 — 직접 마우스 입력(FReply), TAttribute Pull | ✅ |
| 1 | SHealthBar 코어 — GAS Push 델리게이트, TSlateAttribute, 고스트 바(ActiveTimer), 스타일셋 | ✅ |
| 2 | Boss/Monster HP — 줄(라인) 체력 보스 바, SActorCanvas형 월드 패널 | 대기 |
| 3 | 버프/디버프 트레이 — STileView, OnPaint 라디얼 쿨다운 (UMG 래핑 필수) | 대기 |
| 4 | 데미지 폰트 — SWidget 오브젝트 풀 + FCurveSequence (+풀링 `UWorldSubsystem` 리팩터) | 대기 |
| 5 | 스킬 슬롯 바 — 데이터 기반 슬롯 구성 (UMG 래핑 필수) | 대기 |

**전 Stage 공통 체크리스트**
1. 데이터 바인딩 경계 명시 — 소스가 통지를 제공하면 Push(델리게이트), 아니면 Pull(TAttribute)
2. Widget Role 선택 근거 — Leaf / Compound / Panel
3. Invalidation 전략 — 신고 등급 명시, SlateDebugger/stat slate로 검증
4. 비주얼 폴리시 패스 필수 — 회색 박스 상태로 종료 금지
5. 측정 증거 — before/after 캡처 또는 stat 수치를 Docs/Media에 보존

## Stage 1 산출물 — SSaytHealthBar

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
