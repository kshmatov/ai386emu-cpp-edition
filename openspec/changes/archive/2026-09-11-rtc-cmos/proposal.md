## Why

DSL загружается с неверным временем и ложными данными о floppy, поскольку порты RTC/CMOS не реализованы. Следующий блок платформы 1.6–1.7 должен предоставить ОС часы и сведения о фактической конфигурации.

## What Changes

- RTC класса MC146818/DS12887: календарь, форматы BCD/binary и 12/24 часа, UIP/SET, alarm/update/periodic IRQ8.
- CMOS памяти и floppy по конфигурации VM.
- Начальное время из UTC часов хоста либо заданного RFC3339 значения; дальнейший ход по виртуальному времени.

## Capabilities

### Modified Capabilities

- `machine-core`: RTC/CMOS и IRQ8.
- `config-cli`: настраиваемое начальное время RTC.

## Impact

Новый модуль RTC, Machine, конфигурация/CLI, тесты устройства и загрузки DSL.

## Non-goals

Сохранение NVRAM между запусками, DST, NMI-источники, BIOS INT1A, frontend и другие устройства.
