## ADDED Requirements

### Requirement: Загрузка и сохранение real32/64/80

FPU SHALL загружать и сохранять real32, real64 и real80 с сохранением знака zero/infinity/NaN и payload там, где формат назначения его представляет. Real32/64 NaN SHALL расширяться в quiet extended NaN с соответствующим payload. Сохранение real32/64 SHALL округлять конечные значения по PC/RC и устанавливать PE при потере точности. Загрузка denormal real32/64 SHALL устанавливать DE; при DM=0 destination и TOP SHALL остаться неизменными, при DM=1 загрузка SHALL продолжиться. Real80 denormal SHALL загружаться без DE. FSTP SHALL выполнять pop только после успешной записи.

#### Scenario: Example — special values

- **WHEN** загружаются и сохраняются оба знака zero, infinity и QNaN real32/64
- **THEN** знак и представимый payload возвращаются в память, QNaN остаётся quiet и IE не возникает

#### Scenario: Example — real80

- **WHEN** raw80 SNaN с payload проходит FLD m80 и FST m80
- **THEN** десять байт совпадают побитово, signaling bit не изменён

#### Scenario: Example — denormal

- **WHEN** загружается минимальный denormal real32/64 с DM=0
- **THEN** устанавливаются DE/ES/B, TOP и destination не меняются

#### Scenario: Example — narrowing

- **WHEN** конечное extended значение сохраняется в real32/64
- **THEN** применяется текущий RC/PC, PE устанавливается только при неточном результате, FSTP освобождает регистр после записи

Источник: [Intel SDM Volume 1, §8.5.2, §8.5.6 и §4.8.3.5](https://cdrdv2-public.intel.com/671436/253665-sdm-vol-1.pdf).
