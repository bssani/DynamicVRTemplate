# S4 구현 질문 — 정렬 가져오기

명세: `Docs/Specs/S4-calibration.md`

작업 브랜치: `codex/S4-calibration` (베이스 `main@cd9ae51`, S3 머지 확인)

작성: Codex, 2026-11-24. Claude의 구현 리뷰 전 질문 기록이다.

## 질문

### Q4-1 정렬 레벨의 차량 배치 지시가 다름 — 답함

- **답(Claude, 2026-11-24):** 명세 D5가 맞다. `L_Calibrate`에 `BP_SampleVehicle`을 하나 둔다(정렬 component·점 4개가 차량에 있다). HANDOFF의 "차를 두지 않는다"는 **차체 Actor(`ADvtCarBody`)를 두지 않고 차량 인스턴스의 `Cars`를 비운다**는 뜻이었다 — Claude가 잘못 줄여 썼다. 명세 D5에 이 조건을 적었고 HANDOFF도 고쳤다. §5의 두 레벨 차량 인스턴스 scale 확인도 그대로 한다.

- **문제(Codex):** `HANDOFF.md:12`는 "`L_Calibrate`에는 차를 두지 않는다(D5)"라고 한다. 명세 D5(`Docs/Specs/S4-calibration.md:57`)는 "`L_Calibrate`에 `BP_SampleVehicle` 하나, `AVRCalMarkerCalibrator` 하나"를 배치하라고 한다. §5(`:84`)도 두 레벨의 차량 인스턴스 scale 확인을 요구한다.
- **질문:** `L_Calibrate`에 `BP_SampleVehicle`을 배치하는 것이 맞는가? 차량 Actor 자체를 두지 않는 뜻이라면 정렬 component와 네 점을 어느 Actor에 둘지도 명세에 정해 달라. 차체 선택 목록만 비운다는 뜻이라면 그 조건을 명시해 달라.
- **현재 상태(질문 당시):** 작업 브랜치만 만들었다. 플러그인 복사·코드·설정·에셋 변경, 빌드·자동화·에디터 작업은 시작하지 않았다. AGENTS.md의 충돌 시 중지 규칙에 따라 답을 기다린다. Quest T-04는 `not-tested`다.

### Q4-2 정렬점 번호는 헤드셋에 보이는 글이다 — 답함 (Claude가 찾음)

- **문제(Claude, 2026-11-24):** 명세 D6이 정렬점 번호를 "글이 아닌 표시"라고 적었다. 키트 헤더로 보면 번호는 `UTextRenderComponent`(노란 숫자 `Order`, 3 cm)이고, 정렬 도구가 G에서 켜고 H에서 끈다(`VRCalMarkerCalibrator.cpp:585`, `:612`). AGENTS.md "헤드셋 안에는 어떤 메시지도 보이지 않는다"와 어긋났다.
- **답(사용자, 2026-11-24):** 정렬 레벨만 예외로 허용한다. 키트는 고치지 않는다. 명세 D6·§0·§2·§7·§8·§9 Q2, AGENTS.md, 계획서 D-06에 예외를 적었다. 주행 레벨에는 정렬 도구가 없어 번호가 나오지 않는다. 구현할 것은 바뀌지 않는다(키트 그대로 복사).
- **함께 고침:** 명세 머리 베이스 "S1 머지 뒤" → "S3 머지 뒤"(HANDOFF와 맞춤. Codex 베이스 `cd9ae51`은 그대로 맞다).
- **다시 대조함(Claude):** 명세가 인용한 키트 줄(`VRCalMarkerCalibrator.cpp:102-105`·`:508`·`:510`·`:636`·`:646`, `VRCalAlignmentComponent.cpp:24`·`:92-95`·`:122-124`·`:194-199`), 로그 문구(`Describe` — `Applied`/`No saved value`/`NeedsCheck`), 설정 절(`[/Script/OculusXRHMD.OculusXRHMDRuntimeSettings]`), 키트 시험 수(46), 플러그인 파일 종류(`.gitignore`에 걸리는 파일 없음)가 `e915647`과 맞다.
