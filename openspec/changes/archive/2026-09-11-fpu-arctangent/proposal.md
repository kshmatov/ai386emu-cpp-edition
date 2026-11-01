> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

## Why

FPATAN ещё вызывает UD, оставляя неполным набор x87 для пользовательских математических библиотек.

## What Changes

- D9 F3 вычисляет atan2(ST1, ST0), сохраняет результат в ST1 и снимает ST0.
- Квадранты, signed zero/infinity, NaN и masked/unmasked IE/DE/UE/PE.
- 256-битные промежуточные вычисления с округлением в extended precision по RC независимо от PC.
- Native матрица, регрессии и общий цикл проверок.

## Capabilities

### New Capabilities

### Modified Capabilities
- `cpu`: FPATAN.

## Impact

fpu_atan.go, общий завершающий шаг округления для F2XM1/FPATAN; остальные инструкции FPU остаются в маршруте.
