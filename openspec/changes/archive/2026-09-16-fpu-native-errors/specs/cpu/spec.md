## ADDED Requirements

### Requirement: Отложенное native-исключение x87
CPU SHALL при CR0.NE и pending unmasked x87 status доставлять16 на следующей waiting x87/FWAIT до её побочных эффектов. No-wait контрольные формы SHALL позволять прочитать/очистить состояние.

#### Scenario: Размаскированное деление на ноль
- **WHEN** ZE размаскировано, выполнено конечное ненулевое число/0, затем FNOP
- **THEN** деление сохраняет результат и TOP, выставляет ZE/ES/B; FNOP вызывает16 с адресом FNOP в кадре исключения

#### Scenario: WAIT и TS
- **WHEN** MP=1 и TS=1
- **THEN** FWAIT вызывает7 прежде16; при отсутствии MP/TS и pending завершается без побочных эффектов
