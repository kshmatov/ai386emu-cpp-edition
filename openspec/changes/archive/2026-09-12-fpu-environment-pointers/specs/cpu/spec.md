## ADDED Requirements

### Requirement: FIP/FDP/FOP и environment/context x87

После успешной FPU-инструкции CPU SHALL сохранять FIP как адрес её начала, FOP как 11-bit opcode/modrm и FDP как effective data address для memory form; register form SHALL сохранять предыдущий FDP. FLDENV/FNSTENV SHALL поддерживать 14/28-byte формы, FSAVE/FRSTOR — 94/108-byte формы в real и protected mode. Образы SHALL сохранять CW/SW/tag/TOP и raw80 register bytes. До любой записи образа все страницы и границы должны быть проверены; при fault память и FPU SHALL остаться неизменными.

#### Scenario: Example — pointer recording

- **WHEN** выполняется register и memory FPU arithmetic instruction
- **THEN** FIP/FOP соответствуют началу и кодировке инструкции, FDP memory form равен effective address

#### Scenario: Example — context widths

- **WHEN** environment/save образ сохраняется и восстанавливается в 14/28/94/108-byte real/protected форме
- **THEN** CW/SW/tag/TOP и raw80 special payload восстанавливаются побитово

#### Scenario: Example — fault atomicity

- **WHEN** последняя страница context image недоступна
- **THEN** возникает memory fault до частичной записи, FPU pointers и registers не меняются

Источник: [Intel SDM Volume 1, §8.1.7 и §8.5](https://cdrdv2-public.intel.com/671436/253665-sdm-vol-1.pdf).
