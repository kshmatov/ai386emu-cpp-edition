# Проверка RF, 2026-10-14

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

Intel486 Programmer’s Reference Manual 240486-001, §9.3.3 (printed9-4,
PDFpage220) явно предписывает автоматический RF в saved EFLAGS для faults.
Table9-7 (PDFpage241) классифицирует #NM/#MF как faults и #DF как abort.
[Первичный PDF](https://www.ardent-tool.com/CPU/docs/Intel/486/manuals/240486-001.pdf).
Native Core x87 #MF дал saved RF=1. QEMU8.2.2 TCG486 дал RF=0 также для
контрольного #GP, при проверенных raw stack offsets; это ограничение QEMU
oracle, а не основание изменить Intel486 контракт.

Артефакты в `/tmp/ai386-cpu-verification-20261014`:

- `fild-idt-tests.log` / `rf-red.log`: до исправления FILD288 и RF0 fault
  variants event-class матрицы падают только на saved RF; вектор, handler,
  EIP/CS и неизменность FPU/CR2 проверены до failed assertion.
- `rf-green.log`: после добавления explicit fault classification все
  288 FILD и 72 event-class cases PASS, пакет0.067с. Включены P/V,
  RF0/1, #DE/#BR/#UD/#NM/#TS/#NP/#SS/#GP/#PF/#MF/#AC, debug trap,
  software INT7/INT13, IRQ7, NMI, direct/nested #DF.
- Начальный production fix использовал `exception.go`/`setRF` для
  сохранённого кадра; связанный RF lifetime и task gate описан ниже. Живой RF handler=0, traps/interrupts/abort не
  получают принудительную установку, nested #DF сбрасывает pending
  metadata fault. Общая M/X остаётся открытой.

Время фиксируется Go/guest runner; точный токен-счётчик недоступен.
Общие CPU/race/vet и guest после исправления дописываются по завершении.

## Восстановление RF и task gate

Intel486 POPF instruction entry PDF724/printed26-236 исключает RF/VM из
изменяемых флагов; Embedded DX4 §4.1 исключает POPF/IRET/task switch из
автоматического RF clearing. `intel486-popf-iret.txt` содержит извлечение.
`rf-lifetime-red.log`: реальный #DB→IRETD терял восстановленный RF;
CALL/JMP task switch и POPF также теряли бит. Регрессии проверяют два
privilege paths IRETD, 16 task-switch combinations CALL/JMP/INT/IRET и
48 POPF combinations R/P16/P32/CPL3/V × O16/32 × old/image RF0/1.
`rf-lifetime-green.log`: PASS0.222с вместе с прежними flags/ALU/V86 tests.

`rf-taskgate-red.log`: #NM task gate сохранял EIP=2 вместо0; GP/PF/DF
не помещали error code на новый стек. `rf-taskgate-stack-red.log`: SS32
с ESP=0x10800 адресовался как SS16, потому что SS.B терялся в cache.
`rf-taskgate-green.log`: PASS0.038с FILD/RF/Task; 56 event-class/TSS RF/
SS.B combinations, decoded #NM task IRET retry и8 gate16 controls.
Target TSS RF проверяется отдельно от saved old RF; обычные INT/IRQ,
debug trap и abort не получают принудительный saved RF. No blanket M/X.

Независимый root review: `rf-taskgate-review-red.log` воспроизводит потерю
CS.D (MOV EAX получил 0x5678 вместо 0x12345678) и ошибочный target TSS DPL
check для CPL3 #NM через IDT. Минимальные исправления descriptor cache и
условия direct access дали `rf-taskgate-review-green.log`, PASS 0.063 с.
Полный CPU после этих правок PASS 50.351 с (`cpu-final-reviewed.log`).
Предыдущий полный проход обнаружил только устаревшие RF assertions в
VM86 POPF и LAR/LSL IRETD; исправлен исключительно RF, остальные проверки
сохранены. `vet-final-reviewed.log`, strict validate и diff-check PASS.

Финальный race всего CPU PASS 206.831 с (`cpu-race-final-reviewed.log`).
Machine FPU routing PASS 0.006 с; vet CPU/machine, strict OpenSpec и
`git diff --check` PASS. Root final production review: PASS, нет
high/blocking findings. Гостевая итоговая сборка SHA256
4fc90f17702ca47fa87329cdbc626b03e7d4922dfe527b5b457f2e7553600bf7.

После review усилен только test: task-gate error slot заполняется sentinel
0xbeefdead; проверяется low error code и неизменность reserved high word
0xbeef. Production и machine-final.test не менялись. Фокусный rerun PASS
(`rf-taskgate-reserved-word.log`).

Итоговые guest runs на machine-final.test:
- FreeBSD PASS 289.05 с, wrapper 0 / 290 с, freebsd-final.log.
- DSL PASS 1154.19 с, wrapper 0 / 1154 с, dsl-final.log.

Оба: реальный FPU STATUS:0 / OK / DONE, checksum 3774605809 13,
штатные flush/unmount/halt и explicit quit; все FPU/checksum/halt PNG
просмотрены. Исходные HD SHA256 до/после идентичны. Независимые reboot
без CD и без повторного FPU подтвердили сохранность отчётов обеих ОС
на исходной post-BDA сборке, отдельно от приёмки итогового бинарника.
Подробности и пределы доказательств: docs/project-verification/cpu.md.
