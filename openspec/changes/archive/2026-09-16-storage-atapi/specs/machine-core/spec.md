## ADDED Requirements

### Requirement: Прерывания устройств хранения через PIC

Машина SHALL передавать IRQ6/14/15 контроллеру PIC, учитывая маски и назначенные гостем векторы.

#### Scenario: IDE после remap
- **WHEN** гость назначил slave PIC базу 28h и разрешил IRQ14
- **THEN** завершение IDE вызывает вектор 2Eh, а не BIOS-вектор 76h
