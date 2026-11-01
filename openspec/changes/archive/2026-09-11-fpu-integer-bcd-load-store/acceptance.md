# Приёмка задачи 2.4

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

2026-09-11. FILD/FIST/FISTP для integer16/32/64 и FBLD/FBSTP для packed BCD проверены на предельных значениях, знаках, RC и масках. Integer stores с немаскированным IE или PE не меняют память и не выполняют pop; masked invalid записывает indefinite. Недопустимые BCD-цифры дают IE с той же atomicity.

Targeted integer/BCD tests PASS. Полный `go test ./...` PASS (CPU 6.907 с), `go test -race ./internal/cpu` PASS (46.706 с), `go vet ./...` PASS, `go build ./...` PASS. OpenSpec strict PASS. Изменение синхронизировано в CPU-спеку и архивировано.

CPU 9/29, roadmap 39/52. Следующий шаг 2.5 — FCOM/FUCOM/FTST/FXAM и stack/constant commands.
