# Независимая проверка BIOS

Проверены незакоммиченные изменения `internal/bios` относительно HEAD в ветке `feature/freedos-doom`. Контракт: `research.md`, `design.md`, `status.md` этой директории. Review проводился независимо от реализации; дополнительные тесты запускались через Go overlay из `/tmp/ai386-bios-review`.

## Результат повторной проверки

Обе найденные регрессии исправлены. Открытых подтверждённых замечаний к BIOS-изменениям нет. Проверка самого установщика FreeDOS и загрузки установленного HD продолжается у root; этот отчёт их успешность не подтверждает.

Обновление результата FLAGS теперь выполняют гостевые инструкции ROM thunk в `internal/bios/bios.go`, поэтому доступ к стеку проходит через CPU MMU. Native guard в `internal/bios/hooks.go` только выбирает обработчик. IRQ0 использует короткий `INT 08; IRET` и сохраняет исходный interrupt frame без изменений. Новые BIOS entry по `F000:(1000h+vector*64)` не пересекаются между собой и с IRQ0 по F000:0200.

## Исправленные находки

### P1 — цепочка IRQ0 портила FLAGS прерванного кода — закрыто

Исходное место: `internal/bios/timer.go:11` и удалённая запись saved FLAGS в `internal/bios/hooks.go`.

Гостевой IRQ0 `XOR AX,AX; JMP FAR old BIOS IRQ0` меняет арифметические флаги перед передачей управления BIOS. Первоначальная реализация общего hook записывала их в сохранённые FLAGS прерванной программы. При входных FLAGS=0xA03 получалось 0x246. Исправленная версия сохраняет 0xA03 и увеличивает BDA tick. Воспроизводящий тест `TestReviewTimerChainPreservesInterruptedFlags` теперь PASS.

### P2 — запись saved FLAGS обходила paging в V86 — закрыто

Исходное место: удалённые `Bus.ReadMem/WriteMem` по `SegBase(SS)+SP+4` в `internal/bios/hooks.go`.

При V86+PG, IOPL=3, отображении linear stack 0x6000 → physical 0x16000 и SP=0x7000 вызов `PUSHF; CALL FAR [0054h]`, AH=88h, CF=1 возвращал правильный AX=15360, но ошибочный CF=1. Native hook писал по линейному адресу как по физическому. После переноса работы со стеком в ROM инструкции возвращается CF=0. Тест `TestReviewV86PagedFarCallReturnsServiceFlags` теперь PASS.

Оба теста сначала воспроизвели FAIL на первоначальном исправлении и PASS с исходными `bios.go`/`timer.go` из HEAD. После доработки оба PASS на текущих исходниках. Постоянные регрессии добавлены исполнителем в `internal/bios/hooks_regression_test.go`.

## Проверки

- Полный `go test ./internal/bios -count=1` — PASS.
- Независимые overlay-тесты — PASS: IRQ0 chaining; V86 с переназначенной страницей стека; сохранность BX/BP/SI/DI и IF/DF; возврат при SP=7000h и переходе SP через ноль; восстановление сохранённого TF.
- Существующие BIOS-тесты проверяют гостевую замену IVT, FAR CALL, цепочку INT13 с успешным и ошибочным CF, восстановление IF и INT12 по актуальному BDA.
- Полный запуск `go test ./internal/bios ./internal/machine -count=1`: 196 PASS, 5 SKIP, один FAIL. Единственный FAIL — `TestNetworkEndToEndThroughNE2000`: sandbox запрещает `listen udp4 127.0.0.1:0: socket: operation not permitted`. Прямой повтор этого теста подтвердил причину. Это ограничение среды; сетевую приёмку должен завершить root вне данного ограничения.
- Прямой native-hook вызов при исходном IVT продолжает обходить IDT/проверки PM/V86. Это поведение существовало до изменения и не объявляется новой регрессией. Исправленная новая регрессия V86 проверена отдельно, как описано выше.

Повтор независимых тестов:

```sh
GOCACHE=/tmp/ai386-bios-review/go-cache go test -overlay=/tmp/ai386-bios-review/overlay.json ./internal/bios -run TestReview -count=1 -v
```

Исходник тестов: `/tmp/ai386-bios-review/review_probe_test.go`. Overlay не меняет рабочие исходники. Контрольная первоначальная пара тестов на HEAD воспроизводится с `/tmp/ai386-bios-review/before-overlay.json` и фильтром `-run 'TestReviewTimerChain|TestReviewV86Paged'`.

## Правила проекта

Проверены `AGENTS.md` и `/home/kis/.codex/RTK.md`. Ветка соответствует feature-ветке git-flow, изменения покрыты testify-тестами, настраиваемых пользовательских параметров не добавлено. Подтверждённых нарушений правил не найдено. Полные линтеры и гостевая приёмка остаются частью интеграционной проверки root.

Итог: замечаний по правилам — 0; открытых замечаний по корректности BIOS — 0. Найденные P1 и P2 закрыты после независимого повторного воспроизведения.

## Дополнительная проверка CPU: HIMEMX / unreal mode

Проверены `internal/cpu/{modes.go,accessor.go,ea.go,unreal_mode_test.go}` после сохранения hidden limit/AR при загрузке real-mode сегментов и снятия принудительного лимита 64 КиБ для действительных кэшей. В актуальном `ea.go` учтён reset: при `realMode && !cache.valid` остаётся лимит 64 КиБ. Без этой границы `CPUReset`, оставляющий DS/SS cache.valid=false, делал бы обычные обращения до первой загрузки сегментов недопустимыми; в проверенном срезе эта проблема устранена.

Функциональных замечаний к текущей реализации не обнаружено. Проверка прав protected mode не меняется; проверки VM86 продолжают использовать 64 КиБ независимо от старого hidden cache. `loadSegment` сохраняет hidden limit/AR только для действительных real-mode data caches, обновляет selector/base; V86 переинициализирует cache, а путь CS остаётся прежним.

Проверки:

- `GOCACHE=/tmp/ai386-bios-review/go-cache go test ./internal/cpu -count=1` — PASS, 33.504 s; полный вывод `/tmp/ai386-cpu-review/full-test.log`.
- `TestUnrealModeCopyRetriesAfterGP` и `TestVirtual86DoesNotUseUnrealModeLimit` — PASS.
- Независимые overlay-тесты — PASS: MOV из DS сразу после reset; PUSH/POP через SS сразу после reset без MOV SS; слово, оканчивающееся ровно на обычном/расширенном limit; слово, пересекающее каждый из этих limit, с ожидаемым #GP; сохранение base/limit/AR при reload DS/ES/SS/FS/GS в real mode и возврат к 64 КиБ в V86; real-mode POP при SS.B=1 и ESP>64 КиБ.

Повтор независимых CPU-проверок:

```sh
GOCACHE=/tmp/ai386-bios-review/go-cache go test -overlay=/tmp/ai386-cpu-review/overlay.json ./internal/cpu -run 'TestReview|TestUnreal|TestVirtual86DoesNot' -count=1 -v
```

Файл тестов `/tmp/ai386-cpu-review/review_cpu_test.go`. Рабочие CPU-исходники review не менял. Полный первый запуск на промежуточном срезе до reset fallback был остановлен; приведённый PASS относится к актуальному исправлению.

Неблокирующий документационный недочёт: комментарий `checkLimit` в `internal/cpu/ea.go` всё ещё называет лимит real mode фиксированным 64 КиБ. Его стоит согласовать с новой семантикой действительных hidden caches. Новых высоких или средних замечаний нет.

## Дополнительная проверка INT 10h / AH=08h для FDI

Проверены `internal/bios/int10.go` и `internal/bios/int10_test.go`. Новый case читает текущую позицию курсора, получает символ и атрибут через `VGA.TextCell`, возвращает AL/AH без изменения курсора, видеопамяти или FLAGS. `TextCell` использует текущую текстовую геометрию, окно цветного/монохромного режима и CRTC start address; повторного вычисления адреса и расхождения с существующим AH=09h не появляется.

`TestInt10ReadCharacterForWindowRepaint` проверяет именно наблюдавшийся цикл FDI: AH=08h возвращает AX=1F46h для символа F, затем AH=09h меняет атрибут на 70h, сохраняя символ и позицию курсора. Подтверждённых замечаний в рамках исправления перерисовки текущей страницы нет. Внутри новых пяти строк нет диагностических побочных эффектов.

Ограничение существующей реализации видео-BIOS сохраняется: отдельные BIOS-страницы и их курсоры по BH не моделируются; AH=02h/03h/09h уже используют общий аппаратный курсор, новый AH=08h согласован с этим поведением. Эта проверка не подтверждает полную реализацию AH=08h для всех страниц или распознавание символов в графических режимах. Для подтверждённого FDI text-mode/current-page сценария это не блокирующее замечание.

`GOCACHE=/tmp/ai386-bios-review/go-cache go test ./internal/bios ./internal/vga -count=1` — PASS. Фактическую перерисовку установщика после исправления проверяет root.

## Финальная дополнительная проверка INT 13h/AH=43h и hd_size

Проверены `internal/bios/int13.go`, `internal/bios/int13_extended_write_test.go`, `internal/config/config.go`, `internal/config/config_test.go`. Подтверждённых функциональных замечаний в заявленном сценарии FreeFDISK (AL=0, базовый 16-байтный DAP, writable HD) и загрузке размера диска нет.

AH=43h выбирает устройство по DL, проверяет read-only, минимальную длину DAP и ненулевое количество блоков. Полный 64-битный LBA и конец диапазона проверяются до первой записи; выражение `blocks > total-lba` выполняется только после `lba < total`, поэтому проверка не переполняется. Преобразование LBA к int безопасно после сравнения с `NumSectors()`, возвращающим int. Само содержимое передаётся существующему `writeSectorsFrom`; ошибки `WriteSector` возвращаются гостю через CF/AH. Предварительная проверка исключает частичную запись для неверного диапазона; атомарность при ошибке самого устройства этим не заявляется.

Постоянные тесты покрывают запись нескольких секторов и чтение обратно через EDD/CHS, ненулевые DS:SI и segment:offset буфера, read-only без изменения данных, короткий/пустой DAP, высокие биты LBA и пересечение конца диска. Независимый overlay дополнительно проверил последний допустимый сектор, пустой диск, максимальный uint64 LBA и отказ `WriteSector`: все PASS.

`hd_size` теперь явно декодируется из документированного TOML-ключа. Fallback к прежнему `hdsize` применяется только при отсутствии основного поля; заданное, но некорректное основное значение не скрывается fallback. Существующий ParseRAM сохраняет форматы размера и проверки переполнения. Независимо проверены прежние написания `HDSize`, `HDSIZE`, `hdsize`, а также `hd_size`/`HD_SIZE`; все дают нужный размер. Подтверждены ошибки пустого/некорректного `hd_size` при корректном legacy и приоритет корректного `hd_size` над некорректным legacy.

Проверки:

```sh
GOCACHE=/tmp/ai386-bios-review/go-cache go test ./internal/bios ./internal/config -count=1
GOCACHE=/tmp/ai386-bios-review/go-cache go test -overlay=/tmp/ai386-final-review/overlay.json ./internal/config ./internal/bios -run '^TestReview' -count=1 -v
```

Первый запуск — 150 PASS (BIOS 99, config 51). Второй — все независимые проверки PASS. Исходники overlay: `/tmp/ai386-final-review/{disk_probe_test.go,config_probe_test.go}`; рабочие файлы review не изменял.

Этот review подтверждает базовую EDD-запись AL=0 и совместимость ключей конфигурации. Расширенные форматы DAP и отдельный режим write-with-verify AL=1 здесь не проверялись. Полная гостевая установка и запуск HD остаются критериями фактической приёмки root.

## Диагностика VINFO: C: ошибочно определяется съёмным

Фактическое наблюдение root после FORMAT: `DIR C:` читает FAT16 с меткой FREEDOS2025, но `VINFO /V /D C:` печатает `Drive is removable`. В извлечённом `/tmp/ai386-fdi-STAGE500.BAT` код 2–4 попадает в WrongTypeDrive и выходит без отдельного диалога. Это согласуется с остановкой установщика после Welcome.

Из `/tmp/ai386-fdinst.img` извлечён `/FREEDOS/BIN/VINFO.COM` (3112 байт, UPX) в `/tmp/ai386-vinfo.com`. Исходники V8Power скачаны в `/tmp/ai386-v8power-source`, HEAD `cb29883a516b885a4cb4f50f3c0cabafed4073c7`. `DiskStatus` сначала проверяет CD-ROM через INT2F/AX1500, затем действительность диска через INT21/AX4408, наличие файловой системы через FindFirst, сеть через INT21/AX4409 и съёмность повторным INT21/AX4408. Для съёмности используется CF=0, AX=0; код VINFO в этом случае 2. Другие коды: 3 CD-ROM, 4 network, 5 not formatted, 15 invalid drive. Источник: [V8Power DSKTLS.INC](https://github.com/LoopZ/V8Power/blob/cb29883a516b885a4cb4f50f3c0cabafed4073c7/SOURCE/DSKTLS.INC), [описание VINFO](https://github.com/LoopZ/V8Power/blob/cb29883a516b885a4cb4f50f3c0cabafed4073c7/HELP/en/vinfo.en).

Причина в BIOS подтверждена цепочкой точной версии FreeDOS kernel `ke2043` (commit `36428f4cd250f913bde53198be1160a9de01633d`; локальные исходники `/tmp/ai386-fdos-kernel-source`, файлы нужной версии доступны через `git show ke2043:kernel/...`):

1. `init_readdasd` вызывает INT13/AH15. Только CF=0/AH=3 даёт DF_FIXED; AH=2 даёт DF_CHANGELINE; ошибка оставляет нулевые признаки. При создании DDT результат добавляется в `ddt_descflags`. [initdisk.c](https://github.com/FDOS/kernel/blob/ke2043/kernel/initdisk.c).
2. `blk_Media` возвращает S_BUSY для DF_FIXED; без этого признака возвращает статус без S_BUSY. [dsk.c](https://github.com/FDOS/kernel/blob/ke2043/kernel/dsk.c).
3. INT21/4408 передаётся как C_REMMEDIA, а результат AX равен 1 при S_BUSY, иначе 0. [ioctl.c](https://github.com/FDOS/kernel/blob/ke2043/kernel/ioctl.c).

До исправления BIOS не реализовывал AH15 и возвращал CF=1. Следовательно, уже при загрузке ядра HD терял DF_FIXED, DOS сообщал removable, VINFO возвращал 2. Исправление требует повторной загрузки DOS, чтобы обновить сохранённые DDT-признаки.

## Независимая проверка исправления AH=15h

Проверены `internal/bios/int13.go` и `internal/bios/int13_drive_type_test.go`. Для HD возвращаются CF=0, AH=3 и 32-битное количество секторов в CX:DX; старшее слово проверено отдельным тестом. Для floppy возвращаются CF=0/AH=1, без заявления поддержки отсутствующего change-line сервиса; CX/DX сохранены. CD остаётся неподдерживаемым в legacy AH15 (CF=1/AH=01), неизвестный DL проходит существующий путь DriveNotReady. Данные диска не изменяются.

`go test ./internal/bios -count=1` — 102 PASS. Независимые overlay-тесты `TestReviewDiskTypeCDStaysUnsupported` и `TestReviewDiskTypeGuestChain` — PASS: первый проверяет CD, второй заменяет IVT13 гостевым JMP FAR на оригинальный BIOS entry и проверяет возврат AH=3, CX:DX=16, CF=0, сохранение IF и стека.

```sh
GOCACHE=/tmp/ai386-bios-review/go-cache go test -overlay=/tmp/ai386-final-review/overlay.json ./internal/bios -run '^TestReviewDiskType' -count=1 -v
```

Подтверждённых замечаний к исправлению AH15 нет. Полную установку после повторной загрузки DOS проверяет root.

## INT 4Bh: отсутствующий VDS и независимая проверка fallback

Исходная гостевая трасса `/tmp/ai386-freedos-int0-trace.txt` показала UDVD2, INT4B с AX=8103h, DX=000Ch и указателем DDS; нулевой IVT4B отправлял выполнение в таблицу векторов, где AAM 0 порождал #DE. В извлечённом UDVD2.SYS после INT4B проверяется CF; AL не анализируется. CF=1 переводит драйвер на существующий путь без VDS. Дизассемблирование сохранено в `/tmp/ai386-udvd2-disasm.txt`.

Первичный контракт [Microsoft Virtual DMA Services Specification, Q93469](https://jeffpar.github.io/kbarchive/kb/093/Q93469/) требует перед вызовами проверять bit 5 по адресу 0040:007Bh. При нулевом бите провайдер VDS не установлен; результат ошибочного вызова в этом состоянии спецификация не задаёт. AL=0Fh явно определён для зарезервированных функций установленного провайдера, поэтому переносить это требование на отсутствующий провайдер и AX=8103h оснований нет. Существующая реализация [DOSBox-X INT4B_Handler](https://raw.githubusercontent.com/joncampbell123/dosbox-x/master/src/ints/bios.cpp) возвращает CF=1 без изменения AX. Такой fallback подходит наблюдаемому драйверу, не объявляя поддержку VDS. Простое IRET сохраняло бы исходный CF и могло выглядеть как успешный вызов.

Проверены `internal/bios/bios.go` и `internal/bios/int4b_test.go`: установлены ROM entry и IVT4B, native handler меняет только CF, стандартная защита `setBIOSHook` позволяет гостю заменить вектор. BIOS оставляет BDA-признак VDS нулевым. Постоянные тесты проверяют INT и PUSHF/CALL FAR с регистровым набором UDVD2, сохранение IF, SP и DDS, а также приоритет гостевого обработчика.

Независимый overlay `TestReviewUnavailableVDSProbeAndChain` дополнительно проверяет AX=8102h/8103h/8104h/810Dh, гостевой JMP FAR на старый BIOS entry, отсутствие изменения AX/BX/BP/SP, сохранение всех исходных FLAGS кроме CF (0A96h → 0A97h) и неизменность нулевого BDA VDS bit.

```sh
GOCACHE=/tmp/ai386-bios-review/go-cache go test -overlay=/tmp/ai386-final-review/overlay.json ./internal/bios -run '^TestReviewUnavailableVDS' -count=1 -v
GOCACHE=/tmp/ai386-bios-review/go-cache go test ./internal/bios -count=1
```

Оба запуска PASS. Подтверждённых замечаний к fallback нет. Root сообщил, что фактическая приёмка с CF=1 прошла прежний сбой STAGE600 и дошла до выбора клавиатуры. Продолжение установки проверяется отдельно.

Для следующих шагов из исходного FDINST.IMG извлечён полный каталог в `/tmp/ai386-fdi-files`; точные английские строки находятся в `FREEDOS/SETUP/EN/FDSETUP.DEF`: `Transfer system files to drive`, `Force new boot sector code on drive`, `packages do you want to install?`, `Do you want to install now?`, `is now complete.`, `Do you want to reboot now?`. Скрипт STAGE800 по умолчанию выбирает отказ (вариант 2), STAGE900 — перезагрузку (вариант 1).

## JEMM / SHSUCDX: NT при входе через interrupt/trap gate

Root получил точную трассу после завершённой установки: SHSUCDX устанавливает NT через POPF (FLAGS=27246h), INT2F передаёт управление JEMM, но live NT сохранялся (FLAGS=7046h). Поэтому IRETD по адресу 1109A7h пытался выполнить task return вместо возврата по кадру и получал #NP. Позднейшие рекурсивные ошибки обработчика повреждали стек и таблицы страниц. Проверка native BIOS-буферов обнаружила отдельный обход paging, но его связь с этим сбоем не подтверждена; DOS ke2043 использует промежуточный DiskTransferBuffer для высоких целевых адресов и гостевое копирование.

Архитектурный контракт подтверждён первичным [Intel SDM Vol. 3A](https://www.intel.cn/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.pdf), §6.12.1.2 (PDF стр. 200): вход через interrupt/trap gate сохраняет исходные FLAGS на стеке, затем очищает live TF, VM, RF и NT. IF очищается только для interrupt gate. IRET восстанавливает сохранённые флаги. Для task gate действует отдельный контракт §7.3/§7.4 (PDF стр. 248/250): вызов задачи прерыванием устанавливает NT новой задачи для последующего возврата через backlink.

Проверены `internal/cpu/stubs.go` и `internal/cpu/interrupt_nt_regression_test.go`. Очистка NT добавлена после записи oldEFLAGS в кадр; task gate раньше возвращает результат loadTaskGate и под новую очистку не попадает. Минимальный постоянный тест проверяет оба 32-битных шлюза, сохранение гостевого NT в кадре, live NT=0, IRETD обратно в VM86 с восстановлением FLAGS, стека и всех сегментов.

Независимый overlay `/tmp/ai386-cpu-review/review_nt_test.go` проверяет четыре типа шлюзов (interrupt/trap, 16/32 бита), CPL0→0 и CPL3→0: saved NT сохраняется, live NT/TF/RF очищены, IF зависит от типа шлюза, DF/CF не изменяются. Все восемь сочетаний PASS. Существующие тесты IDT task gate и сохранения полного TSS также PASS и подтверждают NT=1 после task switch.

```sh
GOCACHE=/tmp/ai386-bios-review/go-cache go test -overlay=/tmp/ai386-cpu-review/nt-overlay.json ./internal/cpu -run 'TestReviewNT|TestV86InterruptGateClearsNT|TestIDTTaskGateStartsNestedTask|TestTaskGateSavesAndLoadsComplete32BitTSSState' -count=1 -v
```

Подтверждённых замечаний к исправлению NT нет. Рабочие исходники review не менял; фактическую повторную загрузку default JEMMEX проверяет root.
