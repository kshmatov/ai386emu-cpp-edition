## Why

Linux доходит до создания первого процесса и требует FNSAVE для сохранения x87.

## What Changes

Добавить FNSAVE/FRSTOR, сохранение control/status/tag и восьми extended регистров; FLDCW/FNSTCW/FNSTSW memory и FNCLEX.

## Capabilities

### Modified Capabilities
- cpu: контекст x87.

## Impact

CPU, тесты формата и защиты памяти, DSL.
