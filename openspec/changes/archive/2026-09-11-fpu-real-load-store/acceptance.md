# Приёмка задачи 2.3

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

2026-09-11. Реализованы real32/64/80 load/store с сохранением raw special values. Real32/64 NaN расширяется в quiet extended с переносом payload и знака; при обратном сужении payload возвращается в представимый IEEE диапазон. Real80 проходит побитово.

Загрузка denormal real32/64 выставляет DE до расширения. При DM=0 устанавливаются ES/B, destination и TOP сохраняются; при DM=1 загрузка продолжается. Real80 denormal DE не вызывает. FST/FSTP используют PC/RC и PE, pop выполняется после записи.

До исправления регрессии теряли знак/payload NaN и не выставляли DE. После исправления targeted FPU tests проходят.

`go test ./...` PASS (CPU 9.104 с), `go test -race ./internal/cpu` PASS, `go vet ./...` PASS, `go build ./...` PASS, OpenSpec strict PASS. Дельта синхронизирована в базовую CPU-спеку; change архивирован.

CPU 8/29, roadmap 39/52. Следующий шаг 2.4 — integer и BCD load/store.
