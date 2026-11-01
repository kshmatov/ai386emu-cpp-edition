## Why

CPU принимает IRQ сразу после STI/MOV SS/POP SS, хотя следующая инструкция должна выполняться до IRQ. Это может прервать обновление пары SS:SP.

## What Changes

- Блокировка maskable IRQ на следующую инструкцию после STI при IF0 и успешных MOV/POP SS.
- Контроллер IRQ не опрашивается во время блокировки; запрос сохраняется.
- Reset и завершение следующей инструкции снимают блокировку.
- Проверка IOPL возвращает явный результат: отказ прекращает CLI/STI/IN/OUT/string I/O до побочных эффектов.
- Тесты порядка исполнения, fault, STI/HLT, PIC и общий цикл проверок.

## Capabilities

### New Capabilities

### Modified Capabilities
- `cpu`: задержка maskable IRQ после STI и MOV/POP SS.

## Impact

CPU dispatch/Step/ackIRQ и проверки; NMI и debug traps остаются отдельными остатками полноты CPU.
