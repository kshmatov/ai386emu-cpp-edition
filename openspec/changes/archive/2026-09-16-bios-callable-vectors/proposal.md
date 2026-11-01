## Why

FreeBSD loader вызывает BIOS через адрес из IVT, а не через непосредственную INT. Нулевые записи IVT приводят к исполнению памяти по адресу 0.

## What Changes

- Установить вызываемые входы IVT для реализованных BIOS-сервисов.
- Вызов PUSHF/CALL FAR через IVT возвращает результат и флаги BIOS, сохраняет стек вызывающего кода.

## Non-goals

Внешний BIOS ROM, подмена загрузчика, новые BIOS-сервисы.

## Capabilities

### New Capabilities

Нет.

### Modified Capabilities

- bios: вызов существующих BIOS-сервисов через IVT.

## Impact

BIOS bootstrap, регрессии прямого вызова через IVT и повтор FreeBSD.
