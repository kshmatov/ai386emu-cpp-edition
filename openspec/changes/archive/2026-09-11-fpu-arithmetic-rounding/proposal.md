## Why
Арифметика использует округление significand без границ extended и не завершает post-computation исключения по архитектурным правилам. Это оставшаяся часть 2.2b.

## What Changes
- PC24/53/64, все RC, точное округление без двойного округления на subnormal границе.
- OE/UE/PE и C1, masked saturation/denormal и unmasked scaled результат.
- Native корпус границ и независимые точные регрессии.

## Capabilities
### New Capabilities
Нет.
### Modified Capabilities
- `cpu`: диапазон и округление add/sub/mul/div.

## Impact
Общий arithmetic путь, эталоны и тесты; сравнения, конверсии и unary команды принимаются отдельно.
