# Регрессия Linux/FreeBSD после исправления прерываний Doom

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

Живые гостевые проверки 2026-10-14 для `doom-interrupt-compatibility`, задача 3.3.
Проверяется сохранность загрузки, PS/2, VGA и HD после BIOS IRQ1 и перехода
`Machine.Step` на `CPU.ClockCycles`. Это не приёмка всей ISA, сети или звука.

## Воспроизведение и артефакты

Рабочий каталог: `/tmp/ai386-os-regression-20261014`. Обе ВМ используют отдельные
копии базовых HD (`cp --reflink=auto --sparse=always`), RAM 64 МиБ, исходные ядра.
Сборка текущего рабочего дерева:

```sh
GOCACHE=/tmp/ai386-status-go-cache go test -c \
  -o /tmp/ai386-os-regression-20261014/machine.test ./internal/machine
```

Каждый запуск выполнен через `sh tools/fpu-reference/guest/run-freebsd-session.sh`
с отдельными `--log` и `--lock`; финальный `AI386_FREEBSD_RUN_EXIT` подтверждает
завершение процесса, а не только появление терминальной сессии. Полные команды
сохранены в начале логов. PS/2-команды сохранены в `*.jsonl`, снимки в `*.png`.
Все команды гостевых оболочек передаются make/break scan codes через `KeyEvent`.
Ни образ, ни память гостя снаружи для проверки файлов не редактируются.

SHA256 неизменённых баз:

| База | SHA256 |
| --- | --- |
| `/tmp/ai386-dsl-final-hd.img` | `b629394156e45d59ecd1fc30442bb4282743d45a8140d2321bee5fc213c9f4af` |
| `/tmp/ai386-freebsd-final-hd.img` | `6aa54f7333f12c5998f6baf68ff1d0c1313cd1096a95b4c90988013fd3438831` |

## FreeBSD 4.11 — PASS

Первый запуск: `freebsd.log`, `PASS 267.63s`, wrapper `status=0`, 268 секунд.
Опции: `-test.run '^TestFreeBSDDiagnostic$' -test.v -test.timeout=2h`,
`-freebsd-image=/tmp/ai386-freebsd-4.11-miniinst.iso`,
`-freebsd-hd=/tmp/ai386-os-regression-20261014/freebsd.img`,
`-freebsd-installed -freebsd-control -freebsd-steps=2000000000`.
Boot order содержит только HD, BIOS DL=80h проверен тестом; диагностический
ISO остаётся подключённым, но загрузка с него или fallback не разрешены.
Оригинальное ядро достигло login после 559000000 инструкций.

Через PS/2 выполнены root login, чтение прежнего файла и запись нового:

```sh
cat /root/ai386-persist.txt
echo ai386osreg > /root/ai386-os-regression.txt
cat /root/ai386-os-regression.txt
sync
shutdown -h now
```

Отдельные строки вывода `ai386persist` и `ai386osreg` проверены строгими
`until`-условиями с началом новой строки: ввод команды сам по себе не проходит
проверку. Сохранены `freebsd-persist.png`, `freebsd-write-read.png` и
`freebsd-halt.png`. Визуально проверен halt: `syncing disks... done`,
`The operating system has halted.`. Только после этого передан explicit quit.

Второй независимый запуск той же рабочей копии: `freebsd-reboot.log`,
`PASS 247.97s`, wrapper `status=0`, 248 секунд. HD login после 558000000
инструкций; root login и `cat /root/ai386-os-regression.txt` снова дали
`ai386osreg`, после чего выполнены sync и штатный shutdown.
Сохранены `freebsd-reboot-read.png` и `freebsd-reboot-halt.png`.
Снимок чтения после перезапуска визуально проверен: marker и новое приглашение
`#` присутствуют. Запись сохранилась между независимыми загрузками.

## DSL 4.4.10 — PASS

Рабочая копия `dsl.img`, лог `dsl.log`, команды `dsl.jsonl`.
Опции: `-test.run '^TestDSLDiagnostic$' -test.v -test.timeout=2h`,
`-dsl-image=/home/kis/go/src/gitverse.ru/kshmatov/ai386/dsl-4.4.10.iso`,
`-dsl-hd=/tmp/ai386-os-regression-20261014/dsl.img`,
`-dsl-installed -dsl-boot-steps=500000000 -dsl-extra-steps=4000000000`,
`-dsl-continue-faults -dsl-require-shell -dsl-control-at-shell`,
`-dsl-screen=/tmp/ai386-os-regression-20261014/dsl-final.png`.
CD отключён диагностикой, boot order только HD, DL=80h проверен тестом.
Строгий вход в исходное ядро подтверждён после 121986504 инструкций.
Гостевая FS сообщила `/dev/hda1: clean`, затем достигнута `root@tty1[/]#`.
Тест отправил `echo ai386ready` через PS/2 и проверил отдельную строку вывода.
После этой строгой проверки control выполнил те же команды чтения прежнего
файла, записи и чтения нового файла, sync и shutdown, что и FreeBSD выше.
`cat /root/ai386-persist.txt` дал `ai386persist` после 7900000 инструкций;
`cat /root/ai386-os-regression.txt` дал `ai386osreg` после 1600000 инструкций.
`until` проверял маркеры с началом новой строки, без зачёта текста ввода.

Штатная остановка: `/dev/hda1 umounted`, `DSL halted.` после 59600000
инструкций shutdown; только затем explicit quit. `PASS 930.02s`, wrapper
`status=0`, 930 секунд. Сохранены `dsl-persist.png`, `dsl-write-read.png`,
`dsl-halt.png`, `dsl-final.png`; снимки записи/чтения и halt визуально проверены.
Новый файл DSL прочитан в этом запуске; отдельный перезапуск DSL не выполнялся.
Чтение прежнего `ai386persist` подтверждает доступность ранее сохранённых данных.

## Результат и границы

Linux и FreeBSD прошли живую регрессию загрузки с HD, работы PS/2 и оболочки,
VGA, чтения старого файла, записи/чтения нового и штатного размонтирования.
FreeBSD дополнительно сохранила новый файл между двумя независимыми загрузками.
Задача 3.3 подтверждена живыми ОС, а не только unit-тестами.

`bases-before.sha256` и `bases-after.sha256` совпали после всех трёх запусков;
исходные HD не изменились. Полные логи, JSON-команды, копии HD и PNG сохранены
в рабочем каталоге. Общие тесты/линтеры/сборка и сценарии Doom проверяются
отдельно; этот отчёт не заявляет их результаты или общую приёмку проекта.

## Повтор после BDA449 — PASS обеих ОС

Прогоны выше выполнены до узкой правки INT10 AH00/BDA449. После неё
запущена дополнительная свежая регрессия обеих ОС:
`/tmp/ai386-os-regression-post-bda-20261014`.
Собран текущий `machine.test`, SHA256:
`96db0d30ef54a15a55293075d6ba4f15fcf2306683ff756fff8fad26b7546fb6`.
Обе исходные final-HD снова копируются отдельно; прежние рабочие HD
не используются. До запуска сохранён `bases-before.sha256`.

Команды и PS/2 workflow совпадают с описанными выше, пути артефактов
заменены на новый каталог. По-прежнему RAM64МиБ, HD-only boot order,
FreeBSD startup2B и DSL boot500M/extra4B, test timeout2h.
Для FreeBSD достаточно одного нового запуска: сохранность между boots
уже отдельно подтверждена предыдущим независимым reboot; здесь проверяется
последнее BIOS состояние при boot/shell/file/sync/shutdown.

Первый DSL launch имел ошибку ISO path и завершился до создания VM;
`dsl-launch-path-error.log` сохранён отдельно. После исправления пути
запущен исходный неизменённый ISO с той же нетронутой свежей HD-копией.
Это ошибка команды запуска, не отказ загрузки гостевой ОС.
PASS каждой ОС зафиксирован только после guest halt и окончательного wrapper exit.

FreeBSD после BDA fix завершилась: `PASS 276.08s`, wrapper `status=0`,
276 секунд; HD login после558000000 инструкций. PS/2 root login,
`ai386persist`, запись/чтение нового `ai386osreg`, sync и штатный halt
прошли. Новые `freebsd-write-read.png` и `freebsd-halt.png` визуально
проверены: marker/приглашение и `syncing disks... done`,
`The operating system has halted.` присутствуют. Лог `freebsd.log`
и JSON сохранены.

DSL после BDA fix завершилась: `PASS 1015.57s`, wrapper `status=0`,
1015 секунд. Строгий kernel entry после121986504 инструкций, FS clean,
автоматический PS/2 echo `ai386ready`, затем чтение прежнего `ai386persist`,
запись/чтение нового `ai386osreg` и sync/shutdown. Чтение marker проверено
строгими отдельными строками. Новые `dsl-write-read.png` и `dsl-halt.png`
визуально просмотрены: оба marker, `/dev/hda1 umounted`, `DSL halted.`
присутствуют. Только после guest halt передан quit. Сохранены также
`dsl-persist.png`, `dsl-final.png`, полный `dsl.log` и PS/2 JSON.

Повтор подтвердил HD boot, PS/2, VGA, оболочку, файлы и штатную остановку
на последней BIOS-версии с BDA449 fix; предыдущие результаты не заменяют
его свидетельства. После обоих запусков повторён SHA256 исходных баз:
`bases-after.sha256` совпадает с `bases-before.sha256`, HD bases не изменены.
Это дополнительная ОС-регрессия для последнего firmware change, не новая
приёмка полноты CPU/FPU, сети или физических аудиоустройств.
