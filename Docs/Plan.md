# DynamicVRTemplate 계획서

작성 2026-11-24 (Claude). 결정이 바뀌면 이 문서를 고치고 아래 "개정 이력"에 남긴다.

## 1. 한 줄

참가자가 실물 시팅 벅(시트·휠·가속·브레이크 페달)에 앉아 VR 속 차를 몰며 앞쪽 hood 높이를 본다. 차(차체 디자인)는 여러 대이고 차마다 hood 변형이 있다. 진행자가 Play 중에 차와 hood를 바꾼다. 관전자는 PC 모니터에서 지금 차·hood를 보고, 헤드셋에는 글이 하나도 없다.

## 2. 결정 (사용자 2026-11-24)

| ID | 결정 | 내용 |
|---|---|---|
| D-01 | **정렬 = VRCalibrationKit, 설치 때 한 번** | vBoR에서 빼낸 마커(ArUco 4개) + 앵커(선택) 정렬 플러그인(`C:\Git\VRCalibrationKit`)을 쓴다. 마커는 **시팅 벅의 고정된 곳**에 붙인다. 키트는 VR Pawn을 차량 Actor에 attach하므로 차가 달려도 같은 정렬이 유지된다(키트 `Docs/Integration-Guide.md` §1·§11) |
| D-02 | **모든 차의 SgRP·주행 특성은 같다** | 다른 것은 차체 디자인과 hood다. 그래서 **차량 Actor는 하나**이고, 차 교체 = 그 안의 차체 mesh 세트 교체, hood 교체 = 그 차의 hood mesh 교체다. 정렬점·눈 위치·물리·휠 위치는 그대로다. **같은 레벨에서, 주행 중에도** 바꿀 수 있다(물리·정렬을 건드리지 않으므로 위험이 없다). 차마다 레벨을 따로 두지 않는다 |
| D-03 | **주행 입력 = 벅의 휠·가속·브레이크 페달, RawInput** | 벅 장치는 simracing 소프트웨어를 거쳐 엔진 `RawInput` 플러그인으로 읽는다(사용자 2026-11-24 Q-01). 장치 VID/PID·축 설정은 `Config/DefaultInput.ini`의 `[/Script/RawInput.RawInputSettings]`(AI_Driving `Config/DefaultInput.ini:1-2` 예시), 축 → 조향·가속·브레이크 연결은 Enhanced Input. **포스 피드백 없음.** 키보드 주행은 개발용 |
| D-04 | **VR Pawn과 차량을 나눈다** | 플레이어는 카메라·손만 가진 VR Pawn을 possess한다. PlayerController가 휠·페달 입력을 차량의 `ChaosWheeledVehicleMovementComponent`로 넘긴다. 키트는 "Player 0 Pawn 루트를 차에 attach"하므로(키트 `VRCalAlignmentComponent.cpp:24`·`:123`) 차량을 possess하면 차 자체가 옮겨진다 — 그래서 나눈다. 키트는 고치지 않는다 |
| D-05 | **휠 손 붙기 = AI_Driving 방식, 고정 포즈** | 손바닥이 휠 테두리에서 `GripEnterDistance` 안이면 휠에 붙은 고정 포즈 손(왼손 9시·오른손 3시 쪽, BP에서 배치)을 보이고, `GripReleaseDistance`보다 멀어지면 추적 손을 보인다(히스테리시스, AI_Driving 기본 10 / 18 cm). 추적이 끊기면 잡은 상태 유지. 붙는 손은 휠 밑에 붙어 휠과 같이 돈다. **붙는 범위(테두리·enter·release 링)는 에디터 뷰포트에서 보이고 Play 중에는 숨긴다.** 출처 `C:\Git\AI_Driving\Source\AI_Driving\VR\VRHandPresenceComponent.*`(런타임 DrawDebug만 있음 → 에디터 표시는 새로) |
| D-06 | **헤드셋 안에 어떤 메시지도 없음** | 디버그·경고·안내 글 전부. 알림은 `UE_LOG`. 진행자 창·관전 화면은 PC에만 |
| D-07 | **관전(spectator) 화면** | PC 모니터에 참가자 시야 + 지금 **차 이름**과 **hood 변형 이름**만(사용자 2026-11-24 Q-05). 경로는 AI_Driving·vBoR P5h/P5k 방식(Slate → 렌더 타깃 → `ISpectatorScreenController` `TexturePlusEye`, 뷰포트에 올리지 않음 — 올리면 헤드셋에만 간다, AI_Driving `README.md`). 배치는 vBoR P5k(시야 4:3 왼쪽, 오른쪽 칸) |
| D-08 | **진행자(operator) 창** | vBoR P5i 방식의 따로 뜨는 OS 창(`SWindow`) + 에디터 툴바 버튼(P5j). 버튼 = 키보드와 같은 명령. 내용은 §5 |
| D-09 | **이 리포 = 기능만, 맵은 회사 PC에서** | 사용자가 이 프로젝트를 회사 PC로 옮겨 회사 평지 도로 맵과 CAD를 붙인다(사용자 2026-11-24 Q-02). 리포에는 기능 시험용 단순 평지 맵과 샘플 차만 둔다. 회사 쪽 에셋은 `Content/CAD/`·`Content/Local/`(git 밖) |
| D-10 | **프로젝트** | `C:\Git\DynamicVRTemplate`, GitHub `bssani/DynamicVRTemplate` **public**. UE 5.7.4, Meta XR 1.205.0(git 밖), 에디터 MCP 플러그인은 vBoR `Plugins/`에서 복사(git 밖) |
| D-11 | **작업 방식 = vBoR·ReachStudy와 같음** | Claude 명세·리뷰, Codex 앱 구현. 아주 작은 일은 Claude가 직접 |
| D-12 | **주 콘텐츠는 hood가 주행 중 어떻게 보이느냐** | 주행은 수단이다(사용자 2026-11-24). 주행 감각·차량 튜닝·속도 표시에 일을 들이지 않는다. Chaos 차량은 템플릿 기본값에서 시작하고, 흔들림·멀미가 hood 보기를 방해할 때만 손본다 |
| D-13 | **차량 Reset = 가까운 reset point로, 페이드** | 차가 박거나 길을 벗어나면 진행자가 Reset한다(키 `Backspace` + 진행자 창 버튼). 레벨에 놓은 reset point Actor(화살표 = 차 방향) 중 **차에서 가장 가까운 곳**으로 옮긴다. 하나도 없으면 Play 시작 때 차의 자리로. 순서: 검게 페이드 아웃 → 이동·속도 0 → 페이드 인(`PlayerCameraManager::StartCameraFade`, AI_Driving `AI_DrivingPawn.cpp:595-640`의 즉시 검게 → 페이드 인을 아웃·인으로). **AI_Driving과 달리 recenter하지 않는다**(정렬이 깨짐). VR Pawn은 차에 붙어 있어 같이 옮겨진다. 페이드는 글이 아니므로 D-06에 걸리지 않는다. 자동 충돌 감지 없음 |

## 3. 질문 → 결정 (사용자 2026-11-24)

| ID | 결정 |
|---|---|
| Q-01 | simracing 소프트웨어 + RawInput, 포스 피드백 없음 → D-03. 주 콘텐츠는 hood 보기 → D-12 |
| Q-02 | 리포는 기능만, 회사 PC로 옮겨 맵·CAD를 붙인다 → D-09 |
| Q-03 | 진행자 창에 속도 없음. Reset = 가까운 reset point + 페이드 → D-13, §5 |
| Q-04 | 가상 휠 위치는 고정(벅 휠), **mesh는 차마다 다르다** → 휠 mesh는 차 세트에 들어간다(§4) |
| Q-05 | 관전 화면은 차·hood 이름만 → D-07 |
| Q-06 | 차 `PageUp`/`PageDown`(이전·다음), hood `[`/`]`, 번호 직접 선택은 진행자 창 |

## 4. 구성

| 요소 | 내용 |
|---|---|
| VR Pawn (C++) | 카메라(HMD 고정) + 추적 손 두 개(Meta XR 손 컴포넌트, AI_Driving `AI_DrivingPawn.cpp:135`). 이동 없음(키트 attach를 방해하지 않게) |
| PlayerController (C++) | 휠·페달(RawInput)·개발용 키보드 → 차량 입력. 차·hood 교체 키, Reset 키 → 같은 명령 |
| 차량 Actor (C++ 부모 + BP) | Chaos 바퀴 차량 하나. 공통 물리, 정렬점 4개(키트 `VRCal Align Point`), 가상 휠 자리(고정), 차 세트 목록. 차 세트 = 차체 mesh 묶음 + **휠 mesh** + hood 변형 목록(Q-04) |
| Reset point (C++ Actor) | D-13. 레벨에 몇 개든 놓는다. 화살표 방향 = 차 방향 |
| 차·hood 교체 | 명령 `SelectCar(i)`·`NextCar`·`PreviousCar`, `SelectHood(i)`·`NextHood`·`PreviousHood`. 상대 명령은 전환 뒤 쿨다운. 차를 바꾸면 hood는 그 차의 첫 변형(또는 직전 번호 — S3에서 정함). 보이기·collision만 바꾸고 원래 값으로 되돌린다(ReachStudy `AReachDoorVariants` 방식) |
| 휠 손 붙기 컴포넌트 (C++) | D-05. 에디터 표시는 scene proxy 또는 컴포넌트 시각화로 뷰포트에 그리고 게임에서는 숨김(S2에서 정함) |
| 정렬 | VRCalibrationKit(복사해 넣고 출처 커밋을 적음). GameMode가 Stage 원점·WorldToMeters 100. 정렬 레벨은 따로(카메라는 거기서만) |
| 관전 화면 | D-07 |
| 진행자 창 | D-08, §5 |

## 5. 진행자 창 내용 (Q-03)

- **차**: 지금 차 이름 `(k/N)`, 이전·다음, 목록에서 고르기
- **Hood**: 지금 hood 이름 `(k/N)`, 이전·다음
- **정렬**: 상태(`Applied` / `NeedsCheck` / `NoSavedValue`)와 이유, 앵커 찾는 중 표시, `Reapply alignment` 버튼(키트 `RequestReapply`)
- **차량**: `Reset vehicle` 버튼(D-13, 가까운 reset point로 페이드 이동)
- **손**: 왼손·오른손 추적 여부, 휠 잡음/놓음
- 넣지 않음: 속도, Recenter(정렬이 깨짐), 헤드셋으로 보내는 글

## 6. 단계

| ID | 명세 | 핵심 | 완료 조건 |
|---|---|---|---|
| **S0** | 프로젝트 부트스트랩 | C++ 프로젝트, 규칙 문서, Meta XR·MCP 플러그인 복사(git 밖), git·GitHub | **완료 2026-11-24** (Claude). 빌드 결과는 개정 이력 |
| **S1** | 주행 rig | VR Pawn, PlayerController 입력 전달(RawInput 휠·페달 + 개발용 키보드), 공통 차량 C++ 부모 + 샘플 차 BP, 시험용 평지 맵, Reset point + 페이드 Reset(D-13), VR 렌더 설정(Instanced Stereo와 가상 텍스처 충돌 — AI_Driving README) | 자동화 시험: 입력 전달, 가장 가까운 reset point 고르기·없을 때 시작 자리. Quest: 벅에서 주행, Reset (T-01) |
| **S2** | 휠 손 붙기 | D-05 컴포넌트, 에디터 표시·게임 숨김, 고정 포즈 손 | 자동화 시험: 진입·이탈 히스테리시스, 추적 끊김 유지, 게임에서 숨김. Quest (T-02) |
| **S3** | 차·hood 교체 | §4 차 세트·명령·키, 쿨다운 | 자동화 시험: 교체 뒤 보이는 세트가 하나, 주행 중 교체에 물리 영향 없음, 동시 입력 (T-03) |
| **S4** | 정렬 가져오기 | VRCalibrationKit을 넣고(키트 PR #1 머지 뒤) 차량에 정렬점, 정렬 레벨 | 키트 시험 통과. Quest: 정렬 → 재시작 복원, 차 교체 뒤 같은 자리 (T-04) |
| **S5** | 관전 화면 | D-07 | 헤드셋에는 안 보이고 모니터에만 (T-05) |
| **S6** | 진행자 창 | D-08, §5, 툴바 버튼 | 버튼 = 키 같은 명령 시험 (T-06) |
| **S7** | 회사 환경 통합 | CAD 차체·hood(`Content/CAD/`), 회사 맵(`Content/Local/`), 실제 차 세트 | Quest 전체 흐름 (T-07) |

순서: S1 → S2 → S3 → S4 → S5 → S6 → S7. S2와 S3은 서로 의존하지 않는다. S4는 VRCalibrationKit PR #1 머지를 기다린다.

## 7. 시험 (Quest 3, 상태는 전부 `not-tested`)

| T-ID | 내용 |
|---|---|
| T-01 | 벅 휠·페달로 출발·가속·제동·조향. 가상 휠이 실물 휠과 같이 돈다. Reset하면 페이드 뒤 가장 가까운 reset point에, 정렬은 그대로 |
| T-02 | 손을 휠로 가져가면 붙고, 떼면 추적 손. 경계에서 깜빡임 없음. 휠을 잡고 돌리는 동안 손이 사라지지 않음 |
| T-03 | 주행 중 차·hood 교체(키보드·진행자 창). 차가 튀거나 멈추지 않음 |
| T-04 | 정렬 저장 → 에디터·Link 재시작 → 같은 자리. 차 교체 뒤에도 같은 자리. 앵커 켬/끔 |
| T-05 | 모니터에 시야 + 차·hood 이름, 헤드셋에는 글 없음 |
| T-06 | 진행자 창 모든 버튼, 정렬 상태 표시 |
| T-07 | 회사 맵·CAD로 전체 흐름 |

## 8. 위험

- **VR hood 시야는 실차와 다르다.** 정렬 오차(점별 ~1.5 cm, vBoR M2), 헤드셋 FOV·해상도, 가상 주행. 결과 해석에 같이 적는다.
- **주행 중 화면 흔들림·멀미.** 차체 pitch·roll이 Pawn에 그대로 전해진다(키트 §11). 필요하면 S1에서 서스펜션 값을 다룬다.
- 휠을 잡은 손은 광학 추적이 자주 끊긴다(주먹이 손을 가림) — D-05의 "끊김 = 잡은 상태 유지"로 덮는다.

## 개정 이력

- 2026-11-24 — 초안. 사용자 답 반영: SgRP·주행 같음(D-02, 차 교체는 같은 레벨 mesh 교체), 벅에 휠·페달(D-03), 고정 포즈(D-05), 헤드셋 글 없음(D-06), 평지 맵(D-09), 작업 방식(D-11). Q-01~Q-06 열림.
- 2026-11-24 — S0 완료(Claude). `DynamicVRTemplateEditor` Win64 Development 빌드 성공, 경고·오류 0(115 s, MetaXR 포함). 플러그인: ChaosVehiclesPlugin, XRBase, OpenXR, OpenXRHandTracking. Meta XR 1.205.0과 MCP 플러그인(McpAutomationBridge·UnrealAgent)은 vBoR `Plugins/`에서 로컬 복사, git 밖.
- 2026-11-24 — Q-01~Q-06 사용자 답: D-03 RawInput·포스 피드백 없음, D-12 주 콘텐츠 = hood 보기, D-13 Reset(가까운 reset point + 페이드, recenter 없음), D-09 리포는 기능만, Q-04 휠 mesh는 차 세트에, 진행자 창에서 속도 뺌.
