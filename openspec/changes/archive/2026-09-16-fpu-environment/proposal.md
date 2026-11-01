## Why
В оставшемся объёме x87 отсутствуют FLDENV/FNSTENV, хотя FSAVE/FRSTOR уже используют тот же формат окружения.

## What Changes
Выделить существующее кодирование окружения и добавить14/28-байтовые инструкции с атомарным доступом к памяти.

## Capabilities
### Modified Capabilities
- `cpu`: x87 environment load/store.

## Impact
FPU context и регрессии; формат полного save/restore сохраняется.
