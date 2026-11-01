# FreeBSD: гостевая FPU-приёмка

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

## Область и исходное состояние

Задачи 5.1a–d, 5.2a–b. Исполнители по выбранному пользователем экономному
плану: GPT-5.6 Terra (high) — раннер, отчёт и рабочие HD;
GPT-6 Astra (high) — ABI, диагностика x87 и проверка результата.
Этот отчёт не закрывает DSL-часть 5.1/5.2 или итоговую сверку 5.4.

Исходный симптом: `AI386_FPU_STATUS:1` без текста отчёта.
Прежний вывод об ошибке FYL2X не подтверждён. Также прежние вызовы
инструмента выводили только `output`, теряя `session_id`: возврат из такого
вызова не подтверждал завершение процесса. Последующие запуски сохраняют
полный ответ и опрашивают активную сессию до `exit_code`.

## Рабочий HD

- База: `/tmp/ai386-freebsd-final-hd.img`.
- Копия: `/tmp/ai386-freebsd-5.2a.CvypUL/freebsd-working.img`.
- Размер обоих файлов перед запуском: 2147483648 байт.
- SHA-256 базы и подготовленной копии:
  `6aa54f7333f12c5998f6baf68ff1d0c1313cd1096a95b4c90988013fd3438831`.
- Копирование завершилось с exit code 0; побайтовое сравнение успешно.
  Копия имеет отдельный inode, открытых владельцев до запуска не обнаружено.
- Неполный прежний `/tmp/ai386-freebsd-fpu-baseline.img`
  (1178783744 байт) исключён из приёмки.

В исходном `/tmp/ai386-freebsd-fpu-status2.log` разделы a/f/g/e отмечены
`FILESYSTEM CLEAN; SKIPPING CHECKS`. В журнале повторного использования
`/tmp/ai386-freebsd-fpu-current.log` есть `SALVAGED` для f/e.
Это свидетельство изменения состояния ФС между прогонами, не измерение
производительности среды. Новая приёмка подтвердила чистую
остановку и повторную загрузку одного рабочего HD.

Для дальнейших запусков добавлен
`tools/freebsd-hd-prepare/prepare-freebsd-working-hd.sh`:

```sh
bash tools/freebsd-hd-prepare/prepare-freebsd-working-hd.sh \
  --expected-size 2147483648 --output-root /tmp \
  /tmp/ai386-freebsd-final-hd.img
```

Он создаёт уникальный каталог, проверяет размер, исходный hash до/после
копирования, hash копии, `cmp`, отдельную файловую идентичность и доступных
владельцев через lsof/fuser; пишет manifest только после успешной проверки.
Незавершённая копия удаляется из точно созданного mktemp-каталога.
Тесты используют небольшие временные файлы, включая открытый исходник
и ошибку копирования. Проверка процессов ограничена видимостью хоста:
в sandbox отсутствие вывода ps/lsof не является доказательством завершения
известной активной сессии. Раннер использует единый lock для рабочего HD;
он защищает сотрудничающие запуски, но не произвольную внешнюю ВМ,
запущенную в обход раннера. Базовый образ после первой сессии сохранил
прежний SHA-256 `6aa54f7333f12c5998f6baf68ff1d0c1313cd1096a95b4c90988013fd3438831`.

## ABI и x87

Подтверждён дефект гостевого syscall stub, не CPU. FreeBSD i386 читает
аргументы начиная с `ESP+4`, пропуская слот адреса возврата даже при прямом
`INT 80h`. Добавлены `pushl $0` перед `write`/`exit` и освобождение всех
16 байт после `write`. Основание:
[FreeBSD Developers' Handbook §12.3.1](https://docs-archive.freebsd.org/doc/7.0-RELEASE/usr/share/doc/en/books/developers-handbook/x86-system-calls.html).
Исходный stub передавал адрес строки вместо fd, а `exit` получал `argc=1`.

`TestGuestProbeSyscallABI` собирает настоящий исходник и проверяет ELF
OSABI, аргументы, текст и коды четырёх вариантов для Linux/FreeBSD.
До исправления FreeBSD-вариант обнаруживал fd `0x0804a000` вместо 1.
Регрессии частичного стека FNSAVE/FRSTOR и CPL3 #NM/CLTS/IRETD также
проходят. Производственный код CPU/FPU в рамках этих шести задач не менялся.

В настоящем ядре FreeBSD 4.11 получены все ожидаемые результаты:

```text
AI386_ABI_CASE:old:1:
AI386_ABI_CASE:abi0:0:AI386_ABI_WRITE
AI386_ABI_CASE:abi7:7:AI386_ABI_WRITE
AI386_ABI_CASE:fail:1:AI386_FPU_FAIL 1
AI386_ABI_CASE:fpu:0:AI386_FPU_OK
AI386_ABI_OK
```

`abi0`/`abi7` не выполняют x87. `fail` намеренно требует неверный результат
FYL2X. Успешная исходная FPU-программа проверяет FYL2X, FYL2XP1, FSIN,
FCOS, FSINCOS, FPTAN, арифметику, environment/context и masked zero divide.
Таким образом, после исправления ABI прежний симптом исчезает целиком;
основания для дополнительного исправления CPU не обнаружены.

## Первая завершённая сессия

Основа сборки: `22fa7b8dbbfb19a05d25b473631ba6f4443e378a` плюс изменения
этой приёмки в рабочем дереве. Сборка: `go test -c ./internal/machine
-o /tmp/ai386-freebsd-economy.test`.

```sh
/tmp/ai386-freebsd-economy.test -test.v \
  -test.run '^TestFreeBSDDiagnostic$' -test.timeout=30m \
  -freebsd-image=/tmp/ai386-freebsd-4.11-miniinst.iso \
  -freebsd-hd=/tmp/ai386-freebsd-5.2a.CvypUL/freebsd-working.img \
  -freebsd-installed -freebsd-control -freebsd-steps=1500000000 \
  -freebsd-shared=/tmp/ai386-freebsd-5.2a.CvypUL/share \
  > /tmp/ai386-freebsd-5.2a.CvypUL/first.log 2>&1
```

Терминальная сессия `55695` опрашивалась до окончательного `exit_code=0`.
Тест завершился `PASS`, длительность 1434.68 с (включая ожидание команд).
Причина завершения: после штатного halt отправлена управляющая команда
`{"quit":true}`, не таймаут или принудительная остановка процесса.

Команды через эмулируемую PS/2-клавиатуру после входа root:

```sh
mount -t cd9660 /dev/acd1c /mnt
sh /mnt/all.sh
sh /mnt/runfpu.sh
cksum /root/ai386-fpu-report.txt
sync;shutdown -h now
```

Для ожиданий `AI386_ABI_OK`, `AI386_FPU_DONE` и
`The operating system has halted` одновременно задавались `until` и
`require_text`: отсутствие ожидаемого текста приводит к отказу теста.
Строгий запуск сохранил отчёт на постоянном разделе HD:

```text
AI386_FPU_START
AI386_FPU_RUN
AI386_FPU_STATUS:0
AI386_FPU_OK
AI386_FPU_DONE
3774605809 13 /root/ai386-fpu-report.txt
syncing disks...
done
The operating system has halted.
--- PASS: TestFreeBSDDiagnostic (1434.68s)
PASS
```

SHA-256 свидетельств первой сессии:

| Артефакт | SHA-256 |
|---|---|
| `/tmp/ai386-freebsd-economy.test` | `fcfc8cd31b2e6f80cb94e43929a8b464939b71efb8297bd418fed7ec5fbbf8a2` |
| `first.log` | `81f7b7b250aaf64147db395ae74e393457a1db02a3e377cb1ec00a8478940f31` |
| `share/fpu` | `be26382826260f994c64b6069a9e7891eff79b39e983719c323fed39f5e5fb6c` |
| первый `share/runfpu.sh` | `57721f52fd6764c28d7329938cce50899ba7a994b510393e1d5b0f92625d7717` |

После этой сессии скрипт дополнительно усилен для отбраковки смешанного
отчёта `AI386_FPU_OK` + `AI386_FPU_FAIL 1`. Повторная сессия должна сначала
прочитать прежний отчёт, затем проверить окончательную версию скрипта.

## Промежуточный запуск и установленная причина SALVAGED

Вторая загрузка через новый host-runner закончилась на login без команд:
при запуске инструмента не был указан `tty:true`, stdin закрылся после
`CONTROL READY`. Журнал `/tmp/ai386-freebsd-5.2a.CvypUL/second.log`
содержит окончательный `AI386_FREEBSD_RUN_EXIT status=0
duration_seconds=124` (тест 123.92 с), но не чтение отчёта и не shutdown.
Поэтому этот запуск **не засчитан** как приёмка 5.2b. SHA-256 журнала:
`50fb01a2407a49c4e477caa9c37b742815ca634ab4124d0cc94687afb9157b74`.

Он подтвердил `FILESYSTEM CLEAN; SKIPPING CHECKS` для a/f/g/e после
первого штатного halt. После его выхода без гостевого shutdown третья
загрузка того же HD показала `WARNING: / was not properly dismounted`
и `FREE BLK COUNT(S) WRONG IN SUPERBLK (SALVAGED)` для f/e. Это
контролируемое сравнение устанавливает источник предупреждений:
повторное использование HD после окончания процесса без штатного
размонтирования гостевой ФС. Исходный чистый образ не повреждён;
объяснение через предполагаемую производительность среды не требуется.

Исправлен обнаруженный дополнительный false PASS: control loop теперь
требует явную команду `{"quit":true}`. EOF, в том числе после нескольких
команд, возвращает ошибку. README требует интерактивный терминал,
сохранение полного ответа инструмента и опрос session_id до exit_code;
`quit` отправляется после подтверждённого halt гостя.

## Повторная загрузка и итоговые проверки

Третья интерактивная сессия `65961` завершилась окончательным exit code 0,
`TestFreeBSDDiagnostic` PASS за 198.34 с, runner duration 198 с. До
повторного запуска программы она прочитала прежний отчёт:

```text
3774605809 13 /root/ai386-fpu-report.txt
AI386_FPU_OK
```

Затем окончательный `runfpu.sh` выдал `AI386_FPU_STATUS:0`, `AI386_FPU_OK`,
`AI386_FPU_DONE`; повторный cksum совпал. После `sync;shutdown -h now`
подтверждены синхронизация дисков и halt, затем отправлен explicit quit.
Журнал `third.log` имеет SHA-256
`9da29abac0f91cc7c960695c46dcffe186afdc3563bb308ada149d646ea54c15`.
Бинарник `/tmp/ai386-freebsd-5.2b.test` этой сессии:
`494ab4c8829a1bf70d83899e0c8bb9922d308a2ff959bcf834c8b57039591120`.

Контрольный чистый повтор на сборке с исправлением EOF: сессия `47627`,
окончательный `exit_code=0`, тест PASS за 156.78 с, runner duration 157 с.
Все разделы a/f/g/e — `FILESYSTEM CLEAN; SKIPPING CHECKS`, ни SALVAGED,
ни предупреждения о неправильном размонтировании нет. До перезаписи
прочитан тот же отчёт с cksum `3774605809 13`; окончательная FPU-проба
повторно завершилась STATUS 0 / OK / DONE. После неё cksum тот же.
Подтверждены `syncing disks... done`, `The operating system has halted`,
затем explicit quit и снятие lock после завершения процесса.

```sh
go test -c ./internal/machine -o /tmp/ai386-freebsd-final-accepted.test
sh tools/fpu-reference/guest/run-freebsd-session.sh \
  --log /tmp/ai386-freebsd-5.2a.CvypUL/final.log \
  --lock /tmp/ai386-freebsd-5.2a.CvypUL/freebsd-working.img.lock -- \
  /tmp/ai386-freebsd-final-accepted.test \
  -test.run '^TestFreeBSDDiagnostic$' -test.v -test.timeout=20m \
  -freebsd-image=/tmp/ai386-freebsd-4.11-miniinst.iso \
  -freebsd-hd=/tmp/ai386-freebsd-5.2a.CvypUL/freebsd-working.img \
  -freebsd-installed -freebsd-control -freebsd-steps=2000000000 \
  -freebsd-shared=/tmp/ai386-abi-gxA0Ui
```

Запуск с `tty:true`; через stdin переданы JSON с scan codes для `root`,
Enter и следующих команд. Для чтения требуется одновременно `until:
AI386_FPU_OK` и `require_text: 3774605809 13`; для выполнения — DONE и
STATUS:0; для остановки — полный текст halt. Лимит каждого ожидания —
100000000 инструкций, после cksum — 5000000.

```sh
cksum /root/ai386-fpu-report.txt;cat /root/ai386-fpu-report.txt
mount -t cd9660 /dev/acd1c /mnt;sh /mnt/runfpu.sh
cksum /root/ai386-fpu-report.txt
sync;shutdown -h now
```

После остановки — управляющее `{"quit":true}`. SHA-256 окончательных
артефактов:

| Артефакт | SHA-256 |
|---|---|
| `/tmp/ai386-freebsd-final-accepted.test` | `32f4281e026e82d85bfda6de9d554035a57ed92c925d74d842db3a549c5a745d` |
| `final.log` | `82cca20f9f953c808feaf76a4c4fdb678a747541e04839e1ba062b81eda0fa87` |
| `run-freebsd-session.sh` | `882af207f7f83a34c9751ecebea23b2bf44db32ae56240caf2fe42d18fae4102` |
| окончательный `runfpu.sh` в исходниках и shared CD | `9b661053ef1004f80e1db6126f230aec8f16f2e451f4a39e5a3c08723b46d6ca` |

Задачи 5.1a–d и 5.2a–b приняты. Рабочий HD оставлен после чистого halt;
база не изменена. DSL-части 5.1/5.2 позднее приняты на побайтово
идентичной сборке: [DSL FPU-приёмка](dsl-fpu-acceptance.md).
Итоговая сверка 5.4 остаётся открытой.

## Проверки кода и приёмочных инструментов

Окончательные проверки после исправлений ревью:

| Команда | Результат |
|---|---|
| `go test ./... -count=1` | PASS, terminal session `38897`, exit 0; `/tmp/ai386-freebsd-final-tests-retry.log` |
| `go test -race ./... -count=1` | PASS, terminal session `61797`, exit 0; `/tmp/ai386-freebsd-final-race-retry.log` |
| `go test ./internal/cpu -run TestConformanceSingleStepTests -count=1 -v` | PASS, 37640 passed / 0 failed, 941 opcode files; `/tmp/ai386-freebsd-economy-conformance.log` |
| `go vet ./...` | PASS, exit 0 |
| `go build ./...` | PASS, exit 0 |
| `openspec validate --all --strict` | PASS, 63 passed / 0 failed |
| `sh -n` для обоих guest/runner-скриптов, `bash -n` для HD preparation | PASS |
| `git diff --check` | PASS |

Conformance-раннер, эталоны, маски и допуски не изменялись. Новые проверки
host-скриптов могут явно пропускаться на хостах без необходимых POSIX/GNU
инструментов; в этой приёмке они выполнены. Первые попытки полного прогона
возвращали ошибку подготовки из-за read-only `/home/kis/.cache/go-build`;
они не засчитаны. Повтор с разрешённым доступом к кешу завершился успешно.

Негативные регрессии проверяют ненулевой статус, пустой/неверный отчёт,
сочетание OK и FAIL, ошибки копирования/chmod/записи/чтения/sync,
невозможность записи START/COMMAND и EXIT host-журнала и занятый lock.
Ошибка заголовка не запускает дочернюю команду; ошибка итогового журнала
возвращает отказ и сохраняет lock. `until` и `require_text` проверяются
независимо, поэтому дополнительный слабый текст не отменяет обязательный
маркер завершения. Astra повторно проверила устранение замечаний ревью;
открытых замечаний по этим изменениям не осталось.
