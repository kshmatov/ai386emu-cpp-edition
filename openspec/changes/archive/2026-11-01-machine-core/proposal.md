# Proposal: Machine-core — достоверность диагностических сценариев

## Why

Объединение работ по системе `machine-core` (диагностическая инфраструктура).
Диагностика Debian засчитывала country redraw как keyboard: структурные
метрики не определяют название этапа.

## What Changes

- Этап diagnostic требует своего заголовка из проверенного ISO;
  partial country не засчитывается как keyboard.
- Независимый stage identity с fixtures и provenance; неизвестный этап
  отклоняется.

## Capabilities

### New Capabilities

Нет.

### Modified Capabilities

- `machine-core`: достоверность Debian diagnostic.

## Non-goals

Production VGA, OCR произвольных ОС, завершение установки Debian.

## Impact

Только opt-in тест и небольшие PNG fixtures.

## Источники

Объединён scoped change: `debian-diagnostic-stage-identity`.
