# Приёмка задачи 3.1

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

2026-09-12. Добавлена FYL2X D9 F1: range-reduced extended log2, result в ST1 и pop ST0, domain/special handling для negative/zero/infinity/NaN, ZE/IE и masked/unmasked TOP.

Targeted tests проверяют нормальный результат, отрицательный домен, zero-divide, `0·(-∞)`, `∞·0`, signed infinity и численные точки от subnormal до max finite. После полного `gofmt` и `go fmt ./...` повторены `go test ./...` (PASS, CPU 7.306 с), `go vet ./...` (PASS), `go build ./...` (PASS) и `go test -race ./internal/cpu` (PASS, 48.750 с). OpenSpec strict проходит. Дельта синхронизирована и архивирована.

CPU 13/29, roadmap 39/52. Следующий шаг 3.2 — FYL2XP1.
