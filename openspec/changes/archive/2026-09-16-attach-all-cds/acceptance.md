# Гостевая проверка трёх CD

FreeBSD4.11 с установленного2ГиБ HD, RAM64МиБ, `/tmp/ai386-descriptor-test`. Подключены установочный ISO и каталоги `/tmp/ai386-cd-acceptance-1`, `/tmp/ai386-cd-acceptance-2` с marker.txt (`ai386-cd-1`/`ai386-cd-2`).

Через гостевые PS/2/VGA:

```sh
mkdir /cd0 /cd1 /cd2
mount -t cd9660 /dev/acd0c /cd0
mount -t cd9660 /dev/acd1c /cd1
mount -t cd9660 /dev/acd2c /cd2
cat /cd0/cdrom.inf
cat /cd1/marker.txt /cd2/marker.txt
touch /cd1/readonly
touch /cd2/readonly
```

Получены `CD_VERSION = 4.11-RELEASE`, оба контрольных маркера и два `Read-only file system`. Снимки: `/tmp/ai386-freebsd-three-cds.png`, `/tmp/ai386-freebsd-cd-readonly.png`.

Общие test/vet/build успешны; race config/machine PASS (machine1.314 с). Конфигурационные и ATAPI-тесты покрывают все0–3 ISO, дополненные shared до трёх устройств, и отказ при превышении до создания HD.

Пользователь разрешил конфликт снимка: фиксируется дерево, содержимое читается с хоста. Базовая спецификация, дельта и README согласованы. Новые тесты подтверждают изменение прочитанных данных при сохранении имён/размеров/LBA, ограничение выросшего файла первоначальным размером, ошибки при укорочении и удалении. Гостевой кэш может задерживать видимость изменений. Общие test/vet/build и strict61/61 успешны, race storage1.160с/machine1.300с/config PASS. Задача3.5 завершена.
