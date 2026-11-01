# Платформа, VGA и окно

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

Дата: 2026-10-14.

## VGA
Свежий tools/vga-reference/matrix.py сопоставил точные RGB кадры с QEMU/SeaBIOS для режимов 00–07, 0D–13 и VBE101: 16/16 с запрограммированной палитрой и 16/16 с default palette, ноль различий. Артефакты /tmp/ai386-project-vga-explicit и /tmp/ai386-project-vga-default, команды и журналы в каждом режиме. Это проверка конкретных кадров, не доказательство всех временных свойств VGA.

## Настоящее окно Doom при 100 МГц
Две загрузки FreeDOS: Safe4 и JEMM3862, свежая desktop diagnostic сборка /tmp/ai386-project-window; Xvfb :172, XTest Escape/Up/Control_L, WM_DELETE_WINDOW.

Оба прогона PASS и exit0. Safe: x69206016→69203639, y−236978176→−231241019, патроны50→49, menu0, IRQ1 68→306. JEMM: x69206016→69205122, y−236978176→−234818330, патроны50→49, menu0, IRQ1 68→292. PNG menu/fire просмотрены: меню читаемо, после Escape игровой кадр, выстрел и 49 патронов.

Артефакты /tmp/ai386-project-window-safe100m и /tmp/ai386-project-window-jemm100m. Предыдущие GUI1МГц в обоих режимах остаются в docs/doom-acceptance.md; суммарная GUI матрица2/4×1/100МГц подтверждена.

## Debian VGA16: исправлен ложный критерий этапа

Первый прогон сообщил PASS3/3 за782.58с, но его keyboard PNG при просмотре оказался перерисовкой country. Этот результат третьего этапа отклонён. Generic structural candidate не определял название экрана.

Scoped change debian-diagnostic-stage-identity добавляет сравнение заголовков из визуально принятого эталона того же проверенного ISO, сохраняя прежние panel/change/stability проверки. Red реального country-redraw кадра и positive keyboard control подтверждены; после исправления focused tests/vet PASS. Fixtures, координаты и hashes в internal/machine/testdata/debian-stage/README.md. Независимый review networkagent PASS.

Новый прогон на текущем CPU/audio/NE2000: PASS743.36с, runner0. Этапы приняты при1.144/1.291/1.441 млрд шагов. Все три PNG просмотрены: Choose a language/English, Choose a country/United States, Keymap to use/American English. Разрешение640×480. Артефакты /tmp/ai386-project-debian-vga16-fixed, журнал /tmp/ai386-project-debian-vga16-fixed.log. Это проверка framebuffer и PS/2 трёх меню, не законченная установка Debian.

Время фактического завершения17:03UTC; точные токены недоступны.
