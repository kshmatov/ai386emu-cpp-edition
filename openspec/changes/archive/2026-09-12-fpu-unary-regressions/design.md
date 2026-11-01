## Context

Общий raw80 и arithmetic exception contract теперь используется unary обработчиками. Приёмка должна зафиксировать результат и исключения, не считать наличие теста заменой реализации новых FYL2X/FSIN.

## Decisions

FSQRT отрицательного конечного значения даёт IE/indefinite при маске, отрицательный zero сохраняет знак. FRNDINT применяет RC и PE без изменения TOP. FSCALE использует усечённый integer exponent ST1 и сохраняет оба регистра. Ранее принятые эталоны запускаются повторно без изменения допусков.

## Validation

Targeted special/rounding/stack tests, ранее принятые F2XM1/FPATAN/FPREM/FPREM1/FXTRACT, затем полный test/vet/race/build и strict.
