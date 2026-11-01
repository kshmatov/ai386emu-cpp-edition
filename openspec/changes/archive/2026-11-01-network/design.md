# Design: Network

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

Объединение design-решений по системе сети. Мотивация — proposal.md.

## 1. NE2000 + user-mode NAT (ne2000-usernat)

### Context

Эмулятор запускает DSL/FreeBSD с HD, но гости без сети. Нужен исходящий
доступ к реальной сети хоста без TAP, root и внешних процессов.
Подробности: `research.md`, `status.md`, `dsl-network-acceptance.md`,
`freebsd-network-acceptance.md`, `network-verification-2026-09-22.md`.

### Decisions

- **Карта** `internal/ne2000`: reset, регистры DP8390 + ASIC, PROM с MAC,
  remote DMA, память пакетов; TX/RX-кольцо, фильтрация MAC/broadcast,
  ISR/IMR, IRQ-колбэк в PIC.
- **NAT** `internal/usernat`: Ethernet/ARP/IPv4/ICMP/DHCP с локальными
  ответами маршрутизатора; DNS-прокси и UDP/TCP NAT через инъецируемые
  resolver/dialer; таблица потоков — ответ только в поток, созданный гостём;
  ограниченная concurrent-очередь кадров, отмена контекста, Close.
- **MAC**: уникальные локально администрируемые unicast-адреса при
  параллельном создании VM без явного MAC.
- **Конфиг**: `network`, `net_base`, `net_irq`, MAC, IPv4-подсеть;
  CLI `--network on|off`; при отключённой сети порты и IRQ не регистрируются.
- **TCP**: handshake, seq/ack, окно, payload, FIN/RST, timeout, retransmit
  по `Advance`. Без входящего listener.

### Risks

- [Ограниченное RX-кольцо при больших ответах] → см. §2 и §3.
- [Утечки goroutine/сокетов] → `-race`, Close при активных потоках.

## 2. RX-кольцо (ne2000-rx-boundary)

### Корень

Многостраничный пакет перескакивал BNRY и затирал непрочитанные кадры
(FreeBSD: 1024 HTTP проходит, 8457/32768 timeout, 32768 — NIC memory corrupt).

### Решение

Переполнение отбрасывает пакет **до** изменения RX-памяти: OVW, CURR и
непрочитанная RAM не меняются. После продвижения BNRY гостем — следующий
пакет принимается с корректным next-page и PRX.

## 3. TCP-retransmit (ne2000-tcp-retransmit-recovery)

### Корень

После фикса RX FreeBSD получает HTTP 8457, но 32768 остаётся на 0%:
повторная отправка всего неподтверждённого окна снова переполняет кольцо
при каждом timeout.

### Решение

При истечении таймера повторяется только **первый** неподтверждённый
TCP-сегмент (data/SYN/FIN); следующий становится кандидатом после
cumulative ACK. Ограничения (незаписанный OVW/ACK trace прежнего отказа)
сохранены в `verification.md` / `ne2000-tcp-retransmit-recovery.status.md`.

## Non-goals

Модель пропускной способности, congestion control, смена окна получателя,
настройка скорости NIC, изменение формата кадров и TLS.

## Источники

`ne2000-usernat` (design 6.9K + acceptance docs), `ne2000-rx-boundary`,
`ne2000-tcp-retransmit-recovery` (research/review/verification/status).
