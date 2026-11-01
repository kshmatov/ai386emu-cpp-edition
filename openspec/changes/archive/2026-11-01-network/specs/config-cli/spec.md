## ADDED Requirements

### Requirement: Параметры сети


Конфигурация TOML SHALL поддерживать `network`, `network_base`, `network_irq`,
`network_mac` и параметры IPv4-подсети NAT. CLI SHALL предоставлять `--network`
 со значениями on/off; его значение SHALL иметь приоритет над TOML. Некорректные
 значения SHALL отклоняться до создания VM.

#### Scenario: Приоритет CLI
- **WHEN** TOML включает сеть, а CLI содержит `--network=off`
- **THEN** VM создаётся без сетевого адаптера
