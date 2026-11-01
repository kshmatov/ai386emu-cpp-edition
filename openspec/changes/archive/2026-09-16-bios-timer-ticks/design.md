## Context
Дизассемблирование загрузчика FreeBSD: A860 и C120 читают физический 46Ch в цикле ожидания клавиши. PIT уже формирует IRQ, но BIOS их маскирует и не имеет обработчика.

## Goals / Non-Goals
Подключить обычную цепочку PIT → PIC → IVT → BIOS. Не обновлять память из machine.Step в обход гостевого обработчика.

## Decisions
Нативный INT08 обновляет счётчик; ROM-вход вызывает его и возвращается через IRET. Настройка PIT выполняется machine только при успешной BIOS-загрузке. Обработчик не меняет регистры CPU. Полночь следует IBM PC AT Technical Reference, раздел BIOS timer interrupt: https://bitsavers.trailing-edge.com/pdf/ibm/pc/at/1502494_PC_AT_Technical_Reference_Mar84.pdf.

## Risks / Trade-offs
IRQ0 может выявить ошибки доставки v86; повторить FreeBSD и Linux. Гость может заменить IVT/PIC/PIT; обновление BDA не должно обходить его настройки.
