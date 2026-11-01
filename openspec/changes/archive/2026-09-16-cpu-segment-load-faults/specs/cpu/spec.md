## ADDED Requirements

### Requirement: Fault-atomic segment loads

CPU SHALL сохранять старые selector/cache при page fault чтения операнда/дескриптора и записи accessed bit. Отложенный PF SHALL сохранять vector/error/CR2. Null selectors 0–3 SHALL допускаться для DS/ES/FS/GS с невалидным кэшем; SS SHALL вызывать GP(0). Проверки типа/прав SHALL предшествовать presence; неверный SS type вызывает GP, отсутствующий допустимый SS вызывает SS, отсутствующие data segments вызывают NP. Error code SHALL исключать RPL. Допустимые readable code descriptors SHALL проверяться согласно Intel MOV.

Основание: [Intel 80386 Programmer Reference, MOV (реплика оригинала)](https://www.ardent-tool.com/CPU/docs/Intel/386/manuals/prref386/MOV.htm).

#### Scenario: Page fault during load
- **WHEN** operand, descriptor либо accessed-write недоступны через paging
- **THEN** PF остаётся исходным, а selector/cache не меняются.

#### Scenario: Segment checks
- **WHEN** загружается null, неверный type/privilege либо absent segment
- **THEN** применяются указанные проверки без частичного изменения регистра.
