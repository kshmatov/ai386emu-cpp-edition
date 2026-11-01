## ADDED Requirements

### Requirement: Debian diagnostic проверяет название этапа

Диагностика SHALL принимать language/country/keyboard только при наличии соответствующего заголовка вместе с содержательной стабильной панелью.

#### Scenario: Country redraw вместо keyboard
- **WHEN** кадр страны отличается по площади от прежнего и проходит genericcandidate
- **THEN** keyboard не принимается без Keymap to use
