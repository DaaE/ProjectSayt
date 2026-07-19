# 리네이밍 + 주석 스윕 작업 지시서

> **[완료] 2026-07-19 작업 완료.** 이 문서는 실행 지시서가 아니라 이력 참조용입니다.
> 문서 내 `Sayu`/`ProjectGAS` 표기는 리네임 **대상**으로서 의도적으로 원문 유지합니다.
>
> **실제 결과와 다른 부분:**
> - 2단계에 `Binaries/` 삭제가 누락되어 있었음 — 실제로는 `Intermediate/`와 함께 삭제 필요
> - 2단계의 "구 CoreRedirects는 3단계까지 유지" 판단은 틀렸음 — 구 리다이렉트의
>   `NewName`이 죽은 경로가 되면서 `GE_ComboWindow`, `GE_Cooldown_BasicAttack` 등
>   구버전 에셋이 부모 클래스를 못 찾음. 3단계 진입 시 `NewName`을 최종 목적지로
>   갱신해야 했음(중간 경유 없이 직접 점프)
> - 3단계 리다이렉트 `OldName`의 모듈 경로: 구 모듈(`/Script/ProjectGAS`) 기준과
>   신 모듈(`/Script/ProjectSayt`) 기준 **양쪽 모두 등록**해야 안전했음
> - 3단계 클래스 목록에 `UENUM`(`ESayuPlacementHighlight`)과
>   비-USTRUCT 구조체(`FSayuDamageStatics`)가 누락되어 있었음
> - `Config/DefaultGame.ini`의 `PrimaryAssetTypesToScan` / `AssetBaseClass`,
>   `USaytGameDataSettings::CategoryName` 문자열이 지시 목록에서 빠져 있었음
> - 4단계(주석 스윕)는 연기 — 후반 Phase에서 코드 재방문 시 점진 정리로 전환
>
> 관련: `Docs/BUILD_TROUBLESHOOTING.md`(빌드 환경 문제)

> 실행 모델: 이 문서는 단독 사양서다. 세션 시작 시 이 문서와 `CONVENTIONS.md`를 읽고,
> 아래 단계 순서대로 진행하며 각 단계 끝의 검증을 통과한 뒤에만 다음 단계로 간다.
> 결정이 필요한 모호한 지점이 나오면 임의 판단하지 말고 DaaE에게 질문한다.

**목표**: 로컬 폴더/repo/프로젝트/모듈 `ProjectGAS` → `ProjectSayt`,
클래스 접두어 `Sayu` → `Sayt`, 학습 주석 스윕. 완료 후 ProjectGAS/Sayu라는 이름은
git 이력에만 남는다(이력은 보존 — 리셋하지 않기로 확정됨).

**불변 규칙**
- `Plugins/` 하위(GameplayMessageRouter 등 Epic 저작 코드)는 어떤 치환·헤더 수정에서도 제외
- Epic 기본 헤더가 남은 8개 스텁 파일(Target.cs 2, Build.cs 2, 모듈 진입점 4)의 저작권 줄은 유지
- 커밋: Conventional Commits, 접두어 영문 + 본문 한글, 본문에 큰따옴표 금지
- 단계별로 커밋 분리 (문제 발생 시 원인 격리)

---

## 0단계 — 준비

- 에디터 완전 종료. 작업 브랜치 생성: `git checkout -b rename-sayt`
- `Saved/`의 개발용 세이브 파일 삭제 예정임을 인지 (구 클래스명 참조)

## 1단계 — 로컬 폴더 + GitHub repo (5분)

1. GitHub Settings에서 repo명 `ProjectGAS` → `ProjectSayt` (자동 리다이렉트됨)
2. 로컬 폴더명 변경 후 `git remote set-url origin https://github.com/DaaE/ProjectSayt.git`
3. 검증: `git fetch` 정상 동작

## 2단계 — 프로젝트/모듈명 (ProjectGAS → ProjectSayt, SayuEditor → SaytEditor)

1. 파일/폴더 개명:
   - `ProjectGAS.uproject` → `ProjectSayt.uproject` (내부 `"Name"`, Modules 항목도 수정)
   - `Source/ProjectGAS/` → `Source/ProjectSayt/`, `Source/SayuEditor/` → `Source/SaytEditor/`
   - `Source/ProjectGAS.Target.cs` → `ProjectSayt.Target.cs`,
     `ProjectGASEditor.Target.cs` → `ProjectSaytEditor.Target.cs` (내부 클래스명·모듈 문자열 포함)
   - 각 `*.Build.cs` 파일명·클래스명·모듈 참조 수정
   - 모듈 진입점: `ProjectGAS.h/.cpp` → `ProjectSayt.h/.cpp`
     (`FProjectGASModule` → `FProjectSaytModule`, `IMPLEMENT_PRIMARY_GAME_MODULE` 인자,
     `SayuEditor.h/.cpp` → `SaytEditor.h/.cpp`, `FSayuEditorModule` → `FSaytEditorModule`)
2. API 매크로 전수 치환 (Source/ 스코프): `PROJECTGAS_API` → `PROJECTSAYT_API`,
   `SAYUEDITOR_API` → `SAYTEDITOR_API`
3. **Config 섹션 헤더 수정 (필수, 누락 시 설정값 유실)**: `Config/*.ini`에서
   `/Script/ProjectGAS.` 로 시작하는 모든 섹션명을 `/Script/ProjectSayt.` + 새 클래스명으로 —
   특히 `[/Script/ProjectGAS.SayuGameDataSettings]` (DataTable 경로가 여기 저장됨)
4. Core Redirects 등록 — `Config/DefaultEngine.ini`:
   ```ini
   [CoreRedirects]
   +PackageRedirects=(OldName="/Script/ProjectGAS", NewName="/Script/ProjectSayt", MatchSubstring=false)
   +PackageRedirects=(OldName="/Script/SayuEditor", NewName="/Script/SaytEditor", MatchSubstring=false)
   ```
5. `Intermediate/`, `Binaries/`, `Saved/`, `.idea/`(캐시) 삭제 → 프로젝트 파일 재생성 → 전체 리빌드
6. 검증: 에디터 기동, PIE에서 전투/인벤토리/세이브 정상. 커밋:
   `refactor: 프로젝트 및 모듈명을 ProjectSayt로 변경 - Core Redirects 등록`

## 3단계 — 클래스 접두어 (Sayu → Sayt) + 에셋

1. C++ 리네이밍은 Rider의 Rename 리팩터링(Shift+F6)으로 클래스 단위 실행 (파일명 동반 변경).
   Slate/비-UObject 타입(`SSayu*`, `FSayuStyle`, `FSayuTuningModel`)은 리다이렉트 불필요 —
   코드 치환만으로 완결.
2. UObject 타입은 아래 전수 목록대로 ClassRedirects/StructRedirects 등록
   (2단계의 PackageRedirects와 별개로 클래스명 자체가 바뀌므로 필요):
   ```ini
   ; 게임 모듈 — 클래스 (OldName은 구 모듈 경로 기준)
   +ClassRedirects=(OldName="/Script/ProjectGAS.SayuCharacter", NewName="/Script/ProjectSayt.SaytCharacter")
   ```
   위 형식으로 다음 전부: SayuCharacterBase, SayuCharacter, SayuNPCCharacter,
   SayuCombatImpactListener, SayuEffectPoolManager, SayuPooledEffect,
   SayuGameplayAbility, SayuAbility_BasicAttack, SayuAbility_BuffSelf,
   SayuAttributeSet_Combat, SayuExecCalc_Damage, SayuMMC_Damage, SayuGameplayEffect,
   SayuNotifyState_GASEvent, SayuNotify_GASEvent, SayuGameDataSettings,
   SayuGameDataSubsystem, SayuItemDefinition, SayuItemFragment,
   SayuItemFragment_EquipEffect, SayuItemInstance, SayuItemIdSubsystem,
   SayuItemDragDropOperation, SayuInventoryComponent, SayuSaveGame,
   SayuUIInputModeSubsystem, SayuHUDWidget, SayuInventoryWidget,
   SayuInventoryItemWidget, SayuInventorySlotWidget, SayuHealthBarWidgetStyle
   ```ini
   ; 에디터 모듈 — 클래스
   +ClassRedirects=(OldName="/Script/SayuEditor.SayuAssetAuditRowObject", NewName="/Script/SaytEditor.SaytAssetAuditRowObject")
   +ClassRedirects=(OldName="/Script/SayuEditor.SayuAssetManagerInspectorWidget", NewName="/Script/SaytEditor.SaytAssetManagerInspectorWidget")
   ; 구조체
   +StructRedirects=(OldName="/Script/ProjectGAS.SayuCombatStatsRow", NewName="/Script/ProjectSayt.SaytCombatStatsRow")
   +StructRedirects=(OldName="/Script/ProjectGAS.SayuDamageMessage", NewName="/Script/ProjectSayt.SaytDamageMessage")
   +StructRedirects=(OldName="/Script/ProjectGAS.SayuInventoryEntry", NewName="/Script/ProjectSayt.SaytInventoryEntry")
   +StructRedirects=(OldName="/Script/ProjectGAS.SayuHealthBarStyle", NewName="/Script/ProjectSayt.SaytHealthBarStyle")
   ```
3. 로그 카테고리·스타일 등록명 등 문자열 식별자: `LogSayu*` → `LogSayt*`,
   FSayuStyle의 `"Sayu.HealthBar"` 등 등록 키, 콘솔 커맨드 `Sayu.HealthBar.*` → `Sayt.HealthBar.*`
   (문자열이라 리다이렉트 밖 — grep으로 전수 확인)
4. Content 에셋: `BP_Sayu*`/`WBP_*` 등 에셋명 개명은 에디터에서 → 우클릭 Fix Up Redirectors.
   `DT_SayuCombatStats` 개명 시 GameDataSettings의 테이블 경로 갱신 확인
5. 전체 리빌드 + 에디터 기동. 검증(아래 체크리스트) 통과 후:
   **File > Save All(전 에셋 재저장)** → 재검증 → 커밋:
   `refactor: 클래스 접두어를 Sayt로 통일 - 에셋 리다이렉트 및 재저장 포함`
6. 리다이렉트 제거는 즉시 하지 않는다 — 한 Stage 정도 유지 후(재저장 누락 안전망),
   별도 커밋으로 `[CoreRedirects]` 섹션 삭제 + 구 세이브 파일 삭제

### 3단계 검증 체크리스트 (전부 통과 필수)
- [ ] **모든 GameplayEffect 에셋을 열어 Modifier의 Attribute 슬롯 확인** — GAS의
      Attribute 직렬화는 개명에 취약한 알려진 부위. None으로 비면 데미지가 조용히 0이 됨
- [ ] `DT_SayuCombatStats`(개명 후 이름)의 RowStruct가 새 구조체를 가리키는지
- [ ] WBP_HUD·인벤토리 위젯 BP의 부모 클래스/BindWidget 정상
- [ ] Enhanced Input 에셋과 캐릭터 BP의 액션 참조 정상
- [ ] PIE: 콤보 공격·데미지·체력바·고스트·인벤토리 드래그·F5/F9 세이브(새 파일로) 동작
- [ ] Output Log에 redirect 실패/Unknown class 경고 0건
- [ ] `grep -rn "Sayu\|ProjectGAS" Source/ Config/` 잔존 0건 (Docs·주석 제외 단계에서 처리)

## 4단계 — 학습 주석 스윕 (Source/ 전체)

분류 기준 — 판단이 갈리면 유지 쪽으로:
- **유지**: 설계 의도·trade-off·불변식·함정 설명
  (예: Base 클램프의 잠식 버그 설명, 그리드 축소 금지 사유, 초기화 순서 주의)
- **제거**: 다른 언어/엔진 비교("C#의 ~와 같음", "Unity로 치면"), 엔진 기초 문법
  설명("#pragma once란", "UPROPERTY 각 지정자 설명", "GENERATED_BODY는 첫 줄에"),
  구어체 튜토리얼 문장("~예요/~해요" 체 설명문), 자동 생성 잔여 주석
  ("Sets default values...", "Called when the game starts...")
- 제거 전 원문 보존이 필요하면 git 이력으로 충분 — 별도 이관 불필요
- 스윕 후 남은 주석은 문어체로 통일

커밋: `docs: 학습용 주석 스윕 - 설계 의도 주석만 유지`

## 5단계 — Docs/README 스윕

- `Docs/*.md`, `README.md`의 Sayu/ProjectGAS 표기를 Sayt/ProjectSayt로 갱신
  (역사 서술로 남길 곳 — 예: "코드네임 변천" — 은 의도적으로 유지 가능, DaaE 확인)
- 커밋 후 `rename-sayt` 브랜치를 main에 머지

## 완료 정의
빌드 클린 + 검증 체크리스트 전항 통과 + grep 잔존 0(의도 유지분 제외) +
PIE 스모크 테스트 정상. 이후 새 대화창에서 진행 상황을 이 문서 기준으로 보고.
