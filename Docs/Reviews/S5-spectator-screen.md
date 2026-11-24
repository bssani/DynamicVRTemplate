# S5 리뷰 — 관전 화면

명세: `Docs/Specs/S5-spectator-screen.md`

작업 브랜치: `codex/S5-spectator-screen` (베이스 `main@158bf4b`)

## 리뷰 1회차 — 통과 (Claude, 2026-11-24, `ffc87f3`)

고칠 것 없음. 머지 가능(사용자 승인 뒤).

- **배치(D3):** h = min(1, (A − m)/E), 시야 x 0 → hE/A, 칸 x hE/A → 1. 엔진 생성자 인자 순서(`HeadMountedDisplayTypes.h:171` — 시야 min/max, 텍스처 min/max, `bDrawEyeFirst`, `bClearBlack`, `bUseAlpha`)와 맞게 `false, true, false`. E1 네 비율 값을 손으로 다시 계산해 맞다.
- **배율:** `DrawWidget`은 `MakeRoot(DrawSize / Scale, Scale)`로 그린다(엔진 `WidgetRenderer.cpp:157`). 그래서 위젯은 1080 기준 크기로 배치되고, 줄바꿈 폭 `DesignSize().X − 66`(여백 32 + 32 + 선 2)이 맞다.
- **시작·정리(D1·D2):** HMD 세 조건 + `CanEverRender`, 차량 하나·`Cars` ≥ 1·선택 있음. 시작한 적이 있을 때만 모드를 되돌린다(E5 "모드를 건드리지 않음"). `FWidgetRenderer`는 `BeginCleanup`. 차량·HMD가 사라지면 정리.
- **칸(D4·D5·D6):** CAR/HOOD 제목, 이름, 구분선, 왼쪽 선, hood 없으면 구분선까지 숨김. 빈 이름 `Car k`/`Hood k`. 매 그리기마다 다시 읽음, 이벤트 구독 없음. 번호·속도 없음.
- **팔레트·Config(D7·D8):** `UI/DvtUiStyle.h`에 sRGB 변환. 프로퍼티 10개 `Config`, `DefaultGame.ini`에 새 절 없음.
- **시험:** 로그에서 직접 셈 — S5 6개 포함 88개 통과. 빌드 경고·오류 없음. S1–S4 코드·에셋·Config 변경 없음.
- **README:** 명세 §7 항목이 모두 있다. 설정 절 경로 `[/Script/DynamicVRTemplate.DvtSpectatorSubsystem]`가 실제 클래스와 맞다.
- **참고(고치지 않아도 됨):** 자동화는 `-nullrhi`로 돌아 `CanEverRender`가 거짓이므로, E5 시험의 "30 s 뒤 조용히 끝남"은 처음부터 대기하지 않는 경로만 본다. 헤드셋 없이 그릴 수 있는 경로(0.5 s마다 다시 시도 → 30 s 포기)는 `-nohmd` PIE 50 s 실행에서 S5 경고·오류가 없는 것으로만 확인됐다. 동작은 코드로 보아 맞다(`Tick`의 `PendingSeconds >= 30`).

T-05는 `not-tested`(Quest 3 + Link, 사용자). 특히 §8-1 — PC 시야가 검지 않은지, 헤드셋에 칸이 안 보이는지.
