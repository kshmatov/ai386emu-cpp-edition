## Context

FIP/FDP/FOP записываются после FPU instruction, а context операции сериализуют состояние отдельно. Полная memory translation должна завершиться до записи образа.

## Decisions

FIP — адрес начала текущей FPU инструкции, FOP — 11-bit opcode/modrm, FDP — effective data address для memory form; real mode сохраняет физический адрес согласно формату Intel. Environment 14/28 и full save 94/108 сохраняют CW/SW/tag/TOP и raw80 без float64 промежуточного формата. Page/segment fault оставляет память и FPU без изменений.

## Validation

Register/memory pointer records, all four context widths/modes, raw SNaN payload round-trip, page fault atomicity, full test/vet/race/build/strict.
