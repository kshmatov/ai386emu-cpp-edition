# Сверка CPU-спецификации перед итоговой приёмкой

Состояние на 2026-09-16. Матрицы допустимых форм **M (integer/system)** и
**X (x87)** остаются открытыми. Этот документ фиксирует порядок будущей
синхронизации требований; он не служит доказательством полноты 5.4 и не
закрывает roadmap 2.2, 2.8 или 2.5.

## Завершённые scoped changes, отсутствующие в базовой CPU-спецификации

У перечисленных changes все задачи отмечены выполненными. Их отдельные
`Requirement` отсутствуют по заголовку в
[базовой спецификации](../../specs/cpu/spec.md). Более широкие формулировки
базовой спецификации не заменяют их точные сценарии. Два обнаруженных
конфликта `fpu-packed-bcd` уже разрешены ниже.

| Change | Отсутствующее требование |
|---|---|
| [cpu-string-io-faults](../cpu-string-io-faults/specs/cpu/spec.md) | Возобновление строкового I/O после page fault |
| [cpu-segment-load-faults](../cpu-segment-load-faults/specs/cpu/spec.md) | Fault-atomic segment loads |
| [fpu-native-errors](../fpu-native-errors/specs/cpu/spec.md) | Отложенное native-исключение x87 |
| [cpu-cmpxchg-faults](../cpu-cmpxchg-faults/specs/cpu/spec.md) | CMPXCHG write cycle and restart |
| [fpu-packed-bcd](../fpu-packed-bcd/specs/cpu/spec.md) | x87 packed BCD; PE и invalid digit согласованы, точные сценарии ещё требуется перенести |
| [fpu-extract](../fpu-extract/specs/cpu/spec.md) | x87 extract exponent and significand |
| [fpu-environment](../fpu-environment/specs/cpu/spec.md) | Окружение x87 без регистров данных |
| [cpu-selector-instructions](../cpu-selector-instructions/specs/cpu/spec.md) | Команды селекторов |
| [cpu-interrupt-shadow](../cpu-interrupt-shadow/specs/cpu/spec.md) | Maskable interrupt shadow |
| [cpu-i486-operands](../cpu-i486-operands/specs/cpu/spec.md) | Ширина и привилегии команд i486 |
| [fpu-exponential](../fpu-exponential/specs/cpu/spec.md) | F2XM1 exponential |
| [cpu-retf-segment-width](../cpu-retf-segment-width/specs/cpu/spec.md) | Размер кода после RETF |
| [fpu-remainders](../fpu-remainders/specs/cpu/spec.md) | x87 partial remainders |
| [cpu-smsw](../cpu-smsw/specs/cpu/spec.md) | Чтение слова состояния машины |
| [cpu-far-load-faults](../cpu-far-load-faults/specs/cpu/spec.md) | Atomic far data loads |
| [cpu-xadd-decoding](../cpu-xadd-decoding/specs/cpu/spec.md) | Граница команды и перенос XADD |
| [cpu-descriptor-accessed](../cpu-descriptor-accessed/specs/cpu/spec.md) | Descriptor accessed bit uses supervisor paging |
| [cpu-v86-monitor](../cpu-v86-monitor/specs/cpu/spec.md) | Полный переход v86 через монитор |

## Порядок синхронизации после закрытия M/X

Активная дельта `fpu-packed-bcd` ранее требовала подавлять FBSTP при
немаскированной PE. Принятый контракт Intel фиксирует запись и pop при
изолированной PE; активная дельта исправлена и содержит отдельный сценарий.
Её правило для FBLD с цифрой A–F трактует результат
как неопределённый; базовая и итоговая CPU-спецификации теперь согласованы
с ним по решению пользователя.
[Intel SDM Vol. 2, FBLD](https://cdrdv2-public.intel.com/774492/325383-sdm-vol-2abcd.pdf)
подтверждает неопределённый результат и отсутствие проверки цифр. Текущий IE
эмулятора и соответствующая регрессия сохраняются как допустимое определение
поведения на таком входе, без обязательного архитектурного требования.

Архивная дельта `fpu-real-load-store` подавляла push у FLD denormal
real32/64 при немаскированной DM. [Intel SDM Vol. 2, FLD](https://cdrdv2-public.intel.com/774492/325383-sdm-vol-2abcd.pdf)
прямо требует push и в этом случае. Базовая спецификация исправлена;
при итоговой синхронизации архивную редакцию нельзя возвращать поверх неё.

Дополнительная сверка совпадающих заголовков обнаружила другие старые
редакции, которые нельзя применять поверх базы целиком:

- [архивная fpu-integer-bcd-load-store](../archive/2026-09-11-fpu-integer-bcd-load-store/specs/cpu/spec.md)
  подавляет FISTP при изолированной немаскированной PE и требует IE для
  FBLD с цифрой A–F. Оба положения заменены принятым Intel-контрактом;
  архив также не содержит позднего сценария FBSTP precision.
- [архивная fpu-unary-regressions](../archive/2026-09-12-fpu-unary-regressions/specs/cpu/spec.md)
  не содержит уже принятого точного PC/RC/PE/C1 контракта FSQRT. База
  сохраняет усиленную редакцию.
- [fpu-basic-arithmetic](../fpu-basic-arithmetic/specs/cpu/spec.md)
  ещё содержит этапное ограничение, а [dsl-boot](../dsl-boot/specs/cpu/spec.md)
  — старое правило CPUID и прежнюю редакцию «Ширина адресов переходов и
  возвратов Linux» без поздних O/A и CS.limit сценариев.
- Архивную [cpu-conformance-suite](../archive/2026-08-30-cpu-conformance-suite/specs/cpu/spec.md)
  также нельзя повторно синхронизировать: поздняя база уточнила успешный
  HLT и диагностику возможного дефекта раннера или исходного эталона.

1. Сверить каждый из 18 `ADDED Requirements` выше с текущей базовой
   спецификацией и добавить его точные сценарии. Это отдельные заголовки;
   при их переносе не заменять уже существующие требования и сценарии.
2. Сохранить усиленные редакции совпадающих требований. В частности,
   [fpu-initialization](../fpu-initialization/specs/cpu/spec.md) содержит
   прежний `MODIFIED` для «Исполнение набора команд i386/i486» с правилом
   `#UD` для нереализованной x87. Его сценарий `#NM` уже присутствует и
   совпадает с базовой спецификацией.
   [dsl-boot](../dsl-boot/specs/cpu/spec.md) содержит прежнюю формулировку
   о неподдержанной CPUID в «Определение i486 через флаг AC» и старую
   ширину переходов; базовая спецификация уже содержит принятые IntelDX4,
   O/A и CS.limit сценарии. Повторная полная синхронизация этих старых
   дельт перезапишет принятые уточнения.
3. Согласовать [итоговую дельту](specs/cpu/spec.md) с базой. Её
   `MODIFIED` для «Исполнение набора команд i386/i486» должен заменить
   промежуточное исключение FPU, сохранив сценарии реального режима и
   команд i486. Правило «Integer и packed BCD FPU load/store» об
   изолированной unmasked PE уже исправлено в итоговой дельте и базовой
   спецификации: результат записи и предусмотренный pop фиксируются,
   PE/ES/B остаются. При дальнейшей синхронизации сохранить это правило
   и остальные integer/BCD сценарии.
4. Удалить из базового «Базовая арифметика конечных операндов x87» только
   устаревшую оговорку о том, что полный набор и немаскированные исключения
   находятся вне этапа; сохранить перечисленные операции и три сценария.
   Заменить завершающее примечание базовой спецификации о «промежуточных
   ограничениях FPU» исторической ссылкой без действующего ограничения.
5. После синхронизации проверить, что каждое исходное требование и сценарий
   базовой спецификации сохранены либо явно заменены итоговой редакцией;
   выполнить `openspec validate --all --strict`. Затем завершить приёмку
   roadmap 2.2/2.8/2.5 по их отдельным критериям и архивировать changes
   в согласованном порядке.

До доказанного покрытия M/X и итоговых проверок задачи 5.4 и roadmap
2.2/2.8/2.5 остаются открытыми.
