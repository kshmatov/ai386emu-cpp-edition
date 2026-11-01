# Приёмка загрузки сегментов

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

2026-09-11. Регрессии operand/descriptor/accessed PF, null 0–3, SS type/presence, DS privilege/presence и readable code сначала падали, после исправления проходят. Selector/cache фиксируются после accessed-write; read16 прекращается на первом PF, сохраняя CR2.

Исправлено старое ожидание TestPMSSExecTypeFaults: Intel MOV определяет GP для неправильного типа SS; SS применяется к отсутствующему допустимому stack descriptor. Источник приведён в спецификации.

Общий `go test ./...` успешен (CPU 6.209 с), `go vet ./...`, `go build ./...`, CPU race (42.704 с) и OpenSpec strict успешны. Полнота остальных инструкций, POP/LSS stack rollback и interrupt shadow этим scoped change не заявляется.
