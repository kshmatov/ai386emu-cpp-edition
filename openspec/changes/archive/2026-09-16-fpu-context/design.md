## Context

FNSAVE DD /6 используется Linux copy_thread.

## Decisions

Сохранять 94/108 байт по размеру операнда; формат окружения зависит от real/protected mode. Перед записью переводить все страницы, затем фиксировать данные и сбрасывать x87. FRSTOR читает весь блок до изменения состояния. Числа сериализуются в extended80; исходные NaN payload и регистры с empty tag требуют сохранения исходных байтов при FRSTOR.

## Sources

[Intel Vol.1, figures 8-9–8-12](https://cdrdv2-public.intel.com/789574/253665-sdm-vol-1.pdf).
