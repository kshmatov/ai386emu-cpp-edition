# Design: BIOS

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

Объединение design-решений по системе BIOS. Мотивация — proposal.md.

## 1. BDA-курсор страницы 0 (fix-bda-cursor)

### Корень

COMMAND.COM при backspace берёт позицию из **BDA** (0040:0050 col,
0040:0051 row), а не из INT 10h AH=03h. BIOS до фикса не писал BDA при
AH=00h/02h/0Eh — там оставалось (0,0). Backspace из (0,0): `col==0` →
`row--` (uint8: 0→255), `col=79` → `AH=02h(255,79)` → CRTC 20479 →
`CursorPosition()=(51,15)` → `WriteChar` игнорирует row≥25 → echo исчезает.

### Решение

`writeBDACursor(bus, row, col)` в `internal/bios/int10.go`:
- AH=00h (SetMode) → (0,0);
- AH=02h → DH/DL;
- AH=0Eh (teletype) → итоговые (row, col) после переноса/прокрутки.

Hardware scroll (`crtcStart`) **не** причина. Смежные наблюдения (free-space
пауза `dir`, критерий «dir закончился» в headless-тестах) — в `verification.md`.

## 2. Текущий видеорежим в BDA (doom-bios-video-mode)

### Корень

Actual I_ShutdownGraphics Doom проверяет byte BDA449 на 13h перед INT 10
mode3. BIOS переключал устройство, но оставлял BDA449=3 → гость пропускал
восстановление текста после DEMO1.

### Решение

Успешный INT10 AH00 обновляет номер текущего режима в BDA449. Неизвестный
режим сохраняет прежнее поле и состояние VGA. Red/green регрессия условного
shutdown и повторная DEMO1.

## 3. IRQ1 и JEMMEX (doom-interrupt-compatibility)

### Корень

- IRQ0 при JEMMEX: незавершённый EOI / условные такты зависели от
  повторного внутреннего обхода страниц → игровой таймер `8 → 8`.
- IRQ1: BIOS оставлял линию маскированной и без своего обработчика —
  Doom (свой INT 9) не получал скан-коды без ручной правки PIC.

### Решение

- Условные такты не зависят от повторного обхода страниц (та же граница
  физ/лог, что и в TB-фиксе).
- BIOS устанавливает исходный IRQ1 handler при POST и разрешает IRQ1:
  handler обслуживает выходной буфер PS/2, выдаёт EOI, сохраняет события
  для INT16. Гостевые replacement/chaining (IVT9) и явно установленные
  маски PIC сохраняют приоритет.
- Диагностика: `tools/doom` на отдельной копии HD (счётчик, RAM/PNG, INTA/PIC).

## 4. Boot-fail UX (boot-fail-halt-feedback)

### Корень

`freedos.img` (install-target, partition inactive) с `--boot=hd` не загрузочен.
FreeDOS-MBR крутит 54 тика INT 1Ah (~48 с wall при clock_hz=100 МГц из-за
tight-loop INT 1A×2), затем INT 18h печатает «System halted.» и паркует
`CLI;HLT`. Логика BIOS корректна; wall-задержка — модель времени.

### Решение

- Машиночитаемое состояние `booting`/`halted` для фронтенда (без гонок с
  RunInteractive).
- Индикация прогресса/HLT в desktop/CLI.
- Снижение накладных INT 1A/BIOS-хуков в tight-loop без смены cycles/детерминизма.
- Предупреждение CLI, если HD в очереди `--boot` без активного раздела/0x55AA
  (запуск не запрещается).
- README: install vs `freedos-ready`, `--boot=cd,hd` / `freedos-ready.toml`.

### Non-goals (в рамках boot-fail)

Менять контракт INT 18h, очередь `--boot` и запрет roster-only CD.

## Источники

`fix-bda-cursor` (design восстановлен), `doom-bios-video-mode`,
`doom-interrupt-compatibility`, `boot-fail-halt-feedback` (design восстановлен).
