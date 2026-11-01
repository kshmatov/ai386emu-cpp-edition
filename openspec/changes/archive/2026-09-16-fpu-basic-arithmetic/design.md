## Context

Linux исполняет проверку FDIV: FLD, FDIV, FMUL, FLD, FSUBP, FISTP, FNINIT.

## Goals / Non-Goals

Поддержать эту группу как общие операции, без специальных адресов ядра. Полный набор x87, transcendental, точный underflow/overflow extended exponent и немаскированные исключения остаются отдельными этапами.

## Decisions

Использовать math/big.Float с 64-битной мантиссой по умолчанию (PC=11), PC=00/10 дают 24/53 бита для результата арифметики; RC выбирает округление. FLD binary64 загружает значение точно; стек содержит восемь физических регистров и TOP/tag. Ошибки памяти не меняют стек.

## Sources

[Intel instruction reference](https://www.intel.com/content/dam/support/us/en/documents/processors/pentium4/sb/25366521.pdf).
