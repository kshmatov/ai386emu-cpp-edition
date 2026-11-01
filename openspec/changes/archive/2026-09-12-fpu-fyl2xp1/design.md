## Context

FYL2XP1 принимает ST0 как аргумент log2(ST0+1), ST1 как множитель, записывает результат в ST1 и освобождает ST0.

## Decisions

Положительные аргументы используют двоичную редукцию и atanh-ряды; значения в диапазоне (-1,0) вычисляются напрямую. ST0<-1 и 0×±∞ дают IE, ST0=-1 с ненулевым ST1 даёт ZE и signed infinity, а -1×0 — IE. NaN обрабатывается общим raw80 контрактом.

## Validation

Проверены численные точки, граница -1, zero/inf/NaN/stack semantics, полный test/vet/race/build и strict validation.
