> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

## Why

Сверка CPU выявила раннюю запись селектора/кэша до завершения MOV sreg. Ошибка чтения операнда или accessed-write должна сохранять старый регистр. Null selector включает значения 0–3.

## What Changes

- Фиксировать селектор и кэш только после успешного чтения/проверок и accessed-write.
- Не заменять отложенный page fault новым исключением сегмента.
- Проверять null selectors, тип, права и presence в порядке Intel MOV.
- Регрессии faults/success и общий цикл test/vet/build/race.

## Capabilities

### New Capabilities

### Modified Capabilities
- `cpu`: загрузка сегментов в protected mode.

## Impact

internal/cpu/accessor.go, modes.go, protmode.go, data.go; интерфейс SetSeg сохраняется.
