# Приёмка задачи 2.7

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

2026-09-12. Проверены FIP/FDP/FOP для register/memory, raw80 special round-trip во всех context forms и atomicity при page fault. Реализация поддерживает 14/28-byte environment и 94/108-byte save/restore в real/protected режимах.

После gofmt повторены `go test ./...` (PASS), `go vet ./...` (PASS), `go test -race ./internal/cpu` (PASS, 46.644 с ранее до форматирования) и `go build ./...` (PASS). OpenSpec strict PASS. Дельта синхронизирована в базовую CPU-спеку и архивирована.

CPU 12/29, roadmap 39/52. Все задачи FPU 2.1–2.7 закрыты; следующий этап — недостающие трансцендентные команды 3.1–3.5.
