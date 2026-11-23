# S2 휠 손 붙기 — 추적 손 ↔ 휠에 붙은 고정 포즈 손, 붙는 범위는 에디터에서만 보임

| 항목 | 값 |
|---|---|
| 단계 | 계획서 §6 S2. 시험 T-02 |
| 결정 근거 | 계획서 D-05(고정 포즈, 에디터에서 보이고 Play 중 숨김), D-06(헤드셋 글 없음), D-04(VR Pawn·차량 분리). 사용자 2026-11-24: "어느 정도 가까이 가면 붙을지, 그리고 손이 붙는 bound를 editor에서 볼 수 있게, 플레이 중에는 숨기고", "고정 포즈가 쉽지 않나" |
| 베이스 브랜치 | **S1이 `main`에 머지된 뒤의 `main`** (S1의 `ADvtVehicle`·`ADvtVRPawn`·`ADvtPlayerController`·`SteeringWheelPivot`를 쓴다) |
| 작업 브랜치 | `codex/S2-steering-grip` |
| 리뷰 파일 | `Docs/Reviews/S2-steering-grip.md` |
| 에디터 작업 | 있음 — 손 에셋 복사, 샘플 차 BP 손 배치(§5) |

## 0. 확인한 사실

- **출처 구현:** AI_Driving `UVRHandPresenceComponent`(`C:\Git\AI_Driving\Source\AI_Driving\VR\VRHandPresenceComponent.h`·`.cpp`). 가져올 것:
  - 손바닥(`EHandKeypoint::Palm`)에서 테두리 원까지의 거리(`.cpp:230-264`): 휠 로컬(스케일 뺌)에서 축 방향 거리와 반지름 방향 거리(`반지름 거리 − WheelRadius`)로 `sqrt(축² + 반지름²)`. 즉 **테두리를 감싼 도넛(torus)까지의 거리**다.
  - 히스테리시스(`.cpp:220-226`): 잡음 상태는 거리 > release면 놓음, 놓음 상태는 거리 < enter면 잡음. 기본 enter 10 cm, release 18 cm(`.h:117`, `:122`), 반지름 18 cm(`.h:94`).
  - 추적 끊김 = 지금 상태 유지(`.cpp:203-208`), 추적 기능이 아예 없거나 무효 = 잡음(`.cpp:195-199`).
  - 추적 손 자리 잡기(`.cpp:306-330`): Meta 손 컴포넌트는 손가락만 포즈하고 컴포넌트 위치는 그대로 두므로 손목(`EHandKeypoint::Wrist`) world transform으로 옮긴다. Meta 손 tick 뒤에 돌도록 tick 선행 조건을 건다(`.cpp:80-90`).
  - 보이기(`.cpp:16-30`): 잡음이면 붙은 손만, 놓음이면 추적 손만.
- **가져오지 않을 것:**
  - `bDrawDebug` 런타임 그리기(`.cpp:333-370`, `DrawDebugCircle`·`DrawDebugSphere`). world에 그리는 선이라 **헤드셋에도 보인다**(D-06 위반). 에디터 표시는 새로 만든다(D4).
  - "운전석 차인지" 판정(`.cpp:102-121`). 이 프로젝트는 차가 하나이고 possess하지 않는다(S1 D1). 그 판정을 옮기면 손이 늘 잡음으로 묶인다.
  - 휠 돌리기(`DriveWheel`, `.cpp:275-304`). S1 D5가 한다.
- **손 추적 API:** `IHandTracker`(`Engine/Source/Runtime/HeadMountedDisplay/Public/IHandTracker.h:41` `IsHandTrackingStateValid`, `:48` `GetKeypointState`, `:50` `GetAllKeypointStates`). 찾는 법은 모듈러 피처(AI_Driving `.cpp:32-38`). **MetaXR는 `IHandTracker`를 등록하지 않는다** — `OpenXRHandTracking` 플러그인이 등록한다(AI_Driving `README.md:99-101`). 이 리포 `DynamicVRTemplate.uproject`에 `OpenXRHandTracking`이 켜져 있다(S0).
- **추적 손 mesh:** `UOculusXRHandComponent`(`Plugins/MetaXR/Source/OculusXRInput/Public/OculusXRHandComponent.h:28`, `UPoseableMeshComponent` 자식). `SkeletonType`(`:39`)·`MeshType`(`:43`). `ConfidenceBehavior` 기본 `HideActor`(`:47`) → AI_Driving은 `None`으로 바꿔 보이기를 한 곳에서만 정했다(`AI_DrivingPawn.cpp:143-146`).
- **에디터 표시:**
  - `UArrowComponent`가 같은 패턴이다: 생성자 `bHiddenInGame = true`(`Engine/Source/Runtime/Engine/Private/Components/ArrowComponent.cpp:236`), `CreateSceneProxy`(`:249`).
  - 원 그리기는 `DrawCircle(FPrimitiveDrawInterface*, Base, X, Y, Color, Radius, NumSides, DepthPriority, Thickness, …)`(`Engine/Source/Runtime/Engine/Public/PrimitiveDrawingUtils.h:256`).
  - proxy 예시는 `FSphereSceneProxy`(`Engine/Source/Runtime/Engine/Private/Components/SphereComponent.cpp:111-171`, `GetDynamicMeshElements` `:133`, `GetViewRelevance` `:171`). `bHiddenInGame`은 `USceneComponent`(`SceneComponent.h:221`).
- **붙은 손 에셋:** AI_Driving `Content/VR_SteeringV2/VR_Steering/Hands/`(`SK_MannequinHand_Right`, 포즈 `MannequinHand_Right_Grab`, 재질). 오른손 하나뿐이라 왼손은 Z 스케일 −1로 뒤집는다. AI_Driving에서 맞춘 자리 값(`SteeringMesh` 기준)은 `AI_DrivingPawn.cpp:122-125` 주석에 있다.

## 1. 결정

| # | 결정 | 내용 |
|---|---|---|
| D1 | **`UDvtSteeringGripComponent`** (C++ `UPrimitiveComponent`) | `ADvtVehicle`이 생성자에서 만들어 `SteeringWheelPivot` 밑에 둔다(휠과 같이 돈다). **컴포넌트 로컬 X축 = 휠 회전축, 테두리는 로컬 YZ 평면의 원**(축 프로퍼티를 두지 않는다 — BP에서 컴포넌트를 돌려 맞춘다). 프로퍼티: `RimRadiusCm`(기본 18), `GripEnterDistanceCm`(기본 10), `GripReleaseDistanceCm`(기본 18), 범위 0–50. release < enter이면 판정에 release = enter를 쓰고 BeginPlay에서 `Warning` 한 줄. 충돌 없음 |
| D2 | **손 판정 = AI_Driving 규칙 그대로, 손마다 따로** | 매 tick 손마다 손바닥 점으로 §0 거리(컴포넌트 로컬, 스케일 뺌)를 재고 §0 히스테리시스를 쓴다. 시작은 둘 다 잡음. 추적 기능이 없거나 무효 → 잡음. 그 손만 추적이 끊김 → 지금 상태 유지. 판정은 엔진 없이 시험할 수 있는 순수 함수 두 개로 뺀다: `거리(로컬 점, 반지름)`, `다음 상태(지금, 추적 기능 유효, 그 손 추적됨, 거리, enter, release)`(§6). 상태가 바뀔 때 `UE_LOG` 한 줄(AI_Driving `.cpp:130-144`처럼 거리 포함)과 `OnGripChanged(Hand, State)`(dynamic multicast, S6 진행자 창이 쓴다). 게터 `GetGrip(Hand)` |
| D3 | **보이기** | 잡음 → 붙은 손 보임, 추적 손 숨김. 놓음이고 추적됨 → 추적 손 보임, 붙은 손 숨김. **놓음이고 그 손 추적이 끊김 → 둘 다 숨김**(AI_Driving과 다름 — 마지막 자리에 멈춘 손을 보이지 않게). 보이기는 이 컴포넌트만 바꾼다(Meta 손 `ConfidenceBehavior = None`) |
| D4 | **에디터 표시 = scene proxy, Play 중 숨김** | 생성자 `bHiddenInGame = true`(§0 ArrowComponent와 같음). proxy가 그리는 것(테두리 평면 = 로컬 YZ): ① 테두리 원 `RimRadiusCm`(흰색) ② enter 범위: 반지름 `R−e`·`R+e`인 평면 원 둘 + 축 방향 `±e`로 띄운 반지름 `R` 원 둘(초록) — 도넛의 윤곽 ③ release 범위: 같은 방식으로 `r`(주황). `R−e`·`R−r`가 0보다 작으면 그 원은 그리지 않는다. 프로퍼티를 바꾸면 바로 다시 그린다(render state dirty). bounds = 반지름 `R + max(e, r)`의 구. **런타임에 world 디버그 선·구를 그리지 않는다**(§0, D-06). PIE·VR Preview·패키지 게임에서 안 보이고, 에디터 뷰포트(Play 아님)에서는 차량 BP 뷰포트와 레벨 뷰포트 모두에서 보인다 |
| D5 | **붙은 손 = `ADvtVehicle`의 skeletal mesh 두 개** | `LeftGripHand`·`RightGripHand`(`USkeletalMeshComponent`, `SteeringWheelPivot` 밑, 충돌 없음). mesh·포즈·자리는 BP에서 정한다(코드에 손 에셋을 두지 않는다, AI_Driving `AI_DrivingPawn.cpp:117-121` 이유와 같음). 포즈는 애니메이션 하나(single node) 또는 Anim BP — BP 설정 |
| D6 | **추적 손 = `ADvtVRPawn`의 `UOculusXRHandComponent` 두 개** | 루트(Stage 원점) 밑. `SkeletonType`·`MeshType` 좌·우, `ConfidenceBehavior = None`, 충돌 없음. 위치는 D2와 같은 tick에서 손목 transform으로(§0, 회전 보정 프로퍼티 `TrackedHandRotationOffset` 기본 0, AI_Driving `.h:136`). 차량 tick이 Meta 손 tick 뒤에 오도록 선행 조건 |
| D7 | **연결** | S1 D4에서 controller가 Pawn을 차량에 붙인 직후 `Grip->SetTrackedHands(Pawn의 왼손, 오른손)`. 연결 전이나 Pawn·차량이 없으면 판정하지 않는다(붙은 손 보임) |
| D8 | **모듈 의존** | `DynamicVRTemplate.Build.cs`에 `OculusXRInput`(Meta 손 컴포넌트), `HeadMountedDisplay`(이미 있음, `IHandTracker`), `RenderCore`·`RHI`가 proxy에 필요하면 그것. Meta XR은 이 프로젝트에 늘 있다(AGENTS.md) — AI_Driving의 `WITH_METAXR_HANDS` 분기는 두지 않는다 |
| D9 | **손 에셋은 커밋한다** | AI_Driving `Content/VR_SteeringV2/VR_Steering/Hands/`를 **같은 경로**로 파일 복사한다(참조 경로가 맞게). 사용자 2026-11-24: 에셋 라이선스는 상관없다(S1 D11과 같음) |

## 2. 지켜야 할 조건

- 헤드셋에도 PC 화면에도 글·디버그 선을 그리지 않는다. `DrawDebug*`, `AddOnScreenDebugMessage`, `AddToViewport`, `PrintString` 화면 출력 금지. 알림은 `UE_LOG`.
- 손 상태는 표현일 뿐이다. 차량 입력·조향·휠 회전(S1 D5)에 영향을 주지 않는다.
- S1 동작(입력·Reset·attach)은 바뀌지 않는다.
- 판정 영역은 프로퍼티 값 그대로다. 숨은 여유를 더하지 않는다(에디터 그림 = 판정).

## 3. 범위 밖

- 손이 다가온 자리에 붙기(가장 가까운 rim 점) — 사용자가 고정 포즈로 정함(D-05).
- 차마다 다른 휠 반지름·손 자리 — S3(차 세트가 이 컴포넌트 값을 바꾼다).
- 손가락 맞닿음·물리, 컨트롤러.
- 진행자 창 손 상태 표시 — S6(`OnGripChanged`·`GetGrip`을 쓴다).

## 4. 구현자가 정할 것

- proxy 원의 변 수·선 두께·색 값(흰·초록·주황 계열 안에서).
- 손바닥 키포인트 읽기를 `GetAllKeypointStates`로 할지 `GetKeypointState`로 할지(둘 다 §0 헤더).
- 파일 배치.

## 5. 에디터 작업 (MCP, 코드 빌드·시험 뒤)

- D9 손 에셋 복사 뒤 에디터에서 참조 오류 없이 열리는지 본다.
- `BP_SampleVehicle`: `SteeringGrip`을 샘플 휠 테두리에 맞춘다(회전·`RimRadiusCm`). 붙은 손 mesh·포즈를 정하고 9시·3시 쪽 테두리에 보이게 놓는다(왼손 Z 스케일 −1). 에디터 뷰포트에서 테두리·enter·release 원이 휠과 맞는지 스크린샷으로 본다.
- PIE에서 원이 안 보이는지 스크린샷으로 본다. 저장하고 에디터를 닫는다.

## 6. 경계 상황과 시험 (자동화 `DynamicVRTemplate.S2.*`)

| # | 상황 | 기대 |
|---|---|---|
| E1 | 거리 — 테두리 위 점 (0, 18, 0), R 18 | 0 |
| E2 | 거리 — 중심 (0,0,0) | 18 |
| E3 | 거리 — 테두리에서 축으로 5, 밖으로 12 (5, 30, 0) | 13 |
| E4 | 놓음, 거리 9.9 / 10 (enter 10) | 잡음 / 놓음 (enter는 "작으면") |
| E5 | 잡음, 거리 18 / 18.1 (release 18) | 잡음 / 놓음 (release는 "크면") |
| E6 | 잡음, 거리 15 (enter 10 < 15 < release 18) | 잡음 유지. 놓음에서 같은 거리 → 놓음 유지 |
| E7 | 잡음·놓음 각각에서 그 손 추적 끊김 | 상태 그대로 |
| E8 | 추적 기능 무효(손 상태 무관) | 잡음 |
| E9 | release 5 < enter 10 | 판정은 release = 10으로. 잡음, 거리 10 → 잡음, 10.1 → 놓음 |
| E10 | 보이기 표 | 잡음 → (추적 숨김, 붙은 손 보임). 놓음+추적 → (보임, 숨김). 놓음+끊김 → (숨김, 숨김) |
| E11 | 컴포넌트 기본값 | `bHiddenInGame` true, 충돌 없음, 기본 R 18 / e 10 / r 18 |
| E12 | 왼손 상태만 바뀜 | 이벤트 1회(Left), 오른손 상태·이벤트 없음 |

E1~E10은 순수 함수로, E11은 CDO로, E12는 컴포넌트에 합성 입력을 넣는 시험 경로(구현자가 정함 — 시험 전용 setter 등)로 시험한다. 실제 손은 Quest(T-02).

## 7. 완료 조건

- non-unity와 unity 빌드 성공, 경고·오류 0.
- `DynamicVRTemplate.*` 전체 시험 통과(S1 시험 포함).
- §5 결과: 에디터 뷰포트에서 원이 보이고 PIE에서 안 보이는 스크린샷 두 장(리뷰용, `Saved/`에 두고 커밋하지 않음 — 경로를 보고에 적는다).
- `git status`에 Meta XR·MCP 플러그인 없음. `Content/VR_SteeringV2/VR_Steering/Hands/`는 커밋에 있다.

## 8. 로컬에서 확인할 것 (사용자, Quest 3 + 벅)

- T-02: 손을 휠에서 떼면 추적 손, 휠로 가져가면 붙은 손. 경계에서 깜빡이지 않는다. 휠을 잡고 크게 돌리는 동안 붙은 손이 사라지지 않는다(추적 끊김 유지). 손을 무릎에 내려 시야 밖으로 내보내면 손이 허공에 멈춰 있지 않다(D3).
- enter·release 값이 벅 휠에 맞는지(멀리서 붙거나 잡고 있는데 떨어지면 값 조정).
- 헤드셋에 원·선·글이 보이지 않는다.
