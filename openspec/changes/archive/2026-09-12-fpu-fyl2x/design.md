## Context

FYL2X принимает ST0 как положительный аргумент log2 и ST1 как множитель, записывает результат в ST1 и освобождает ST0. Для точности требуется range reduction и atanh series, а не float64 host call.

## Decisions

Поддерживается положительный finite ST0, ±0 и +∞ по архитектурным special правилам. Отрицательный ST0 и нулевая множительная комбинация infinity дают IE; finite nonzero×zero logarithm zero даёт ZE и signed infinity. NaN selection выполняется общим raw80 handler, DE/PE — общим transcendental завершением.

## Validation

Targeted domain/stack tests, independent math.Log2 comparison for finite points, previous FPU regressions, full test/vet/race/build/strict.
