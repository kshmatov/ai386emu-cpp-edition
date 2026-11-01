## ADDED Requirements

### Requirement: Integer и packed BCD FPU load/store

FPU SHALL поддерживать FILD/FIST/FISTP для integer16/32/64 и FBLD/FBSTP для packed BCD. Conversion SHALL применять RC; переполнение integer и значение вне 18 цифр BCD SHALL сигнализировать IE. При masked IE destination получает indefinite, при unmasked IE destination memory, source и TOP сохраняются. Немаскированный PE при store SHALL подавлять запись и pop; masked PE SHALL записывать округлённое значение и устанавливать PE. FBLD с nibble >9 SHALL быть invalid и не должен частично изменять стек при unmasked IE.

#### Scenario: Example — integer limits

- **WHEN** FISTP сохраняет значения на границе signed 16/32/64
- **THEN** representable значения округляются по RC и pop происходит после записи, overflow даёт indefinite только при маске IE

#### Scenario: Example — precision atomicity

- **WHEN** FISTP получает 2.5 с PM=0
- **THEN** устанавливаются PE/ES/B, память и TOP остаются без изменений

#### Scenario: Example — BCD validation

- **WHEN** FBLD читает packed BCD с цифрой A–F и IM=0
- **THEN** устанавливаются IE/ES/B, TOP и destination остаются неизменными

Источник: [Intel SDM Volume 1, §8.5.6 и таблица 8-10](https://cdrdv2-public.intel.com/671436/253665-sdm-vol-1.pdf).
