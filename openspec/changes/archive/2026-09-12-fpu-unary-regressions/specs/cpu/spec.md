## ADDED Requirements

### Requirement: Unary x87 и сохранение принятых эталонов

FSQRT SHALL обрабатывать отрицательный zero как отрицательный zero, а отрицательное конечное значение как invalid. FRNDINT SHALL округлять по RC, устанавливать PE при неточности и сохранять TOP. FSCALE SHALL умножать ST0 на степень двойки с целым показателем, полученным усечением ST1, и сохранять ST1. Изменения общего exception/rounding пути SHALL сохранять ранее принятые результаты FPREM/FPREM1/FXTRACT/F2XM1/FPATAN.

#### Scenario: Example — FSQRT

- **WHEN** FSQRT получает -0 или -1
- **THEN** -0 сохраняет знак, а -1 даёт IE и masked indefinite

#### Scenario: Example — FRNDINT

- **WHEN** FRNDINT получает 2.5 при каждом RC
- **THEN** результат равен 2/2/3/2, PE установлен, TOP не меняется

#### Scenario: Example — FSCALE and regressions

- **WHEN** FSCALE получает ST0=1.5 и ST1=2.9
- **THEN** ST0 становится 6, ST1 остаётся 2.9; принятые F2XM1/FPATAN/FPREM/FXTRACT корпуса проходят без изменения допусков

Источник: [Intel SDM Volume 1, §8.1.5, §8.3 и таблица 8-10](https://cdrdv2-public.intel.com/671436/253665-sdm-vol-1.pdf).
