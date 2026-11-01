# Приёмка 2.2b и завершение 2.2

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

2026-09-11. Добавлено завершение arithmetic результата по extended диапазону: PC/RC, masked overflow saturation, subnormal rounding из точного значения, unmasked exponent scaling ±24576, PE/ES/B и C1. Обычный путь сохраняет стандартную big.Float арифметику; рациональное вычисление выполняется только для masked tiny результата, чтобы избежать двойного округления.

Native корпус 38016 случаев проходит побитово по raw80, defined SW BAFFh, TOP и тегу destination. Оси: PC24/53/64 × RC4 × OE/UE/PE masks8 × pop/no-pop × sign2; halfway/соседи, cancellation, min/max extended, tiny и 64 дополнительные пары с seed486. Повторный запуск генератора сохранил SHA256 `d85407e8b069dbd6a24e127438f7424e877c6c321d9ce82685af06ae90cbb9d9`; provenance рядом с данными.

Независимые точные тесты проверяют ±(1+2^-p) по всем PC/RC и TOP с masked/unmasked PE, точный subnormal, scaled UE/OE, половину минимального subnormal и infinity при overflow. Формы real32/64 и integer16/32 memory подтверждают то же округление и разрешённую запись при unmasked PE. Старый pre-exception корпус24576 проходит без изменений.

До исправления раннер FAIL: PE выставлялся без ES/B при PM=0. После исправления все матрицы PASS. `go test ./...` PASS (CPU7.138с), `go vet ./...` PASS, `go build ./...` PASS, `go test -race ./internal/cpu` PASS (48.787с); scoped strict PASS.

2.2a и 2.2b завершены, родитель2.2 закрыт. CPU7/29, roadmap39/52. Следующий шаг2.3 — real32/64/80 load/store; их округление и исключения не принимаются автоматически по арифметическим тестам.
