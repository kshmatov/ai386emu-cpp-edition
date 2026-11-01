# Ход работы FreeDOS/Doom

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

Дата: 2026-10-14. Ветка `feature/freedos-doom`; исходные незакоммиченные
изменения сохранены. Коммиты не запрошены.

| Работа | Состояние | Подтверждение |
| --- | --- | --- |
| Воспроизведение IRQ0 | завершено | `irq0-research.md`, исходный frozen timer и последний IRQ |
| Исправление времени | завершено | page-walk traffic исключён из device clock; paged/flat red→green регрессия |
| Исправление IRQ1 | завершено | BIOS handler, POST mask, INT16/chaining/mouse/make-break tests |
| Таймер Doom | PASS 2×2 | JEMMEX/Safe Mode, 1/100 МГц |
| Управление через PS/2 | PASS 2×2 | меню, движение player mobj, расход патронов, повторный IRQ1 |
| Звук Doom | PASS 2×2 | исправлены reset levels CT1745; эффект проходит guest data/DMA/IRQ5/PCM |
| Linux/FreeBSD | PASS и после BDA fix | свежие HD boot, PS/2 shell, VGA, файлы, shutdown; ранее FreeBSD повторный boot |
| Окно Doom | PASS 2/4 при 1 МГц | изолированный Xvfb, реальный XTest ввод, видимое меню/движение/стрельба |
| Полная демозапись | PASS 2×2 | BDA449 исправлен; все 5026 gametics, rendered PNG, видимый timing и отдельный DOS prompt |
| Итоговые проверки | PASS | полный test/race, vet/build, Linux/Windows desktop, strict OpenSpec 13/13 |

Ссылки: [review](review.md), [sound review](sound-review.md),
[OS regression](os-regression.md), input (`../tools/doom/guest-input.md`),
sound (`../tools/doom/guest-sound.md`), DEMO1 (`../tools/doom/guest-demo.md`),
[demo review](demo-review.md), [BDA review](bda-review.md).

Физическая приёмка аудио Linux/Windows, полные CPU/FPU M/X и HTTPS
остаются отдельными задачами [остатка проекта](remaining-acceptance.md).
Задачи Doom 9.5–9.8 завершены; весь проект ещё требует критериев остатка.

Точный расход токенов недоступен инструментам сессии. Длительности реальных
гостевых запусков сохранены в соответствующих отчётах; суммировать их как
общее время нельзя, поскольку часть прогонов выполнена параллельно.
