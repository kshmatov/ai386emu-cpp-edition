# CPU/FPU: самостоятельная проверка 2026-10-14

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

Проверяется рабочее дерево на базе `b929c0b9b053788097f61bc0b83acbb79b0094d4`
с предшествующими незакоммиченными изменениями. Артефакты:
`/tmp/ai386-cpu-verification-20261014`. Исходные HD и сохранённые эталоны
не перезаписываются. Полная матрица M/X **не принята**: успешный корпус
и гостевая программа подтверждают только свои перечисленные сценарии.

## План и границы доказательств

1. Повторить сохранённые conformance/reference проверки и негативные тесты
   их формата; отдельно исполнить machine routing NE=0 IRQ13 / NE=1 #MF.
2. На доступном native x87 и QEMU получить новые наблюдения в `/tmp`;
   проверить их существующим исполнителем эмулятора без изменения масок,
   допусков, исходных fixtures или списка случаев.
3. Собрать обе ABI гостевого `fpu_probe.S`, выполнить строгий `runfpu.sh`
   в установленных DSL/FreeBSD на независимых копиях HD. Подтвердить
   STATUS:0, OK, DONE, checksum, штатную остановку; после независимой
   загрузки прочитать прежний отчёт без повторного запуска программы.
4. Сопоставить фактические тесты с отложенными M/X. Любая найденная ошибка
   реализации требует отдельной регрессии и узкого согласованного design.

[Отложенные задачи](../../openspec/changes/archive/2026-09-16-cpu-functional-completion/deferred-next-release.md),
[аудит](../../openspec/changes/archive/2026-09-16-cpu-functional-completion/coverage-audit-progress.md),
[integer M](../../openspec/changes/archive/2026-09-16-cpu-functional-completion/integer-inventory.md),
[x87 X](../../openspec/changes/archive/2026-09-16-cpu-functional-completion/x87-inventory.md)
прочитаны и сопоставлены с текущими файлами. Все 170 test-файлов,
упомянутых integer-реестром, и 57 файлов x87-реестра существуют; наличие
файла само по себе не доказывает полноту сценариев.

## Выполненные автоматические проверки

Во всех Go-командах `GOCACHE=/tmp/ai386-status-go-cache`.

| Проверка | Результат и предел |
| --- | --- |
| `go test ./internal/cpu -run 'TestConformanceSingleStepTests\|TestFPUReference\|TestX87PreexceptionReference\|TestX87RoundingReference\|TestTranscendentalPlanIntegrity\|TestGuestProbeSyscallABI' -count=1 -v` | PASS, 6.687 с, `reference-tests.log`; 37 640 conformance случаев, 941 opcode file, 0 failures. Корпус 80386 не заменяет protected/DX4 M. |
| Сохранённый `TestFPUReferenceCorpus` и отрицательные проверки формата | 480 native + 480 bare-metal; точные finite FADD/FSUB/FMUL/FDIV/FST64 × PC/RC/TOP, raw80 живых регистров, CW/SW по BAFF, tag, memory guards и EFLAGS. Не покрывает численные границы всей X. |
| Сохранённые preexception/rounding | 24 576 IE/DE/ZE и 38 016 rounding/range наблюдений; определённые поля по существующим тестам. |
| Transcendental plan integrity | Проверка сохранённого корпуса планирования; сама по себе не исполнение всех команд эмулятором. |
| GuestProbeSyscallABI | Оба ABI, FPU успех, ABI status 0/7 и намеренный FPU failure; это stub-регрессия, не гостевая ОС. |
| `python3 -m unittest discover -s tools/fpu-reference -p 'test_*.py'` | 10 tests PASS, `python-tests.log`. |
| `go test ./internal/machine -run 'FPU\|Legacy' -count=1 -v` | PASS, `machine-fpu-tests.log`: masked PIC IRQ13/retrigger при NE=0 и native #MF при NE=1. |
| `python3 tools/fpu-reference/exception_probe.py` | PASS, `exception-probe.log`: настоящий QEMU `-cpu 486`, #NM/#MF/WAIT. |

## Новые аппаратные и QEMU наблюдения

Доступны GCC 13.3.0, GNU binutils, Python3, native x87 на Intel Core
 i5-10400F, Linux x86_64 WSL2; системный QEMU 8.2.2 с TCG `-cpu 486`.
Современный Core и QEMU не являются физическим IntelDX4.

```sh
python3 tools/fpu-reference/generate.py --backend native \
  --output /tmp/ai386-cpu-verification-20261014/native.json
python3 tools/fpu-reference/generate.py --backend bare-metal \
  --output /tmp/ai386-cpu-verification-20261014/bare-metal.json
```

Каждая команда создала 480 записей и полный `.capture` с END. Проверены
происхождение, hash capture, точный состав и результаты всех 960 записей
существующими `readReference` и `checkReference`. Временный Go-тест
`external_reference_test.go` подключён через `-overlay=.../overlay.json`;
файлы репозитория/fixtures не подменялись. Команда:

```sh
AI386_VERIFICATION_REFERENCES=/tmp/ai386-cpu-verification-20261014 \
GOCACHE=/tmp/ai386-status-go-cache go test \
  -overlay=/tmp/ai386-cpu-verification-20261014/overlay.json \
  ./internal/cpu -run '^TestVerificationExternalReferences$' -count=1 -v
```

PASS, 0.128 с, `fresh-reference-tests.log`. FIP/FDP адресов разных
процессов и undefined поля не сравниваются согласно исходному контракту;
точные результаты и маска BAFF не ослаблены.

Генераторы `preexceptions.py` и `rounding.py` запущены абсолютным путём
из рабочего каталога артефактов, где заранее создан
`internal/cpu/testdata/fpu-reference`. Поэтому их относительные output-пути
сохранили новые данные исключительно в `/tmp`. Оба свежих текстовых
корпуса побайтово равны сохранённым в репозитории:

| Корпус | Записей | SHA256 |
| --- | ---: | --- |
| preexceptions | 24 576 | `3a0b76091115cb680fe25da31c7cb88217949327b2000d47334fbf965ef96106` |
| rounding | 38 016 | `d85407e8b069dbd6a24e127438f7424e877c6c321d9ce82685af06ae90cbb9d9` |

Свидетельства: `fresh-reference-comparison.log`, новые JSON provenance,
`preexceptions-generation.log`, `rounding-generation.log`. Повтор совпадения
подтверждает воспроизводимость этих конкретных корпусов на текущем хосте.

## Что остаётся в M/X

| Область | Существующие доказательства | Незакрытый критерий |
| --- | --- | --- |
| Integer ALU, shift/rotate, bit operations | `arithmetic_form_matrix`, `alu_mode_matrix`, `alu_page_fault`, `rotate_carry_matrix` и отдельные fault tests | Все формы × ModRM/SIB/segment × CPL1/2 × R/W/U/WP/A/D × fetch/operand/stack fault и restart не сведены в полную M. |
| MOV/stack/control transfers/строки | Развёрнутые decoded матрицы, ENTER depths, REP0, CALL/RET priority, XCHG SIB | Нельзя переносить доказательство полного SIB XCHG на другие memory-команды; остаются их адресные и fault combinations. |
| Системные selector/table/control/debug/cache команды | LAR/LSL CPL0/3/GDT/LDT, LLDT/LTR/table matrices, MOV CR/DR mod-ignore, MOV TR, INVLPG | CPL1/2 и полные intersections; совместный GD/CPL3 аппаратный приоритет не установлен; полный cache/TLB test-register state space. |
| FILD | `fpu_fild_completion_test.go`: signed widths, PC/RC/TOP, decoded modes, faults/restart; pending проверка пока direct execFPU/real IVT | Зафиксированный остаток: реальная доставка pending #MF/#NM через IDT в protected/VM86. |
| FIST/FISTP и FBLD/FBSTP | raw80/PC/RC/TOP/маски, decoded modes, FIST pending IDT, BCD page/expand-down/sticky matrices | Multi-fault комбинации вне проверенных цепочек; #AC для m80 BCD требует именно архитектурного основания/эталона поколения. |
| FST/FSTP real32/64 | Narrowing/range/classes/sign/PC/RC, sticky и смешанные PTE, expand-down | Эти проверки не доказывают FLD, real80 и register-store варианты и полные оси X. |
| FPREM/FPREM1 | Exact quotient матрица и один iterative сценарий с большим exponent gap в `fpu_remainder_test.go` | Полная матрица длинных partial-итераций отсутствует; нельзя писать, что итерации вообще не тестируются. |
| FNSTSW/FSTSW и context/control | FNSTSW AX decoded state, #NM; отдельные memory/fault и waiting границы | AX-матрица не покрывает memory/waiting формы; все layout/pointer/mode/fault комбинации context остаются X. |
| FCHS/FABS, FXCH, FFREE, TOP rotate, FNOP | Широкие decoded raw80/TOP/PC/RC/tag/pending срезы | Проверенные срезы не дают автоматического закрытия fetch/prefix/sticky/legacy комбинаций всей X; 66/67 register-only регрессии не засчитываются как допустимые формы. |
| Арифметика, сравнения, constants, transcendental | Native корпусы плюс независимые точные случаи и scoped реализации | Полная X каждой формы по всем классам/маскам/направлениям/памяти/приоритетам остаётся открытой. |

Reserved PC=01, undefined FBLD digits, недокументированные aliases,
reserved размерные префиксы register-only x87 не превращены в обязательный
архитектурный oracle. SMM/RSM остаётся согласованным исключением объёма.
Итоговая синхронизация базового CPU-контракта (5.4c) зависит от M/X;
roadmap 2.2/2.8/2.5 и 5.4 этим отчётом не закрываются.

## Дополнительная регрессия FILD и исправленный RF

До production изменений согласован test-only дизайн: FILD i16/i32/i64 ×
protected/VM86 × A16/A32 × O16/O32 × DS/ES × TOP0/7 × pending #MF /
TS-before-MF / EM-before-MF — 288 decoded случаев. Операнд пересекает
segment limit; настоящий IDT handler и кадр доказывают приоритет #NM/#MF
перед памятью, сохранность FPU/CR2 и корректный fault IP/CS/EFLAGS.
`fpu_fild_pending_idt_test.go` выявил отсутствие RF во всех fault frames.

Основание исправления — именно [Intel486 Programmer’s Reference
240486-001](https://www.ardent-tool.com/CPU/docs/Intel/486/manuals/240486-001.pdf),
§9.3.3 printed9-4/PDF220: CPU устанавливает RF в saved EFLAGS при faults;
Table9-7/PDF241 различает #NM/#MF и abort #DF. Native x87 SIGFPE дал
EFLAGS 10246/RF 1. QEMU 8.2.2 TCG 486 дал RF 0 даже для контрольного #GP:
NM frame `[00100276,8,2]`, MF `[00100266,8,6]`, GP
`[fffc,0010029a,8,2]`. Проверены offsets после pushal40/44 и error selector.
Это ограничение QEMU oracle, не основание ослабить Intel486 контракт.
Артефакты: `rf-native.c/log`, `rf-probe/result-gp-control-verified.log`.

После согласования создан и применён [scoped change
cpu-fault-resume-flag](../../openspec/changes/cpu/design.md).
Исправлены сохранение RF при faults, его время жизни после IRET/task
switch/POPF и сохранение fault IP/RF в старом TSS при task gate.
Intel486 POPF instruction entry PDF724/printed26-236 исключает RF/VM из
изменяемых флагов; Embedded DX4 §4.1 исключает POPF/IRET/task switch из
автоматического RF clearing. Поэтому POPF сохраняет прежний RF, IRET и
32-bit TSS восстанавливают RF нового контекста, обычная успешно выполненная
инструкция очищает RF.

Task gate теперь передаёт error code на новый стек; сохраняет CS.D/SS.B
из descriptor и не проверяет DPL целевого TSS как direct CALL при IDT
fault. Target TSS RF не заменяется RF старой задачи. Все изменения имели
сначала RED на прежнем коде, включая MOV EAX,imm32, декодированный как
MOV AX,imm16, и отказ CPL3 fault войти в корректный DPL0 task handler.
Контроль direct CPL3→TSS по-прежнему требует #GP.

| Регрессия | Проверяемый результат |
| --- | --- |
| FILD 288 | Фактический handler/frame, fault priority, FPU/CR2 без изменений |
| Event-class 72 | P/V × RF 0/1: faults, debug trap, software INT7/13, IRQ7, NMI, direct/nested DF |
| IRETD retry 2 | #DB→IRETD→MOV без повторного breakpoint, затем breakpoint снова активен |
| Task switch 16 | CALL/JMP/INT/IRET × old/target RF 0/1 |
| POPF 48 | R/P16/P32/CPL3/V × O16/32 × old/image RF 0/1 |
| Task-gate events 56 | NM/GP/PF/DF/debug trap/INT7/IRQ7 × old/target RF 0/1 × SS.B0/1 |
| Task-gate retry/CS.D/DPL | Реальный NM→IRET→FLD1; 32-bit MOV; IDT-vs-direct privilege |
| Gate16 controls 8 | Word FLAGS не содержит высокий RF, живой RF handler очищен |

Логи RED/GREEN и команды: [verification.md](../../openspec/changes/cpu/verification.md).
Старые assertions изменены только на архитектурный RF, прочие флаги,
IP/CS и результаты остались обязательными. Этот срез не закрывает M/X.

## Transcendental повтор

Свежий `transcendental-plan.json`: 960 независимых Decimal targets и
11520 native CW/SW/tag/live raw80 snapshots совпадают с сохранёнными
данными; comparison JSON одинаков, превышений 4.5 ULP нет. FIP/FDP процесса
и payload пустых регистров не сравниваются. `transcendental-generation.log`
и `transcendental-comparison.log` сохраняют доказательства. Отдельное
исполнение эмулятора `go test ./internal/cpu -run
'TestX87(Trig|FSINCOS|FPTAN|FYL2X)|TestF2XM1|TestFPATAN' -count=1 -v`
PASS 1.030 с (`transcendental-tests.log`).

## Гостевые проверки и сохранность

`prepare.py` собирает оба ABI `fpu_probe.S` через as32/ld elf_i386;
FreeBSD использует --defsym FREEBSD=1 и ELF OSABI FreeBSD. Read-only
shared CD содержит ELF и неизменённый runfpu.sh. Рабочие HD созданы через
cp --reflink=auto --sparse=always, cmp подтверждает равенство базе перед
запуском. Сессии используют run-freebsd-session.sh, отдельные locks,
сохранённые PS/2 JSONL и явный quit только после строгого guest halt.
Новый EOF не считается успехом. FPU/checksum markers требуют начала строки,
поэтому ввод команды не может засчитаться как её результат.

Первая сборка machine.test SHA256
`96db0d30ef54a15a55293075d6ba4f15fcf2306683ff756fff8fad26b7546fb6`
совпадает с принятой post-BDA OS regression, предшествует RF исправлению.

| Сессия этой сборки | Результат |
| --- | --- |
| FreeBSD first | PASS 272.75 с, wrapper 0 / 273 с; STATUS:0/OK/DONE, checksum 3774605809 13, syncing disks/done/halt; freebsd-first.log |
| FreeBSD reboot без CD/повторного FPU | PASS 294.78 с, wrapper 0 / 295 с; прежний отчёт OK и checksum, штатный halt; freebsd-reboot.log |
| DSL first | PASS 1210.39 с, wrapper 0 / 1211 с; STATUS:0/OK/DONE и checksum; /dev/hda1 umounted, DSL halted; dsl-first.log |
| DSL reboot без CD/повторного FPU | PASS 1284.05 с, wrapper 0 / 1284 с; прежний OK/checksum, /dev/hda1 umounted и halt; dsl-reboot.log |

Скриншоты первых сессий и обоих независимых reboot просмотрены: содержат реальные
отчёты/checksum/halt. RAM 64 MiB, HD-only installed boot; FreeBSD steps 2B,
DSL boot 500M / extra 4B, timeout 2h. Полные CLI записаны в начале каждого log.
Промежуточные machine-rf.test сессии не являются итоговой приёмкой всей
RF lifetime/task-gate правки: FreeBSD PASS 343.78 с, DSL PASS 1287.24 с.
Итоговая изменённая сборка и её guest результаты записаны ниже.

Итоговые CPU проверки: весь пакет PASS 50.351 с
(`cpu-final-reviewed.log`), весь пакет с `-race` PASS 206.831 с
(`cpu-race-final-reviewed.log`); machine FPU routing, vet CPU/machine,
strict OpenSpec validate и diff-check PASS. Независимый root review после
CS.D/DPL исправлений: нет high/blocking findings.

Итоговая сборка machine-final.test SHA256
`4fc90f17702ca47fa87329cdbc626b03e7d4922dfe527b5b457f2e7553600bf7`
включает все согласованные CPU, audio и network production fixes.
Обе FPU сессии запущены на новых dsl-final.img/freebsd-final.img;
логи dsl-final.log/freebsd-final.log, команды/JSONL рядом.

Исходные HD проверены повторно: bases-before.sha256 и bases-after.sha256
совпадают побайтово. DSL SHA256
`b629394156e45d59ecd1fc30442bb4282743d45a8140d2321bee5fc213c9f4af`,
FreeBSD SHA256
`6aa54f7333f12c5998f6baf68ff1d0c1313cd1096a95b4c90988013fd3438831`.
Оригинальные images не использовались для записи гостями.

Итоговый FreeBSD **PASS 289.05 с**, wrapper status 0 / 290 с:
`freebsd-final.log` содержит STATUS:0, OK, DONE, checksum
`3774605809 13 /root/ai386-fpu-report.txt`, syncing disks / done и halt.
Просмотрены `freebsd-final-fpu.png`, `freebsd-final-checksum.png`,
`freebsd-final-halt.png`; результат подтверждён визуально.

Итоговый DSL **PASS 1154.19 с**, wrapper status 0 / 1154 с:
`dsl-final.log` содержит STATUS:0, OK, DONE, checksum
`3774605809 13 /root/ai386-fpu-report.txt`, `/dev/scd0 umounted`,
`/dev/hda1 umounted`, `DSL halted.` и explicit quit.
Просмотрены `dsl-final-fpu.png`, `dsl-final-checksum.png`,
`dsl-final-halt.png`; результат подтверждён визуально.

Обе ОС прошли FPU/HD/halt на одной итоговой сборке с исправлениями CPU,
аудио и сети. Независимый reboot с чтением прежнего отчёта доказан отдельно
на исходной post-BDA сборке обеих ОС. Узкий RF/task-gate change принят;
полная M/X и зависимые от неё roadmap-пункты этим отчётом не закрываются.
