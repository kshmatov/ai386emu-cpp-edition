# Review: IRQ1 BIOS и tools/doom

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

2026-10-14. Проверен только новый IRQ1 handler, его подключение при POST,
новые keyboard regression tests и `tools/doom`. Ранее существовавшие
незакоммиченные изменения BIOS/CPU не входят в заключение. Основание:
`AGENTS.md`, design/tasks/specs изменения `doom-interrupt-compatibility`.
Производственный код в ходе review не изменялся.

## Standards

Блокирующих нарушений стандартов в IRQ1 исправлении не обнаружено.
Handler небольшой, использует существующий `setBIOSHook`, не вводит
отдельную очередь и сохраняет архитектуру PIC/PS2. Новые тесты используют
testify и проверяют поведение через реальные гостевые инструкции.

Обязательное условие финальной проверки: удалить временные
`[DEBUG-doom-irq0]`, `CPU.DoomWalkCycles`, `Machine.DoomSkipWalkCycles`,
`PIC.DoomDebugContext`, `AI386_DOOM_SKIP_WALK_TIME` и зависимые вызовы.
На момент review они ещё присутствуют; root указал их как временную
instrumentation. Это требование cleanup, а не review постоянного дизайна.
После удаления повторить сборку и tests/vet tools, чтобы исключить
оставшиеся ссылки на удалённый API. Исторические debug-логи в документах
могут остаться как свидетельства и должны отличаться от исполняемого кода.

## Spec и корректность

Блокирующих производственных дефектов IRQ1 не найдено:

- Установка ROM/IVT9 выполнена до открытия IRQ1; маска FC применяется
  только в Bootstrap при наличии клавиатуры, поэтому дальнейшая гостевая
  маска не переписывается.
- `keyboard.In(0x64)&0x21 == 1` читает 60h только при OBF=1 и AUX=0.
  Пустой буфер и mouse-byte не потребляются.
- Hook не вызывает `KeyTake`, не меняет CPU-регистры/флаги; IRET возвращает
  аппаратный frame. Native hook использует существующую проверку IVT/ROM,
  поэтому гостевой replacement и chaining сохраняются.
- Тесты подтверждают make/break, повторные IRQ после EOI, INT16 make queue,
  regs/flags/stack, guest replacement без программирования PIC, chaining и
  явно установленную маску гостя. Без клавиатуры IRQ1 остаётся закрытым.

### Замечание R1: критерий входа timer diagnostic слабее описания

**P2, validation/documentation.** `tools/doom/main.go` проверяет flat CS,
загруженную I_GetTime-подпись и timer>=2. README утверждает, что проверка
ждёт настоящий цикл, вызывающий I_GetTime. Код не связывает найденную
подпись с текущим PC/call-site и не доказывает, что в измеряемом окне
гость остаётся в игре. Это позволяет принять подготовленную mapped-копию
routine как активную; реальный ложный PASS имеющегося Doom-прогона review
не установил.

Disposition: либо описать инструмент строго как loaded-routine/timer
progression diagnostic с этим ограничением, либо дополнительно проверять
реальный игровой PC/call-site и повторно валидировать выбранную routine
в mapping перед вторым чтением. Не использовать exit0 инструмента как
приёмку игрового меню/уровня — README правильно оговаривает это отдельно.

### Замечание R2: новая relocation discovery не проверена тестами

**P2, validation.** `findTimerAddress` сканирует physical RAM, затем
подтверждает полную последовательность через текущий linear mapping и
связь code/data по offset. Это разумная защита от unmapped/copy decoys.
Существующие `main_test.go` проверяют только copyDisk и readLinear,
не discovery. Обязательное правило тестов AGENTS применимо к новой логике.

Disposition: добавить проверки обоих фактических relocation, unmapped
или overwritten decoy, несовпадения surrounding suffix/relative-address,
повторной validation после смены mapping. Принятие первого кандидата с
подписью не заменяет привязку к исполняемой игре из R1.

### Замечание R3: защита AUX/spurious не имеет регрессии

**P3, coverage.** Условие handler корректно по текущему PS2 status, но
новые BIOS IRQ1 tests не вызывают ROM handler при пустом OBF либо AUX
OBF=1. Нет дефекта кода по inspection; будущий регресс может случайно
удалить условие и украсть mouse packet.

Disposition: проверить, что spurious call не меняет пустой буфер и
мышиный байт остаётся тем же после IRQ1 hook, а последующая клавиша
по-прежнему обслуживается с EOI. Не переписывать PS2 или PIC ради теста.

## Риск ложного PASS actual-game input test

`TestDoomGuestInputCapture` не ограничивается числом INTA/нечёрным кадром:
проверяет код menuactive load, player↔mobj backpointer и исходные координаты
E1M1; после Esc требует menuactive=1, после второго Esc — 0; после Up —
изменение координат конкретного player mobj; после Ctrl — расход патронов.
Поэтому явного ложного PASS для меню/движения/стрельбы не выявлено.

Счётчик IRQ>=2 сам по себе не доказывает make+break, однако совместно с
реальными изменениями game state и отдельной make/break unit regression
он полезен. Тест работает только в Safe Mode при 1 МГц, с фиксированным
DOS 1.9 relocation. Его PASS не закрывает JEMMEX/100 МГц, оконный frontend,
звук или roadmap 9.8. Последующие условия приёмки остаются открытыми.

## Верификация review

Выполнено с `GOCACHE=/tmp/ai386-status-go-cache`, `-count=1`:

```sh
go test ./tools/doom
go test ./internal/bios -run 'BIOSKeyboardIRQ|BIOSWithoutKeyboard'
go test ./internal/machine -run '^TestBootGuestKeyboardIRQ'
go vet ./tools/doom ./internal/bios
```

7 тестов прошли (2 tools, 3 BIOS, 2 Machine), vet успешен. Опциональный
actual-game тест в данном повторе пропущен без `-doom-input-config`;
его успешный реальный запуск сообщил root, review не выдаёт это за свой
повтор. Timer 1 МГц/100 МГц и общие acceptance checks выполняются root.

Итог по осям: Standards — обязательный pending cleanup; Spec — R1/R2
по надёжности диагностического критерия и тестам discovery, R3 по покрытию
AUX/spurious. Высокоприоритетных дефектов самого IRQ1 исправления не найдено.

## Повторный review: ClockCycles и устранение замечаний

Проверена новая production-дельта после однофакторного подтверждения
clock-гипотезы root: `CPU.pageWalkCycles`, `CPU.ClockCycles`, два инкремента
в физических page-table helpers, использование общей разницы в Machine.Step.
Основание — обновлённые design и machine-core spec scoped change.
Части старой дельты SetSeg/EA/BIOS в этот timing-review не включены.

Высокоприоритетных дефектов алгоритма не найдено:

- По всем production call-sites `readBus32`/`writeBus32` используются только
  в `translateLinearPrivilege` для PDE/PTE и A/D. Их инкременты `cycles`
  и `pageWalkCycles` происходят одновременно; метрика `Cycles` сохраняет
  прежний diagnostic traffic, новая метрика исключает именно эти операции.
- `CPUReset` заменяет CPU whole-struct literal, поэтому оба счётчика
  сбрасываются. Reset не выполняется внутри текущего `CPU.Run`, значит
  обычный Machine.Step не пересекает обнуление метрики.
- Fault после частичного page walk не даёт отрицательный delta: сняты
  только internal increments, успешные guest memory accesses и обращения
  доставки исключения сохраняются. Нулевая гостевая работа продвигает
  время не менее чем на один такт по прежнему `max(...,1)`.
- PIT/NAT, RTC/VGA и SB16 получают одну и ту же метрику; дробные
  аккумуляторы и HLT idle fallback сохранены. Изменение не добавляет TLB,
  не меняет трансляцию, page permissions, A/D или restart semantics.
- Оба накопителя имеют тип `int`, как прежний `cycles`. На 64-битном host
  переполнение не является практическим новым риском. На 32-битном host
  исходная метрика и раньше имела ограниченный диапазон; миграция всех
  счётчиков к широкой арифметике — отдельная задача, не подтверждённый
  дефект данного исправления.

Глобальный риск остаётся приемочным: частота устройств относительно paging
guest меняется намеренно. Поэтому старые успешные Linux/FreeBSD-сессии не
заменяют повтор текущей сборки; root выполняет гостевые и общие проверки.
Аппаратное потактовое равенство не заявляется, как и требует новая спека.

Disposition ранее найденных замечаний:

| Замечание | Текущее состояние |
|---|---|
| Cleanup | Выполнено: поиск debug prefix/API/skip-time flag в `internal` и `tools/doom` не дал совпадений. Исторические документы могут сохранять их как свидетельства. |
| R1 | Закрыто: README теперь описывает loaded-function diagnostic, явно не menu acceptance; `timer` при каждом чтении повторно сравнивает routine/signature через текущий mapping. |
| R2 | Закрыто: тест обеих фактических relocation отвергает ранний unmapped copy, смену mapping и нарушенный suffix. |
| R3 | Остаётся необязательным замечанием покрытия AUX/spurious; корректность status guard по inspection подтверждена. |

Повторно выполнено с тем же GOCACHE и `-count=1`:

```sh
go test ./internal/cpu -run '^TestPagingClockPreservesGuestWorkRateAndAccessDirtyBits$'
go test ./tools/doom
go test ./internal/machine -run 'TestPITWakesHaltedGuestThroughPIC|TestRTCClockAndIRQ8WakeHaltedCPU|TestClockOptionRejectsInvalidFrequency'
```

9 тестов прошли (1 CPU, 5 tools, 3 Machine). CPU-регрессия проверяет
одинаковое guest work flat/paging, сохранённую разницу diagnostic traffic
и A/D. Изменения для исправления review не внесены этим проверяющим.
Итог повторного gate: Standards и Spec без high/blocking code findings;
финальная гостевая приёмка и общий regression gate остаются ответственностью root.
