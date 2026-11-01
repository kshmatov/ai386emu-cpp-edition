## 1. Storage

Дизайн не требуется: локальное формирование статуса синхронного ATA-диска. Доказательство потребителя: [FreeBSD ata_getparam](https://github.com/freebsd/freebsd-src/blob/releng/4.11/sys/dev/ata/ata-all.c).

- [x] 1.1 Добавить регрессию READY|DSC|DRQ и исправить статусы ATA. Приёмка: FAIL до исправления, PASS после, ATAPI-тесты успешны; время и токены.
- [x] 1.2 Выполнить полные test/vet/build, race storage/machine и повтор FreeBSD. Приёмка: HD распознан драйвером; время и токены.
