## ADDED Requirements

### Requirement: Чтение слова состояния машины

CPU SHALL исполнять SMSW (0F 01 /4), сохраняя младшие 16 бит CR0 в регистр или двухбайтовую область памяти без изменения CR0 и флагов. Инструкция доступна на всех уровнях привилегий; LOCK вызывает #UD. Старшая половина 32-битного регистра архитектурно не определена и не используется как критерий приёмки.

#### Scenario: Регистр в реальном режиме

- **WHEN** SMSW AX выполняется при CR0=6000001Eh и EAX=AABBCCDDh
- **THEN** EAX=AABB001Eh, CR0 и EFLAGS не меняются

#### Scenario: Размер памяти

- **WHEN** SMSW [SI] с префиксом 66h записывает CR0=6000001Eh в память AABBCCDDh
- **THEN** память содержит AABB001Eh

#### Scenario: Пользовательский режим

- **WHEN** SMSW AX выполняется при CPL=3
- **THEN** AX получает младшее слово CR0 без #GP

#### Scenario: LOCK

- **WHEN** SMSW имеет префикс LOCK
- **THEN** CPU генерирует #UD, регистр назначения не меняется

Источник: [Intel SDM, SMSW](https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-2b-manual.pdf).
