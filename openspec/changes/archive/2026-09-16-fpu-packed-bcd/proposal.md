> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

## Why

FBLD/FBSTP пока вызывают #UD, хотя входят в x87 целевого i486DX4. Это следующий блок оставшейся задачи2.8.

## What Changes

- Загрузка и сохранение18 десятичных цифр в10 байтах, знак и signed zero.
- Округление RC, masked invalid indefinite, unmasked invalid/precision без записи/pop, stack fault.
- Проверка полного операнда до изменения состояния/памяти.

## Capabilities

### New Capabilities

### Modified Capabilities

- `cpu`: packed BCD x87.

## Impact

internal/cpu; существующие бинарные integer/real формы сохраняются. Полнота x87 остаётся отдельной задачей.
