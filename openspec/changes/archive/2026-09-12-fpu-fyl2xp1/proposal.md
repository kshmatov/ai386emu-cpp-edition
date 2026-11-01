## Why

В наборе x87 отсутствовала FYL2XP1 (D9 F9), необходимая для полной совместимости трансцендентальных операций.

## What Changes

- Реализовать ST1×log2(ST0+1) в extended precision с range reduction.
- Зафиксировать domain, NaN, zero-divide, stack и pop semantics.
- Добавить численные регрессии и ссылку на Intel SDM.

## Capabilities

### New Capabilities

Нет.

### Modified Capabilities

- `cpu`: FYL2XP1.

## Impact

Декодер и трансцендентный путь x87 CPU.
