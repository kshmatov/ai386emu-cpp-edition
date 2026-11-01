## Why

Integer и packed BCD FPU load/store требуют отдельных границ, округления и atomicity, которые не покрываются real load/store.

## What Changes

- Полные FILD/FIST/FISTP 16/32/64 и FBLD/FBSTP проверки.
- Invalid/precision, indefinite, RC, masked/unmasked и pop без частичной записи.
- Проверка недопустимых BCD-цифр.

## Capabilities

### New Capabilities

Нет.

### Modified Capabilities

- `cpu`: integer и packed BCD FPU memory operations.

## Impact

CPU FPU memory path и тесты; context/compare остаются отдельными задачами.
