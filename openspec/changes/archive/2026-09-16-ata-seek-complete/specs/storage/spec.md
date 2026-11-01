## ADDED Requirements

### Requirement: ATA seek complete
IDE SHALL устанавливать DSC (бит 4 Status) для готового ATA HD после reset, при готовности данных IDENTIFY/PIO и после завершения передачи. При BSY готовность SHALL не объявляться. ATAPI SHALL сохранять собственные фазы протокола.

#### Scenario: Обнаружение HD драйвером
- **WHEN** гость посылает IDENTIFY DEVICE подключённому HD
- **THEN** Status содержит DRDY|DSC|DRQ, а после чтения 256 слов остаётся DRDY|DSC
