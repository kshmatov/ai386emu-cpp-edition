# Декомпозиция шага 2.2

Критерий родительской задачи сохранён полностью. До реализации разделены две приёмки:

- [x] 2.2a IE/DE/ZE в add/sub/mul/div: приоритет NaN/invalid/zero divide перед DE, оба знака, регистровые и real32/64/integer memory формы, masked/unmasked, сохранение destination и запрет pop. Scoped change fpu-arithmetic-preexceptions.
- [x] 2.2b PC/RC и extended-диапазон: точное округление add/sub/mul/div, границы normal/subnormal/overflow, OE/UE/PE, C1 и разрешённая фиксация scaled результата при unmasked post-computation exception. Независимый численный и native эталоны, полный test/vet/race, приёмка.

Родитель 2.2 закрывается только после обеих приёмок. Опорный источник: [Intel SDM Volume1, §4.9.2 и §8.5.1–8.5.6](https://cdrdv2-public.intel.com/671436/253665-sdm-vol-1.pdf). Профиль DX4 сохраняется; различия поколений проверяются по Embedded Intel486 manual.
