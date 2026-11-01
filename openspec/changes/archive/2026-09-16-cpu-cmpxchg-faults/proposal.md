> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

## Why

Сверка2.2 обнаружила: CMPXCHG меняет флаги до успешной записи и при неравенстве не выполняет write cycle. Это нарушает page-fault/restart и доступ к read-only памяти.

## What Changes

- Проверять read fault до изменения состояния.
- Выполнять запись при обоих исходах сравнения, затем менять accumulator/flags.
- Регрессии8/16/32 бит с missing/read-only страницами и успешным обменом.

## Capabilities

### New Capabilities

### Modified Capabilities
- `cpu`: fault semantics CMPXCHG.

## Impact

internal/cpu/i486.go. Не заменяет полную ISA-приёмку2.2.
