> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

## Why

POP sreg меняет SP до проверки дескриптора; LDS/LES/LSS/LFS/LGS меняют GPR до успешной загрузки сегмента. При fault обработчик получает частично изменённое состояние.

## What Changes

- Внутренняя загрузка сегмента возвращает успех; публичный SetSeg сохраняет интерфейс.
- GPR/стек фиксируются после успешной загрузки сегмента.
- POP SS вычисляет приращение по прежней ширине стека.
- read32 сохраняет первый PF; регрессии и test/vet/build/race.

## Capabilities

### New Capabilities

### Modified Capabilities
- `cpu`: атомарность POP sreg и загрузок дальнего указателя.

## Impact

internal/cpu data/call/modes/protmode/memory; interrupt shadow отдельно остаётся в аудите CPU.
