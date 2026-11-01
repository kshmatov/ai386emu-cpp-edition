## Context
Общий обработчик уже сохраняет raw80 для NaN. Denormal real32/64 при расширении становится normal, поэтому признак исходного denormal нужно сохранять отдельно.

## Goals / Non-Goals
Исправить pre-computation ошибки во всех формах общей арифметики. Округление, диапазон результата и сравнения относятся к следующим приёмкам.

## Decisions
Приоритет: stack/SNaN/unsupported, QNaN, численный invalid или zero divide, затем DE. Для finite nonzero/zero сигнализируется только ZE; infinity/zero не является zero divide. Raw80 subnormal и pseudo-denormal сигнализируют DE при арифметике; real32/64 передают признак исходного denormal после расширения. Unmasked pre-computation ошибка фиксирует SW, сохраняя данные и TOP.

## Risks / Trade-offs
Native x87 подтверждает исходы, определённые Intel, но не заменяет DX4-контракт. Undefined C0/C2/C3 не сравниваются. Точность/диапазон результата проверяются отдельно в 2.2b, без ослабления её требований.

## Validation
Перестановки zero/normal/subnormal/pseudo-denormal/infinity/NaN обоих знаков, все pre-exception маски и pop/no-pop. Тесты памяти real32/64 и integer подтверждают декодирование и сохранение source при unmasked ошибке. Full test/vet/race и strict.
