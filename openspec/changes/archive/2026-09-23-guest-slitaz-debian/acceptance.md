# Приёмка SliTaz 4.0 Base и Debian 3.1 Sarge i386

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

Дата: 2026-09-22. Ветка: `feature/guest-slitaz-debian`.

## Образы и воспроизведение

| Гость | ISO | SHA-256 | RAM |
|---|---|---|---|
| SliTaz 4.0 Base | `slitaz-4.0-base.iso` | `7f669e323fe19fc7521de7e4c48fc08a4906050a0ed6c477a2d7e7f4f8c2020c` | 64 МиБ |
| Debian 3.1r8 i386 netinst | `debian-31r8-i386-netinst.iso` | `5cee250e89b2993cf9053c74602868f45b6d1b1386d41803dba42c6622a4558e` | 128 МиБ |

SliTaz получен с `https://mirror.slitaz.org/i486/stable/4.0/iso/flavors/slitaz-4.0-base.iso`, Debian — с `https://cloudfront.debian.net/cdimage/archive/3.1_r8/i386/iso-cd/debian-31r8-i386-netinst.iso`. ISO лежат вне репозитория. Диагностический тест проверяет их точные SHA-256 перед запуском.

```sh
go test ./internal/machine -run '^TestAdditionalLinuxDiagnostic$' -count=1 -v -timeout=35m \
  -additional-linux-kind=slitaz \
  -additional-linux-image=/tmp/ai386-guest-distros/slitaz-4.0-base.iso \
  -additional-linux-require='login:' \
  -additional-linux-steps=4000000000

go test ./internal/machine -run '^TestAdditionalLinuxDiagnostic$' -count=1 -v -timeout=35m \
  -additional-linux-kind=debian \
  -additional-linux-image=/tmp/ai386-guest-distros/debian-31r8-i386-netinst.iso \
  -additional-linux-require='Press F1 for help'

go test ./internal/machine -run '^TestAdditionalLinuxDiagnostic$' -count=1 -v -timeout=35m \
  -additional-linux-kind=debian \
  -additional-linux-image=/tmp/ai386-guest-distros/debian-31r8-i386-netinst.iso \
  -additional-linux-boot-command='linux debian-installer/framebuffer=false' \
  -additional-linux-require='Choose a language' \
  -additional-linux-steps=2000000000
```

Тест SliTaz автоматически нажимает Enter на пункте загрузки по умолчанию. За лимит 4 000 000 000 инструкций он загрузил ядро и продолжал распаковку initramfs; `login:` за этот предел не появился, поэтому этот запуск ожидаемо завершился непройденным рубежом. Первый тест Debian подтверждает чтение ISO, запуск BIOS и вывод меню загрузчика. Третий пройден: Debian принял параметр загрузки и показал меню установщика `Choose a language` после 1 065 000 000 инструкций (426 с на текущем стенде). Ни один из этих тестов **не** означает завершённой установки ОС. Без параметров обычный пакет тестов пропускает длительный тест.

## Результат ручной диагностики

Для обоих ISO через PS/2 был отправлен Enter. У SliTaz ISOLINUX 4.05 до исправления не принимал ввод: он многократно вызывал INT 16h/AH=11h, отсутствовавший в BIOS. Добавлена поддержка AH=10h/11h с регрессионным тестом. После исправления загрузчик считал `/boot/bzImage` и `/boot/rootfs.gz`, Linux распаковал ядро, вывел `SMP disabled`, `Brought up 1 CPUs`, `NET: Registered protocol family 16`. Затем появился стек с `stop_machine_cpu_stop+0x42/0x96`. Снимок RAM показал причину: ядро записало `invalid opcode: 0000` на `F3 90` (`PAUSE`). Декодер исправлен: на i486 `PAUSE` исполняется как `NOP`, согласно [Intel SDM](https://cdrdv2-public.intel.com/835781/325462-sdm-vol-1-2abcd-3abcd-4.pdf). Повторный прогон прошёл эту точку, зарегистрировал сетевой стек и распаковал initramfs. После `Freeing unused kernel memory` процесс `init` завершился, и ядро вывело `Kernel panic - not syncing: Attempted to kill init!`. В стеке присутствует `do_invalid_op`; точный код, вызвавший исключение в `init`, пока не установлен.

Debian ISOLINUX 2.04 считал `/install/vmlinuz` и `initrd.gz`; установщик Linux 2.4 достиг экрана `[!!] Choose language`. В обычном режиме снимок VGA содержал только заголовок, затем экран стал чёрным. [Руководство Debian 3.1](https://archive.debian.org/debian/dists/sarge/main/installer-i386/current/doc/manual/en/ch05s02.html) рекомендует `debian-installer/framebuffer=false` при пустом экране. С этим параметром меню языка отображается целиком. Через PS/2 выбраны English, United States и American English; далее проверяются обнаружение оборудования и установка.

Следующий рубеж установщика — `Detect and mount CD-ROM`. В исходном прогоне устройство `/dev/cdroms/cdrom0` было найдено, но монтирование завершилось ошибкой. На диагностической консоли Linux были видны `hdb: command error: status=0x41`, `error=0x50` и I/O errors на секторах 2184, 1172 и 64. Запись пакетов показала отсутствующие команды READ DISC INFORMATION (`51h`) и SEEK(10) (`2Bh`). После их реализации Debian смонтировал тот же ISO и загрузил 100% компонентов установщика, включая `partman`, `netcfg` и дисковые модули. Регрессионный тест повторяет двухбайтовый probe READ DISC INFORMATION, полный ответ, допустимый SEEK и ошибку LBA вне образа.

Повторный гостевой прогон дошёл до `Detecting network hardware`. За лимит 7 000 000 000 инструкций экран разметки не появился; установка на HD и загрузка установленной системы не подтверждены. Образ HD был отдельным новым файлом размером 2 ГиБ.

Сеть внутри этих гостей не проверена: SliTaz не достиг оболочки, Debian пока не дошёл до настройки сети. Состояние NE2000/user NAT следует из отдельных сетевых проверок, а не из этих прогонов.

## Проверки проекта

- `go test ./...` — пройдено, включая тесты с локальными TCP/UDP-сокетами.
- `go test -race ./...` — пройдено.
- `go vet ./...` — пройдено.
- `go build ./...` — пройдено.
- `openspec validate --all --strict` — 11 из 11 проверок пройдено.
- `git diff --check` — пройдено.
- Review против `AGENTS.md` и этого change: критичных замечаний к добавленным BIOS/CPU/ATAPI изменениям нет. Тяжёлые ISO-тесты остаются opt-in; их ограничения описаны выше.

## Следующие дефекты

1. Довести автоматический прогон SliTaz до `init`, затем установить точную инструкцию или ошибку памяти, завершающую процесс.
2. Проверить обнаружение ISA NE2000 установщиком Debian, затем завершить установку и загрузить систему с HD.
3. После достижения оболочки проверить DHCP и исходящее соединение для каждого гостя.
