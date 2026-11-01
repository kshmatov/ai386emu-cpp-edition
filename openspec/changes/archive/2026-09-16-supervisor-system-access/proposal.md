## Why

Первый INT 80h из user mode DSL останавливается: доступ к IDT/GDT/TSS и kernel stack ошибочно использует U/S-права текущего CPL=3.

## What Changes

Явные supervisor-доступы для системных структур и стекового кадра при переходе на kernel stack, с сохранением paging и CR0.WP.

## Capabilities

### Modified Capabilities
- cpu: системные доступы при смене привилегий.

## Impact

Paging, descriptors, interrupt delivery, регрессии и DSL.
