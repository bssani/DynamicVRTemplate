# S6 진행자 창 — 따로 뜨는 PC 창(버튼 = 키와 같은 명령, 상태 표시) + 에디터 툴바 버튼

| 항목 | 값 |
|---|---|
| 단계 | 계획서 §6 S6. 시험 T-06 |
| 결정 근거 | 계획서 D-06(헤드셋 글 없음), D-08(vBoR P5i 방식의 따로 뜨는 OS 창 + 에디터 툴바 버튼 P5j, 버튼 = 키와 같은 명령), §5(창 내용, 사용자 2026-11-24 Q-03: 속도 없음, Recenter 없음), Q-06(번호 직접 선택은 진행자 창), D-13(Reset) |
| 베이스 브랜치 | **S5가 `main`에 머지된 뒤의 `main`** (S4 정렬 다시 적용, S5 공용 팔레트를 쓴다) |
| 작업 브랜치 | `codex/S6-operator-window` |
| 리뷰 파일 | `Docs/Reviews/S6-operator-window.md` |
| 에디터 작업 | 없음(C++ Slate, 에디터 모듈, SVG 아이콘 하나) |
| 상태 | 명세 — 구현 전(미리 씀, 2026-11-24). S4·S5 머지 뒤 줄 번호를 다시 맞춘다 |

## 0. 확인한 사실

창은 vBoR P5i(`C:\Git\vBoR\Docs\Specs\P5i-operator-panel.md`, 구현 `Source/vBoR/Interaction/VBoRScenarioOperator.cpp`·`VBoROperatorPanel.h`), 툴바는 P5j(`Docs/Specs/P5j-operator-toolbar.md`, 구현 `Source/vBoREditorTools/Private/VBoROperatorToolbar.cpp`)를 따른다. 둘 다 UE 5.7에서 기기·에디터로 확인됐다. **다른 점이 하나 있다: vBoR에서는 진행자가 누르는 동안 참가자가 운전하지 않았다. 여기서는 참가자가 벅 휠·페달로 운전하는 중에 진행자가 창을 누른다.**

- **벅 휠·페달 입력은 Slate 초점을 따라간다.** RawInput 플러그인은 축·버튼을 `FSlateApplication::OnControllerAnalog`·`OnControllerButtonPressed`로 넘긴다(엔진 `Engine/Plugins/Experimental/RawInput/Source/RawInput/Private/Windows/RawInputWindows.cpp:1017-1037`). Slate는 그 입력을 **그 사용자의 초점 경로**로만 보낸다(엔진 `Runtime/Slate/Private/Framework/Application/SlateApplication.cpp:5073-5082` `ProcessAnalogInputEvent`, `:6547-6584`). 진행자 창이 초점을 가지면 게임 뷰포트가 휠·페달 값을 받지 못한다 — **운전이 끊긴다.** 키보드 단축키도 같은 이유로 게임에 안 간다.
- **RawInput은 등록할 때의 활성 창 하나에만 붙는다.** `hwndTarget` = 그때 활성 top-level 창, 플래그 0(`RIDEV_INPUTSINK` 없음)이다(같은 파일 `:100`, `:129-158`). 다른 창이 앞에 오면 Windows가 `WM_INPUT`을 보내지 않을 수 있다(확인 안 됨 — §8 T-06에서 본다).
- **창을 누르면 Windows가 그 창을 활성화한다.** Slate는 `WM_MOUSEACTIVATE`에 0(활성화함)을 돌려준다(엔진 `Runtime/ApplicationCore/Private/Windows/WindowsApplication.cpp:3756-3763`). `EWindowActivationPolicy::Never`는 처음 보일 때만 막는다(`WindowsWindow.cpp:293`, `:721-722`). 그래서 "누른 뒤 초점·활성을 게임 창으로 되돌림"이 필요하다(D2).
- **창 API:** `FSlateApplication::AddWindow`(`Runtime/Slate/Public/Framework/Application/SlateApplication.h:1576`), `SetAllUserFocusToGameViewport`(`:650`, 구현 `SlateApplication.cpp:2625`). `SWindow`(`Runtime/SlateCore/Public/Widgets/SWindow.h`): `Title` `:287`, `ScreenPosition`·`ClientSize` `:297`·`:300`, `FocusWhenFirstShown` `:327`, `ActivationPolicy` `:333`, `HasCloseButton` `:339`, `SupportsMaximize` `:342`, `BringToFront` `:575`, `IsWindowMinimized` `:758`, `Restore` `:928`. `SButton`의 `IsFocusable`(`Runtime/Slate/Public/Widgets/Input/SButton.h:141`, 기본 참 `:62`).
- **창을 만들지 않을 때:** `FApp::CanEverRender()` 거짓(`-nullrhi`), `GIsAutomationTesting` 참. **`-nullrhi` 자동화에서도 Slate는 켜져 있어 `FSlateApplication::IsInitialized()`로는 못 막는다**(vBoR P5i D1).
- **명령(S1·S3, 지금 코드):** 키 → `ADvtPlayerController`의 private 함수(`Drive/DvtPlayerController.cpp:92-96` 연결, `:119-122` 차·hood, `:124` `ResetVehicle`) → 차량 명령(`Drive/DvtVehicle.h:39-50`). 받을지 판정은 `DvtSelection::FSelection`(`Cars/DvtSelectionLogic.h` `SelectCar`·`SelectHood`·`StepCar`·`StepHood` — 범위 밖·같은 번호·1개 이하·쿨다운이면 false)과 `DvtDrive::FResetSequence::Request`(`Drive/DvtDriveLogic.h:37-44`, 진행 중이면 false) **안에 섞여 있다** — 창이 버튼을 흐리게 하려면 같은 판정을 따로 읽을 수 있어야 한다(D4).
- **정렬(S4, 키트 `e915647`):** `UVRCalAlignmentComponent`의 `State`(`Applied`/`NoSavedValue`/`NeedsCheck`), `Reason`, `IsFindingAnchor()`, `RequestReapply()`(키트 `VRCalAlignmentComponent.cpp:194-199` — 앵커를 찾는 중이면 false, 아니면 저장값을 다시 읽고 true). S4 D4가 `F9` → controller → `RequestReapply`를 만든다.
- **손(S2):** `UDvtSteeringGripComponent::GetGrip(Hand)`(`Grip/DvtSteeringGripComponent.h:46`). **추적 여부를 읽는 함수는 없다** — Tick 안 지역 값이다(`DvtSteeringGripComponent.cpp:114-133`).
- **팔레트(S5 D7):** S5가 공용 헤더(예: `UI/DvtUiStyle.h`)를 만든다.

## 1. 결정

| # | 결정 | 내용 |
|---|---|---|
| D1 | **따로 뜨는 OS 창, 월드 서브시스템이 가짐** | `UDvtOperatorSubsystem`(`UTickableWorldSubsystem`, Game·PIE만). 월드의 첫 PlayerController가 `ADvtPlayerController`이고 차량이 연결돼 있으면 창을 만든다(주행 레벨 `L_DriveTest` — `L_Calibrate`는 엔진 `APlayerController`라 만들지 않음, S4 D5). 준비될 때까지 Tick에서 다시 본다. 제목 `DynamicVRTemplate Operator`. **닫기 버튼 없음**(실수로 닫으면 버튼이 사라짐), 최소화 됨, 최대화 안 됨. 월드가 끝나면(`Deinitialize`) 닫는다. **같은 에디터 세션 안에서는 자리·크기를 기억해 다음 Play에 같은 자리에 띄운다**(닫기 전에 읽어 정적 값에 둠). 처음은 client 560 × 860. 내용은 세로 스크롤 상자 안(창을 줄여도 잘리지 않게). **보이게 하는 공개 함수 `ShowOperatorWindow()`**(`DYNAMICVRTEMPLATE_API`): 없으면 만들고(조건이 맞을 때), 최소화면 풀고, 앞으로. Alt+F4 등으로 OS가 닫았으면 다시 만든다. D7 툴바가 이것만 부른다. **만들지 않는 조건:** §0(`CanEverRender` 거짓, `GIsAutomationTesting` 참) — `ShowOperatorWindow()`를 불러도 만들지 않음 |
| D2 | **창은 초점을 가져가지 않는다 — 운전이 끊기지 않게** | §0 때문에 넷을 모두 한다. ① 창: `ActivationPolicy(Never)`, `FocusWhenFirstShown(false)`. ② 모든 버튼·스크롤 상자: 초점을 받지 않음(`IsFocusable(false)` 등). ③ 버튼을 누르면 명령을 넣은 **직후 `FSlateApplication::SetAllUserFocusToGameViewport()`**로 초점을 게임으로 되돌리고, 게임 뷰포트가 든 창을 앞으로(활성) 되돌린다. ④ 창을 처음 띄울 때와 `ShowOperatorWindow()` 뒤에도 ③의 되돌림을 한다. **그래서 창은 키를 받지 않는다** — 단축키는 늘 게임 창이 받는다(vBoR P5i D5의 "창이 키를 받아 넘김"은 하지 않는다. 같은 키가 두 번 들어갈 일도 없다). 되돌림은 시험할 수 있게 한 함수로 둔다(자동화에서는 Slate 호출 대신 부른 횟수를 본다) |
| D3 | **버튼 = 키와 같은 명령** | controller에 공개 명령 함수를 두고 **키와 버튼이 같은 함수를 부른다**: `PreviousCar`·`NextCar`·`PreviousHood`·`NextHood`(지금 private → public), 새 `SelectCar(int32)`·`SelectHood(int32)`(차량 명령을 그대로 부름), `ResetVehicle`(이미 public), S4의 정렬 다시 적용 함수. 버튼으로만 되는 동작은 번호 직접 선택(Q-06 — 키가 없음)뿐이고, 이것도 차량의 S3 절대 명령 그대로다(쿨다운에 막히지 않고 쿨다운을 시작). 버튼 이름 옆에 키를 적는다(§1-1) |
| D4 | **누를 수 없는 버튼은 흐리게 — 판정은 명령과 같은 함수** | 명령이 버릴 버튼은 흐리게 하고 눌리지 않게 한다. **판정을 명령 안에서 꺼내 명령과 창이 같이 쓴다**(둘로 갈리면 창은 눌리는데 명령이 버리거나 그 반대가 생김): `FSelection`에 const `CanSelectCar(i)`·`CanSelectHood(i)`·`CanStepCar(Now, Cooldown)`·`CanStepHood(Now, Cooldown)`를 더하고 `SelectCar`… 가 먼저 그것을 부르게 바꾼다(동작 같음 — S3 시험 그대로 통과). `FResetSequence`에 const `CanRequest()`. 정렬 다시 적용은 정렬 component가 있고 `!IsFindingAnchor()`일 때(키트 `RequestReapply`와 같은 조건 — 키트는 고치지 않는다). 각 판정은 버튼 툴팁에 쓸 영어 이유도 돌려준다(§1-1 표) |
| D5 | **상태 표시(영어)** | 계획서 §5. ① 차: `<이름> (k/N)`, ② hood: `<이름> (k/N)`, hood 없으면 `No hoods`. 이름이 비면 S5 D5와 같이 `Car k`·`Hood k`. ③ 정렬: `Applied` / `Needs check — <Reason>` / `No saved alignment`, 앵커를 찾는 중이면 뒤에 ` · Finding anchor…`. 정렬 component가 없으면 `No alignment in this level`. `Reason`은 키트 문자열 그대로, 넘치면 말줄임(툴팁에 전체). ④ 손: 왼손·오른손마다 `Tracked` / `Not tracked` / `Hand tracking off`(추적기 없음·무효) 와 `On wheel` / `Off wheel`(S2 `GetGrip` — 추적이 끊겨도 잡은 상태 유지가 그대로 보임). **넣지 않음:** 속도, Recenter, 헤드셋으로 보내는 글(계획서 §5) |
| D6 | **손 추적 읽기 함수(S2에 읽기만 더함)** | `UDvtSteeringGripComponent`에 마지막 표본을 기억하는 값과 `IsHandTracked(Hand)`·`IsHandTrackingAvailable()` 같은 const 읽기 함수를 더한다. 값은 지금 Tick이 이미 구한 `TrackerValid`·`Tracked`를 `ApplyHandSample`에서 저장만 한다. 잡기 규칙·보이기·로그·이벤트는 바꾸지 않는다 |
| D7 | **에디터 툴바 버튼 = vBoR P5j** | 새 에디터 전용 모듈 `DynamicVRTemplateEditorTools`(`.uproject` `Modules`에 `"Type": "Editor"`. 에디터 타깃 이름 `DynamicVRTemplateEditor`와 헷갈리지 않게). 런타임 모듈은 이 모듈을 참조하지 않는다(게임 빌드에 안 들어감). `LevelEditor.LevelEditorToolBar.PlayToolBar`에 버튼 하나(`UToolMenus::RegisterStartupCallback` → `ExtendMenu` → `FToolMenuEntry::InitToolBarButton`, 모듈을 내릴 때 등록·스타일 해제). 이름표 `Operator`, 툴팁 `Show the operator window. Restores it if minimized, reopens it if closed.` 누르면 `GEditor->PlayWorld`의 `UDvtOperatorSubsystem::ShowOperatorWindow()`만 부른다(창을 따로 만들지 않음). **흐림:** Play 중이 아니면 `Start Play first.`, Play 중인데 진행자 창을 만들 수 없는 레벨(D1 조건 — 예: `L_Calibrate`)이면 `This level has no drive controller.` 판정은 월드를 받는 함수 하나. **아이콘:** vBoR `Resources/Editor/OperatorPanel.svg`(사용자 디자인, 20 × 20 슬라이더)를 이 리포 `Resources/Editor/OperatorWindow.svg`로 복사, 흰 선으로 두고 브러시 색 `#5b8cff`, `FSlateVectorImageBrush`로 스타일 등록. 엔진 줄은 vBoR P5j §8(같은 UE 5.7) |
| D8 | **새로 고침** | 상태·흐림은 적어도 초당 10회 다시 읽는다(진행자가 낡은 상태를 보지 않게). 쿨다운(S3, 0.5 s) 동안 이전·다음 버튼이 흐렸다가 켜지는 것은 정상이다 |
| D9 | **디자인 — vBoR P5i D9 팔레트·모양** | 색은 S5 공용 팔레트 + 창용 몇 개(vBoR P5i D9 hex): 바탕 `#0b0f17`, 패널 `#131a26`, 지금 선택·강조 바탕 `#1c2534`, 구분선 `#2e3a4e`, 보조 버튼 테두리 `#66748a`, 글자 `#eef2f7`, 보조 글자 `#9aa6b8`, 주 버튼 `#5b8cff`(위 글자 `#0b0f17`), 머리 띠 `#0d2a66`(위 글자 `#ffffff`), 정상 `#3cc285`, 확인 필요 `#f2b544`. 글꼴 Roboto `Regular`·`Bold`, **디자인 px × 0.75 = pt**. 버튼 높이 40 px, 모서리 6 px, 오른쪽에 키 표시. 모양 넷: 주(채움), 보조(테두리), 흐림(테두리 `#2e3a4e`, 바탕 `#0b0f17`, 글자 `#66748a`), 지금 선택(테두리 `#5b8cff`, 바탕 `#1c2534`, `Current` 꼬리표 — 눌리지 않음). **주 버튼은 하나뿐:** 정렬이 `NeedsCheck`이고 받을 수 있을 때의 `Reapply alignment`. 다른 때는 주 버튼 없음. 배치(위 → 아래): ① 머리 띠 — 차 이름·hood 이름(D5 ①②) ② `Car` 패널 — `Previous car`·`Next car`, 그 아래 차 목록(차마다 버튼 하나, 지금 차 = 지금 선택 모양) ③ `Hood` 패널 — 같은 모양 ④ `Alignment` 패널 — 상태 줄(정상·확인 필요 색 점), `Reapply alignment` ⑤ `Vehicle` 패널 — `Reset vehicle` ⑥ `Hands` 패널 — 두 줄 ⑦ 맨 아래 한 줄 `Shortcut keys work in the game window.` 그림 파일은 만들지 않는다(vBoR `Docs/Design/P5i-operator-panel/Main.dc.html`을 느낌의 참고로만) |

### 1-1. 버튼

| 패널 | 버튼(영어) | 키 | 명령 | 흐림(이유) |
|---|---|---|---|---|
| Car | `Previous car` | PageUp | `PreviousCar` | 차 1대 이하(`Only one car`), 쿨다운(`Switching…`) |
| Car | `Next car` | PageDown | `NextCar` | 같음 |
| Car | 차 이름마다 | — | `SelectCar(i)` | 지금 차(`Current` 모양) |
| Hood | `Previous hood` | `[` | `PreviousHood` | hood 1개 이하(`Only one hood`·`No hoods`), 쿨다운 |
| Hood | `Next hood` | `]` | `NextHood` | 같음 |
| Hood | hood 이름마다 | — | `SelectHood(i)` | 지금 hood(`Current` 모양) |
| Alignment | `Reapply alignment` | F9 | S4 정렬 다시 적용 | 정렬 component 없음(`No alignment in this level`), 앵커 찾는 중(`Finding anchor…`) |
| Vehicle | `Reset vehicle` | Backspace | `ResetVehicle` | Reset 페이드 중(`Reset in progress`) |

차가 없는 주행 레벨(`Cars` 빔)이면 Car·Hood 패널은 `No cars in this vehicle` 한 줄만 보인다.

## 2. 지켜야 할 조건

- 헤드셋에 아무 글도 없다(D-06). 창은 PC에만.
- **창을 누르는 동안에도 운전 입력이 끊기지 않는다**(D2).
- 키 동작은 그대로다(같은 함수를 부르게 옮길 뿐). S1–S5 시험과 단언은 바꾸지 않는다. S2·S3 코드는 D4·D6의 판정·읽기 함수만 더하고 동작을 바꾸지 않는다.
- 키트 파일을 고치지 않는다(S4 D1).
- 게임 타깃 빌드에 에디터 모듈이 들어가지 않는다.
- 자동화·헤드셋 없는 실행에서 창을 만들지 않고 경고·오류 없이 지나간다.
- Recenter 버튼 없음(D-13 — 정렬이 깨짐).

## 3. 범위 밖

| 하지 말 것 | 이유 |
|---|---|
| 속도·주행 값 표시 | 계획서 §5, D-12 |
| Recenter, 정렬 다시 보정(M → S) 버튼 | D-13. 다시 보정은 `L_Calibrate`(S4 §8) |
| `L_Calibrate`용 진행자 창 | 정렬 도구 키(M·G·S)와 Output Log로 한다(S4 D6) |
| 창에서 키 받기 | D2 — 창은 초점을 갖지 않는다 |
| 확인 대화상자 | 되돌릴 수 없는 버튼이 없다(Reset도 다시 누르면 됨) |
| 관전 화면 바꾸기 | S5 |
| UMG 에셋·디자인 html | C++ Slate |

## 4. 구현자가 정할 것

- 클래스·함수 이름(위 이름은 예), 판정 결과 구조체 모양(예: `{bool bAccepted; FText Reason;}`).
- 창 자리 기억 방법(정적 값 권장. 다른 세션까지는 선택).
- 게임 창을 앞으로 되돌리는 방법(D2 ③ — `SWindow::BringToFront(true)` 또는 `HACK_ForceToFront` 등, 결과가 같으면).
- 1–2 px 세부, 상태 문구의 정확한 영어(§1-1·D5에 적은 것은 그대로).

## 5. 에디터 작업

없음. SVG는 파일 복사.

## 6. 경계 상황과 시험 (자동화 `DynamicVRTemplate.S6.*` — 에디터 모듈 시험도 이 이름으로)

| # | 상황 | 기대 |
|---|---|---|
| E1 | 자동화(`-nullrhi`, Slate 켜짐)·`GIsAutomationTesting` | 창을 만들지 않음. `ShowOperatorWindow()`를 불러도 안 만듦. 경고·오류 없음 |
| E2 | 버튼마다 누름(창 없이 버튼 처리 함수를 부름) | 같은 키를 눌렀을 때와 같은 controller 함수가 같은 인자로 한 번 불림. 차량 결과(번호·쿨다운)가 키와 같음 |
| E3 | 판정 일치 — 여러 상태(차 0·1·3대, hood 0·1·2, 쿨다운 안·밖, Reset 진행 중·아님, 정렬 component 있음·없음·앵커 찾는 중)에서 모든 버튼 | "켜짐 ⇔ 지금 누르면 명령이 받음". 이유 문구가 §1-1과 같음 |
| E4 | 쿨다운 | 다음 차 뒤 0.4 s: 이전·다음 차 흐림, 차 목록은 켬(절대 명령) / 0.5 s: 켬 |
| E5 | 번호 직접 선택 | `SelectCar(2)` → 차 3번째, hood 번호는 S3 D6대로 유지. 지금 차 버튼은 `Current`, 흐림 |
| E6 | 상태 문구 | D5 ①–④가 각 상태에서 맞음(이름 빔 → `Car k`, hood 없음 → `No hoods`, `Needs check — <Reason>`, 앵커 찾는 중 꼬리, component 없음) |
| E7 | 손 | 추적기 없음 / 왼손 추적·오른손 끊김 / 잡음·놓음 → `Hand tracking off` / `Tracked`·`Not tracked` / `On wheel`·`Off wheel`. S2 시험(잡기 규칙·보이기)은 그대로 통과 |
| E8 | 초점 되돌림 | 버튼 처리 뒤 되돌림 함수가 한 번 불림(명령이 버려져도 — 흐린 버튼은 눌리지 않지만 처리 함수 직접 호출로 확인). 창 설정 값: `ActivationPolicy` Never, `FocusWhenFirstShown` 거짓, 버튼 초점 안 받음 |
| E9 | 창 자리 기억 | 닫을 때 자리·크기를 저장하고 다음 생성 인자에 씀(창 없이 저장값으로) |
| E10 | 시작 조건 | 첫 PlayerController가 `ADvtPlayerController` + 차량 → 만들 수 있음 / 엔진 `APlayerController`(정렬 레벨) → 안 됨 / 차량 없음 → 안 됨 |
| E11 | 버튼 모양 | 각 상태에서 고른 모양(주·보조·흐림·지금 선택)이 D9와 같음. 주 버튼은 0개 또는 1개(`NeedsCheck`일 때 `Reapply alignment`만) |
| E12 | 기본 색 | D9 hex를 sRGB로 바꾼 값과 같음(S5 공용 값 재사용) |
| E13 | 문구 폭 | 버튼 이름·키·상태 문구가 기본 창 폭(560 px) 패널 안에 한 줄로 들어감. 긴 `Reason`·24자 넘는 이름은 말줄임(Slate 글꼴 측정) |
| E14 | 툴바 — Play 안 함 / Play 중 주행 레벨 / Play 중 진행자 창을 못 만드는 레벨 | 흐림 `Start Play first.` / 켬, 누르면 `ShowOperatorWindow` 한 번 / 흐림 `This level has no drive controller.` |
| E15 | 툴바 등록·아이콘 | Play 툴바에 항목 한 번만(모듈 다시 시작해도), SVG 파일이 있고 스타일이 브러시를 찾음 |
| E16 | 기존 시험 | `DynamicVRTemplate.*`·`VRCalibrationKit.*` 모두 통과, 기존 단언 그대로 |

## 7. 완료 조건

- non-unity(`-DisableUnity -Module=DynamicVRTemplate`, `-Module=DynamicVRTemplateEditorTools`)와 unity 빌드 성공, 경고·오류 0. 게임 타깃(`DynamicVRTemplate` Win64 Development)도 한 번 빌드해 에디터 모듈이 안 들어가는지 본다.
- `DynamicVRTemplate.*`와 `VRCalibrationKit.*` 전체 통과(시험 전 `Config/UserEngine.ini` 없음).
- 헤드셋 없는 PIE `L_DriveTest`: 창이 뜨고(렌더링되는 에디터라 만들어짐), 버튼으로 차·hood·Reset이 되고, **누른 뒤 키보드 주행(개발용 키)이 바로 계속되는지** 본다. 스크린샷 하나(`Saved/Screenshots/` — 커밋하지 않음)와 결과를 보고에.
- README 새 절 "Operator window"(영어): 언제 뜨나(주행 레벨 Play), 툴바 `Operator`, 버튼과 키 표, 창은 초점을 가져가지 않음(누른 뒤 휠·페달·키는 게임으로), 두 모니터 배치(Windows 디스플레이 확장 — 관전 창은 관전 모니터, 진행자 창은 진행자 모니터).
- `git status`에 Meta XR·MCP 플러그인·`Saved/` 없음.
- 보고 ①–⑤를 마지막 커밋 본문과 `HANDOFF.md`에. T-06은 `not-tested`.

## 8. 로컬에서 확인할 것 (사용자, Quest 3 + Link + 벅, VR Preview `L_DriveTest`)

1. **가장 먼저(위험, §0):** 가속 페달을 밟고 휠을 돌리는 중에 진행자 창 버튼(`Next hood` 등)을 누른다. **차가 계속 가속하고 휠이 계속 듣는지** 본다. 끊기면 그대로 멈추고 알려 준다(RawInput 등록 창 문제일 수 있음 — 다음 수를 정한다).
2. 누른 뒤 키보드 단축키(`PageDown`, `]`, `Backspace`, `F9`)가 바로 되는지 본다.
3. 모든 버튼을 한 번씩: 차·hood 이전·다음·목록, `Reset vehicle`(페이드 뒤 가까운 reset point), `Reapply alignment`(S4 — 저장값이 있으면 로그 `Alignment …`, 없으면 로그 없이 창의 정렬 줄이 `No saved alignment`). 흐린 버튼이 상태와 맞는지(쿨다운 0.5 s 깜빡임은 정상).
4. 정렬 줄이 `Applied`인지, Link를 끊었다 이으면 `Finding anchor…`를 거쳐 `Applied`로 돌아오는지 본다.
5. 손을 휠에 대고 떼며 `On wheel`·`Off wheel`, 손을 시야 밖으로 빼며 `Not tracked`를 본다.
6. 창을 최소화하거나 뒤로 보낸 뒤 툴바 `Operator`로 다시 앞으로 오는지, Play를 멈추면 닫히고 다음 Play에 같은 자리에 뜨는지 본다.
7. 헤드셋에는 아무 글도 없는지 본다.
8. 결과를 Plan T-06에 적는다.

## 9. 사용자 질문

없음(계획서 §5·Q-03·Q-06으로 닫힘). §8-1이 실패하면 그때 정한다.

## 개정 이력

| 날짜 | 개정 | 내용 |
|---|---|---|
| 2026-11-24 | 처음 | 작성(사용자 요청 "명세서 미리 써놔"). vBoR P5i/P5j 방식. vBoR과 달리 창이 초점을 가져가지 않음(D2 — 벅 RawInput이 Slate 초점을 따라감, 엔진 근거 §0). 번호 직접 선택(Q-06), 판정 함수를 명령과 공용(D4), 손 추적 읽기 함수(D6), 툴바는 같은 명세에 |
