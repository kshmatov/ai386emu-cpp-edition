# Design: CPU

Объединение design-решений scoped changes `cpu-fault-resume-flag` и
`tb-correctness-device-clock`. Контекст и мотивация — proposal.md.

## 1. RF в fault frame (cpu-fault-resume-flag)

### Context

При доставке #NM/#MF и других fault-классов эмулятор сохранял RF=0 в кадре
EFLAGS. Intel486 требует RF=1 в saved EFLAGS при fault, чтобы повтор
faulting instruction после IRET не создавал повторный instruction breakpoint.

### Decisions

- **Fault-class** (кроме #DF abort) сохраняют RF=1 в EFLAGS кадра
  protected/VM86 mode.
- **Traps, IRQ/NMI и abort #DF** сохраняют исходный RF без принудительной
  установки (RF устанавливается только для fault).
- **IRET** восстанавливает RF из кадра; **task switch** — из TSS; **POPF**
  сохраняет прежний RF (POPF не выставляет RF, если только не через
  POPF в VM86 с определёнными условиями — зафиксировано в specs/cpu).
- **Fault через task gate**: RF/IP сохраняются в старом TSS, error code
  уходит на стек новой задачи; RF целевого TSS не изменяется.
- Регрессии: decoded FILD с pending #MF/#NM, таблицы fault/trap/interrupt/abort,
  приоритет fault перед памятью, IRET/task switch/POPF RF lifetime.

### Risks / Trade-offs

- [Расхождение классификации события] → таблица event class в тестах;
  сверка с Intel486 SDM (fault vs trap vs interrupt).
- [Старые регрессии ожидали RF=0] → обновлены ожидаемые RF вместе с фиксом,
  остальные биты EFLAGS сохранены.

## 2. TB-идентификация, phantom-execution, device-clock, JIT (tb-correctness-device-clock)

### Корень: TB по логическому EIP

TB-кэш (`tbCache[512]`) индексируется по физадресу первого байта блока, но
`stepTB`/`tbGo` сравнивали только логический EIP (`op.eip == c.eip`). После
far jmp / far ret / IRET то же значение EIP в другом CS указывает на другой
код. Диагноз (бисект 2026-10-17): `TestFreeDOSInstallMenu` печатал
«Invalid Opcode» (stored `PUSH 0x50` при байтах `MOV SI,SS`, CS=0xFFFF).

**Решение:** `tbOp.phys` — физический адрес op; `stepTB` сверяет
`tbPhys(op.eip)==op.phys` на входе в блок; `tbGo` — для chain-link и lookup.

### Phantom-execution

`stepTB`/`fillTB`/`tbValidate` на отказе возвращали `canUseTB()` (true) —
слот инкрементился без исполнения. Контракт: любой отказ TB → `false` →
fallback `Step()` / refill.

### Device-clock floor

`RunBatch` считал `cycles = max(delta, 1)` на **батч** — PIT ехал в ~32 раза
медленнее, ISOLINUX зависал. Fix: `max(delta, max(executed, 1))` — каждая
инструкция даёт ≥1 цикл (контракт `Step()`).

### JIT INC/DEC CF

`emitSetFlags` затирал CF гостя host-флагом. INC/DEC на x86 не меняют CF.
Fix: `emitSetFlagsKeepCF` (маска `0x8D4`).

### JIT по умолчанию выключен

`jitCanNative` требует `opsize32`; boot-куски ISOLINUX давали расхождения.
`disableJIT=true` в `cpu.New()`; `CPUReset` сохраняет флаг явно.

### Batch vs Step

| Конфигурация | wall | cycles | Welcome |
|---|---:|---:|---|
| `RunBatch(32)` | 38.1 s | 708M | reached |
| `Step()` | 35.0 s | 708M | reached |

Batch не выигрывает → `machine.run()` → `Step()`.

## Non-goals

Доработка JIT-эмиссии до включения по умолчанию. Lazy S/Z/P. Потактовая
точность CPU. M/X-матрица FPU (открыта в roadmap 2.8).

## Источники

- `cpu-fault-resume-flag` (design восстановлен частично; контракт — specs/cpu).
- `tb-correctness-device-clock` (design сохранён полностью).
