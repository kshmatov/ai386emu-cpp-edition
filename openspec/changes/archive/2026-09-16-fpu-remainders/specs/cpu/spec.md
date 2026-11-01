## ADDED Requirements

### Requirement: x87 partial remainders

CPU SHALL выполнять D9 F8 FPREM и D9 F5 FPREM1. Для конечных операндов результат SHALL быть точным и независимым от RC/PC: quotient округляется к нулю либо к ближайшему чётному соответственно. При разности порядков>=64 SHALL выполняться частичная редукция с N=32 и C2=1; завершение SHALL очищать C2 и переносить Q2/Q1/Q0 абсолютного quotient в C0/C3/C1. Нулевой остаток SHALL сохранять знак делимого. Ошибка invalid при нулевом делителе/бесконечном делимом и denormal SHALL соблюдать маски; unmasked ошибка не меняет данные/TOP. Стек SHALL не сниматься.

Основание: [Intel SDM2A FPREM/FPREM1](https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-2a-manual.pdf). N архитектурно implementation-dependent32–63; модель выбирает32. Полнота NaN payload продолжается в2.8.

#### Scenario: Exact remainder
- **WHEN** выполняется операция над конечными числами, включая tie, отрицательные значения и64-битную мантиссу
- **THEN** остаток и quotient flags соответствуют математическому результату без precision error.

#### Scenario: Iteration and exceptions
- **WHEN** разница порядков велика или операнд вызывает исключение
- **THEN** C2 позволяет повторять редукцию до завершения; masked/unmasked исключения сохраняют установленный контракт.
