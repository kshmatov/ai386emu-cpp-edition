## Why

Real32/64/80 load/store не сохраняли все special encodings и payload NaN, а denormal при загрузке не сигнализировал DE до расширения.

## What Changes

- Сохранение знака, quiet bit и payload special values при real32/64/80 load/store.
- DE для denormal real32/64 при загрузке, с atomicity при немаскированной маске.
- Проверка сужения, округления, pop и защиты памяти.

## Capabilities

### New Capabilities

Нет.

### Modified Capabilities

- `cpu`: real floating-point load/store.

## Impact

CPU FPU memory path и тестовые эталоны. Integer/BCD и полная матрица context остаются отдельными задачами.
