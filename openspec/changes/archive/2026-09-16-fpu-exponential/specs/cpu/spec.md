## ADDED Requirements

### Requirement: F2XM1 exponential

CPU SHALL исполнять D9 F0 для x в [-1,1] как 2^x-1 с extended precision независимо от PC и с учётом RC. Вычисление SHALL сохранять малые ненулевые аргументы и знак нуля. Результаты SHALL согласовываться с native x87 в пределах двух ULP (аппроксимация трансцендентных инструкций). Неопределённый Intel результат вне диапазона SHALL детерминированно сохранять операнд.

Empty stack и SNaN/unsupported SHALL обрабатываться как IE; quiet NaN сохраняет payload/sign. DE при unmasked SHALL не менять регистр. UE SHALL округлять в extended subnormal при masked и масштабировать результат на 2^24576 при unmasked. PE SHALL отмечать аппроксимацию ненулевого аргумента; unmasked PE сохраняет вычисленный результат и ES/B для последующего #MF.

Источник: [Intel SDM F2XM1](https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-2a-manual.pdf). Native x87 используется как дополнительный численный эталон, не как утверждение о побитовом совпадении с каждой моделью486.

#### Scenario: Finite operands
- **WHEN** исполняется F2XM1 на нуле, границах либо дробном аргументе
- **THEN** результат соответствует диапазону и точности; TOP не меняется.

#### Scenario: Exceptions
- **WHEN** операнд пустой/NaN/denormal либо результат tiny/inexact
- **THEN** применяются маски и результаты указанных исключений.
