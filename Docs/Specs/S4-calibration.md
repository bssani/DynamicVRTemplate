# S4 정렬 가져오기 — VRCalibrationKit을 넣고, 차량에 정렬점, 정렬 레벨

| 항목 | 값 |
|---|---|
| 단계 | 계획서 §6 S4. 시험 T-04 |
| 결정 근거 | 계획서 D-01(키트, 벅 고정 마커, 설치 때 한 번), D-02(SgRP 같음 → 정렬점은 차량에 고정, 차를 바꿔도 그대로), D-04(키트가 VR Pawn을 차량에 붙임), D-06(헤드셋 글 없음), D-13(recenter 없음) |
| 베이스 브랜치 | **S1이 `main`에 머지된 뒤의 `main`** (S2·S3과 무관 — 손·차 교체를 건드리지 않는다) |
| 작업 브랜치 | `codex/S4-calibration` |
| 리뷰 파일 | `Docs/Reviews/S4-calibration.md` |
| 에디터 작업 | 있음 — 차량 BP 정렬점, 정렬 레벨(§5) |

## 0. 확인한 사실

키트 경로는 `C:\Git\VRCalibrationKit` `main@e915647`(PR #2 머지) 기준이다. PR #1(`864946c`, 리뷰 2회차 통과 — vBoR `Docs/Reviews/K1-vrcalibrationkit-extract.md` 2회차) 뒤로 바뀐 것은 문서(README, `Docs/Integration-Guide.md`)뿐이고 `Plugins/VRCalibrationKit/`는 같다(`git diff --quiet 864946c e915647 -- Plugins/VRCalibrationKit` 통과, 2026-11-24 확인). 붙이는 법은 키트 `Docs/Integration-Guide.md` §4–§10.

- **기기 결과**(키트 리포, Quest 3 + Link, 2026-11-24, 한 PC·한 방):
  - 마커 보정 Max XY 1.13 cm. 앵커 복원 drift 0.57 cm.
  - Link 재연결로 원점이 137 cm·14° 옮겨진 뒤 앵커로 복원하고 마커로 다시 재면 0.6 cm 안(dX +0.61, dY −0.20 cm).
  - 앵커를 찾는 데 1.6–4.2 s(10 s 넘으면 포기).
  - 차량을 옮기면 시점이 따라온다 — attach 방식이 기기에서 동작한다(이 리포의 주행에 해당).
  - **가디언을 다시 그리면 약 6 cm 어긋난다** → 다시 보정(M → S). 키트 Integration-Guide §7 "When to calibrate again".
  - 재부팅 뒤·다음 날 복원은 아직 확인하지 않았다.
- **루트 scale ≠ 1 거부는 조용하다:** `State = NeedsCheck`가 되고 `Reason` 칸에만 남는다. 화면·로그에는 아무것도 나오지 않는다. 키트는 고치지 않는다(사용자 결정) → 이 리포에서 차량 루트 scale = 1을 지키고 §5·E10으로 확인한다.

- **모듈:** `VRCalibrationCore`(모든 플랫폼, 정렬 component·점·저장·앵커)와 `VRCalibrationMarker`(Win64, 카메라·ArUco·측정). `.uplugin`이 `OculusXR`·`XRBase`·`OpenCV`(Win64)를 켠다(`Plugins/VRCalibrationKit/VRCalibrationKit.uplugin`). Interaction SDK는 필요 없다.
- **프로젝트 설정**(Integration-Guide §4.3, 키트 `Config/DefaultEngine.ini:10-13`): `XrApi=NativeOpenXR`(이 리포에 이미 있음), `bAnchorSupportEnabled=True`, `bPassthroughCameraAccessEnabled=True`.
- **차량 쪽**(§5):
  - `UVRCalAlignmentComponent` 하나. 레벨에 하나만 있어야 한다.
  - 자식으로 `UVRCalAlignPointComponent` 4개, `Order` 1–4. 마커 ID = `FirstMarkerId + Order − 1`, 프로필 없으면 1–4.
  - 점들이 20 cm 이상 퍼져 있어야 한다. `FloorLocalZCm` = 실물 바닥 높이(차량 로컬, cm).
  - **차량 루트 scale = 1** — 아니면 `NeedsCheck`, Pawn을 옮기지 않는다(`VRCalAlignmentComponent.cpp` `InitializeSetup`의 scale 검사, 키트 리뷰 R1-1).
- **Pawn 쪽:** Player 0 Pawn, 루트 + HMD 고정 카메라, `bUseControllerRotationYaw` false.
  - 키트는 이 조건과 `WorldToMeters == 100`을 검사한다(`InitializeSetup`, "Pawn/root/WorldToMeters check required").
  - 엔진 `APawn` 기본값이 이미 false다(`Engine/Source/Runtime/Engine/Private/Pawn.cpp:97`). S1 `ADvtVRPawn`은 이 값을 바꾸지 않는다.
- **키트가 Pawn을 붙이는 방식:**
  - component `BeginPlay`에서 `GetPlayerPawn(this, 0)`을 잡는다.
  - 저장값을 읽어 적용할 수 있으면 `AttachToActor(차량, KeepWorldTransform)` 뒤 `SetActorRelativeTransform(맞춘 값)`을 한다. 성공하면 `State = Applied`(또는 `NeedsCheck`) + `OnPawnMoved`.
  - **적용하지 않으면(`NoSavedValue`, `NeedsCheck`, 앵커 찾는 중) Pawn을 건드리지 않는다**(`VRCalAlignmentComponent.cpp` `InitializeSetup`·`LoadSetupAlignment`).
  - 앵커를 찾으면 나중에 다시 적용한다.
- **Pawn이 키트 BeginPlay 전에 있다:** 엔진은 Player Pawn을 만든 뒤에 world `BeginPlay`를 한다. PIE `Engine/Source/Runtime/Engine/Private/GameInstance.cpp:537`(SpawnPlayActor) → `:565`(BeginPlay), 패키지·맵 로드 `UnrealEngine.cpp:16265` → `:16279`. `ADvtGameMode`(`AGameModeBase`)는 로그인 때 Pawn을 만든다. `ADvtGameMode::StartPlay`가 `WorldToMeters`·Stage를 `Super::StartPlay` 전에 둔다(S1).
- **정렬 도구 `AVRCalMarkerCalibrator`:**
  - 첫 PlayerController에 `EnableInput`하고 키 `M`·`G`·`H`·`S`를 옛 방식(`BindKey`)으로 받는다(`VRCalMarkerCalibrator.cpp:101-105`). **`S`는 S1 주행 키(브레이크)와 겹친다.**
  - 진행 글을 `AddOnScreenDebugMessage`로도 띄운다(`Publish`, `:641-646`) — 이 리포의 `UDvtScreenMessageSubsystem`(S1 D8)이 Game·PIE에서 화면 메시지를 끄므로 **헤드셋에 글이 뜨지 않고 Output Log(`LogVRCalMarker`)에만 남는다**.
  - 마커 확인용 작은 구를 `DrawDebugSphere`로 그린다(`:636`, 글이 아님).
  - 정렬점은 에디터에서 보이고 게임에서 숨는다(`VRCalAlignPointComponent.cpp` 생성자, `ShowPoint`는 정렬 도구가 부름).
- **recenter**가 오면 키트는 `NeedsCheck`가 된다(`HandleRecenter`). 이 프로젝트는 recenter를 부르지 않는다(D-13).
- **S1 임시 attach**(`ADvtPlayerController::AttachVRPawn`, `DvtPlayerController.cpp`)는 `BeginPlay`와 `OnPossess`에서 무조건 `DriverOrigin`에 snap한다. 키트와 순서가 정해져 있지 않다. 키트가 먼저 붙인 뒤 controller가 붙이면 **정렬을 덮어쓴다** → D3에서 고친다.

## 1. 결정

| # | 결정 | 내용 |
|---|---|---|
| D1 | **키트를 복사해 넣는다, 고치지 않는다** | `C:\Git\VRCalibrationKit` `e915647`의 `Plugins/VRCalibrationKit/`를 이 리포 `Plugins/VRCalibrationKit/`로 그대로 복사한다(`git -C C:\Git\VRCalibrationKit archive e915647 Plugins/VRCalibrationKit`를 풀기 — 작업 트리 상태와 무관하게. 키트 작업 트리에 커밋 안 된 `Config/DefaultInput.ini`가 있지만 플러그인 밖이라 상관없다). 커밋한다(키트 리포도 public). **키트 파일을 고치지 않는다** — 고칠 것이 생기면 멈추고 리뷰 파일 "질문"에 적는다(키트 리포에서 고친다). 출처 커밋은 README에 적는다 |
| D2 | **차량 C++에 정렬 component와 점 4개** | `ADvtVehicle` 생성자에서 `UVRCalAlignmentComponent` 하나와 `UVRCalAlignPointComponent` 4개(`AlignPoint1`…`4`, `Order` 1–4, 차량 루트 밑)를 만든다. 점 위치·`FloorLocalZCm`·`Label`은 BP에서 정한다. **SgRP가 모든 차에서 같으므로(D-02) 점은 차량에 있고 차 BP(S3 `ADvtCarBody`)에는 없다.** `DynamicVRTemplate.Build.cs`에 `VRCalibrationCore` |
| D3 | **VR Pawn attach — 키트가 우선, 없으면 S1 자리** | `AttachVRPawn`을 바꾼다: **Pawn의 attach 부모 Actor가 이미 차량이면 아무것도 하지 않는다.** 아니면 지금처럼 `DriverOrigin`에 snap한다(정렬값이 없거나 `NeedsCheck`, 앵커를 기다리는 동안에도 차와 같이 움직이게). 이 규칙이면 어느 쪽이 먼저 돌아도 맞다. ① 키트 적용 → controller 건너뜀. ② controller가 `DriverOrigin`에 붙임 → 키트가 나중에 적용하며 `AttachToActor`·상대 transform으로 덮어씀. ③ 키트 미적용 → `DriverOrigin` 유지. 판정은 순수 함수 하나(§6). S2가 이미 머지됐으면 손 연결(S2 D7 `SetTrackedHands`)은 attach를 건너뛰어도 그대로 한다 |
| D4 | **정렬 다시 적용 키 `F9`** | `IMC_Drive`에 `IA_ReapplyAlignment`(`F9`) → controller가 차량 정렬 component의 `RequestReapply()`를 부른다(앵커 찾는 중이면 키트가 false를 돌려줌 — 로그 한 줄). S6 진행자 창 버튼이 같은 함수를 부른다. 키트 샘플의 `A`는 주행 키라 쓰지 않는다 |
| D5 | **정렬 레벨 `L_Calibrate` + 주행 없는 GameMode** | `ADvtCalibrationGameMode`(`ADvtGameMode` 자식): Pawn `ADvtVRPawn`, PlayerController = 엔진 `APlayerController`(주행 입력 없음 → 정렬 도구 키 `S`와 브레이크가 겹치지 않음). `L_Calibrate`에 `BP_SampleVehicle` 하나, `AVRCalMarkerCalibrator` 하나, 바닥·빛, World Settings GameMode = 이것. `L_DriveTest`(주행)에는 정렬 도구를 두지 않는다(카메라는 정렬 레벨에서만, 키트 §6) |
| D6 | **헤드셋 글 없음 그대로** | S1 `UDvtScreenMessageSubsystem`을 정렬 레벨에서도 그대로 둔다(D-06) — 정렬 도구의 화면 글은 꺼지고 Output Log에만. 마커 확인 구(`DrawDebugSphere`)와 정렬점 번호는 키트 기본대로 정렬 레벨에서만 보인다(글이 아닌 표시 — 설치 담당자용). 정렬 레벨에서도 글을 보이지 않는다(사용자 답 §9 Q1, 2026-11-24) — 진행 글(`Max XY`, `Marker fit vs pawn … dX … dY`)은 PC Output Log(`LogVRCalMarker`)로 본다 |
| D7 | **프로젝트 설정** | `Config/DefaultEngine.ini` `[/Script/OculusXRHMD.OculusXRHMDRuntimeSettings]`에 `bAnchorSupportEnabled=True`, `bPassthroughCameraAccessEnabled=True`. `.uproject`에 `VRCalibrationKit`. 앵커는 키트 기본대로 꺼져 있다(`VRCal.AnchorAlignment`, PC마다 `Config/UserEngine.ini` — git 밖). `Config/DefaultEngine.ini`에 `VRCal.AnchorAlignment` 줄을 넣지 않는다 |
| D8 | **샘플 정렬점 = 자리표시** | `BP_SampleVehicle`의 점 4개는 운전석 바닥 둘레에 40 × 40 cm 쯤 사각형(20 cm 퍼짐 조건을 넉넉히 넘게), `FloorLocalZCm` = 샘플 차 바닥. **실제 벅 마커 자리는 회사 PC에서 사용자가 넣는다**(벅의 고정된 곳을 SgRP 기준으로 잰 값). README에 넣는 법을 적는다 |

## 2. 지켜야 할 조건

- 키트 파일을 고치지 않는다(D1).
- S1 동작(입력·Reset·휠)은 바뀌지 않는다. 바뀌는 것은 attach 규칙(D3)뿐이다. S2·S3 코드가 이미 있으면 건드리지 않는다.
- recenter 금지(D-13). 헤드셋·PC 화면 글 금지(D-06, D6).
- public: Meta XR·MCP 플러그인은 커밋하지 않는다. `Saved/Calibration/`(정렬값)·`Config/UserEngine.ini`는 커밋하지 않는다(이미 git 밖인지 `git status`로 확인).

## 3. 범위 밖

- 진행자 창의 정렬 상태 표시·버튼 — S6(`State`·`Reason`·`OnRecord`·`RequestReapply`를 쓴다).
- 벅 프로필 여러 개(`Bucks`) — 벅은 하나다.
- 실제 벅 마커 위치 측정·입력(회사 PC, 사용자).
- 키트 변경.

## 4. 구현자가 정할 것

- 복사 방법(D1 결과가 같으면).
- 샘플 정렬점의 정확한 자리(D8 조건 안에서).
- `L_Calibrate`의 모양.

## 5. 에디터 작업 (MCP, 코드 빌드·시험 뒤)

- `BP_SampleVehicle`: 점 4개 자리(D8), `FloorLocalZCm`. 차량 루트 scale이 1인지(BP와 `L_DriveTest`·`L_Calibrate` 인스턴스 모두) 확인하고 값을 보고에 적는다 — 틀려도 키트가 아무 글도 내지 않는다(§0).
- `IA_ReapplyAlignment` + `IMC_Drive`에 `F9`.
- `Content/Samples/Maps/L_Calibrate`(D5).
- 헤드셋 없는 PIE로 `L_DriveTest`를 열어 로그에 `Alignment …` 줄이 `No saved value`로 나오고(Pawn·루트·WorldToMeters 검사는 통과) VR Pawn이 `DriverOrigin`에 붙어 있는지 본다. `L_Calibrate`를 열어 오류 없이 시작하는지 본다(카메라는 헤드셋이 없으면 실패 로그 — 정상). 저장하고 에디터를 닫는다.

## 6. 경계 상황과 시험 (자동화 `DynamicVRTemplate.S4.*`)

| # | 상황 | 기대 |
|---|---|---|
| E1 | 차량 CDO | 정렬 component 1개, 점 4개 `Order` 1·2·3·4(중복 없음) |
| E2 | attach 판정 — Pawn 부모 Actor = 차량(루트에 붙음) | 건너뜀 |
| E3 | attach 판정 — Pawn 부모 Actor = 차량(`DriverOrigin`에 붙음) | 건너뜀(이미 차량) |
| E4 | attach 판정 — 부모 없음 / 다른 Actor | `DriverOrigin`에 붙임 |
| E5 | 순서 ① — 시험 world에서 Pawn을 차량 루트에 상대 transform T로 붙인 뒤 controller attach 실행 | 부모·상대 transform이 T 그대로 |
| E6 | 순서 ② — controller가 `DriverOrigin`에 붙인 뒤 키트처럼 `AttachToActor(차량, KeepWorldTransform)` + 상대 T | 부모 = 차량 루트, 상대 = T. 다시 controller attach → 그대로 |
| E7 | Reapply 키 | 차량 정렬 component의 `RequestReapply`가 한 번 불림(시험용 가로채기 경로는 구현자가 정함) |
| E8 | 정렬 GameMode CDO | PlayerController 클래스가 `ADvtPlayerController`가 아니다, Pawn = `ADvtVRPawn` |
| E9 | 키트 시험 | `VRCalibrationKit.*` 전체 통과(키트 그대로 복사했는지 — 46개) |
| E10 | 샘플 차량 scale | `BP_SampleVehicle` CDO의 루트 component 상대 scale = (1,1,1). 레벨 인스턴스는 §5에서 눈으로 |

E2~E4는 순수 함수, E5·E6은 시험 world(S1 시험 fixture 방식)로. 실제 정렬은 Quest(T-04).

## 7. 완료 조건

- non-unity(`-DisableUnity -Module=DynamicVRTemplate`)와 unity 빌드 성공, 경고·오류 0. 키트 모듈 non-unity도 한 번(`-Module=VRCalibrationCore`, `-Module=VRCalibrationMarker`).
- `DynamicVRTemplate.*`와 `VRCalibrationKit.*` 전체 통과. 시험 전에 `Config/UserEngine.ini`가 없어야 한다(키트 Integration-Guide §7 — 시험 기본값을 덮음).
- §5 확인 결과(로그 줄·attach 부모)를 보고에 적는다.
- README 새 절 "Buck alignment"(영어): 키트 출처 커밋, 마커 인쇄(키트 리포 `Docs/Markers/`, 100 % 60 mm), 정렬점 자리 넣기(SgRP 기준 벅 고정점, `FloorLocalZCm`), `L_Calibrate`에서 M → G → S, 주행 레벨 `F9` 다시 적용, 앵커 켜기(`Config/UserEngine.ini`), 글은 Output Log에만(D6), §8의 다시 보정 표(가디언 다시 그림·벅 옮김 → M → S)와 복원 판정(M만 → |dX|·|dY| 2 cm 안), 차량 루트 scale = 1(틀리면 조용히 적용 안 됨).
- `git status`에 Meta XR·MCP 플러그인·`Saved/`·`UserEngine.ini` 없음. `Plugins/VRCalibrationKit/`는 커밋에 있다.

## 8. 로컬에서 확인할 것 (사용자, Quest 3 + 벅)

- 회사 PC에서 실제 마커 자리를 차량 BP 점 4개에 넣는다.
- T-04:
  1. **보정:** `L_Calibrate`에서 M(마커 넷 초록) → G → S. Output Log `Max XY`가 한계(`PointFitLimitCm`) 안.
  2. **적용:** `L_DriveTest` Play → 로그 `Alignment Applied`, 실물 시트·휠과 가상 운전석이 맞는다. 주행 중에도 맞음이 유지된다(차와 같이 움직임). S3 머지 뒤에는 차·hood 교체 뒤에도 같은 자리. 헤드셋에 글 없음.
  3. **복원 판정(앵커 켬):** Link를 끊었다 다시 잇는다 → `L_Calibrate` Play, 앵커 복원을 기다린 뒤(로그 `Alignment Applied`, 보통 2–4 s) **M만** 누른다(G·S 누르지 않음). 로그 `Marker fit vs pawn (Applied) | dX … | dY …`(키트 `VRCalMarkerCalibrator.cpp:510`)가 **괄호 안 `Applied`이고 |dX|·|dY| 모두 2 cm 안**이면 통과(`NoSavedValue`·`NeedsCheck`면 복원 실패 — 판정 불가). 넘으면 다시 보정(M → S)하고 결과를 적는다. 에디터 재시작 뒤에도 같은 판정.
  4. 앵커 끔(`VRCal.AnchorAlignment=0`)에서 Play 재시작만 → 같은 자리(Stage 원점이 그대로일 때). Link 재연결 뒤에는 어긋나는 것이 정상.
  5. 아직 안 된 것: 재부팅 뒤·다음 날 복원 — 3번 판정으로 확인해 결과를 적는다.
- **운영 규칙(다시 보정 = `L_Calibrate`에서 M → S)** — 키트 Integration-Guide §7 "When to calibrate again"과 같다. README "Buck alignment"에도 넣는다(§7):

  | 일 | 할 것 |
  |---|---|
  | Play 재시작, Link 재연결, 헤드셋 잠자기 | 없음(앵커가 복원) — 의심되면 3번 판정 |
  | **가디언(경계)을 다시 그림** | **다시 보정** — 시험에서 약 6 cm 어긋남 |
  | 벅·마커를 옮김 | 다시 보정 |
  | 로그 `anchor not found` | Link(Spatial Data 켬) 확인, 보정한 쪽을 보고 다시 Play. 계속되면 다시 보정 |

## 9. 사용자 질문 (모두 닫힘)

- Q1 (닫힘, 2026-11-24): 정렬 레벨에서도 정렬 도구 진행 글을 헤드셋에 보이게 할까? → **안 보임**(D6 그대로). PC Output Log로 본다.
