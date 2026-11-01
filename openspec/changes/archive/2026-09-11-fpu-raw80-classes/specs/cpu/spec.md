## ADDED Requirements

### Requirement: Классы raw80 и распространение NaN
x87 SHALL различать zero, normal, subnormal, pseudo-denormal, infinity, QNaN, SNaN и unsupported по исходному extended-кодированию. В add/sub/mul/div QNaN SHALL распространяться без IE; SNaN SHALL выставлять IE и при маске quieting выбранного NaN SHALL сохранять payload. QNaN SHALL предпочитаться SNaN, при одинаковом классе SHALL выбираться больший significand, при равенстве — положительный знак. Unsupported SHALL давать IE и masked indefinite. Unmasked IE SHALL сохранять destination и запрещать pop.

#### Scenario: Example — перестановки NaN
- **WHEN** FADD или FADDP получает QNaN и SNaN в любом порядке
- **THEN** при masked IE результат содержит payload QNaN и IE=1; при unmasked IE оба регистра и TOP сохраняются

#### Scenario: Example — исходные биты и знак
- **WHEN** непустой регистр с SNaN переносится через FLD ST или FST ST либо выполняется FCHS/FABS
- **THEN** payload и signaling bit сохраняются, IE не возникает, изменяется только запрошенный знак

#### Scenario: Example — FXAM и нулевой результат
- **WHEN** FXAM получает pseudo-denormal или unsupported
- **THEN** C3/C2/C0 равны соответственно 110 или 000, C1 отражает исходный знак, численные исключения не возникают
- **WHEN** точное вычитание одинаковых конечных чисел даёт ноль
- **THEN** результат отрицательный только при округлении к минус бесконечности

Источник: [Intel SDM Volume 1, §4.8.3.5 и таблица 4-7](https://www.intel.com/content/dam/support/us/en/documents/processors/pentium4/sb/25366521.pdf). Регистровые переносы и FXAM также сверяются с описаниями соответствующих инструкций Intel; дополнительные native наблюдения не заменяют паспорт DX4.
