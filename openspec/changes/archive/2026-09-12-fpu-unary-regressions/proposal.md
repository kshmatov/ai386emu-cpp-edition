## Why

FSQRT, FRNDINT и FSCALE должны использовать общий контракт исключений и округления, а исправления raw80/NaN и arithmetic не должны ломать принятые FPREM, FXTRACT, F2XM1 и FPATAN.

## What Changes

- Проверка special values, signed zero, RC и stack semantics для трёх unary команд.
- Повтор принятых численных и exception эталонов предыдущих x87 изменений.

## Capabilities

### New Capabilities

Нет.

### Modified Capabilities

- `cpu`: FSQRT/FRNDINT/FSCALE и совместимость принятых x87 команд.

## Impact

CPU FPU unary/transcendental paths и тесты; новые трансцендентные команды 3.x остаются открыты.
