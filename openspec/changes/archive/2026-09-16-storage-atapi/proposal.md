## Why

DSL видит ISO как ATA DISK с искажённым model string, а IRQ14 доставляется в старый BIOS-вектор после remap PIC. Для доступа к KNOPPIX на CD нужен ATAPI.

## What Changes

Добавить сигнатуру/IDENTIFY PACKET, PACKET PIO с 12-байтным CDB, команды чтения и диагностики CD; сохранить независимые task files устройств; маршрутизировать storage IRQ через PIC.

## Capabilities

### Modified Capabilities
- storage: ATAPI CD и PIO-фазы.
- machine-core: remap IRQ устройств хранения.

## Impact

IDE, Machine, unit/integration tests, повторный DSL.
