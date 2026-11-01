## Why

Согласованный FreeBSD 4.11 miniinst не начинает загрузку: BIOS отклоняет El Torito media=3. Требуется чтение встроенного образа дискеты через BIOS.

## What Changes

- Поддержать загрузку El Torito floppy-emulation 1,2/1,44/2,88 МиБ.
- Гость получает DL=0 и читает 512-байтовые сектора через INT 13h с геометрией соответствующей дискеты.
- CD остаётся доступным ATAPI-устройством, запись в встроенный образ отклоняется.

## Non-goals

El Torito hard-disk emulation, запись на CD и эмуляция физического FDC для встроенного образа.

## Capabilities

### New Capabilities

Нет.

### Modified Capabilities

- bios: загрузка и BIOS-чтение встроенной дискеты El Torito.

## Impact

BIOS bootstrap и INT 13h; регрессионные тесты и повтор FreeBSD. Физический CD и конфигурация не меняются.
