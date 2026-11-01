## Context

Загрузка real32/64 сначала преобразует значение во внутренний extended формат, поэтому исходные NaN payload и denormal признак должны сохраняться отдельно. Сохранение должно выбирать формат назначения после чтения полного регистра.

## Decisions

Исходный raw80 кэш используется для special values и real80. NaN при сужении переносит significand payload, устанавливает quiet bit и сохраняет знак. Denormal real32/64 сигнализирует DE до расширения; при DM=0 инструкция не меняет стек и память. Real80 denormal не сигнализирует DE. Floating store выполняет округление по PC/RC, устанавливает PE при потере и выполняет pop только для FSTP.

## Validation

Проверены оба знака zero/infinity/QNaN, payload real32/64, exact raw80 round-trip, denormal masked/unmasked, atomicity и округление на половине ULP. Все проверки выполняются на CPU без зависимости от host floating-point NaN payload.
