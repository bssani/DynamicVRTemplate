# S5 관전 화면 — PC 모니터에 참가자 시야 + 차·hood 이름(헤드셋에는 안 보임)

| 항목 | 값 |
|---|---|
| 단계 | 계획서 §6 S5. 시험 T-05 |
| 결정 근거 | 계획서 D-06(헤드셋 글 없음), D-07(관전 화면: 시야 + 지금 **차 이름**과 **hood 이름**만, 사용자 2026-11-24 Q-05. 경로·배치는 vBoR P5h/P5k) |
| 베이스 브랜치 | **S4가 `main`에 머지된 뒤의 `main`** (코드는 S3에만 기대지만, S4와 같은 파일(`Build.cs`·README)을 고치므로 순서대로) |
| 작업 브랜치 | `codex/S5-spectator-screen` |
| 리뷰 파일 | `Docs/Reviews/S5-spectator-screen.md` |
| 에디터 작업 | 없음(C++ Slate만, 에셋 없음) |
| 상태 | 명세 — 구현 전(미리 씀, 2026-11-24) |

## 0. 확인한 사실

같은 방식이 vBoR(UE 5.7, Native OpenXR, Quest 3)에서 이미 기기로 확인됐다. 구현은 vBoR `Source/vBoR/Spectator/VBoRSpectatorSubsystem.cpp`·`VBoRSpectatorModel.*`(경로 `C:\Git\vBoR`)를 따른다. 그 명세는 vBoR `Docs/Specs/P5h-spectator-screen.md`(경로·다시 시도), `P5k-spectator-layout.md`(배치·색, 2026-10-19 개정의 검은 시야 원인)다.

- **경로:** Slate 위젯 → 렌더 타깃(`FWidgetRenderer`, 엔진 `Runtime/UMG/Public/Slate/WidgetRenderer.h:49` `CreateTargetFor`, `:75` 배율 있는 `DrawWidget`) → 헤드셋 스펙테이터 화면(`ISpectatorScreenController`, `Runtime/HeadMountedDisplay/Public/ISpectatorScreenController.h`, 모드 `TexturePlusEye`, 배치 구조체 `HeadMountedDisplayTypes.h:158-215`). 스펙테이터 화면은 PC 창에만 가고 헤드셋으로 가지 않는다.
- **뷰포트에 올린 위젯(`AddToViewport`)은 헤드셋에만 가고 모니터에는 안 간다**(AI_Driving `README.md:104-112`). 그래서 뷰포트를 쓰지 않는다.
- **`BeginPlay` 때는 헤드셋·PlayerController가 아직 준비되지 않아 대개 실패한다**(AI_Driving `README.md:131-133`). 그래서 0.5 s마다 30 s까지 다시 시도한다(vBoR `VBoRSpectatorSubsystem.cpp:122-135`).
- **이 리포는 `XrApi=NativeOpenXR`이다**(`Config/DefaultEngine.ini:99`). 그래서 엔진 기본 스펙테이터 컨트롤러(XRBase `DefaultSpectatorScreenController`)가 쓰인다. vBoR과 같은 경로다.
- **"시야 먼저"(`bDrawEyeFirst` 참)로 두면 PC 창의 시야가 검게 나온다.** 엔진이 두 번째 복사(칸)에서도 `bClearBlack`을 끄지 않아 창 전체를 다시 지운다. "칸 먼저"일 때만 끈다(엔진 `Engine/Plugins/Runtime/XRBase/Source/XRBase/Private/DefaultSpectatorScreenController.cpp:284-291` vs `:292-296`, 2026-11-24 이 엔진에서 다시 확인). vBoR P5k 머지 뒤 기기에서 겪은 문제다.
- **눈 영상은 영역 비율에 맞춰 가운데를 잘라 채운다**(같은 파일 `:268-275`). 16:9로 채우면 VR 시야 위아래가 잘리고, 4:3이면 헤드셋에서 보는 범위와 비슷하다(vBoR 사용자 관찰, P5h D3).
- **창 크기:** VR에서 `FSceneViewport::GetSizeXY`는 눈 렌더 크기일 수 있다. 그래서 창의 client 크기(`SWindow::GetClientSizeInScreen`)를 읽는다(vBoR `VBoRSpectatorSubsystem.cpp:37-46`).
- **이름을 읽는 곳(S3):** `ADvtVehicle::GetCarName()`·`GetHoodName()`·`GetCarIndex()`·`GetCarCount()`·`GetHoodIndex()`·`GetHoodCount()`(`Drive/DvtVehicle.h:51-62`). 이름이 비어 있으면 빈 `FText`를 돌려준다(`DvtVehicle.cpp:96-106`, `DvtCarBody.cpp:50-53`). 차가 없으면 `GetCarIndex() == -1`이다(`Cars/DvtSelectionLogic.h` `Initialize`).
- **화면 메시지:** S1 `UDvtScreenMessageSubsystem`이 Game·PIE에서 `GAreScreenMessagesEnabled`를 끈다. 관전 화면은 화면 메시지를 쓰지 않으므로 서로 상관없다.
- **모듈:** `FWidgetRenderer`는 `UMG`, 위젯은 `Slate`·`SlateCore`다. 지금 `DynamicVRTemplate.Build.cs`에는 셋 다 없다.

## 1. 결정

| # | 결정 | 내용 |
|---|---|---|
| D1 | **월드 서브시스템 하나, vBoR 경로 그대로** | `UDvtSpectatorSubsystem`(`UTickableWorldSubsystem`, Game·PIE 월드만, `UCLASS(Config=Game)`). `OnWorldBeginPlay`에서 대기를 켜고, Tick에서 0.5 s마다 시작을 다시 시도하며 30 s가 지나면 조용히 그만둔다. 시작 = 위젯 만들기 → 렌더 타깃 → 배치(`SetSpectatorScreenModeTexturePlusEyeLayout`) → 텍스처 → 모드 `TexturePlusEye`. 정리(`Deinitialize`, 차량이 사라짐, 헤드셋이 사라짐) = 텍스처 null, 모드를 `SingleEyeCroppedToFill`로 되돌림, `FWidgetRenderer`는 `BeginCleanup`으로 놓음. **헤드셋이 없거나(`IsStereoscopic3D`·`IsHMDConnected`·`IsHMDEnabled` 중 하나라도 거짓) `FApp::CanEverRender()`가 거짓이면 아무것도 만들지 않고 경고·오류도 내지 않는다** |
| D2 | **시작 조건 — 차가 있는 주행 레벨에서만** | 월드에 `ADvtVehicle`이 **정확히 하나**이고 `GetCarCount() ≥ 1`, `GetCarIndex() ≥ 0`일 때만 시작한다. 아니면(차량 0·2개 이상, `Cars`가 빈 `L_Calibrate`(S4 D5)) 엔진 기본 스펙테이터를 건드리지 않는다. 차량은 약한 포인터로 들고, 사라지면 정리(D1) |
| D3 | **배치 = vBoR P5k D1·D2** | 시야는 화면 높이 전체, 왼쪽에 붙임. 창 비율 A, 시야 비율 E(`EyeAspect`, 기본 4/3), 오른쪽 칸 최소 폭 m(창 높이에 대한 비율, `MinPanelWidth`, 기본 0.37)으로 시야 높이 h = min(1, (A − m)/E). **시야:** x 0 → h·E/A, y (1 − h)/2 → (1 + h)/2. **칸:** x h·E/A → 1, y 0 → 1. 시야 밖은 검게(`bClearBlack` 참). 텍스처 영역 = 칸. **`bDrawEyeFirst` = 거짓**(§0 — 참이면 PC 시야가 검다). `bUseAlpha` 거짓(칸은 불투명). 렌더 타깃 크기 = 칸의 창 픽셀 크기(반올림), 위젯은 창 높이 1080 기준으로 배치하고 `DrawWidget` 배율 = 칸 높이 px / 1080. 그릴 때마다 창 크기를 읽어 배치가 바뀌었으면 배치·렌더 타깃 크기를 다시 넣는다. 같으면 다시 넣지 않는다. 크기 0(최소화)은 무시한다 |
| D4 | **칸 내용 — 차·hood 이름만** | 위에서부터(1080 기준 px·pt): 여백 위아래 40, 좌우 32. ① 작은 제목 `CAR`(Regular 16 pt, 자간 80 — `FSlateFontInfo::LetterSpacing`은 1/1000 em, 보조 글자색) ② 8 px 아래 **차 이름**(Bold 40 pt, 글자색) ③ 32 px 아래 구분선(2 px) ④ 32 px 아래 작은 제목 `HOOD` ⑤ 8 px 아래 **hood 이름**(Regular 36 pt, 글자색). 칸 왼쪽에 2 px 선. 이름은 칸 폭에서 줄바꿈하고 세 줄을 넘으면 말줄임(…). **번호(k/N)·속도·안내 글은 넣지 않는다**(Q-05). 그 차에 hood가 없으면(`GetHoodCount() == 0`) ③–⑤를 숨긴다 |
| D5 | **이름이 비었을 때** | 차 `DisplayName`이 비면 `Car k`, hood가 비면 `Hood k`(k = 1부터 센 번호). 이름 칸이 비어 관객 화면이 비지 않게 한다. 로그는 내지 않는다(매 그리기마다 나오므로) |
| D6 | **새로 그림** | 매 그리기 때 차량에서 이름을 다시 읽는다. 빈도 `RedrawsPerSecond`(기본 10 — 교체 뒤 0.1 s 안에 바뀜). 이벤트 구독은 하지 않는다(읽기만으로 충분하고, 차량을 바꾸지 않는다) |
| D7 | **색·글꼴 — 진행자 창(S6)과 같은 팔레트** | vBoR P5k D4·P5i D9의 어두운 팔레트(사용자 디자인 시스템 CX Toolkit): 칸 바탕 `#131a26`, 이름 `#eef2f7`, 작은 제목 `#9aa6b8`, 칸 왼쪽 선·구분선 `#2e3a4e`, 강조 `#5b8cff`(이번에는 안 씀, S6가 씀). sRGB hex → `FLinearColor::FromSRGBColor(FColor::FromHex(...))`(hex/255를 `FLinearColor`에 그대로 넣으면 선형으로 읽혀 색이 달라진다). 팔레트는 작은 공용 헤더(예: `UI/DvtUiStyle.h`, 상수 + 변환 함수)에 두고 S6이 같이 쓴다. 글꼴은 엔진 Roboto(`FCoreStyle::GetDefaultFontStyle`) `Regular`·`Bold`만 |
| D8 | **값은 `DefaultGame.ini`로 바꾼다** | `EyeAspect`·`MinPanelWidth`·`RedrawsPerSecond`·글꼴 크기 셋·색 넷은 `UPROPERTY(Config)`. 새 월드의 서브시스템이 CDO 값을 쓴다. 바꾸는 법은 README에 적는다(§7) |
| D9 | **시험할 수 있게 나눈다** | 헤드셋·렌더러 없이 시험할 순수 부분: 배치 계산(창 크기 → 시야·칸 영역·렌더 타깃 크기·배율, 바뀜 판정), 시작 조건(D2), 칸 문구(차량 → 차·hood 줄과 보임, D4·D5). 렌더 타깃·스펙테이터 경로는 Quest에서만 본다(§8) |

## 2. 지켜야 할 조건

- 헤드셋 화면은 바뀌지 않는다(D-06). 뷰포트 위젯·`AddOnScreenDebugMessage`·UMG 위젯 에셋을 쓰지 않는다.
- S1–S4 코드·동작은 그대로다. 차량은 읽기만 한다. 기존 `DynamicVRTemplate.*`·`VRCalibrationKit.*` 시험과 단언은 바꾸지 않는다.
- 헤드셋 없는 PIE·자동화(`-nullrhi`)에서 경고·오류 없이 지나간다.
- `Config/DefaultGame.ini`에 새 절을 넣지 않는다(기본값은 코드에. 바꾸는 법만 README).
- public: Meta XR·MCP 플러그인, `Saved/`는 커밋하지 않는다.

## 3. 범위 밖

| 하지 말 것 | 이유 |
|---|---|
| 번호·속도·정렬 상태·손 상태를 관전 화면에 | Q-05 "이름만". 정렬·손은 진행자 창(S6) |
| 16:9 꽉 채우기·반투명 겹치기 | 시야 위아래가 잘린다(§0) |
| 헤드셋 안 UI | D-06 |
| 진행자 창 | S6 |
| 이름 바꾸기·차 교체 | S3 그대로. 읽기만 |

## 4. 구현자가 정할 것

- 클래스·함수·헤더 이름(`UDvtSpectatorSubsystem`, `DvtUiStyle`은 예).
- 순수 부분의 모양(구조체·네임스페이스 함수).
- 렌더 타깃을 다시 만들지 `ResizeTarget`으로 맞출지(vBoR은 `ResizeTarget`).
- 1–2 px 세부.

## 5. 에디터 작업

없음. 에셋·레벨을 저장하지 않는다.

## 6. 경계 상황과 시험 (자동화 `DynamicVRTemplate.S5.*`)

| # | 상황 | 기대 |
|---|---|---|
| E1 | 배치 — 창 16:9(1920 × 1080) / 16:10(1920 × 1200) / 4:3(1440 × 1080) / 21:9(2520 × 1080), E 4/3, m 0.37 | 시야 x 0–0.75·y 0–1, 칸 x 0.75–1 / 시야 x 0–0.76875·y 0.03875–0.96125, 칸 x 0.76875–1 / 시야 x 0–0.7225·y 0.13875–0.86125, 칸 x 0.7225–1 / 시야 x 0–0.571429·y 0–1, 칸 x 0.571429–1. 시야는 픽셀로 4:3. 모두 엔진 배치 검사(`IsValid`) 통과. 텍스처 영역 = 칸. `bDrawEyeFirst` 거짓, `bClearBlack` 참 |
| E2 | 창 크기 바뀜 / 같음 / 0 | 다시 넣음(배치·렌더 타깃 크기) / 다시 넣지 않음 / 무시 |
| E3 | 칸 픽셀 | 16:9 1080: 렌더 타깃 480 × 1080, 배율 1. 3840 × 2160: 960 × 2160, 배율 2 |
| E4 | 시작 조건 — 차량 0 / 2 / 1(`Cars` 빔) / 1(차 2대) | 시작 안 함 / 안 함 / 안 함 / 시작 가능 |
| E5 | 헤드셋 없음·`-nullrhi` | 아무것도 만들지 않음, 모드를 건드리지 않음, 경고·오류 없음(30 s 대기가 끝나도) |
| E6 | 칸 문구 — 차 A(`Sample A`, hood `Low`·`High`, hood 1) | 차 줄 `Sample A`, hood 줄 `High`, hood 묶음 보임. 번호 글자 없음 |
| E7 | 주행 중 교체 — `NextCar` / `NextHood` 뒤 | 다음 문구가 새 차·hood 이름(S3 명령을 시험 world에서 부름) |
| E8 | 이름 빔 — 차 2번째 이름 빔 / hood 1번째 이름 빔 | `Car 2` / `Hood 1` |
| E9 | hood 없는 차 | hood 묶음(구분선 포함) 숨김 |
| E10 | 글이 칸에 들어감 — 최소 칸 폭(m, 1080에서 약 400 px, 여백·선을 빼면 약 334 px)·1080에서 24자 차 이름·24자 hood 이름 | 세 줄 안에 줄바꿈돼 들어감. 60자는 세 줄 뒤 말줄임(높이 제한 상자 + `ETextOverflowPolicy::Ellipsis`). 모든 묶음이 칸 높이 안(Slate 글꼴 측정 — `-nullrhi`에서도 됨, vBoR P5h E17 방식). README에 "이름은 짧게 — 24자 안 권장"을 적는다 |
| E11 | 기본 색 | D7 hex를 sRGB로 바꾼 값과 같음 |
| E12 | Config | D8 프로퍼티가 모두 `CPF_Config`, 클래스가 `Config=Game`, CDO 값이 새 월드 서브시스템으로 감 |
| E13 | 월드 종류 | Game·PIE에서만 만들어짐(Editor·EditorPreview 아님) |
| E14 | 기존 시험 | `DynamicVRTemplate.*`·`VRCalibrationKit.*` 모두 통과 |

## 7. 완료 조건

- non-unity(`-DisableUnity -Module=DynamicVRTemplate`)와 unity 빌드 성공, 경고·오류 0.
- `DynamicVRTemplate.*`와 `VRCalibrationKit.*` 전체 통과(시험 전에 `Config/UserEngine.ini` 없음 — S4).
- 헤드셋 없는 PIE로 `L_DriveTest`를 30 s 넘게 돌려 S5 관련 경고·오류가 없는지 본다(로그 줄을 보고에 적는다).
- README 새 절 "Spectator screen"(영어): 무엇이 보이나(시야 4:3 + 오른쪽 칸, 차·hood 이름만, 헤드셋에는 안 보임), 이름 넣는 곳(`ADvtCarBody::DisplayName`, `UDvtHoodVariantComponent::DisplayName`, 비면 `Car k`·`Hood k`), 값 바꾸기(`Config/DefaultGame.ini`의 `[/Script/DynamicVRTemplate.DvtSpectatorSubsystem]` — 실제 클래스 경로로, 예: `EyeAspect=1.5`, `MinPanelWidth=0.4`, 바꾼 뒤 에디터 다시 열기), PC 창 연결(Windows 디스플레이 확장, 게임 창을 관전 모니터에).
- `git status`에 Meta XR·MCP 플러그인·`Saved/` 없음.
- 보고 ①–⑤를 마지막 커밋 본문과 `HANDOFF.md`에. T-05는 `not-tested`.

## 8. 로컬에서 확인할 것 (사용자, Quest 3 + Link, VR Preview `L_DriveTest`)

1. **먼저:** PC 창에 시야가 나오는지(검지 않은지 — §0 `bDrawEyeFirst`), **헤드셋에는 오른쪽 칸이 보이지 않는지** 본다.
2. PC 창 시야(4:3)가 헤드셋에서 보는 범위와 비슷한지 본다. 다르면 README대로 `EyeAspect`만 바꾼다.
3. 차·hood를 키(`PageUp`/`PageDown`, `[`/`]`)로 바꾸며 이름이 바로 바뀌는지 본다. 몇 m 뒤에서 읽히는지 보고, 작으면 글꼴 크기를 바꾼다.
4. 게임 창 크기를 바꾸거나 다른 모니터로 옮겨 전체 화면으로 해도 배치가 맞는지 본다.
5. `L_Calibrate`(S4)에서는 칸이 없고 엔진 기본 PC 화면인지 본다.
6. 결과를 Plan T-05에 적는다.

## 9. 사용자 질문

없음(Q-05로 닫힘 — 이름만). 칸에 번호(k/N)를 보이고 싶으면 알려 준다.

## 개정 이력

| 날짜 | 개정 | 내용 |
|---|---|---|
| 2026-11-24 | 처음 | 작성(사용자 요청 "명세서 미리 써놔"). vBoR P5h/P5k 경로·배치, `bDrawEyeFirst` 거짓(vBoR 기기 문제), 칸 = 차·hood 이름만, 이름 빔 → `Car k`·`Hood k`, 팔레트는 S6와 공용 |
