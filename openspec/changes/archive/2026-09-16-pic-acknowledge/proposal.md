## Why

Таймер перезаписывает ожидающий вектор диска при IF=0, и Linux сообщает lost interrupt. PIC должен удерживать запросы до подтверждения процессором.

## What Changes

- Отложенное подтверждение PIC при принятии IRQ процессором.
- Сохранение одновременных IRQ0/IRQ14 и приоритета таймера.

## Capabilities

### Modified Capabilities

- `machine-core`: подтверждение IRQ без потери одновременных запросов.

## Impact

PIC, CPU, Machine, интеграционные тесты загрузки DSL.

## Non-goals

Другие режимы 8259 (ротация, auto-EOI, level trigger).
