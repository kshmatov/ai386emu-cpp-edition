# Независимый review, 2026-10-14

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

Reviewer: cpu_verification; gate по orchestrate, стандарты и scoped spec.
Прочитаны proposal/design/spec/verification, diff tcp.go и полный связанный
код outstanding/ACK/FIN/flow lifetime, новый tcp_recovery_test.go.

**Результат: no findings; blocking/high замечаний нет в согласованном scope.**

- outstanding заполняется в порядке отправки; cumulative ACK сохраняет
  порядок оставшихся элементов. Поэтому индекс 0 соответствует earliest
  unacknowledged, в том числе для SYN и FIN.
- Advance повторяет только текущий первый элемент и обновляет только его
  sentAt. Просроченные поздние сегменты не обходят его новый срок.
- После полного ACK пустой outstanding прекращает повтор; flow timeout
  обрабатывается до retransmit. Все изменения выполняются под прежним mu.
- Новая регрессия проходит через реальные TCP frames/net.Pipe, проверяет
  три data segments и FIN, точные повторные кадры, cumulative ACK,
  отсутствие раннего повторения и прекращение после FIN ACK.
- Существующая регрессия проверяет SYN retransmit и flow timeout.

Независимая команда:

```sh
GOCACHE=/tmp/ai386-status-go-cache go test ./internal/usernat \
  -run '^TestTCPNAT(RetransmitAndTimeoutUseAdvance|RetransmitsOnlyEarliestUnacknowledgedSegment)$' \
  -count=1 -timeout=30s
```

PASS: 2 теста. Запуск с loopback-доступом; sandbox до этого блокировал
net.ListenTCP старого теста, новый net.Pipe тест проходил и в sandbox.
Production-файлы reviewer не менял.

[RFC 6298 §5.4](https://www.rfc-editor.org/rfc/rfc6298.html#section-5)
проверен по первичному источнику. Review подтверждает только согласованный
выбор earliest segment. Полный RFC 6298 (включая restart timer после нового
ACK, backoff и динамический RTO) не заявляется. Live guest/NIC recovery
не засчитывается этим review: соответствующий запуск ещё выполняется.
