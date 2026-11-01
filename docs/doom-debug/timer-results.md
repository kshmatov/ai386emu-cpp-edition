# Финальная матрица игрового таймера

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

2026-10-14, текущая ветка `feature/freedos-doom`, RAM 16 МиБ. Инструмент
`tools/doom` копирует базовый HD перед каждой загрузкой, не меняет PIC mask,
читает timer через валидную клиентскую mapping DOS Doom 1.9. Запуск:

```sh
GOCACHE=/tmp/ai386-status-go-cache go run ./tools/doom \
  -config=build/freedos/freedos-ready.toml -mode=2 -clock-hz=1000000 \
  -artifacts=/tmp/новый-каталог
```

Для JEMMEX/100МГц: `-clock-hz=100000000 -startup-steps=800000000`.
Safe Mode: `-mode=4`. Прогресс измеряется в окне минимум 2 млн вызовов `Machine.Step`;
дополнительное ожидание возврата из monitor в клиент включено в sample steps.

| Режим / clock | Timer | IRQ0 INTA | Шаги sample | Артефакты | Итог |
| --- | --- | --- | --- | --- | --- |
| JEMMEX / 1 МГц | 142→326 | 3622→4358 | 2000657 | `/tmp/ai386-doom-final-m2-1m` | exit0 |
| JEMMEX / 100 МГц | 2→3 | 243→248 | 2000000 | `/tmp/ai386-doom-final-m2-100m-large` | exit0 |
| Safe Mode / 1 МГц | 8→156 | 2226→2820 | 2000000 | `/tmp/ai386-doom-safe-timer` | exit0 |
| Safe Mode / 100 МГц | 2→3 | 29→34 | 2138358 | `/tmp/ai386-doom-final-m4-100m` | exit0 |

Шаг включает также отдельную ACK-фазу доставки IRQ, поэтому это не
строгий счётчик retired instructions. Точное число шагов и state зафиксированы в `result.txt` каждой копии.
`before/after` содержат RAM/PNG/регистры, `devices.txt` — финальные PIC/PIT
пробы. Это диагностическое продвижение таймера загруженного Doom, а не
самостоятельная приёмка меню, управления или демозаписи.

Исходный JEMMEX/1МГц остановился на 8→8, ISR0 остался установлен.
[Исследование последнего IRQ0](irq0-research.md) отделяет причину от
неиспользованных гостем возможностей PIC. После исключения повторных
внутренних page walks из clock timer продолжил работу. Unit regression (`../internal/cpu/paging_clock_test.go`)
проверяет одинаковую guest work и A/D для paged/flat памяти.

При JEMMEX/100МГц исходный startup budget 100 млн исчерпался при UDVD,
ещё до загрузки Doom; этот запуск не принят. Бюджет 800 млн завершил
загрузку, после чего наблюдалось продвижение таймера.
