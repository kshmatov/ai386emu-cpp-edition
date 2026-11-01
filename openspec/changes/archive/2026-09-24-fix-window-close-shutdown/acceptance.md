# Приёмка закрытия окна

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

Дата: 2026-09-23.

## Результат

Кнопка закрытия окна теперь явно отменяет контекст оконной сессии до выхода из Ebitengine. Worker `Machine.RunInteractive` завершается, frontend дожидается его и возвращает управление CLI, после чего существующий `Machine.Stop` закрывает устройства и носители. Пользовательское закрытие считается штатным завершением; ошибки worker-а и оконного цикла сохраняются.

Регрессионный desktop-процесс был запущен с синтетическим гостем и временным HD. После появления гостевого маркера реальному X11-окну отправлено событие `WM_DELETE_WINDOW`. Внешние SIGINT/SIGTERM и прямой вызов `cancel` не использовались.

- Окружение финальной оконной проверки: приватный Xvfb `DISPLAY=:99`, 1280×800×24, Unix-сокет в отдельном mount namespace, программный OpenGL.
- Процесс: PID 13384 самостоятельно завершился с кодом 0 до таймаута 10 секунд.
- stdout: `ai386: virtual machine started and stopped successfully`.
- stderr: пуст.
- HD: сигнатура гостя `0x0386` во втором секторе сохранилась после завершения процесса.
- Артефакты прогона: `/tmp/ai386-window-close-xvfb-report/process.log` и `/tmp/ai386-window-close-xvfb-report/result.txt`.

## Автоматические проверки

- `go test ./... -count=1` — PASS, включая существующую проверку штатной остановки по SIGINT/SIGTERM; CPU 35.806 с.
- `go test -tags desktop ./internal/frontend ./cmd/ai386 -count=1` — PASS.
- `AI386_DESKTOP_E2E=1 go test -tags desktop ./cmd/ai386 -run '^TestWindowCloseEndsCLIProcess$' -count=1 -v` — PASS, 2.65 с.
- `go test -race ./... -count=1` — PASS; CPU 161.223 с.
- `go test -race -tags desktop ./internal/frontend ./cmd/ai386 -count=1` — PASS.
- `go vet ./...` и `go vet -tags desktop ./...` — PASS.
- `go build ./...` и desktop-сборка — PASS.
- `python3 -m py_compile tools/frontend-smoke/check.py` — PASS.
- Изолированный Xvfb smoke `tools/frontend-smoke/check.py ... --close-only` — PASS.

## Покрытые сценарии

- Событие закрытия отменяет активный worker и не оставляет заблокированный `Console.Exchange`.
- Повторная остановка безопасна, ошибка worker-а не теряется.
- Удерживаемые клавиши и кнопки формируют события отпускания; уже отменённый контекст не задерживает завершение.
- Закрытие окна завершает весь CLI-процесс с кодом 0 и сохраняет запись гостя на HD.
- Остановка по Ctrl+C/SIGTERM и headless-режим продолжают работать.
