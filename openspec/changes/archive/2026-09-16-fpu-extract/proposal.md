> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

## Why

FXTRACT отсутствует в x87 целевого i486DX4. Команда выделяет порядок и значащую часть без округления; продолжает2.8.

## What Changes

- D9F4 с точным разложением64-битной мантиссы независимо от RC/PC.
- Zero, infinity, NaN, denormal и stack fault, соблюдение масок.

## Capabilities

### New Capabilities

### Modified Capabilities
- `cpu`: FXTRACT.

## Impact

internal/cpu; полнота остальных x87-инструкций остаётся открытой.
