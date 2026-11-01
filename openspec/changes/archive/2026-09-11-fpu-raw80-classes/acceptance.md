# Приёмка шага 2.1

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

2026-09-11. Исправлены ошибочный IE на QNaN, потеря raw payload при переносах и операциях знака. Единые правила используются арифметикой, FPATAN, F2XM1, FXTRACT и остатком.

Регрессии воспроизвели ошибки до исправления. Матрица 49 пар finite/QNaN/SNaN/unsupported в двух порядках, masked/unmasked и FADD/FADDP проходит; оба знака, FXAM, signed zero по RC и widening payload real32/64 проверены. Исправлена ошибочная ожидаемая строка тестовой матрицы для SNaN+finite по таблице Intel; контракт реализации не ослаблен.

`go test ./...` PASS (CPU 6.756 с), `go vet ./...` PASS, `go test -race ./internal/cpu` PASS (45.750 с). Прежние F2XM1/FPATAN/FXTRACT/FPREM и сохранённые эталоны проходят. Strict scoped change PASS.

Дополнительный native FXAM probe: `gcc -O2 -Wall -Wextra -Werror tools/fpu-reference/fxam.c -o /tmp/ai386-fxam`; результат C3/C2/C0 для zero/subnormal/pseudo-denormal/normal/infinity/QNaN/SNaN/unsupported: 4000/4400/4400/0400/0500/0100/0100/0000. Это дополнительная проверка исходной классификации, а не полный аппаратный эталон DX4.

Следующие задачи 2.2–2.7 сохраняют полные матрицы округления, конверсий, сравнений, стека и окружения. CPU 6/29, roadmap 39/52.
