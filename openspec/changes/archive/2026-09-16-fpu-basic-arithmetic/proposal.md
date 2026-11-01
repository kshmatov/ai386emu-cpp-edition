## Why

DSL после таймера падает на FLD в штатной проверке x87 FDIV.

## What Changes

Добавить стек x87 и FLD m64, FMUL/FDIV m64, FSUBP, FISTP m32 с расширенной точностью конечных операндов и округлением.

## Capabilities

### Modified Capabilities
- cpu: базовая арифметика x87.

## Impact

CPU, регрессии FPU, приёмка DSL.
