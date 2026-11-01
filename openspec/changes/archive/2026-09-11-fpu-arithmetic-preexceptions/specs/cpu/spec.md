## ADDED Requirements

### Requirement: Исключения до вычисления арифметики x87
В add/sub/mul/div x87 SHALL обрабатывать input exceptions до изменения destination и TOP. После stack/unsupported/SNaN и распространения QNaN численный invalid или zero divide SHALL иметь приоритет перед denormal operand. Арифметика с subnormal или pseudo-denormal extended, а также с исходным denormal real32/64 из памяти SHALL сигнализировать DE. Unmasked IE/DE/ZE SHALL сохранять destination, источники и TOP, выставляя соответствующий флаг и ES/B. При masked DE вычисление SHALL продолжаться; masked ZE SHALL давать infinity со знаком XOR. ZE SHALL возникать только для finite nonzero/zero, включая denormal numerator; infinity/zero SHALL давать infinity без ZE.

#### Scenario: Example — приоритет ZE перед DE
- **WHEN** FDIV делит denormal на ноль
- **THEN** возникает ZE без DE; при unmasked ZE исходные регистры и TOP сохраняются, при masked ZE получается signed infinity

#### Scenario: Example — расширение операнда памяти
- **WHEN** FADD читает минимальный denormal real32 или real64 при DM=0
- **THEN** устанавливаются DE/ES/B, ST0 и память сохраняются; точное расширение операнда не скрывает DE

#### Scenario: Example — бесконечность и ноль
- **WHEN** FDIV делит infinity на ноль любого знака
- **THEN** результатом является infinity со знаком XOR без ZE

Источник: [Intel SDM Volume1, §4.9.2 и §8.5.1–8.5.3](https://cdrdv2-public.intel.com/671436/253665-sdm-vol-1.pdf). Правила post-computation OE/UE/PE принимаются отдельным шагом.
