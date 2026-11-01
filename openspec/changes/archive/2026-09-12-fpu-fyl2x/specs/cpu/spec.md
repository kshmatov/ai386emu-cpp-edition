## ADDED Requirements

### Requirement: FYL2X

FYL2X (D9 F1) SHALL вычислять ST1×log2(ST0) в extended precision, записывать результат в ST1 и освобождать ST0. ST0<0 SHALL сигнализировать IE; при masked IE destination получает indefinite, при unmasked source/TOP сохраняются. ST0=±0 и ST1 finite nonzero SHALL сигнализировать ZE и вернуть infinity со знаком, противоположным ST1. ST0=0 и ST1=0, а также ST0=∞ и ST1=0 SHALL быть invalid. NaN SHALL использовать общий raw80 selection/quieting contract.

#### Scenario: Example — normal and pop

- **WHEN** ST1=3 и ST0=2
- **THEN** ST1 становится 3, ST0 освобождается

#### Scenario: Example — domain and zero

- **WHEN** ST0=-1 либо ST0=+0, ST1=2
- **THEN** первый случай даёт IE/indefinite, второй ZE и -∞; unmasked pre-exception сохраняет TOP

#### Scenario: Example — infinity zero

- **WHEN** ST0=+∞ и ST1=0
- **THEN** возникает IE, а не ZE

Источник: [Intel SDM Volume 1, таблица 8-10 и §8.5.3](https://cdrdv2-public.intel.com/671436/253665-sdm-vol-1.pdf).
