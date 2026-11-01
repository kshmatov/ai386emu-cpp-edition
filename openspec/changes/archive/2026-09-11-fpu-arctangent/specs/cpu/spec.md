## ADDED Requirements

### Requirement: FPATAN arctangent

CPU SHALL исполнять FPATAN как atan2(y=ST1,x=ST0), включая все квадранты и определённые пары signed zero/infinity. Результат SHALL помещаться в ST1, затем стек снимает ST0. Мантисса64бита SHALL использовать RC независимо от PC; native матрица SHALL подтверждать отклонение не более двух ULP.

Empty stack/unsupported/SNaN SHALL обрабатывать IE; quiet NaN сохраняет payload. Unmasked IE/DE SHALL не менять операнды/TOP. UE/PE SHALL сохранять вычисленный результат и pop; unmasked UE масштабирует его на2^24576. Tiny masked результат SHALL округляться в extended subnormal, сохраняя знак нуля. Обычные пары0/0 и infinity/infinity SHALL обрабатываться геометрически, без искусственного деления и IE.

Источник: [Intel SDM, FPATAN](https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-2a-manual.pdf); native x87 — дополнительный численный эталон.

#### Scenario: Quadrants and special values
- **WHEN** поданы конечные числа, знаковые нули либо бесконечности
- **THEN** получен соответствующий угол с правильным знаком и pop.

#### Scenario: Exceptions
- **WHEN** возникает IE/DE/UE/PE
- **THEN** маска определяет сохранение операндов либо вычисленного результата по указанным правилам.
