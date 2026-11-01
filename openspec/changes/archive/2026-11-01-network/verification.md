# Проверка 14 октября 2026

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

Исходный guest loop — `/tmp/ai386-network-20261014/freebsd-fixed-retry.log`.
На сборке только с исправлением RX boundary HTTP 1024 и 8457 байт прошли
с точным MD5; 32768 байт остались на 0% за 600M инструкций, без прежнего
`NIC memory corrupt`. Harness завершился FAIL 767.62 с; это не PASS сети.

Быстрый loop:

```sh
GOCACHE=/tmp/ai386-status-go-cache go test ./internal/usernat \
  -run '^TestTCPNATRetransmitsOnlyEarliestUnacknowledgedSegment$' -count=1 -timeout=10s
```

Тест проходит через SYN/ACK, host net.Pipe, три настоящих TCP data frames
и FIN. До правки Advance RED 0.005 с: один timeout повторяет всё окно.
Окончательный тест повторён на read-only Go overlay прежнего Advance:
RED 3/3 за 0.007 с, ожидается 1 кадр, фактически 4. Overlay не меняет
production-файлы. Журналы: `tcp-recovery-red.log`,
`tcp-recovery-final-test-red.log` в рабочем каталоге.

После минимальной правки Advance: GREEN 20/20 за 0.029 с;
`go test -race ./internal/usernat -count=1 -timeout=2m` PASS 1.410 с;
`go vet ./internal/usernat` PASS. Проверены отсутствие раннего повтора,
запрет обходить первый сегмент, повтор без ACK, продвижение cumulative
ACK, FIN retry и остановка после полного ACK. Существующий тест отдельно
покрывает SYN retry и удаление flow по timeout.

Первичный контракт — [RFC 6298 §5.4](https://www.rfc-editor.org/rfc/rfc6298.html#section-5):
по timeout повторяется первый неподтверждённый сегмент. Это исправление
не заявляет полную реализацию RFC (динамический RTO/backoff и общий
congestion control не добавляются).

Независимый review PASS, blocking/high нет (`review.md`); reviewer повторил
новый тест и существующий SYN retry/timeout. `go test -race ./internal/machine -run '^Test(Network|GuestNetwork)'`
также PASS 1.107 с.

Финальный guest loop `freebsd-tcp-fixed.log` — PASS 452.70 с,
runner status 0 / duration 453 с. HTTP и проверенный HTTPS 1024, 8457,
32768 байт совпали по размерам и MD5, получен строгий
`AI386_LARGE_NETWORK_PASS`. Wrong-CA контроль дал явные verify errors;
штатный halt и финальные PNG просмотрены. Подробная матрица и fingerprints
в `docs/project-verification/network.md`.

Read-only Inspect подтвердил реальный FreeBSD RX ring 0x26..0x40.
Контроль до/после: на том же guest после RX boundary fix 32 КиБ оставались
на 0% за 600M; после единственной production правки TCP Advance прошли
HTTP и HTTPS 32 КиБ. События OVW/ACK прежнего отказа не трассировались,
поэтому точная повторная OVW последовательность остаётся объяснением,
а не отдельным записанным packet trace. Runtime регрессия устранена.
Диагностический overlay находится только в /tmp; production не содержит
[DEBUG-rx-boundary].
Токены инструментами не предоставляются; время отдельных проверок выше.

## Проверки после последней TCP правки

- `go test ./... -count=1` — PASS, итоговый exit 0.
- `go test -race ./internal/usernat -count=1 -timeout=2m` — PASS 1.410 с.
- `go test -race ./internal/machine -run '^Test(Network|GuestNetwork)' -count=1 -timeout=2m` — PASS 1.107 с.
- `go vet ./...` — PASS.
- Linux/Windows amd64 `go build ./...` и `go build -tags desktop ./...` — все четыре exit 0.
- `openspec validate ne2000-tcp-retransmit-recovery --strict` и scoped diff-check — PASS.

Все команды использовали GOCACHE=/tmp/ai386-status-go-cache. Журналы
`final-full-test.log`, `final-vet.log`, `final-build-*.log` и
`tcp-recovery-*-race.log` лежат в рабочем каталоге. Сборки сообщили
предупреждение о read-only metadata cache в /home/kis/go/pkg/mod;
сборка завершилась успешно. Полный race проекта был выполнен root до
последней TCP правки; после неё повторены затронутые сетевые пакеты.
