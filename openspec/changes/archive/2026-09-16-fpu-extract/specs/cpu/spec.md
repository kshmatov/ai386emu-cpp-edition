## ADDED Requirements

### Requirement: x87 extract exponent and significand

CPU SHALL выполнять D9F4 FXTRACT: ST1 получает истинный двоичный порядок, ST0 — значащую часть со знаком и модулем в[1,2). RC/PC не округляют результат. Для zero masked ZE SHALL давать ST1=-infinity, ST0=signed zero; infinity SHALL давать ST1=+infinity и ST0=исходную infinity. Unmasked исключение SHALL сохранять данные/TOP. Denormal SHALL нормализоваться при masked DE. Переполнение/опустошение стека SHALL выставлять IE/SF/C1.

Основание: [Intel SDM2A FXTRACT](https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-2a-manual.pdf).

#### Scenario: Exact decomposition
- **WHEN** гость применяет FXTRACT к конечному ненулевому числу
- **THEN** significand × 2^exponent точно восстанавливает исходное число, включая64-битную мантиссу и denormal.

#### Scenario: Exceptions and special values
- **WHEN** вход zero/infinity/NaN или стек не позволяет выполнить команду
- **THEN** результаты и статус соответствуют маскам; unmasked ошибка не изменяет стек.
