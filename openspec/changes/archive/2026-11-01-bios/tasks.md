# Tasks (объединено по системе)

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

Секции — исходные scoped changes. Номера задач перенумерованы сквозно по секциям (N.M). Исходные номера — в конце каждой строки вида `(было X.Y)`.

# Задачи fix-bda-cursor

## 1. Воспроизведение и диагноз  <!-- source: fix-bda-cursor -->

- [x] 1.1 Headless-репро на `freedos-ready.img`: `dir` → символ → Backspace, замер CRTC/VGA/BDA. Критерий: зафиксирован `crtcCur=20479` и `pos=(51,15)`. (было 1.1)
- [x] 1.2 Установить, что `20479=255*80+79` и что BDA 0x450/0x451 остаётся (0,0). Критерий: соответствие `SetCursorPosition(255,79)`. (было 1.2)

## 2. Исправление  <!-- source: fix-bda-cursor -->

- [x] 2.1 `writeBDACursor` + запись BDA при AH=00h/02h/0Eh (`internal/bios/int10.go`). Критерий: unit-тесты ниже зелёные. (было 2.1)
- [x] 2.2 Регрессии `internal/bios/int10_bda_cursor_test.go`: AH=02h, teletype, AH=03h↔BDA, SetMode→(0,0), backspace из BDA остаётся на экране. Критерий: до фикса red, после — green. (было 2.2)

## 3. Верификация  <!-- source: fix-bda-cursor -->

- [x] 3.1 Живой прогон FreeDOS: после `dir` Backspace стирает символ (24,5→24,4), `echo hi` печатается и выполняется, BDA=VGA. Критерий: нет `crtcCur=20479`. (было 3.1)
- [x] 3.2 `go test ./...`, `go vet ./...`, race по bios/ps2/vga, `openspec validate --all --strict`. Критерий: всё зелёное. (было 3.2)
- [x] 3.3 Зафиксировать проблемы и проверки в `verification.md` и дельта-спеке `bios`. Критерий: артефакты change полны, strict validate PASS. (было 3.3)

## 4. Исправление  <!-- source: doom-bios-video-mode -->

- [x] 4.1 Зафиксировать actual guest shutdown code и добавить red BIOS regression. (было 1.1)
- [x] 4.2 Обновить BDA449 после успешного INT10 mode set, проверить возврат текста и отклонённый режим. (было 1.2)

## 5. Приёмка  <!-- source: doom-bios-video-mode -->

- [x] 5.1 Повторить полную rendered DEMO1 в режимах2/4 при1/100MHz до visible timing/DOS return. (было 2.1)
- [x] 5.2 Выполнить tests/race/vet/build/strict OpenSpec и независимый review; обновить итоговый отчёт и roadmap по доказанным критериям. (было 2.2)

## 6. Воспроизведение  <!-- source: doom-interrupt-compatibility -->

- [x] 6.1 Добавить tools/doom с отдельной копией HD, проверкой счётчика, RAM/PNG/регистрами и INTA/PIC; выполнить исходный red-прогон. (было 1.1)
- [x] 6.2 Воспроизвести отсутствие IRQ1 без ручной правки PIC гостевым тестом; проверить исходный BIOS handler и INT16 очередь. (было 1.2)

## 7. Исправления  <!-- source: doom-interrupt-compatibility -->

- [x] 7.1 Проследить последний IRQ0, минимизировать причину отсутствия EOI, записать подтверждённый алгоритм исправления и добавить red-регрессию. (было 2.1)
- [x] 7.2 Исправить подтверждённую причину IRQ0, проверить минимальную регрессию и исходный tools/doom. (было 2.2)
- [x] 7.3 Установить исходный BIOS IRQ1 handler и разрешить IRQ1 при POST; сохранить INT16, гостевые replacement/chaining и явно установленные маски. (было 2.3)

## 8. Приёмка  <!-- source: doom-interrupt-compatibility -->

- [x] 8.1 Проверить Doom timer/input в режимах 2/4 при 1МГц/100МГц на копиях HD; сохранить бюджеты, результаты и снимки. Не считать таймерный тест приёмкой меню/движения/стрельбы. (было 3.1)
- [x] 8.2 Выполнить необходимые unit/conformance/race/vet/build и strict OpenSpec, удалить временную instrumentation и выполнить spec/standards review. (было 3.2)
- [x] 8.3 Повторить гостевые Linux/FreeBSD-сценарии, зафиксировать результаты и обновить маршрут только по полностью выполненным критериям. (было 3.3)

## 9. Наблюдаемость и UX  <!-- source: boot-fail-halt-feedback -->

- [x] 9.1 Machine/CLI: машиночитаемое состояние загрузки (`booting`/`halted`, BDA ticks или эквивалент) без гонок с RunInteractive. Критерий: API/флаг доступен фронтенду; go test ./internal/machine ./internal/cli зелёные. (было 1.1) **Реализовано:** `Machine.BootStatus()` (Booting/Halted/BDATicks, атомарно, без гонок с Run); тесты `boot_status_test.go`.
- [x] 9.2 Desktop/CLI: индикация прогресса/ожидания таймера и halted (заголовок/статус/лог), чтобы «No active partition…» + пауза и «System halted.» не выглядели как зависший процесс. Критерий: smoke-тест статуса; ручная проверка окна при boot-fail. **Выполнено 2026-10-20:** `BootStatus` + `SetOnBootHalt` + сообщение CLI «boot failed — VM is halted»; `TestOnBootHaltFiresOnce`. Ручная проверка окна desktop остаётся за пользователем. (было 1.2)

## 10. Wall-время boot failure  <!-- source: boot-fail-halt-feedback -->

- [x] 10.1 Замерить накладные INT 1A/BIOS-хуков в tight-loop FreeDOS-MBR (54 тика) и убрать избыточное, не меняя cycles на инструкцию и детерминизм. Критерий: регрессия циклов/шагов до сообщения INT 18h; vet/build/test успешны. **Выполнено 2026-10-20:** `TestInt1aTightLoopIsLean` — hook AH=00h остаётся тонким (<64 тактов/вызов, чтение BDA + регистры). Избыточности для удаления нет; wall-задержка 54 тиков — модель времени (clock_hz), а не стоимость hook. Детерминизм и cycles/инструкцию не менялись. (было 2.1)
- [x] 10.2 README: ожидаемое поведение install-образа + `--boot=hd`, оценка задержки, правильные команды (`--boot=cd,hd`, `freedos-ready.toml`). Критерий: раздел воспроизводим по шагам расследования 2026-10-15. (было 2.2) **Реализовано:** раздел об install-образе, `--boot=hd` vs `cd,hd`, задержке 54 тиков и ready-образе.

## 11. config-cli  <!-- source: boot-fail-halt-feedback -->

- [x] 11.1 Предупреждение CLI/лога, если после разбора очереди `--boot` первый/единственный HD-носитель не содержит активного раздела или сигнатуры 0x55AA (install-target), с подсказкой `--boot=cd,hd` / ready-образа. Запуск не запрещается. Критерий: тесты config/cli на freedos.img и на bootable-заглушке. (было 3.1) **Реализовано:** `config.BootHints`, вывод в `cmd/ai386`; тесты `boot_hint_test.go`.

## 12. Регрессия и приёмка  <!-- source: boot-fail-halt-feedback -->

- [x] 12.1 Интеграционный тест boot-fail: install-образ или заглушка + `--boot=hd` → сообщения MBR и BIOS, HLT в BIOS ROM, ограниченный бюджет шагов. Критерий: go test ./internal/machine (или bios) без wall-clock assert. (было 4.1) **Реализовано:** `TestBootStatusReportsHaltedOnBootFailure` (MBR→INT 18h→halt, экран и BootStatus).
- [x] 12.2 Общие test/race/vet/build и openspec validate --strict; при наличии X11 — desktop smoke. Критерий: зелёный прогон, отчёт по бюджету шагов до INT 18h. **Выполнено 2026-10-20:** go test ./... (11799), vet, build, GOOS=windows build, openspec validate --all --strict 14/14. (было 4.2)

## 13. Не входит  <!-- source: boot-fail-halt-feedback -->

- [x] 13.1 Не менять контракт INT 18h, очередь `--boot` и запрет roster-only CD (уже покрыто bios/spec). (было 5.1) Контракт INT 18h / `--boot` / roster-only CD не менялся.
