## Why

Последняя FPU-задача общего плана требует завершить FIP/FDP/FOP и environment/context во всех формах с сохранением raw80 и atomicity memory fault.

## What Changes

- Запись FIP/FDP/FOP для register и memory FPU instructions.
- Полные 14/28 и 94/108-byte environment/save/restore формы real/protected.
- Raw80 special round-trip и отсутствие частичной записи при page fault.

## Capabilities

### New Capabilities

Нет.

### Modified Capabilities

- `cpu`: x87 environment, context and instruction pointers.

## Impact

CPU FPU context/pointer paths и тесты; guest OS acceptance и remaining transcendental tasks не изменяются.
