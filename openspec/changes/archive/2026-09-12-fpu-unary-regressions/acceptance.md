# Приёмка задачи 2.6

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

2026-09-12. FSQRT special/negative zero, FRNDINT для всех RC, FSCALE с усечённым показателем и сохранением ST1 проверены. Повторены принятые F2XM1, FPATAN, FPREM/FPREM1 и FXTRACT тесты; допуски не менялись.

Targeted и полный `go test ./...` PASS (CPU 8.423 с), `go vet ./...` PASS, `go build ./...` PASS, CPU `-race` PASS, OpenSpec strict PASS. Дельта синхронизирована в базовую CPU-спеку и архивирована.

CPU 11/29, roadmap 39/52. Следующий шаг 2.7 — FIP/FDP/FOP и environment/context.
