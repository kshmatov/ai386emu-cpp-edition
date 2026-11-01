# Tasks (объединено по системе)

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

Секции — исходные scoped changes. Номера задач перенумерованы сквозно по секциям (N.M). Исходные номера — в конце каждой строки вида `(было X.Y)`.

## 1. CPU: RF в fault frame  <!-- source: cpu-fault-resume-flag -->

- [x] 1.1 Зафиксировать RED decoded FILD и таблицы fault/trap/interrupt/abort на текущем CPU. Приёмка: падение именно saved RF, корректные остальные поля. Лог: длительность теста и файлы; точный токен-счётчик инструментами не доступен. (было 1.1)
- [x] 1.2 Исправить fault delivery, RF lifetime после IRET/task switch/POPF, task-gate saved IP/error code/SS.B и ожидаемый RF в старых регрессиях, сохранив проверки остальных битов. Приёмка: FILD288 и event distinction PASS. Лог: команды, длительности и изменение; токен-счётчик не доступен. (было 1.2)
- [x] 1.3 Выполнить CPU unit/race, machine FPU, vet, strict OpenSpec validate и git diff --check. Приёмка: PASS всех команд. Лог: длительности/exit; токен-счётчик не доступен. (было 1.3)
- [x] 1.4 Принять fix независимым review и гостевым boot/FPU/halt на изменённой сборке обеих ОС. Приёмка: отсутствие high/blocking findings и строгие guest markers/exit0; сохранность отчёта между boots документирована отдельно. Лог: SHA сборки, сессии/длительности; токен-счётчик не доступен. (было 1.4)

## 2. TB: физадрес блока вместо логического EIP  <!-- source: tb-correctness-device-clock -->

- [x] 2.1 Воспроизвести «Invalid Opcode» на `TestFreeDOSInstallMenu` (TB on, JIT off) и локализовать расхождение stored vs fresh decode (eip=522, CS=0xFFFF: stored `PUSH 0x50` при байтах `8C D6 8C` = `MOV SI,SS`). Приёмка: бисект показывает, что `op.insn` битый, `op.eip`/`op.next`/handlers корректны. Лог: диагностика TB-DECODE-MISMATCH/TB-FILL в сессии 2026-10-17. (было 1.1)
- [x] 2.2 Добавить `tbOp.phys`; сверять `tbPhys(eip)==op.phys` в `stepTB` (на входе в блок) и `tbGo` (chain-link + lookup). Приёмка: `TestTBAfterFarJumpUsesNewCSCode`, `TestTBAfterFarJumpDifferentCodeAtSameEIP` PASS; `TestFreeDOSInstallMenu` PASS (Welcome + язык). Лог: 4 новых теста + FreeDOS 11.9 s. (было 1.2)
- [x] 2.3 `stepTB`/`fillTB`/`tbValidate` возвращают `false` на любом отказе (fallback `Step()`/refill), не `canUseTB()`. Приёмка: нет phantom-слотов; suite PASS. Лог: 11800 green. (было 1.3)
- [x] 2.4 Регрессионные тесты CS-phys: два сегмента с разным кодом на одном EIP, far jump между ними. Приёмка: `internal/cpu/tb_cs_phys_test.go` PASS. (было 1.4)

## 3. Device-clock: floor в RunBatch  <!-- source: tb-correctness-device-clock -->

- [x] 3.1 `RunBatch` считает `cycles = max(delta, max(executed, 1))`. Приёмка: install-profile доходит до Welcome (не зависает на ISOLINUX), `clock_cycles=708M` (≈ историч. 704M). Лог: PROFILE total_wall=37.62s steps=211.6M clock_cycles=708551580. (было 2.1)
- [x] 3.2 Замер batch vs Step на install-profile. Приёмка: `RunBatch(32)` 38.1 s vs `Step()` 35.0 s при одинаковых 708M cycles → batch не выигрывает; `machine.run()` → `Step()`. Лог: два прогона PROFILE 2026-10-17. (было 2.2)

## 4. JIT: флаги INC/DEC и отключение  <!-- source: tb-correctness-device-clock -->

- [x] 4.1 `emitSetFlagsKeepCF` для INC/DEC (CF гостя не затирается). Приёмка: `TestJITIncDecPreservesCF`, `TestJITDecPreservesCF` PASS (JIT и интерпретатор совпадают). Лог: `internal/cpu/jit_flags_test.go`. (было 3.1)
- [x] 4.2 `disableJIT=true` по умолчанию в `cpu.New()`; `CPUReset` сохраняет `disableJIT`. Приёмка: `TestFreeDOSInstallMenu` PASS с JIT в коде (disableJIT переживает `m.cpu.Reset()`). Лог: PASS 11.92s после фикса `CPUReset`. (было 3.2)

## 5. Валидация  <!-- source: tb-correctness-device-clock -->

- [x] 5.1 `go test ./... -count=1`. Приёмка: 11800 passed / 27 packages. Лог: 2026-10-17. (было 4.1)
- [x] 5.2 `go vet ./...`. Приёмка: no issues. Лог: 2026-10-17. (было 4.2)
- [x] 5.3 `TestFreeDOSInstallMenu` (LiveCD boot → Welcome → Install menu → language screen). Приёмка: PASS, экран содержит «Welcome to FreeDOS 1.4», «Install to harddisk», «What is your preferred language?». Лог: 11.92 s. (было 4.3)
- [x] 5.4 `TestFreeDOSInstallProfile` (boot→Welcome, `clock_hz=1e6`). Приёмка: boot-to-welcome reached=true, boot-menu reached=true; PROFILE total_wall=37.62s clock_cycles=708551580. Лог: `/tmp/ai386-freedos-install-profile.txt`. (было 4.4)
- [x] 5.5 Кросс-сборка `GOOS=windows GOARCH=amd64 go build ./...` и `go build -tags desktop ./cmd/ai386`. Приёмка: без ошибок (JIT — `//go:build unix && amd64`, заглушка `jit_stub.go` для остальных платформ). Лог: 2026-10-17. (было 4.5)
- [x] 5.6 `TestJITCompileTB` (32-битный CS.D=1 protected mode, `disableJIT=false`). Приёмка: jitOps>0, jitExec ставит EAX=5. Лог: fix `jit_raw_test.go` — раньше real mode давал `jitCanNative` false. (было 4.6)
