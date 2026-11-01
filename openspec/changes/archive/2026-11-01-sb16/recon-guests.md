# Разведка гостевых ОС для SB16 (задача 0.1)

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

Дата: 2026-09-17. Метод: статический анализ установленных образов гостевых ОС
и ISO средствами без root (`debugfs` по экстрагированному разделу ext3, `tools/iso-ls`,
`strings` по разделу UFS) плюс источники эталонов, загруженные при планировании.

## DSL 4.4.10 (Linux 2.4.31, OSS)

Образ: `/tmp/ai386-dsl-hd.img` (установка `dsl-hdinstall`, ext3 раздел LBA 63),
перечитан через `debugfs`.

- Модули звука OSS присутствуют (из `/lib/modules/2.4.31/modules.dep`):
  `kernel/drivers/sound/soundcore.o`, `sound.o`, `sb_lib.o`, `sb.o`,
  также `opl3.o`, `uart401.o`. Зависимости: `sb.o -> sb_lib.o -> sound.o ->
  soundcore.o` — загрузка `modprobe sb` разрешает цепочку автоматически.
- Сетевой ISA-адаптер (для этапа 8): `kernel/drivers/net/ne.o -> 8390.o` есть.
- Модули модprobe/insmod/lsmod/rmmod/depmod (`/sbin/*.modutils`) присутствуют.
- `/dev` статический: есть `dsp`, `dsp1`, `sequencer`, `audio`, `midi*`;
  узла `mixer` в списке `/dev` не найдено — при приёмке создать в госте
  `mknod /dev/mixer c 14 0` (OSS-мажор soundcore — стандартный).
- Инструменты прогона: `/bin/bash`, `/bin/busybox` (апплеты `dd`, `ifconfig`,
  `head`, `wc`, `cmp`), `/usr/bin/mpg123`, `netstat`, `route`, `telnet`, `ftp`,
  `expr`, `sleep`. Воспроизведение: `busybox dd if=<pcm> of=/dev/dsp bs=...`
  либо `mpg123` для ручного слухового теста; запись:
  `busybox dd if=/dev/dsp of=<файл> count=<байт> bs=1`.
- Параметры `sb.o` (Linux 2.4, `drivers/sound/sb.c` в скачанном
  `linux-2.4.37`): `io=,irq=,dma=,dma16=,mpu_io=` — целевая конфигурация
  `io=0x220 irq=5 dma=1 dma16=5` совпадает с профилем по умолчанию design.md.

Вывод: способ воспроизводим — загрузка `modprobe sb io=0x220 irq=5 dma=1 dma16=5`,
проверка `/proc/devices` (мажор 14), `cat /dev/sndstat` нет (это ALSA), вместо
него вывод `dmesg | grep -i sb`. Ограничение: точная совместимость с
реальным железом проверяется задачей 5.1.

## FreeBSD 4.11-RELEASE i386

Образ: `/tmp/ai386-freebsd-fpu-acceptance-final.img` (Slice 1 UFS, читан
`strings` без root).

- Модули звука c4b присутствуют в `/modules`: `snd_pcm.ko`(? в 4.11 расширение
  `.ko`), `snd_sbc.ko`, `snd_sb16.ko`, `snd_sb8.ko` — всего семейство
  `snd_*.ko` перечислено в дамп-анализе. Целевой путь: `kldload snd_sbc`
  (ISA non-PnP требует hints: `hint.sbc.0.at="isa"`, `port="0x220"`, `mport`,
  `irq="5"`, `drq="1"`, `drq2="5"` в `/boot/device.hints` или
  `kenv`+`kldload`).
- Сетевой ISA `ne`: статически подтвердить не удалось (в извлечённых строках
  явного `ne.ko`/`if_ne.ko` не видно; в образе есть полный `/usr/src` — модуль
  собираем при необходимости). Подтверждение переносится в живой прогон
  задачи 5.2 (этап 8): `kldload ne`; при отсутствии — `cd /usr/src/sys/modules/ne && make load`.
  Это не блокер этапа 7.

Вывод: для SB16 на FreeBSD способ есть (`snd_sbc`+`snd_sb16` из /modules,
hints для ISA). Воспроизведение/запись: `cat /dev/dsp ...` через OSS-эмуляцию
`/dev/dsp` c4b; для e2e файл: `dd if=<pcm> of=/dev/dsp`, `dd if=/dev/dsp of=<файл>`.

## High-speed команды 90h/91h/98h/99h (OQ3)

По скачанным источникам Linux 2.4.37 `drivers/sound/sb_audio.c`:
высокоскоростные коды используются только для траектории SB2.0 (`sb->hardware
== SB_HW_20/200/2000`), для SB16 (`SB_HW_16`) применяются `DSP_DAC_8BIT_DMA`
(С6х/CEх/С4х) и 16-битные Bх-коды. Вывод для SB16-приёмки: 90h/91h/98h/99h
не требуются — остаётся вне объёма. (Полная строчная сверка — задача 2.1.)

## Риски, зафиксированные для реализации

1. DSL не имеет `/dev/mixer` — приёмка записывает команду `mknod`.
2. ISA-звук FreeBSD non-PnP требует device.hints/kenv — сценарий 5.2 обязан
   явно их устанавливать.
3. Тест «реально слышно» на хосте Linux — ручной (задача 5.3).

## Запуск e2e DSL SB16 (2026-10-20)

```sh
go test ./internal/machine -run '^TestDSLSB16Guest$' -count=1 -v -timeout=45m \
  -args -sb-run -sb-dsl-hd=/tmp/ai386-dsl-hd.img -sb-steps=5000000000
```

Ожидание: ~15–25 минут (2.5–3B шагов до шелла + команды). Критерии в tests
`SBMOD=`, `SBPLAY=`, `SBREC=`.

Быстрый контур (без гостя): `go test ./internal/sb16/ -run 'OSSProbe'` и
`go test ./internal/machine -run TestSB16GuestCommandKeysCovered`.
