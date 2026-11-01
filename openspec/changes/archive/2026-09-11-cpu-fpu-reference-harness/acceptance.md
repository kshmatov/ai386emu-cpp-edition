# Приёмка эталонного раннера x87

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

2026-09-11. Шаг1.4 cpu-functional-completion завершён.

- Общий capture.c исполнен native и bare-metal. Каждый корпус содержит480 случаев (5 операций ×3 PC ×4 RC ×8 TOP), полные FSAVE108/raw80, CW/SW/tag/TOP, память с guards и происхождение.
- Native источник: Intel(R) Core(TM) i5-10400F CPU @ 2.90GHz; gcc (Ubuntu 13.3.0-6ubuntu2~24.04.1) 13.3.0.
- Bare-metal: QEMU emulator version 8.2.2 (Debian 1:8.2.2+ds-0ubuntu1.18), CPU486/TCG, protected mode CPL0, без дисков ОС.
- Повтор обоих генераторов даёт побайтово одинаковые capture. Native SHA256 `44d92d1f1b80b4b5073d2031095a69e576936f4f80c1de645a0787ceb59781d5`; bare-metal `eb990d827d6bc04939fd0d23bd55d9efb8ae772cd50febb7087812bb50fc6582`. SHA256 исходников проверены против metadata.
- Go-раннер выполнил960 случаев без native/QEMU. Проверки повреждения JSON/матрицы/хеша и искусственных ошибок raw80/SW/памяти успешны. Python:4 теста PASS.
- go test ./... PASS (CPU8.913с); go vet ./... PASS; go test -race ./internal/cpu -run TestFPUReference -count=1 PASS2.102с; strict scoped change и родителя/roadmap PASS; go build ./... PASS.
- Применимость: точные простые конечные операции проверяют инфраструктуру. Численные домены/ULP, special values, fault priority, FIP/FDP/FOP cross-mode и privileged exceptions остаются задачами1.5/2–4. Эти ограничения не являются исключением из итоговой полноты CPU.

Формат и воспроизведение: tools/fpu-reference/README.md от корня репозитория. Дельты поведения нет (skip_specs: true), базовая CPU-спека не изменяется. Родитель4/29, маршрут38/52; дальше1.5.
