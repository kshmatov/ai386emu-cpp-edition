## ADDED Requirements

### Requirement: x87 packed BCD

CPU SHALL выполнять DF /4 FBLD и DF /6 FBSTP с10-байтным операндом:18 цифр младшими разрядами вперёд, знак в bit79; биты72–78 при загрузке игнорируются. FBSTP SHALL округлять по RC, сохранять signed zero и выполнять pop только после успешного сохранения. При FBSTP masked invalid SHALL записывать FFFFC000000000000000h; unmasked invalid SHALL сохранять данные стека и память, выставляя pending status. IE подавляет PE текущего преобразования, сохраняя прежний sticky PE. Изолированная unmasked precision SHALL записывать округлённое значение, выполнять pop и выставлять PE/ES/B. Ошибка стека SHALL выставлять IE/SF и C1 согласно направлению ошибки. Не-BCD цифры FBLD имеют архитектурно неопределённый результат; проверка цифр и IE не являются обязательными.

Источник: [Intel SDM Volume2A](https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-2a-manual.pdf), FBLD/FBSTP; [формат indefinite](https://www.intel.com/content/dam/support/us/en/documents/processors/pentium4/sb/25366521.pdf).

#### Scenario: Round trip and rounding
- **WHEN** гость загружает положительный/отрицательный18-значный BCD или signed zero и сохраняет его
- **THEN** значение и знак сохраняются; дробный вход округляется согласно четырём RC.

#### Scenario: Invalid or fault
- **WHEN** источник FBSTP не помещается, содержит infinity/NaN, стек пуст/переполнен либо страница отсутствует
- **THEN** соответствующий статус и masked/unmasked результат соблюдаются; PF не меняет FPU и не пишет часть операнда.

#### Scenario: Isolated precision on store
- **WHEN** FBSTP сохраняет 2.5 с PM=0 и RC nearest-even
- **THEN** память получает packed BCD 2, ST(0) снимается и устанавливаются PE/ES/B.
