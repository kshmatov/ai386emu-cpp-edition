> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

## Why

Этап 2 завершил реализацию CPU i486 и базовой шины. Сейчас эмулятор не имеет подсистемы хранения: HD создаётся как пустой плейсхолдер, нет контроллера дисков, нет чтения/записи секторов. Для загрузки операционной системы (Linux, FreeBSD) необходимы: образ жёсткого диска с данными, контроллер IDE для доступа к дискам, поддержка CD для установки ОС и дисковода для загрузки.

Этап 3 реализует полную подсистему хранения: IDE-контроллер (PIO), HD-образ фиксированного размера с чтением/записью, CD-ROM (ISO9660) для установки ОС, второй CD из каталога хоста для обмена файлами (без материализации ISO-файла) и дисковод 1.44M.

## What Changes

- Новый пакет `internal/storage`: абстракция block device, rawDataStore (HD/floppy), isoDataStore (CD из ISO-файла), virtualIsoStore (CD из каталога хоста)
- Контроллер IDE: PIO-режим, команды READ SECTORS, WRITE SECTORS, IDENTIFY DEVICE; два канала (primary/secondary)
- Контроллер floppy: 1.44M, команды READ/WRITE SECTOR, RECALIBRATE, SENSE INTERRUPT; IRQ6
- Виртуальный ISO9660 для shared CD: при старте VM по каталогу хоста в памяти строятся только метаданные (Volume Descriptor, дерево каталогов, отображение LBA → host-файл); данные секторов читаются напрямую из host-файлов по запросу, без генерации и записи промежуточного .iso-файла (для --shared)
- Интеграция с Bus: подключение портов IDE (0x1F0-0x1F7, 0x3F6, 0x170-0x177, 0x376) и floppy (0x3F0-0x3F7)
- Интеграция с Machine: новые опции (--hd-size, --shared, --floppy), lifecycle
- Расширение CLI и конфигурации: новые флаги и параметры TOML

## Capabilities

### New Capabilities

- `storage`: подсистема хранения — block device, HD raw image, CD-ROM ISO9660 (из файла и виртуальный из каталога хоста), floppy 1.44M, IDE PIO controller, floppy controller

### Modified Capabilities

- `machine-core`: добавление устройств хранения в шину (port/memory ranges), новые опции Machine, интеграция контроллеров
- `config-cli`: новые флаги CLI (--hd-size, --shared, --floppy), параметры конфигурации, валидация

## Impact

- Новые пакеты: `internal/storage`
- Изменяемые пакеты: `internal/machine` (bus, machine, options), `internal/cli`, `internal/config`
- Зависимости: нет новых внешних зависимостей (ISO9660 парсинг/генерация на чистом Go)
- Шина: порты 0x1F0-0x1F7, 0x3F6, 0x170-0x177, 0x376, 0x3F0-0x3F7; IRQ6, IRQ14, IRQ15
