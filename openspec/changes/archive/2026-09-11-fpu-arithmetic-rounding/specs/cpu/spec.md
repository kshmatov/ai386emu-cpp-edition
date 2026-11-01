## ADDED Requirements

### Requirement: Округление и диапазон арифметики x87
Add/sub/mul/div SHALL округлять significand по PC24/53/64 и RC nearest-even/down/up/truncate с диапазоном exponent double extended. C1 SHALL отражать увеличение модуля при округлении. Inexact SHALL устанавливать PE; unmasked PE SHALL сохранять округлённый результат и разрешать предусмотренный командой pop, устанавливая ES/B.

Masked overflow SHALL устанавливать OE/PE и возвращать signed infinity либо предельное конечное значение текущей PC согласно RC. Unmasked overflow SHALL устанавливать OE/ES/B и записывать округлённый результат с exponent, уменьшенным на 24576. Tiny определяется по округлённому результату с неограниченным exponent. Masked underflow SHALL округлять точное значение в subnormal сетке PC, устанавливая UE/PE только при потере точности. Unmasked underflow SHALL устанавливать UE/ES/B и записывать округлённый результат с exponent, увеличенным на 24576; PE SHALL отражать потерю точности significand. Разрешённый арифметический pop SHALL выполняться и при этих post-computation исключениях.

#### Scenario: Example — halfway и PE
- **WHEN** при PC=p выполняется 1 + 2^(-p)
- **THEN** nearest-even даёт 1, rounding up даёт 1 + 2^(1-p); PE=1, C1 устанавливается только во втором случае; PM=0 добавляет ES/B без отмены результата

#### Scenario: Example — underflow
- **WHEN** минимальный normal extended делится на 2 при PC64
- **THEN** UM=1 даёт точный subnormal без UE/PE; UM=0 даёт масштабированный результат 2^8193 и UE/ES/B

#### Scenario: Example — overflow
- **WHEN** максимальный конечный extended умножается на 2 при PC64 и nearest-even
- **THEN** OM=1 даёт infinity и OE/PE; OM=0 сохраняет точный significand с biased exponent 1FFFh и OE/ES/B без PE

Источник: [Intel SDM Volume1, §8.5.4–8.5.6](https://cdrdv2-public.intel.com/671436/253665-sdm-vol-1.pdf). Требование относится к add/sub/mul/div; преобразования памяти и unary операции имеют отдельные критерии приёмки.
