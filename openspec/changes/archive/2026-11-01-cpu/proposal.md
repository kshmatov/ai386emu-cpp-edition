# Proposal: CPU — RF в fault frame, TB-идентификация и device-clock

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

## Why

Объединение работ по системе `cpu`. Две независимые линии:

1. **RF в fault frame** (`cpu-fault-resume-flag`): при доставке #NM/#MF и других
   fault-классов эмулятор сохранял RF=0 в кадре EFLAGS. Intel486 требует RF=1,
   чтобы повтор faulting instruction после обработчика не создавал повторный
   instruction breakpoint.
2. **TB/JIT/device-clock** (`tb-correctness-device-clock`): три дефекта ломали
   гостевую загрузку FreeDOS/ISOLINUX — TB переиспользовал блоки после смены CS,
   отказы TB помечали неисполненные слоты как исполненные (phantom-execution),
   `RunBatch` занижал device-clock. JIT INC/DEC затирали CF гостя.

## What Changes

- Fault-class исключения сохраняют RF=1 в EFLAGS кадра protected/VM86;
  traps, IRQ/NMI и abort #DF — исходный RF; IRET/task switch/POPF —
  согласно контракту RF; fault через task gate — RF/IP в старом TSS.
- TB-записи (`tbOp`) хранят физический адрес op; `stepTB`/`tbGo` сверяют
  `tbPhys(eip)==op.phys`. Любые отказы TB → fallback `Step()`/refill,
  без phantom-слотов.
- `RunBatch` считает циклы как `max(delta, executed)` — каждая инструкция
  даёт ≥1 такт device-clock. `machine.run()` использует `Step()`.
- JIT INC/DEC используют `emitSetFlagsKeepCF`; `disableJIT=true` по
  умолчанию и переживает `CPUReset`.

## Capabilities

### New Capabilities

Нет.

### Modified Capabilities

- `cpu`: RF в сохранённых EFLAGS при исключениях; TB-идентификация по
  физадресу; fallback без phantom-слотов; JIT-флаги INC/DEC и отключение.
- `machine-core`: device-clock floor в `RunBatch`; `machine.run()` через `Step()`.

## Impact

`internal/cpu` (exception delivery, tb.go, icache.go, jit.go, runstep.go),
`internal/machine` (RunBatch), регрессии FILD/#MF, TB CS-phys, FreeDOS install.

## Источники

Объединены scoped changes: `cpu-fault-resume-flag`, `tb-correctness-device-clock`.
