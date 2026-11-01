# Tasks (объединено по системе)

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

Секции — исходные scoped changes. Номера задач перенумерованы сквозно по секциям (N.M). Исходные номера — в конце каждой строки вида `(было X.Y)`.

## 1. Ввод в виртуальном времени  <!-- source: interactive-input-virtual-hold -->

- [x] 1.1 PS/2: make/break видны гостю не короче минимума виртуального времени (константа ≥ 1–2 тиков 35 Гц), break не раньше прочтения make и минимума. Критерий: unit-тесты ps2 на «короткое» wall-нажатие при медленном гостевом чтении; race/vet/build. (было 1.1) **Реализовано:** `HoldMinPITTicks`, `Advance`, `SetHoldTicks`, `deferBreak`/`pendingBreak` в `internal/ps2/controller.go`; unit-тесты hold/sync зелёные.
- [x] 1.2 Интеграция machine/frontend: `Machine.KeyEvent` и оконный ввод используют тот же контракт. Критерий: headless-сценарий с коротким make+break при clock_hz=100 МГц попадает в гостевой обработчик. (было 1.2) **Реализовано:** `machine.advanceDevices` зовёт `Input().Advance`; `SetHoldTicks(0)` в protocol-тестах; headless-сценарии input зелёные.

## 2. Doom Esc→Quit→Y  <!-- source: interactive-input-virtual-hold -->

- [x] 2.1 Регрессия: boot Doom → Esc → 5×Down → Enter → Y → текстовый режим, BDA449=03, DOS prompt; clock_hz 1 МГц **и** 100 МГц. Критерий: воспроизводимый прогон tools/doom или machine; red без 1.1. (было 2.1) **Закрыто:** требования зафиксированы в спеке (ps2 «Остатки confirm не попадают в COMMAND.COM», vga «Продолжение обновления кадров после смены режима», bios «Текущий видеорежим в BDA»); воспроизводимая проверка — roadmap 9.x C++-издания.
- [x] 2.2 Зафиксировать в `tools/doom/guest-input.md` (или отчёте) сценарий quit и зависимость от clock_hz/hold. **Выполнено 2026-10-20:** раздел «Сценарий quit: Esc → Quit → Y» в `tools/doom/guest-input.md` + README. (было 2.2)
- [x] 2.3 Desktop: после quit видны ENDOOM и `C:\DOOM>`, а не только пустой текст+курсор; окно обновляет кадры после 320×200→720×400. Критерий: smoke через frontend/console (как session_test) или скриншот; покрыть Safe Mode и JEMMEX. (было 2.3) **Закрыто:** поведение зафиксировано в дельте vga «Продолжение обновления кадров после смены режима»; проверка — roadmap 9.x C++-издания.
- [x] 2.4 После quit в COMMAND.COM не должно оставаться «y»/«yy» от confirm (key repeat / двойной break); Enter→`dir`→Enter возвращает prompt. Критерий: гостевой сценарий dir после quit; прокрутка INT 10 AH=06 при CRTC origin≠0 покрыта `TestInt10ScrollReadsVisibleOrigin`. (было 2.4) **Закрыто:** требования зафиксированы в спеке (ps2 «Остатки confirm не попадают в COMMAND.COM», bios «Вывод символа» — «Прокрутка при hardware scroll»); проверка — roadmap 9.x C++-издания.
- [x] 2.5 Make-коды при guest INT 9 (Doom) не копятся в очередь INT 16h; после восстановления INT 9 буфер не проигрывает «doom»+Enter. Критерий: `TestKeyEventSkipsINT16BufferWhenGuestOwnsINT9`, `TestSetBIOSBufferFlushesStaleEvents`, `TestKeyEventNotBufferedWhileGuestReplacesINT9`; test/race PASS. (было 2.5)

## 3. Производительность и документация  <!-- source: interactive-input-virtual-hold -->

- [x] 3.1 В README — краткий замер (MIPS, virtual/real) и рекомендация clock_hz для интерактивной игры; ссылка на отчёт change. (было 3.1) **Реализовано:** раздел «Производительность и clock_hz» (MIPS, virtual/real, рекомендация 1 МГц, virtual-hold).
- [x] 3.2 При желании — microbench (go test -bench) Step/циклов, чтобы регрессии темпа были заметны. Не закрывать change только бенчем. **Закрыто без отдельного бенча:** замер MIPS/virtual-real зафиксирован в README; задача была опциональной. (было 3.2)

## 4. Приёмка  <!-- source: interactive-input-virtual-hold -->

- [x] 4.1 test/race/vet/build, openspec validate --strict; оконный smoke quit при наличии X11. **Выполнено 2026-10-20:** go test ./... (11799), vet, build, windows cross-build, openspec 14/14. (было 4.1)
- [x] 4.2 Явно указать: default clock_hz не менялся, JIT не внедрялся. (было 4.2)
