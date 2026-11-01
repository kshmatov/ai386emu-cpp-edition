# DSL: гостевая FPU-приёмка

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

## Область

Оставшаяся DSL-часть задач 5.1 и 5.2 `cpu-functional-completion`.
Проверяются все шесть новых x87-команд, арифметика, окружение и masked
zero divide, строгий отчёт на установленном HD и его сохранность после
штатной остановки и повторной загрузки. Это не итоговая сверка всех форм
CPU/FPU из задачи 5.4.

Сборка `/tmp/ai386-dsl-fpu-current.test` получена командой:

```sh
go test -c ./internal/machine -o /tmp/ai386-dsl-fpu-current.test
```

SHA-256 `32f4281e026e82d85bfda6de9d554035a57ed92c925d74d842db3a549c5a745d`
совпадает с `/tmp/ai386-freebsd-final-accepted.test` из
[принятой FreeBSD-сессии](freebsd-acceptance.md). Следовательно, результаты
FreeBSD и этой DSL-приёмки относятся к побайтово одинаковой сборке.
Основа — commit `22fa7b8` плюс уже существовавшие изменения рабочего дерева;
в этом этапе производственный код эмулятора не менялся.

## Подготовка

- База: принятый установленный `/tmp/ai386-dsl-hd.img`, 2147483648 байт.
- SHA-256 базы: `b629394156e45d59ecd1fc30442bb4282743d45a8140d2321bee5fc213c9f4af`.
- Рабочая копия: `/tmp/ai386-dsl-fpu-813LxO/working.img`, тот же размер,
  отдельный inode 694791 (база 668338), `cp` и последующий `cmp -s` — exit 0.
- Проверка fuser с полной видимостью хоста не обнаружила владельцев базы.
  Неудачная sandbox-проверка не использовалась как доказательство.
- ISO проверяется тестом по SHA-256
  `6d9369c7b8fb0f23880c5ec44f5a4a2d9d628d7a1407ea83bf0b3249eda33537`;
  в installed-режиме загрузочный CD отсоединён, загрузка идёт только с HD.
- Shared-CD содержит Linux ELF32/i386 `fpu` и окончательный `runfpu.sh`.
  Программа собрана из `tools/fpu-reference/guest/fpu_probe.S`:

```sh
as --32 -o /tmp/ai386-dsl-fpu-813LxO/fpu.o tools/fpu-reference/guest/fpu_probe.S
ld -m elf_i386 -o /tmp/ai386-dsl-fpu-813LxO/share/fpu /tmp/ai386-dsl-fpu-813LxO/fpu.o
cp tools/fpu-reference/guest/runfpu.sh /tmp/ai386-dsl-fpu-813LxO/share/runfpu.sh
```

SHA-256 программы: `00f43d6bce78ed79f79482ad6e216bb4367b6024ed53e192f607f18e6de51425`.
FreeBSD branding/stack ABI для этой программы не используются.

## Первая сессия

```sh
sh tools/fpu-reference/guest/run-freebsd-session.sh \
  --log /tmp/ai386-dsl-fpu-813LxO/first.log \
  --lock /tmp/ai386-dsl-fpu-813LxO/working.img.lock -- \
  /tmp/ai386-dsl-fpu-current.test -test.run '^TestDSLDiagnostic$' \
  -test.v -test.timeout=35m \
  -dsl-image=/home/kis/go/src/gitverse.ru/kshmatov/ai386/dsl-4.4.10.iso \
  -dsl-hd=/tmp/ai386-dsl-fpu-813LxO/working.img -dsl-installed \
  -dsl-shared=/tmp/ai386-dsl-fpu-813LxO/share \
  -dsl-boot-steps=500000000 -dsl-extra-steps=4000000000 \
  -dsl-continue-faults -dsl-require-shell -dsl-control-at-shell
```

Host-runner, несмотря на историческое имя, запускает произвольную команду
в foreground и применяется без изменения к DSL. Запуск — с `tty:true`,
сохранён полный ответ инструмента и terminal session `46604`.
`-dsl-control` и `-dsl-control-at-step` не добавлены: они создают отдельные
дополнительные control-сессии. Единственная сессия здесь начинается после
строгого автоматического `echo ai386ready` в госте. Завершение требует
явного JSON `{"quit":true}` после подтверждённого halt; EOF не даёт PASS.

DSL подтвердила исходный kernel entry после 121986501 инструкции,
`/dev/hda1: clean` и строгий `echo ai386ready`. `/proc/sys/dev/cdrom/info`
показал drive name `sr0` (IDE-CD передан гостевому ide-scsi), а `mount`:

```text
/dev/hda1 on / type ext2 (rw,errors=remount-ro)
```

Через PS/2 выполнены:

```sh
mount -t iso9660 /dev/sr0 /mnt;sh /mnt/runfpu.sh
cksum /root/ai386-fpu-report.txt
```

Результат строгой FPU-проверки:

```text
AI386_FPU_START
AI386_FPU_RUN
AI386_FPU_STATUS:0
AI386_FPU_OK
AI386_FPU_DONE
3774605809 13 /root/ai386-fpu-report.txt
```

Ожидание одновременно требует DONE и STATUS:0; скрипт печатает DONE
только после проверки содержимого и sync. Совместно с принятым результатом
FreeBSD это закрывает 5.1.

После `sync;shutdown -h now` первая сессия подтвердила:

```text
/dev/scd0 umounted
/dev/hda1 umounted
DSL halted.
--- PASS: TestDSLDiagnostic (842.13s)
PASS
AI386_FREEBSD_RUN_EXIT status=0 duration_seconds=842 reason=command-exited
```

После halt отправлен explicit quit, инструмент вернул окончательный
exit code 0 для сессии 46604; runner снял lock. Снимок `first-halt.png`
визуально проверен: на нём видны FPU-маркеры, checksum, umount и halt.
SHA-256 журнала `first.log`:
`7965bb27da38034e0663ecbda48ff723c986537792402fc28a6d020c56388f78`;
снимка `first-halt.png`:
`01f16c144ece5c8d5940d124358454e701fc58067065ea990432e5e22c72c351`.

## Повторная загрузка

После окончательного завершения первой запущена terminal session `81557`
на том же `working.img` и той же сборке. Команда аналогична первой,
но имя журнала — `second.log`, а параметр `-dsl-shared` отсутствует:
при повторе не подключены ни ISO, ни shared-CD. ISO по-прежнему указан
только для обязательной проверки исходного hash тестом.

После автоматического строгого echo вводятся:

```sh
cksum /root/ai386-fpu-report.txt;cat /root/ai386-fpu-report.txt
sync;shutdown -h now
```

Программа не запускается и отчёт не перезаписывается. Чтение одновременно
требует `AI386_FPU_OK` и `3774605809 13` (лимит 100 млн инструкций),
остановка — `DSL halted.` и `/dev/hda1 umounted` (лимит 300 млн).
После halt — explicit quit. Сохранены `persist.png` и `second-halt.png`;
оба снимка визуально проверены.

Повтор подтвердил `/dev/hda1: clean, 11866/219968 files, 40959/439606 blocks`.
После строгого echo прочитан именно прежний отчёт с HD, без запуска FPU
и без подключённых CD:

```text
3774605809 13 /root/ai386-fpu-report.txt
AI386_FPU_OK
/dev/hda1 umounted
DSL halted.
--- PASS: TestDSLDiagnostic (763.07s)
PASS
AI386_FREEBSD_RUN_EXIT status=0 duration_seconds=764 reason=command-exited
```

Инструмент вернул окончательный exit code 0 для session 81557; lock снят
после завершения процесса. SHA-256 свидетельств:

| Артефакт | SHA-256 |
|---|---|
| `second.log` | `3b42cef69ee03b2accdd6bd2b982956f564ac9815b1a7ab5e17eb4fc3c8e8db1` |
| `persist.png` | `1cf684b67441d8bad2ea6e60e18ce935943e135f3e061a1894c4c2f171c1e31f` |
| `second-halt.png` | `3e20ea66adf9cb1e02a7fc3c842e92a744010576488582203f6ce199fd76e7ff` |

Совместно с принятой FreeBSD-сессией на идентичной сборке это закрывает
5.2. Задачи 5.1–5.3 завершены, общий прогресс — 56/57. Задача 5.4
(полнота реестров и сверка итогового контракта) остаётся открытой.
Базовый HD не изменён, рабочая копия оставлена после чистого halt.

## Проверки хоста

На этой сборке выполнены без изменения масок, допусков и списка
conformance-случаев:

| Проверка | Результат |
|---|---|
| `go test ./internal/cpu -run 'TestGuestProbeSyscallABI\|TestX87' -count=1` | PASS, 1.020 с, session 46555 / exit 0 |
| `go test ./... -count=1` | PASS, session 81901 / exit 0, `tests.log` |
| `go test -race ./... -count=1` | PASS, session 76591 / exit 0, `race.log` |
| `go vet ./...` | PASS |
| `go build ./...` | PASS |
| `openspec validate --all --strict` | PASS, `openspec.log` |
| `git diff --check` | PASS |

После закрытия 5.1 повторены полный `go test ./... -count=1`
(`tests-after-5.1.log`, session 31775 / exit 0) и `go vet ./...` — PASS.
После закрытия 5.2 они снова выполнены успешно: `tests-after-5.2.log`,
session 52894 / exit 0; vet exit 0. SHA-256 исходного HD после обеих
сессий остался `b629394156e45d59ecd1fc30442bb4282743d45a8140d2321bee5fc213c9f4af`.

Относительные имена логов в этом отчёте относятся к
`/tmp/ai386-dsl-fpu-813LxO`. Полный CPU-набор включает неизменённые
37640 conformance-случаев. Эти проверки не заменяют выполнение программы
и сохранность отчёта в гостевой ОС.
