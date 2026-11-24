# DynamicVRTemplate — 시팅 벅 위 VR 주행 (UE 5.7.4 + Meta Quest 3)

참가자가 실물 시팅 벅(휠·가속·브레이크 페달)에 앉아 VR 속 차를 몰며 앞쪽 hood를 본다. 차 디자인과 차마다 있는 hood 변형을 Play 중에 바꾼다. 관전자는 PC 화면(spectator)으로, 진행자는 따로 뜨는 진행자 창으로 본다.

**작업 전에 [Docs/Plan.md](Docs/Plan.md)를 읽을 것.** 범위·결정·단계·시험이 모두 거기 있다. 이 파일은 코드를 쓸 때 매번 필요한 규칙만 뽑은 것이다.

작업 방식과 규칙의 원본은 vBoR 리포다(ReachStudy와 같다). Codex와 Claude Code가 함께 읽고, `CLAUDE.md`는 이 파일을 가져와 Claude 전용 내용만 덧붙인다.

> **이 리포는 public이다.** 차량 CAD·회사 이름·차명·프로그램명·일정·평가 결과를 커밋·PR·이슈·문서에 넣지 않는다. CAD는 `Content/CAD/`, 현장 전용 에셋(회사 맵 등)은 `Content/Local/`에만 둔다(둘 다 .gitignore). Meta 플러그인은 커밋하지 않는다(Oculus SDK License).

## 작업 방식 — 명세·리뷰는 Claude, 구현은 Codex

- 작업 단위는 [Docs/Specs/](Docs/Specs/)의 명세 파일 하나다. 명세의 범위와 완료 조건만 구현한다. 명세에 없는 기능·리팩터링·"나중에 쓸" 추상화를 더하지 않는다.
- 아주 작은 일(문서·설정 몇 줄, 이름 고치기)은 명세 없이 Claude가 직접 한다(사용자 2026-11-24). 그 밖은 명세 → Codex.
- 명세가 이 파일·계획서·플러그인 헤더와 어긋나면 임의로 해석하지 말고 멈춰서 보고한다. 재량은 명세의 "구현자가 정할 것"에 적힌 항목뿐이다.
- 브랜치는 `codex/<명세 파일 이름>`, 베이스는 명세에 적힌 브랜치다.
- 작업을 끝낼 때 보고한다: ① 바뀐 파일 ② 명세의 완료 조건별 충족 여부 ③ 명세와 다르게 한 것과 이유 ④ 쓴 플러그인·엔진 API를 헤더로 확인했는지(경로:줄) 추정인지 ⑤ "로컬에서 확인할 것" — 빌드, 에디터 작업, Quest 3 시험(계획서의 T-ID).
- 같은 보고를 그 작업 마지막 커밋 메시지 본문에도 쓴다.
- 리뷰 지적은 같은 브랜치에 새 커밋으로 반영한다. amend·rebase로 리뷰한 커밋을 고쳐 쓰지 않는다.
- 리뷰 통과와 로컬 빌드 확인 전에는 `main`에 머지하지 않는다.
- **주고받는 내용은 파일로 넘긴다.** 리뷰는 [Docs/Reviews/](Docs/Reviews/)`<명세 이름>.md`에 회차별로(항목 상태 `열림` / `반영함 (<커밋>)`). 항목 형식은 **문제**(반드시 해결)·**제안**(권장)·**확인할 시험**, 필요할 때만 짧은 스케치. 질문은 리뷰 파일 "질문" 절에 쓰고 멈춘다.
- **진입점은 리포 루트의 `HANDOFF.md`(git 무시).** Claude가 지금 할 일·브랜치·명세·리뷰 파일을 적는다. Codex는 먼저 이 파일을 읽고, "지금 할 일"이 없으면 아무것도 하지 않는다. 끝나면 "Codex 결과" 절을 채운다.
- 구현은 Codex 데스크톱 앱에서 한다. Claude는 `codex exec`로 구현을 시작하지 않는다.
- **일을 키우지 않는다.** 하위 에이전트·자체 리뷰 루프 없음. 빌드(non-unity `-Module=DynamicVRTemplate` + unity)와 `DynamicVRTemplate.*` 전체 시험은 작업 끝에 한 번. 가장 단순한 방법으로 명세를 채우고, 막히면 질문한다.
- 절차·명령·로그 위치는 [Docs/Agent-Workflow.md](Docs/Agent-Workflow.md).

## 코드·에셋은 영어만 — 주석은 짧은 조각문

코드와 에셋에는 한글을 쓰지 않는다. 주석, `Category`·`DisplayName`·`ToolTip`·`UMETA`, 로그, 화면 문구가 모두 영어다. 문서(`Docs/`)·커밋 메시지·리뷰 파일은 한국어.

- 영어 조각문으로 짧게. **왜** 그런지를 쓴다: `Hold grip while tracking is lost (fist hides the hand)`.
- 근거 결정·명세 ID를 괄호로: `(D-05)`, `(S2 §3)`.
- 단위를 적는다(`cm`, `s`, `deg`, `km/h`). UE는 cm다.
- 코드를 그대로 읽어 주는 주석은 쓰지 않는다.

## 확정된 제약 — 바꾸려면 사용자 승인

- **장비는 Meta Quest 3, PCVR(Link).** 컨트롤러 없음, 손은 광학 핸드 트래킹. Android 독립 실행은 고려하지 않는다.
- **실물은 시팅 벅이다.** 시트·휠·가속·브레이크 페달이 있다. 주행 입력은 벅의 휠·페달에서 RawInput으로 온다(계획서 D-03, 키보드는 개발용). 포스 피드백 없음.
- **주 콘텐츠는 주행 중 hood가 어떻게 보이느냐다**(D-12). 주행 감각·차량 튜닝·속도 표시에 일을 들이지 않는다.
- **차량 Reset은 가장 가까운 reset point로 페이드 이동이고, recenter하지 않는다**(D-13). 자동 충돌 감지 없음.
- **모든 차의 SgRP(힙 포인트)와 주행 특성은 같다.** 다른 것은 차체 디자인과 hood다(D-02). 그래서 차 교체·hood 교체는 **같은 레벨, 같은 차량 Actor 안에서 보이는 mesh만 바꾼다.** 물리·정렬·눈 위치는 그대로다. 주행 중에도 바꿀 수 있다.
- **헤드셋 안에는 어떤 메시지도 보이지 않는다**(사용자 2026-11-24). 디버그·경고·안내 글을 VR 화면에 쓰지 않는다(`AddOnScreenDebugMessage`·`AddToViewport`·`PrintString` 금지, 화면 메시지는 꺼 둔다). 관전 UI는 spectator screen으로만(모니터에만 보이는 경로, D-07), 진행자 UI는 따로 뜨는 OS 창으로만(D-08). 알림은 `UE_LOG`로. **예외 하나(사용자 2026-11-24):** 정렬 레벨 `L_Calibrate`에서 VRCalibrationKit 정렬 도구가 G~H 사이에 보이는 정렬점 번호 1–4(설치 담당자용, S4 D6). 주행 레벨에는 예외가 없다.
- **Buck 정렬은 설치 때 한 번이다.** 마커는 벅의 고정된 곳에 붙인다(D-01). 차를 바꿔도 다시 맞추지 않는다. 정렬은 VRCalibrationKit 플러그인으로 한다.
- **손이 휠에 붙는 것은 표현이다.** 손이 휠 테두리 가까이 오면 휠에 붙은 고정 포즈 손을 보이고, 멀어지면 추적 손을 보인다(D-05). 휠을 돌리는 것은 실물 휠 입력이지 가상 손이 아니다. 추적이 끊기면 잡은 상태를 유지한다(주먹이 손을 가림).
- **휠 손 붙는 범위는 에디터 뷰포트에서 보이고 Play 중에는 숨긴다**(D-05).

## 구조 원칙

- **VR Pawn과 차량은 따로다**(D-04). 플레이어는 카메라·손만 가진 VR Pawn을 possess하고, PlayerController가 휠·페달 입력을 지금 차량의 Chaos 이동 컴포넌트로 넘긴다. VRCalibrationKit은 이 VR Pawn을 차량 Actor에 attach한다.
- 차 목록·hood 목록·이름은 코드에 두지 않는다. 에디터에서 데이터(BP·DataAsset)로 고른다. 실제 차명·CAD를 가리키는 데이터는 `Content/CAD/`·`Content/Local/` 안에 둔다(public).
- **입력(키보드·진행자 창 버튼) → 같은 명령 → 상태 변경.** 번호로 고르는 명령은 그 자체로 안전하고, 이전·다음 같은 상대 명령은 전환 뒤 쿨다운으로 막는다(ReachStudy S4 D2와 같음).
- **BP Event Graph에는 표현만.** 상태·판정은 C++.
- C++에는 지금 필요한 기능만. BP는 mesh·값만 바꾸면 바로 적용돼야 한다.

## 하지 말 것

- VR에서 본 hood 시야를 실차 시야 검증으로 서술하지 않는다(정렬 오차, 디스플레이 해상도·FOV, 가상 주행).
- 실제 Quest 3로 확인하기 전까지 시험 상태는 `not-tested`다. 컴파일 = 시험 통과가 아니다.
- Recenter(시야 재정렬)를 진행자 기능으로 넣지 않는다. 정렬이 깨진다.

## 빌드·환경

- UE 5.7.4 (`C:\Program Files\Epic Games\UE_5.7`), C++ 모듈 `DynamicVRTemplate`. 빌드: `Engine/Build/BatchFiles/Build.bat DynamicVRTemplateEditor Win64 Development -Project=<절대경로>\DynamicVRTemplate.uproject`
- **Meta XR 1.205.0(`Plugins/MetaXR/`)은 git에 없다.** 클론한 뒤 직접 복사한다(README). OculusXR API는 기억이 아니라 그 폴더의 헤더로 확인한다.
- 에디터는 `.uasset`을 잠근다. git 브랜치 전환·머지·C++ 리빌드 전에 Unreal Editor를 닫는다.
- **에디터 작업(레벨·에셋·배치)은 에이전트가 MCP로 직접 한다.** 에디터 MCP 플러그인 `Plugins/McpAutomationBridge`·`Plugins/UnrealAgent`는 vBoR에서 복사한 로컬 도구다(git 밖). 연결·순서·하지 않는 것은 [Docs/Agent-Workflow.md](Docs/Agent-Workflow.md) §2.8. 이 프로젝트의 코드·에셋은 이 플러그인을 참조하지 않는다.
