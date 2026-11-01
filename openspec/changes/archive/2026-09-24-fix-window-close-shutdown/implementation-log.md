# Журнал реализации

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

## 2026-09-23 — задачи 1.1–1.2

- Время: около 15 минут, завершено в 20:38 MSK.
- Реализован единый жизненный цикл оконной сессии `cancel → wait`; событие закрытия окна явно отменяет контекст VM.
- Добавлены `testify`-регрессии отмены worker-а, повторной остановки, сохранения ошибки, разблокировки `Console.Exchange` и освобождения удерживаемого ввода.
- Проверки: `go test -tags desktop ./internal/frontend -count=1`, `go test ./internal/frontend -count=1`, `go test -race -tags desktop ./internal/frontend -count=1`, `go vet -tags desktop ./internal/frontend` — PASS.
- Токены: среда выполнения не предоставляет точный счётчик по задаче.

## 2026-09-23 — задача 3.1

- Время: около 17 минут, завершено в 20:58 MSK; основное время занял полный race-прогон.
- PASS: `go test ./... -count=1` (CPU 35.806 с), `go test -tags desktop ./internal/frontend ./cmd/ai386 -count=1`, `go test -race ./... -count=1` (CPU 161.223 с), `go test -race -tags desktop ./internal/frontend ./cmd/ai386 -count=1`.
- PASS: `go vet ./...`, `go vet -tags desktop ./...`, `go build ./...`, `go build -tags desktop -o /tmp/ai386-window-close-final ./cmd/ai386`.
- PASS: `tools/frontend-smoke/check.py ... --close-only` в реальном X11 `:0` и в изолированном Xvfb `:99`; финальный Xvfb PID 13384 завершился с кодом 0, HD-маркер сохранён.
- Существующий процессный тест SIGINT/SIGTERM прошёл в составе обычного и race-прогонов; headless-набор тестов не регрессировал.
- Токены: среда выполнения не предоставляет точный счётчик по задаче.

## 2026-09-23 — задачи 2.1–2.2

- Время: около 10 минут, завершено в 20:42 MSK.
- Добавлен opt-in `testify` e2e-тест `TestWindowCloseEndsCLIProcess`; он собирает desktop CLI, запускает реальное X11-окно и использует smoke-драйвер вместо прямой отмены контекста.
- Smoke-драйвер получил режим `--close-only`, ожидание гостевого маркера до закрытия, проверку PID/кода 0/HD и диагностику таймаута с журналом процесса.
- Проверки: Python syntax PASS; `go test -tags desktop ./cmd/ai386 -count=1` и `go vet -tags desktop ./cmd/ai386` PASS; `AI386_DESKTOP_E2E=1 go test -tags desktop ./cmd/ai386 -run '^TestWindowCloseEndsCLIProcess$' -count=1 -v` PASS (2.65 с). Отдельный smoke-прогон: `WM_DELETE_WINDOW` остановил PID 9412 с кодом 0, гостевой HD-маркер сохранён, вывод процесса содержит только штатное сообщение.
- Первая оконная проверка выполнена в X11 `DISPLAY=:0` с программным OpenGL (`LIBGL_ALWAYS_SOFTWARE=1`); системный Xvfb отсутствует, финальная изолированная проверка подготовлена в задаче 3.1 из временного пакета.
- Токены: среда выполнения не предоставляет точный счётчик по задаче.

## 2026-09-23 — задача 3.2

- Время: около 4 минут, завершено после 20:58 MSK.
- Создан `acceptance.md` с окружением Xvfb, событием `WM_DELETE_WINDOW`, кодом выхода, stdout/stderr, проверкой HD и полной матрицей команд.
- `openspec validate fix-window-close-shutdown --strict` — PASS; `git diff --check` — PASS.
- Итог: требования delta-спеки подтверждены, исключений по приёмке нет.
- Токены: среда выполнения не предоставляет точный счётчик по задаче.
