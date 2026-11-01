# Остаток CPU после гостевой приёмки FreeBSD

2026-09-11. Успешная сессия FreeBSD и37640 conformance случаев не доказывают полноту i486. Это рабочая сверка к задачам2.2/2.7/2.8, не новые отметки выполнения.

Подтверждённые пробелы x87 по basicFPU: FYL2X, FPTAN, FYL2XP1, FSINCOS, FSIN, FCOS пока декодируются как неподдержанные. Уже добавлены окружение, packed BCD, partial remainders, FXTRACT, F2XM1 и FPATAN.

Обработка исключений требует полной матрицы: native #MF и основные ZE/IE/DE сценарии отдельных команд есть; FERR/IRQ13 при NE=0, overflow/underflow extended range, precision и rollback для всех арифметических/памятных форм ещё не приняты. Общая модель NaN payload не завершена; FXTRACT отдельно сохраняет/quieting raw NaN.

MOV sreg operand/descriptor/accessed PF, null selectors0–3, порядок type/privilege/presence и классы #GP/#SS/#NP исправлены и проверены в cpu-segment-load-faults. POP sreg и LDS/LES/LSS/LFS/LGS rollback приняты в cpu-far-load-faults. Maskable IRQ shadow после MOV/POP SS и STI, а также прекращение CLI/STI/I/O при отказе IOPL приняты в cpu-interrupt-shadow. NMI/debug traps, TSS I/O bitmap и остальные системные инструкции ещё требуют сверки.

ICEBP/F1 явно opUnimplemented. Набор/свойства CPUID для конкретного варианта DX4 требуют сверки с моделью процессора перед заявлением полноты. Итог этапа2 остаётся открытым.

Детализация продолжения: [cpu-functional-completion](../cpu-functional-completion/tasks.md) — 29 проверяемых задач, контракт и матрица эталонов. Начальные шаги фиксируют паспорт DX4 и реестры; обнаруженные вопросы CPUID и исторического требования 4 МиБ paging разрешаются до изменения поведения. Это план продолжения, не приёмка полного этапа.
