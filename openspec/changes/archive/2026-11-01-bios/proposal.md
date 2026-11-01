# Proposal: BIOS — BDA, IRQ1, JEMMEX-таймер и наблюдаемый boot-fail

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

## Why

Объединение работ по системе `bios` (и связанных machine-core/config-cli):

1. **BDA-курсор** (`fix-bda-cursor`): COMMAND.COM читает курсор из BDA
   (0040:0050/0051), BIOS не писал его → backspace после `dir` уводил курсор
   в (255,79) и «вешал» ввод.
2. **BDA-видеорежим** (`doom-bios-video-mode`): I_ShutdownGraphics Doom
   проверяет BDA449; BIOS переключал VGA, но оставлял BDA449=3 → гость
   пропускал восстановление текста.
3. **Прерывания Doom** (`doom-interrupt-compatibility`): остановка таймера
   при JEMMEX (незавершённый IRQ0) и отсутствие клавиатурного управления
   (BIOS оставлял IRQ1 маскированным).
4. **Boot-fail UX** (`boot-fail-halt-feedback`): install-образ с `--boot=hd`
   выглядит зависшим ~48 с; нужна наблюдаемость booting/halted и
   предупреждение о незагрузочном HD.

## What Changes

- INT 10h SHALL синхронизировать BDA-курсор страницы 0 при AH=00h/02h/0Eh.
- Успешный INT 10h AH=00h обновляет текущий режим в BDA449.
- BIOS SHALL устанавливать исходный обработчик IRQ1 и разрешать его при POST;
  гостевые замены/chaining и маски PIC сохраняются.
- Условные такты не зависят от повторного внутреннего обхода страниц
  (JEMMEX/IRQ0).
- Путь boot failure наблюдаем (`booting`/`halted`); CLI предупреждает о
  HD без активного раздела/0x55AA; README описывает install vs ready-образ.

## Capabilities

### New Capabilities

Нет.

### Modified Capabilities

- `bios`: BDA-курсор; текущий видеорежим в BDA; исходная доставка IRQ1;
  MODIFIED «Продолжение загрузки по INT 18h» (наблюдаемость).
- `config-cli`: предупреждение о незагрузочном HD в очереди `--boot`.
- `machine-core`: таймер Doom при JEMMEX; условные такты; наблюдаемое
  состояние загрузки/остановки.

## Impact

`internal/bios` (int10, int18, keyboard_irq), `internal/machine`,
`internal/config`/`cli`, FreeDOS/Doom прогоны, README.

## Источники

Объединены: `fix-bda-cursor`, `doom-bios-video-mode`,
`doom-interrupt-compatibility`, `boot-fail-halt-feedback`.
