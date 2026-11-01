## Context

FCOM сигнализирует IE для QNaN и SNaN, FUCOM — только для SNaN. При masked invalid compare возвращает unordered flags, при unmasked сохраняет operands и TOP.

## Decisions

Сравнение принимает raw operands, классифицирует NaN до big.Float и выполняет pop только после успешного завершения. FXAM анализирует исходный raw80 и не меняет численные исключения. Константы загружаются в extended с текущим RC.

## Validation

Оба знака QNaN/SNaN, empty/full stack, FCOM/FCOMP/FUCOMPP, ordered relations, FXAM classes and constants, test/vet/race/build/strict.
