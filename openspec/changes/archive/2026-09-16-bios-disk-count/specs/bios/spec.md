## ADDED Requirements

### Requirement: Число жёстких дисков BDA
BIOS SHALL записывать в физический адрес475h число уникальных зарегистрированных HD с BIOS-номерами80h–DFh; CD и floppy не учитываются.

#### Scenario: HD вместе с CD
- **WHEN** POST регистрирует одинHD и триCD
- **THEN** BDA475h равен1 и загрузчик может начать INT13 probing HD

#### Scenario: Нет HD
- **WHEN** POST регистрирует толькоCD или floppy
- **THEN** BDA475h равен0, в том числе после прежнего ненулевого значения
