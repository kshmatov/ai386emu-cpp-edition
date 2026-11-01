# Матрица аппаратного переключения задач

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

## Граница и источник

Матрица описывает защищённый режим i486DX4. Опора — Intel SDM, том 3A,
«Task Management», §§8.3–8.6 (в частности, таблица 8-2); для особенностей
Intel486 при 16-битном TSS — том 3B, §24.28. Состояние текущей задачи
сохраняется только после успешной проверки нового TSS и требуемых
дескрипторов. Ошибка до точки переключения не меняет TR, регистры, busy,
backlink либо TSS текущей задачи.

## Общие стадии

| Стадия | Проверка / действие | Ошибка при отказе |
|---|---|---|
| 1 | Получить selector TSS напрямую, из task gate либо из backlink при IRET/NT. | #TS для null/некорректного backlink; #GP для некорректного явного selector/gate |
| 2 | Проверить привилегии явно адресуемого TSS или task gate для CALL/JMP и software INT. Аппаратный IRQ, exception и IRET DPL не проверяют. | #GP(selector) |
| 3 | Проверить system descriptor: GDT, TSS 16/32 нужного типа, present и минимальный limit (16-bit: 0x2B, 32-bit: 0x67). | #TS(selector) / #NP(selector) |
| 4 | Проверить available TSS для CALL/JMP/INT/exception либо busy TSS для IRET. | #GP(selector) |
| 5 | Проверить доступность current/new TSS и всех загружаемых descriptor/segment state до принятия состояния. | Архитектурный #TS/#NP/#SS/#GP либо #PF; состояние до точки переключения сохраняется |
| 6 | Сохранить old state; затем изменить busy/NT/backlink согласно причине, установить CR0.TS, загрузить TR и новый state. | Ошибки после точки переключения проверяются отдельными staged-тестами |

## Причины перехода и изменяемое состояние

| Причина | Новый TSS до перехода | busy old/new | backlink и NT | Обязательное состояние |
|---|---|---|---|---|
| JMP через TSS/task gate | available | old очищается, new устанавливается | backlink без изменений; NT берётся из нового EFLAGS | сохранить old, загрузить new; CR0.TS=1 |
| CALL через TSS/task gate | available | old остаётся busy, new устанавливается | записать old TR в backlink new; NT=1 | сохранить old, загрузить new; CR0.TS=1 |
| INT через IDT task gate / exception | available | old остаётся busy, new устанавливается | записать old TR в backlink new; NT=1 | сохранить old, загрузить new; CR0.TS=1 |
| IRET при EFLAGS.NT=1 | busy | old очищается, new остаётся busy | backlink new не меняется; NT old очищается в сохранённом образе, NT new берётся из TSS | сохранить old, загрузить previous task; CR0.TS=1 |

`CALL` означает far CALL; `INT` включает только IDT task gate. Прямой selector
TSS допустим для far CALL/JMP, но не для IDT. При `IRET` без NT обычный путь
возврата не является task switch.

## Образы TSS

| Тип | Сохранить и загрузить | Не входящее в образ |
|---|---|---|
| 32-bit TSS | CR3, EIP, EFLAGS, EAX…EDI, ES/CS/SS/DS/FS/GS, LDTR, backlink; TR cache из descriptor | CR0 кроме установки TS; debug state (проверяется отдельной задачей) |
| 16-bit TSS | IP, FLAGS, AX…DI, ES/CS/SS/DS, LDTR, backlink | CR3, FS/GS и старшие половины GPR/EFLAGS не восстанавливаются |

Селекторы в 32-bit TSS сохраняются 16-битными значениями; верхние 16 бит
ячеек остаются неопределёнными и тесты их не сравнивают.

## Таблица тестов

| ID | Сценарий | Проверка |
|---|---|---|
| TS-01 | far JMP на available 32-bit TSS | old busy очистился, new busy установлен, old state сохранён, CR3/TR/segments/GPR/EIP загружены, TS установлен |
| TS-02 | far CALL на available 32-bit TSS | old busy сохранён, backlink=new содержит old TR, NT=1, new busy установлен |
| TS-03 | software INT через IDT task gate | DPL task gate проверен; при успехе путь эквивалентен nested CALL |
| TS-04 | hardware IRQ и exception через IDT task gate | DPL task gate обходится; backlink/NT/busy следуют nested transition |
| TS-05 | IRET при NT=1 | selector backlink обязан указывать busy TSS; old busy очищен, previous task восстановлен |
| TS-06 | 16-bit TSS через CALL/JMP/IRET | 16-битные поля и ограничение 0x2B; старшие части состояния не считаются восстановленными |
| TS-07 | null, LDT, code/data, unavailable/busy TSS, короткий limit | корректные #TS/#GP без изменения состояния до точки переключения |
| TS-08 | not-present task gate/TSS и невалидные сегменты нового TSS | #NP/#TS/#SS/#GP на требуемой стадии; проверка rollback до точки переключения |
| TS-09 | TSS/descriptor на отсутствующей странице | #PF и CR2 соответствуют первому доступу; инструкция воспроизводимо повторяется после отображения страницы |

Реализация тестов начинается с `internal/cpu/task_switch_test.go`; каждое
правило выше получает отдельный testify subtest. Следующие подзадачи 4.5b–e
закрывают строки TS-01…TS-09 по порядку.
