## Why

Первая подтверждённая Illegal instruction в утилите DSL — FLD m32 по 0804D065. Далее код использует FLD ST, FLDZ, FXCH и FUCOM. Подготовка RAM-диска также требует арифметики awk.

## What Changes

Расширить конечную арифметику x87: real/integer loads/stores, register/memory add/sub/mul/div, compare, stack operations, constants и округление. Исправить различение FSUBP/FSUBRP по Intel opcode table.

## Capabilities

### Modified Capabilities
- cpu: x87 для пользовательских утилит.

## Impact

CPU FPU, тесты операндов/направления/флагов и повторный DSL.
