# Ход самостоятельной проверки

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

Дата: 2026-10-14. Ветка feature/freedos-doom, рабочее дерево содержит
предыдущие незакоммиченные изменения. Коммиты не запрошены.

| Направление | Состояние | Следующее действие |
| --- | --- | --- |
| CPU/FPU | conformance37640, freshnative/QEMU/Decimal PASS; FPU и сохранность FreeBSD PASS, DSL FPU PASS/reboot идёт | RF/taskgate исправлены; post-fix FreeBSD FPU PASS, итоговая DSL продолжается |
| SB16 | DSL exact8192 playback/capture PASS; word/stereo/unsigned16 регрессии исправлены, review PASS | FreeBSD exact capture PASS; padding playback PASS диагностически, свежая полная приёмка продолжается |
| Сеть | DHCP/DNS/externalHTTP обеих ОС PASS, новая inboundUDP race PASS | локальный проверенный HTTPS обеих ОС PASS; NE2000 overflow исправлен, большие ответы повторяются |
| Платформа/ввод | VGA32/32 exactRGB и GUI2/4 при100МГц PASS | Debian false-stage обнаружен и исправлен; свежий прогон идёт; SliTaz отдельный бюджетный прогон |
| Интеграция/review | SB16/DMA/audio/machine test/vet/race PASS, scopedSB16 review PASS | полные ctest/clang-tidy +Linux/Windows headless/desktop PASS, guest проверки продолжаются |

Точное потребление токенов недоступно инструментам сессии. Время гостевых
прогонов фиксируется в журналах; параллельные времена не суммируются.
