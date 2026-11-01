# Doom Esc→Quit→Y: чёрный экран и «зависание»

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

Диагностика 2026-10-15. Production не менялся. Воспроизведение на копии
`build/freedos/freedos-ready.img` (`DOOM -NOSOUND -WARP 1 1`).

## Симптом

`ai386-desktop` + `freedos-ready.toml` (default `clock_hz=100_000_000`):
после Esc → Quit → Y чёрный экран, эмулятор «виснет».

## Факты

| clock_hz | Esc, 5×Down, Enter, Y (3 млн шагов/клавишу) | Итог |
|---|---|---|
| 1 МГц | BDA449=03, frame 720×400, ENDOOM, `C:\DOOM>` | quit работает |
| 100 МГц | BDA449=13, frame 320×200, EIP в игровом цикле, text dump пуст | quit **не доходит**, «чёрный» экран |

Навигация: меню Doom 1.9 shareware — 6 пунктов, Quit последний
(5×Down). При 1 МГц тот же поток клавиш открывает confirm и `Y`
вызывает `I_Quit` → `I_ShutdownGraphics` (BDA449==13h → mode 3).

## Причина

Темп эмуляции ~8.5×10⁶ cycles/s (~4–5.6 млн `Machine.Step`/s).
При `clock_hz=100 МГц` virtual/real ≈ **0.085×**. Удержание клавиши
100–200 мс wall = 8–16 мс виртуальных, тик Doom 35 Гц ≈ 28 мс —
make/break проскакивают между опросами. Меню не доезжает до Quit,
`I_ShutdownGraphics` не вызывается, остаётся mode 13h (TextCell пуст —
визуально чёрный экран).

То же объясняет ~48 с паузы FreeDOS-MBR (54 тика INT 1Ah ≈ 3 с
виртуальных / 0.085 ≈ 35–48 с wall) — см. `boot-fail-halt-feedback`.

## Замер производительности

| Цикл | steps/s | cycles/s | virtual/real @100 МГц | @1 МГц |
|---|---|---|---|---|
| `INC [mem]; JMP` + STI | 4.2e6 | 8.5e6 | 0.085× | 8.35× |
| INT 1A poll | 5.6e6 | ≈0 (hook без bus cycles) | ~0× | — |

`ClockCycles` = bus cycles − page walks; register-only инструкции дают 0
cycles, `Step` всё равно считает `max(delta,1)`.

## Что делать

Change [`interactive-input-virtual-hold`](../../openspec/changes/input/):
минимальная **виртуальная** длительность make/break в PS/2 + регрессия
quit при 100 МГц. Не JIT и не смена default `clock_hz` в этом срезе.

**Обход применён 2026-10-15:** в `build/freedos/freedos-ready.toml` и
`freedos-install.toml` указан `clock_hz = 1000000`. CLI `--clock-hz`
имеет приоритет над TOML.

## Дополнение 2026-10-15: чёрный экран + курсор после quit

После `clock_hz=1000000` пользователь видит **текстовый режим с мигающим
курсором**, но нет `C:\` / ENDOOM.

Headless-прогоны того же toml (SB16, Safe Mode **и** JEMMEX, с `doom`
и с `-nosound -warp`): после Esc, 5×Down, Enter, Y — всегда
`BDA449=03`, frame 720×400, ENDOOM («DOOM: Knee-Deep…») и `C:\DOOM>`.
Путь `I_Quit` → mode3 → ENDOOM → INT 21h/4C корректен.

Состояние «пустой текст + курсор» = mode 3 уже выполнен (очистка
spaces/attr 07, курсор 0,0), но **ENDOOM/prompt не в видеопамяти**.
Либо фронтенд показывает первый кадр после mode 13→3 и не обновляет
дальше, либо гость виснет между `I_ShutdownGraphics` и записью ENDOOM
(в JEMMEX после Y первые шаги ещё в PE: `CR0=e0000011`, затем mode 3).

Рекомендации для повтора: Safe Mode (пункт 4), после «чёрного» экрана
нажать Enter и набрать `dir`; сверить headless
`./build/ai386 --config=build/freedos/freedos-ready.toml --display=headless`
с окном. Регрессия интерактивного desktop quit — в
`interactive-input-virtual-hold` / отдельном desktop-срезе.

## Дополнение 2026-10-15: prompt + «yy», затем `dir` → чёрный экран

Пользователь: после quit **был** `C:\DOOM>` и `yy` в строке; Enter, `dir`,
Enter — чёрный экран без prompt.

Headless (тот же toml, Safe Mode, SB16): после quit аналогично `y` в
строке (остаток confirm Y / key repeat); Enter → `Bad command or
filename - "y"`; `dir` в `C:\DOOM` выводит 21 файл и возвращает
`C:\DOOM>`. Живой prompt.

Найден и исправлен дефект INT 10h AH=06/07: прокрутка читала
`0xB8000+src` напрямую, а писала через CRTC origin (`visibleTextAddress`).
При hardware scroll (origin≠0) на экран попадал пустой VRAM — **чёрный
экран**. Регрессия `TestInt10ScrollReadsVisibleOrigin`. `dir` длинным
списком как раз скроллит окно.

Остаток «y»/«yy» в COMMAND.COM — утечка нажатий confirm в DOS
(см. `interactive-input-virtual-hold` 2.4).
