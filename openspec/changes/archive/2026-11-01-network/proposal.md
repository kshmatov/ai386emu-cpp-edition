# Proposal: Network — NE2000, user NAT, целостность RX и TCP-retransmit

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

## Why

Объединение работ по системе `network`:

1. **NE2000 + user-mode NAT** (`ne2000-usernat`): гостевые ОС не имеют
   сетевого доступа; встроенная NAT-сеть даёт исходящий доступ без TAP,
   root-прав и внешних процессов.
2. **RX-кольцо** (`ne2000-rx-boundary`): многостраничный пакет перескакивал
   BNRY и затирал непрочитанные кадры (8457/32768 timeout, 32768 —
   NIC memory corrupt).
3. **TCP-retransmit** (`ne2000-tcp-retransmit-recovery`): повторная отправка
   всего неподтверждённого окна снова переполняла малое кольцо при каждом
   timeout.

## What Changes

- ISA-адаптер NE2000 с настраиваемым MAC, DP8390/PROM/remote DMA, TX/RX-кольцом,
  ISR/IMR и IRQ-колбэком; TOML/CLI `--network on|off`.
- Встроенный user-mode NAT: ARP, DHCP, IPv4, ICMP, DNS, UDP, TCP; только
  ответы на потоки, созданные гостём; без входящих соединений и проброса портов.
- RX: переполнение отбрасывает пакет до изменения RX-памяти (OVW, CURR
  не меняется); непрочитанные кадры сохраняются.
- TCP: при истечении таймера повторяется только первый неподтверждённый
  сегмент; следующий — после cumulative ACK.

## Capabilities

### New Capabilities

- `network`: эмулируемый адаптер NE2000 и изолированный исходящий доступ.

### Modified Capabilities

- `config-cli`: параметры сети (база, IRQ, MAC, подсеть, `--network`).
- `machine-core`: жизненный цикл сетевого устройства; RX не повреждает
  непрочитанные кадры; TCP timeout retransmit earliest unacknowledged.

## Impact

`internal/ne2000`, `internal/usernat`, `internal/machine`, `internal/config`;
приёмочные прогоны DSL 4.4.10 и FreeBSD 4.11 (HTTP/HTTPS 1024/8457/32768).

## Non-goals

Входящие соединения, проброс портов, TAP/bridge, IPv6, иные сетевые карты,
Wi-Fi, точная модель Ethernet, congestion control.

## Источники

Объединены: `ne2000-usernat`, `ne2000-rx-boundary`, `ne2000-tcp-retransmit-recovery`.
