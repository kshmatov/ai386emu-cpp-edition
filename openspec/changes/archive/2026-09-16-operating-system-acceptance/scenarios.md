# Согласованные ОС и воспроизведение

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

RAM: 64 МиБ. Для каждой ОС отдельный создаваемый эмулятором raw-HD 2 ГиБ. Образы гостевых ядер не изменяются.

| ОС | SHA256 ISO |
|---|---|
| DSL 4.4.10 | 6d9369c7b8fb0f23880c5ec44f5a4a2d9d628d7a1407ea83bf0b3249eda33537 |
| FreeBSD 4.11-RELEASE i386 miniinst | 8c7b5c8aca08fd0ba0a0fdc06a0de4f275efa1bbffab7763d205d1fc3a942e04 |

FreeBSD: [официальный ISO и CHECKSUM.MD5](https://archive.freebsd.org/old-releases/i386/ISO-IMAGES/4.11/); MD5 28b006bcdf5df8b8b7e8f1831085cdae совпал при загрузке. [Поддерживаемое оборудование](https://www.freebsd.org/releases/4.11R/hardware-i386/) и [установка](https://www.freebsd.org/releases/4.11R/installation-i386/).

## Ранняя диагностика

~~~sh
go test ./internal/machine -run TestDSLDiagnostic -v -count=1 -args -dsl-image=/home/kis/go/src/gitverse.ru/kshmatov/ai386/dsl-4.4.10.iso
go test ./internal/machine -run TestFreeBSDDiagnostic -v -count=1 -args -freebsd-image=/tmp/ai386-freebsd-4.11-miniinst.iso
~~~

DSL: оригинальное ядро по EIP=C010005E, CR0.PE/PG=1, сверка байтов. FreeBSD: сначала регистрируется переход PE=1, затем строка версии ядра в VGA. Любая остановка сохраняется с EIP/CR0 и текстом экрана.

## Установка и HD

~~~sh
go build -tags desktop -o /tmp/ai386-desktop ./cmd/ai386
/tmp/ai386-desktop --display window --ram 64M --hd /tmp/ai386-dsl-hd.img --hd-size 2G --cd /home/kis/go/src/gitverse.ru/kshmatov/ai386/dsl-4.4.10.iso --boot cd,hd
/tmp/ai386-desktop --display window --ram 64M --hd /tmp/ai386-freebsd-hd.img --hd-size 2G --cd /tmp/ai386-freebsd-4.11-miniinst.iso --boot cd,hd
~~~

В DSL: загрузить dsl-txt 2, выполнить штатный dsl-hdinstall для нового HD; в FreeBSD — Standard/Minimal installation на новый ad0 с загрузчиком и CD-источником. Фактические ответы установщику и разметка фиксируются при выполнении, до заявления приёмки.

Повторный запуск использует тот же HD и --boot hd,cd. В shell каждой ОС: echo ai386persist > /root/ai386-persist.txt, sync, штатное выключение, повторный запуск, cat /root/ai386-persist.txt, echo ai386ready. Успех — обе строки и повторное приглашение shell через гостевые VGA/PS2. FreeBSD прошла этот сценарий: freebsd-hd-acceptance.md. DSL также прошла сценарий: dsl-hd-acceptance.md; повторный запуск выполнялся без подключённого CD.

Диагностика FreeBSD автоматически передаёт Enter через PS/2 после появления `Booting [kernel]`, чтобы продолжить загрузку без ожидания обратного отсчёта. По умолчанию лимит — 1 млрд инструкций; переопределение: `-freebsd-steps=N`.
