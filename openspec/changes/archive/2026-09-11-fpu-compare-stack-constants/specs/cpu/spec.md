## ADDED Requirements

### Requirement: Compare, FTST, FXAM и constants x87

FCOM/FCOMP SHALL сигнализировать IE для QNaN/SNaN/unsupported, FUCOM/FUCOMPP SHALL сигнализировать IE только для SNaN/unsupported. При masked invalid compare SHALL установить C0/C2/C3=111; при unmasked IE destination, operands и TOP SHALL сохраниться, а FCOMP pop SHALL быть подавлен. Ordered finite compare SHALL установить C0/C2/C3 по отношению less/equal/greater. FXAM SHALL классифицировать empty, zero, denormal, normal, infinity, NaN и unsupported по raw80 без численного исключения. Constants SHALL загружаться как extended values с сохранением TOP/tag.

#### Scenario: Example — FCOM и FUCOM

- **WHEN** сравниваются QNaN и finite
- **THEN** FCOM даёт IE, FUCOM возвращает unordered только при маске IE; при unmasked FCOMP TOP не меняется

#### Scenario: Example — ordered compare

- **WHEN** ST0/ST1 равны, меньше или больше
- **THEN** C3/C2/C0 равны 100/001/000 соответственно

#### Scenario: Example — FXAM empty

- **WHEN** FXAM анализирует empty register
- **THEN** C3/C2/C0 отражают empty class, C1 отражает знак indefinite encoding, IE не возникает

Источник: [Intel SDM Volume 1, §8.2.1, §8.3.3 и таблица 8-10](https://cdrdv2-public.intel.com/671436/253665-sdm-vol-1.pdf).
