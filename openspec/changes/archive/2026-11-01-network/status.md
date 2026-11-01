# Статус продолжения реализации сети

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

## Проверка 22 сентября 2026

Установленные FreeBSD 4.11 и DSL 4.4.10 получили адрес по DHCP, DNS и
исходящий HTTP. FreeBSD `fetch` сохранил 559 байт; DSL `wget` получил `200 OK`
и сохранил 559 байт. Обе гостевые системы штатно остановлены. DSL установила
HTTPS-соединение и получила `403 Forbidden`; FreeBSD сообщила `SSL support
disabled`, поэтому успешная HTTPS-загрузка в обеих ОС не подтверждена.
Подробности и журналы — в `network-verification-2026-09-22.md`.

Все проверки задачи 4.4 прошли. Задача 4.3 остаётся открытой из-за HTTPS в
FreeBSD и непроверенной изоляции незатребованного входящего трафика на уровне
гостевой ОС.

## Текущий фронт

| Часть | Владелец | Состояние | Следующий шаг |
|---|---|---|---|
| Аудит и прогон 4.2 | `network_e2e_audit` | завершён | focused race 20/20 PASS |
| TCP safety | `network_nat_audit` | завершён | focused race 10/10, package 5/5 PASS |
| Таймеры и DHCP | `network_e2e_audit` | завершён | package race 5/5 PASS |
| Machine Stop | `network_guest_acceptance` | завершён | network/stop race 10/10 PASS |
| NE2000 PROM blocker | `network_e2e_audit` | завершён | повторить гостевую приёмку 4.3 |
| Разведка приёмки 4.3 | `network_guest_acceptance` | завершена | расширить harness и выполнить прогоны |
| Интеграция и 4.4 | `/root` | выполняется | focused tests, затем полный набор проверок |

## Порядок интеграции

1. Исправления реализации NAT и жизненного цикла.
2. Исправления/завершение сквозного теста 4.2.
3. Гостевая приёмка 4.3 и запись результатов.
4. Полные проверки 4.4.
5. Итоговый review gate против OpenSpec и правил репозитория.

## Проверки

- `GOCACHE=/tmp/ai386-go-cache-network-continue go test -race
  ./internal/usernat ./internal/machine -count=1 -timeout=60s` — PASS вне
  sandbox; первый запуск в sandbox не дошёл до кода из-за запрета loopback.
- `go test -race ./internal/machine -run '^TestNetworkEndToEndThroughNE2000$'
  -count=20` — PASS.
- После исправлений `go test -race ./internal/usernat ./internal/machine
  -count=1` — PASS.
- После исправления полного чтения PROM `go test -race ./internal/ne2000
  -count=10` и focused `TestNetworkEndToEndThroughNE2000` — PASS.

## Находки review gate

- BLOCKER: TCP timeout/retransmit заданы не в единицах PIT ticks — направлено
  в `NAT-TIMING-DHCP`.
- HIGH: NAT отменяется после ожидания Run — направлено в `MACHINE-STOP`.
- HIGH: TCP socket write под `n.mu` и без deadline — направлено в
  `TCP-SAFETY`.
- HIGH: неограниченный `tcpFlow.pending` — направлено в `TCP-SAFETY`.
- HIGH: DHCP option 1 содержит адрес сети вместо маски — направлено в
  `NAT-TIMING-DHCP`.
- BLOCKER гостевой приёмки: Linux/FreeBSD читают все 32 байта PROM, а
  `readDMA` после первых 12 байт индексировал MAC за границей массива.
  Исправлено моделированием полного NE2000 PROM: дублированные MAC-байты,
  нулевой reserved-диапазон и сигнатура `0x57, 0x57` с дублированием.

Все перечисленные находки исправлены и покрыты регрессиями; финальный review
gate будет выполнен после повторной гостевой приёмки 4.3 и полного набора 4.4.
