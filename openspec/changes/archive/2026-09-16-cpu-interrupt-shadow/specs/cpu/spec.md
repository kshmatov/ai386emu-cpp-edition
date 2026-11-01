## ADDED Requirements

### Requirement: Maskable interrupt shadow

Успешные MOV SS и POP SS, а также STI при прежнем IF0 SHALL задерживать maskable IRQ до выполнения следующей инструкции или доставки исключения. Блокировка SHALL не сбрасывать IF и не потреблять IRQ из PIC. Неуспешная загрузка SS SHALL не создавать блокировку. LSS SHALL не добавлять её. Reset SHALL очищать состояние. Отказ IOPL SHALL завершать инструкцию до изменения IF/регистров или обращения к портам.

Источник: [Intel SDM STI/MOV/POP](https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-2b-manual.pdf), [Intel о загрузке SS](https://www.intel.com/content/dam/support/us/en/documents/processors/pentium4/sb/25366821.pdf).

#### Scenario: Atomic stack update
- **WHEN** IRQ поступает после MOV SS перед MOV SP
- **THEN** MOV SP выполняется первым, и interrupt frame использует обновлённую пару SS:SP.

#### Scenario: STI and HLT
- **WHEN** IRQ ожидает при выполнении STI; HLT
- **THEN** HLT исполняется, после чего IRQ пробуждает CPU с возвратом после HLT.
