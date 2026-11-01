> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

## Why

FreeBSD csh падает SIGSEGV на sigtramp MOV GS,[EAX+14h] с корректным selector2Fh. Core сохранён; markAccessed читает LDT с CPL3, в отличие от уже исправленного readDescriptor. Обновление системного дескриптора должно использовать supervisor paging.

## What Changes

- Чтение/запись accessed-бита с системными правами и сохранением present/CR0.WP.
- Регрессия пользовательской загрузки GS из LDT на supervisor-странице.
- Повтор загрузки FreeBSD и входа root.

## Capabilities

### New Capabilities

### Modified Capabilities
- `cpu`: supervisor доступ к accessed-биту сегментного дескриптора.

## Impact

internal/cpu/protmode.go; гостевая приёмка2.2/9.2. Остальные CPU-блокеры сохраняют отдельную приёмку.
