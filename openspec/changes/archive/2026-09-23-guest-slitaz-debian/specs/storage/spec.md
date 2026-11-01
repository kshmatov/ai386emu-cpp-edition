## ADDED Requirements

### Requirement: Сведения и позиционирование ATAPI CD-ROM

ATAPI CD-ROM SHALL поддерживать команды READ DISC INFORMATION и SEEK(10), необходимые драйверу IDE CD Linux для проверки и монтирования установочного носителя.

#### Scenario: Сведения о завершённом диске

- **WHEN** гость отправляет READ DISC INFORMATION для стандартного типа данных
- **THEN** устройство сообщает завершённый диск с одной завершённой сессией и одной дорожкой, соблюдая allocation length

#### Scenario: Позиционирование на существующий LBA

- **WHEN** гость отправляет SEEK(10) с LBA внутри образа
- **THEN** команда завершается без передачи данных и без ERR

#### Scenario: Позиционирование за концом диска

- **WHEN** гость отправляет SEEK(10) с LBA за пределами образа
- **THEN** ERR и REQUEST SENSE сообщают LBA out of range
