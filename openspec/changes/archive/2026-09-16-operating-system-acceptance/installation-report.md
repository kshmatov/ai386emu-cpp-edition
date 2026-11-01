# Установка FreeBSD — в работе

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

Образ и RAM/HD — scenarios.md. Диск `/tmp/ai386-freebsd-hd.img` создан новым, до начала отсутствовал; размер 2147483648 байт. После ata-seek-complete ядро определяет ad0 (2047MB, 4161/16/63) и acd0. Sysinstall достигнут после 380 млн шагов; окно и выбранный Usage проверены в /tmp/ai386-freebsd-menu.png.

Управление: `go test -c -o /tmp/ai386-machine-test ./internal/machine`, затем бинарник с `-test.run TestFreeBSDDiagnostic -test.v -test.timeout=60m -freebsd-image=/tmp/ai386-freebsd-4.11-miniinst.iso -freebsd-hd=/tmp/ai386-freebsd-hd.img -freebsd-require-installer -freebsd-control`. В stdin идут JSON-команды keys/events/steps/png; клавиши передаются через PS/2. `go test` не передаёт stdin, поэтому используется его собранный бинарник.

Попытка выбрать Standard через Down/Enter выявила отсутствие реакции клавиатуры. ps2-keyboard-initialization исправил IBF, бит запрета порта, IRQ gating, ECHO и reset/identify. После исправлений FreeBSD обнаруживает atkbd0/kbd0; Down выбирает Standard (скриншот /tmp/ai386-freebsd-echo.png), Enter открывает инструкцию разметки диска. Ввод принят. Установка, запуск с HD и сохранность файла пока не подтверждены.

Замечание для этапа FPU: ядро сообщает `npx0: error reporting broken; using 387 emulator`. Полнота исключений x87 остаётся в 2.7–2.8.

Установщик: Standard → FDISK A (весь новый ad0) → Q → Standard MBR → Disklabel A → Q → Minimal → CD/DVD. Авторазметка: / 128MB, swap 113MB, /var 256MB, /tmp 256MB, /usr 1294MB. Подтверждена запись на новый образ; установка выполняется гостем.

Первый прогон остановлен на 22% распаковки bin: VGA повреждена. Найдена плоская E820-карта, объявлявшая область VGA/BIOS usable. Исходник FreeBSD getmemsize() включает такие регионы в phys_avail. Диск /tmp/ai386-freebsd-hd.img оставлен как неполная установка, он не прошёл приёмку. Повторный прогон после bios-memory-map использует новый /tmp/ai386-freebsd-e820-hd.img (2 ГиБ), с двумя дополнительными CD из /tmp/ai386-cd-acceptance-1 и -2 для проверки 3.6.

После E820 fix оба ядра достигнуты (DSL 11.613 с), FreeBSD обнаруживает acd0/1/2. На 2% распаковки cpio сообщает invalid header: checksum error (Alt-F2). Alt-F4 открывает рабочий shell. Сравнение UFS-файлов с исходным tar: все /bin до mv совпадают; первое повреждение mv на смещении 124234, tar offset 1454410 (0x16314a), соответствующий байт gzip 671744 (0xa4000, ровно граница 16 КиБ). Обнаружен дефект INS/OUTS при page fault, регрессии подтверждены; причинная связь с распаковкой пока проверяется. Диск /tmp/ai386-freebsd-e820-hd.img остаётся неполной диагностической установкой.

После INS/OUTS fix чистый прогон /tmp/ai386-freebsd-io-hd.img прошёл прежнюю точку cpio checksum: 3%, chunk 5, 1128448 байт. Установка продолжается; это ещё не приёмка HD boot/persistence.

Текущий прогон достиг 42%, chunk 66/154, 15732736 байт. Снимок /tmp/ai386-freebsd-io-progress.png визуально проверен: повреждения VGA после прежних 22% нет. Приёмка исправления E820 завершена; HD boot/persistence остаются открытыми.

Основной bin-набор FreeBSD распакован полностью, начат crypto (19%). Прежние повреждения tar/VGA не повторились.

## Установка DSL — в работе

DSL 4.4.10 повторно достигла shell и выполнила echo ai386ready после исправлений CPU/E820. Диск /tmp/ai386-dsl-hd.img (2 ГиБ) размечен гостевым fdisk: hda1 bootable type83, цилиндры1–3489, 1758424.5KiB; hda2 type82, цилиндры3490–4161, 338688KiB. Гостевые mkswap/swapon успешны. Запущен /usr/sbin/dsl-hdinstall: hda1, multi-user=n, ext3=n (ext2), подтверждение записи=y. Файловая система создана, копирование CD→HD продолжается. Приёмка загрузки и сохранения ещё не выполнена. При запуске HD нужно выбрать текстовый runlevel 2, как в сценарии CD.

## Установка и первый HD-пуск

FreeBSD bin/crypto/MAKEDEV завершены, Congratulations подтверждён; финальная конфигурация завершена без дополнительных пакетов. На VT4 mount показал ad0s1a→/mnt и отдельные tmp/usr/var, три acd устройства. Управляющий тест завершился ошибкой JSON: длинная строка превысила лимит PTY; guest sync/shutdown в этом запуске не выполнены. Следующие команды ограничены короткими строками.

Первый HD-bootstrap DL80 прошёл, но loader не обнаружил BIOS drive C: BDA475 был0. Причина подтверждена исходником [FreeBSD4.11 biosdisk.c](https://github.com/freebsd/freebsd-src/blob/releng/4.11/sys/boot/i386/libi386/biosdisk.c), bd_init. Исправление bios-disk-count проходит тесты, повтор запущен. Полная HD-приёмка пока открыта.

DSL частично скопирована на HD; Ctrl-C, sync и umount завершились, маркер ai386saved подтверждён. После этого диагностический тест истёк в ожидании stdin. Повтор с тем же HD запущен с лимитом3ч; форматирование не повторяется.

## FreeBSD: установка завершена, проверка перезапуска

Рабочий HD — `/tmp/ai386-freebsd-io-hd.img`,2ГиБ. Принятые исправления E820/INS/OUTS устранили повреждение установки, BDA475 — обнаружение HD загрузчиком, x87 #MF — npx INT16. Первый старт создал RSA/DSA host keys; последующие старты не повторяют генерацию.

Первый root login выявил SIGSEGV csh. Core указал на MOV GS в signal trampoline: markAccessed обращался к supervisor LDT с CPL3. После cpu-descriptor-accessed root/csh работает, `echo ai386ready` подтверждён; прежние signal11 sendmail не повторяются. Три CD прочитаны, запись на shared отклоняется (attach-all-cds/acceptance.md).

Создан `/root/ai386-persist.txt` со строкой `ai386persist`, прочитан командой cat. Затем гостевые sync и shutdown -h now: `syncing disks... done`, `The operating system has halted`. Диагностический тест PASS428.14с, завершён только после гостевого halt. Повтор того же HD запущен с прежними параметрами; проверка сохранности после перезапуска ещё выполняется.

## DSL: копирование завершено

Возобновление штатного dsl-hdinstall без повторного форматирования завершило копирование и настройку root/home. Показаны `The installation process is finished` и предложение загрузчика. Запущен штатный dsl-installboot; выбран GRUB MBR. Дальше требуется установить runlevel2 в menu.lst и проверить запуск с HD без CD.

Штатный grub-install завершился `Installation finished. No error reported`, (hd0) /dev/hda. Автоматическая перезагрузка отклонена для настройки принятого текстового сценария; исходный menu.lst включает frugal, заменяется на2 в установленном HD.
