## Why
FreeBSD не обнаруживает HD: драйвер ожидает READY|DSC|DRQ после IDENTIFY, а эмулятор не устанавливает DSC.

## What Changes
- ATA-диск сообщает завершение позиционирования в готовом состоянии.

## Capabilities
### Modified Capabilities
- `storage`: статус готовности ATA.

## Non-goals
DMA, SMART и дополнительные ATA-команды не входят в эту регрессию.

## Impact
IDE status и тесты; ATAPI сохраняет отдельный протокол статуса.
