## Why

В наборе x87 отсутствовала FYL2X (D9 F1), хотя контракт численной совместимости и требования гостевых ОС включают эту команду.

## What Changes

- Реализация log2(ST0)×ST1 с extended range reduction.
- Domain/special/zero-divide/NaN/stack exception matrix и pop semantics.
- Независимые численные регрессии и документация Intel.

## Capabilities

### New Capabilities

Нет.

### Modified Capabilities

- `cpu`: FYL2X.

## Impact

CPU x87 decoder and transcendental path; FYL2XP1 and trigonometric commands remain separate tasks.
