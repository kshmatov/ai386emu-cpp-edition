## ADDED Requirements

### Requirement: Descriptor accessed bit uses supervisor paging

CPU SHALL читать и при необходимости устанавливать accessed-бит GDT/LDT с правами supervisor независимо от CPL. Проверки present и CR0.WP SHALL сохраняться. Уже установленный accessed-бит SHALL не требовать записи.

#### Scenario: User MOV GS
- **WHEN** CPL3 загружает корректный пользовательский дескриптор из supervisor GDT/LDT
- **THEN** загрузка успешна и accessed установлен; прямое пользовательское чтение этой страницы по-прежнему запрещено.

#### Scenario: Read-only descriptor
- **WHEN** при CR0.WP=1 дескриптор на read-only странице требует установки accessed
- **THEN** возникает supervisor write page fault; при уже установленном accessed запись не нужна.
