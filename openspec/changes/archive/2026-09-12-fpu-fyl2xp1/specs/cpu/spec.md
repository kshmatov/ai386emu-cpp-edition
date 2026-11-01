## ADDED Requirements

### Requirement: FYL2XP1

FYL2XP1 (D9 F9) SHALL вычислять ST1×log2(ST0+1) в extended precision, записывать результат в ST1 и освобождать ST0. ST0<-1 SHALL сигнализировать IE; ST0=-1 и ST1 nonzero SHALL сигнализировать ZE и вернуть signed infinity, а ST1=0 SHALL быть invalid. NaN SHALL использовать общий raw80 selection/quieting contract.

#### Scenario: Example — normal and pop

- **WHEN** ST1=3 и ST0=1
- **THEN** ST1 становится 3, ST0 освобождается

#### Scenario: Example — domain boundary

- **WHEN** ST0=-1 либо ST0<-1, ST1=2
- **THEN** первый случай даёт ZE и -∞, второй IE и indefinite

Источник: [Intel SDM Volume 1, таблица 8-10 и §8.5.3](https://cdrdv2-public.intel.com/671436/253665-sdm-vol-1.pdf).
