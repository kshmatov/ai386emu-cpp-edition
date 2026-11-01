# Design: Machine-core (диагностика)

## Context

Структурные метрики экрана Debian diagnostic не различают country redraw и
keyboard — false PASS. См. proposal.md.

## Decisions

- **Stage identity**: у каждого этапа — свой заголовок из проверенного ISO;
  partial country ≠ keyboard. Неизвестный этап отклоняется, а не угадывается.
- **Fixtures с provenance**: PNG-кадры этапов сохраняются с указанием
  источника (ISO, команда, момент прогона).
- Проверка: red с positive keyboard control → stage identity + fixtures →
  повтор настоящего Debian framebuffer (три PNG) + независимый review.

## Non-goals

Production VGA, OCR произвольных ОС, завершение установки Debian.

## Источники

`debian-diagnostic-stage-identity`.
