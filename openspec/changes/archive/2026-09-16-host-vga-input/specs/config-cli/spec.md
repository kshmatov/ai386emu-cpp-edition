## ADDED Requirements

### Requirement: Настройки оконного фронтенда
Конфигурация SHALL поддерживать display=headless/window (default headless), display_scale=1..4 (default 1), display_fps=1..120 (default 30). CLI SHALL предоставлять --display, --display-scale, --display-fps с приоритетом CLI > TOML > default. Неверные и пустые значения SHALL отклоняться до создания VM. Сборка без оконного backend SHALL явно отклонять display=window до создания дисков.

#### Scenario: Переопределение режима
- **WHEN** TOML задаёт display="window", а CLI задаёт --display=headless
- **THEN** VM запускается без окна

#### Scenario: Неверная частота
- **WHEN** задано --display-fps=0 или --display-fps=
- **THEN** запуск завершается ошибкой конфигурации до создания дисков
