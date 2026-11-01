## Why

Compare-пути x87 не различали signaling/quiet NaN и безусловно меняли стек при немаскированной ошибке.

## What Changes

- FCOM/FCOMP/FUCOM/FUCOMPP: NaN policy, unordered C0/C2/C3, IE и pop.
- FTST, FXAM empty/full и x87 constants с сохранением TOP/тегов.

## Capabilities

### New Capabilities

Нет.

### Modified Capabilities

- `cpu`: x87 compare, examine, stack and constants.

## Impact

CPU FPU compare/register paths и тесты; environment и конверсии остаются отдельными задачами.
