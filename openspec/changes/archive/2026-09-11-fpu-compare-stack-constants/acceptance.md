# Приёмка задачи 2.5

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

2026-09-11. Compare-путь теперь различает FCOM/FCOMP и FUCOM/FUCOMPP: QNaN для FCOM invalid, для FUCOM unordered, SNaN и unsupported invalid; при unmasked IE operands/TOP сохраняются, pop подавляется. Masked compare выставляет unordered C0/C2/C3.

Проверены ordered less/equal/greater, FXAM empty и special raw80 classes, constants D9 E8–EE с TOP/tag. `go test ./...` PASS (CPU 7.551 с), `go vet ./...` PASS, `go build ./...` PASS, `go test -race ./internal/cpu` PASS (46.768 с), OpenSpec strict PASS. Дельта синхронизирована и change архивирован.

CPU 10/29, roadmap 39/52. Следующий шаг 2.6 — FSQRT/FRNDINT/FSCALE и повторная проверка принятых FPREM/FXTRACT/F2XM1/FPATAN.
