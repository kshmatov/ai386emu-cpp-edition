> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

## Why
Общий arithmetic путь не сигнализирует DE и ошибочно выставляет ZE для infinity/zero. Это часть шага 2.2, требующая самостоятельной матрицы приоритетов до исправления округления.

## What Changes
- Pre-computation IE/DE/ZE в add/sub/mul/div по Intel.
- Сохранение источников, destination и TOP при unmasked IE/DE/ZE.
- Нативные эталоны и переносимые Go-регрессии, включая формы памяти.

## Capabilities
### New Capabilities
Нет.
### Modified Capabilities
- `cpu`: pre-computation исключения арифметики x87.

## Impact
CPU arithmetic, тесты и генератор эталонов. Post-computation OE/UE/PE и PC/RC остаются в 2.2b; 2.2 целиком пока открыта.
