# Приёмка DSL с HD

Первый принятый запуск 2026-09-11: DSL4.4.10 установлена официальными dsl-hdinstall/dsl-installboot на `/tmp/ai386-dsl-hd.img`, 2ГиБ, RAM64МиБ. ISO не подключён при HD-загрузке. SHA256 исходного ISO: `6d9369c7b8fb0f23880c5ec44f5a4a2d9d628d7a1407ea83bf0b3249eda33537`.

Установлен GRUB0.97, default0/timeout1. Параметры ядра: `root=/dev/hda1 quiet vga=normal noacpi noapm nodma noscsi 2`. Используется исходный `/boot/linux24`, без подмены ядра и командной оболочки. HD: hda1 ext2, hda2 swap. Первичная настройка паролей завершена в госте; inittab исходной single-user установки запускает root bash на текстовых консолях.

Первый GRUB остановился с Error18 из-за отсутствия BIOS AH48; исправление bios-hd-geometry подтвердило чтение ядра и проверенный вход после121986501 инструкции. Лимит диагностики до входа увеличен с100 до500млн, сами проверки адреса/байтов/режима сохранены.

Первый рабочий прогон:

```sh
/tmp/ai386-dsl-installed-test -test.run TestDSLDiagnostic -test.v -test.timeout=2h -dsl-image=/home/kis/go/src/gitverse.ru/kshmatov/ai386/dsl-4.4.10.iso -dsl-hd=/tmp/ai386-dsl-hd.img -dsl-installed -dsl-boot-steps=500000000 -dsl-extra-steps=3000000000 -dsl-control-at-step=100000000 -dsl-continue-faults -dsl-require-shell -dsl-control -dsl-screen=/tmp/ai386-dsl-installed.png
```

BIOS DL80 проверен; CD detached. fsck сообщил clean. После настройки через PS/2 достигнута `root@tty1[/]#`; строгий тест выполнил `echo ai386ready` и проверил отдельную строку вывода. Затем вручную через тот же PS/2:

```sh
echo ai386persist > /root/ai386-persist.txt
cat /root/ai386-persist.txt
sync
shutdown -h now
```

Вывод cat: `ai386persist`. Остановка: `/dev/hda1 umounted`, `DSL halted.`. Изображение `/tmp/ai386-dsl-hd-halt-visible.png` визуально проверено; диагностический процесс закрыт только после размонтирования, PASS1280.76с. Предупреждения отсутствующих PCI/floppy/USB и первое read-only обновление hostname не мешают принятой работе PS/2/VGA/HD; звук/сеть не проверялись.

Повторная загрузка `/tmp/ai386-dsl-persistence-test` с тем же HD, без CD и раннего control, успешно завершена. Использованы `-dsl-boot-steps=500000000 -dsl-extra-steps=4000000000 -dsl-installed -dsl-continue-faults -dsl-require-shell -dsl-control`; остальные пути и RAM прежние. Сборка содержит исправления CPU вплоть до cpu-far-load-faults и F2XM1.

fsck: `/dev/hda1: clean, 11863/219968 files, 40951/439606 blocks`. После автоматической строгой проверки echo выполнены `cat /root/ai386-persist.txt` и `echo ai386ready`. Получены отдельные строки `ai386persist`, `ai386ready` и новое приглашение root. Снимок `/tmp/ai386-dsl-persist-read.png` визуально проверен. Затем sync/shutdown; `/dev/hda1 umounted`, `DSL halted.`; процесс закрыт после этого, PASS911.76с. Задача9.1 и operating-system-acceptance2.1 завершены.

В результате обе согласованные ОС прошли HD/PS2/VGA/файл/перезапуск. Эта приёмка не заменяет остатки CPU/FPU, звук и сеть.
