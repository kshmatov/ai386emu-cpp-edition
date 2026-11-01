## ADDED Requirements

### Requirement: CMPXCHG write cycle and restart

CMPXCHG SHALL выполнять write cycle назначения независимо от результата сравнения: источник при равенстве, исходное назначение при неравенстве. Read/write fault SHALL сохранять accumulator, source и EFLAGS, не записывая частичный операнд. Правила применяются к8/16/32-битным формам.

Источник: [Intel SDM CMPXCHG](https://cdrdv2-public.intel.com/774492/325383-sdm-vol-2abcd.pdf).

#### Scenario: Mismatched read-only destination
- **WHEN** accumulator отличается от назначения на read-only странице
- **THEN** возникает write page fault, accumulator/flags сохраняются.

#### Scenario: Read fault or successful completion
- **WHEN** чтение операнда неуспешно либо операция завершается успешно
- **THEN** при fault состояние не меняется; при успехе результат/флаги соответствуют сравнению и обмену.
