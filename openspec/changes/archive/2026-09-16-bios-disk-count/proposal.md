## Why
FreeBSD4.11 loader bd_init читает BDA475 до INT13 и не обнаруживает HD при нуле. POST сейчас не заполняет это поле.

## What Changes
Заполнять число уникальных BIOS HD при регистрации устройств.

## Capabilities
### Modified Capabilities
- `bios`: POST disk count.

## Impact
BDA, тесты POST и повтор загрузки установленной FreeBSD.
