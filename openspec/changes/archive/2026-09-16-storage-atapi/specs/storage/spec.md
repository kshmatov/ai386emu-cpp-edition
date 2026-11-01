## ADDED Requirements

### Requirement: Чтение CD через ATAPI PIO

IDE SHALL предоставлять CD как ATAPI-устройство с сигнатурой 14h/EBh, IDENTIFY PACKET и 12-байтным PACKET; чтение данных SHALL соблюдать лимит байтов host, DRQ и interrupt reason. CD остаётся read-only.

#### Scenario: Обнаружение CD
- **WHEN** драйвер пробует ATA IDENTIFY у CD, затем IDENTIFY PACKET
- **THEN** первая команда завершается ABRT с ATAPI-сигнатурой, вторая возвращает CD-ROM тип и корректную строку модели в ATA word order

#### Scenario: Чтение нескольких блоков
- **WHEN** READ(10/12) запрашивает допустимый диапазон LBA
- **THEN** возвращаются исходные 2048-байтные сектора, разбитые по лимиту фазы; завершение имеет reason=3 и DRQ=0

#### Scenario: Ошибка команды
- **WHEN** запрошена неизвестная CDB либо LBA вне образа
- **THEN** ERR и REQUEST SENSE сообщают причину; данные ISO не меняются
