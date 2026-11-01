# Приёмка RTC/CMOS — 2026-09-11

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

Реализованы порты 70h/71h, календарь RTC, форматы BCD/binary и 12/24h, UIP/SET, periodic/alarm/update события и IRQ8 через PIC. CMOS содержит память и фактическое наличие floppy; C/D read-only. Начальное время — UTC хоста либо `rtc_start`/`--rtc-start` RFC3339, затем часы идут по виртуальному времени.

Модульные тесты проверяют високосные годы, полночь и смену года, SET, форматы, частоты RS, reset делителя, флаги/разрешения, acknowledge, CMOS и checksum. Интеграционный тест доставляет IRQ8 обработчику гостя и пробуждает HLT. Конфигурационные тесты проверяют UTC, границы дат, CLI/TOML/default и отказ при явно пустом значении.

Строгий прогон оригинального DSL 4.4.10 ISO (SHA256 указан в [отчёте загрузки DSL](../../dsl-boot/boot-report.md)), RAM 64 МиБ:

```sh
go test ./internal/machine -run TestDSLDiagnostic -v -count=1 -timeout=35m \
  -dsl-image="$PWD/dsl-4.4.10.iso" -dsl-extra-steps=4000000000 \
  -dsl-continue-faults -dsl-stop-vector=6 -dsl-require-shell \
  -dsl-require-rtc -dsl-rtc-start=2026-09-11T12:00:00Z \
  -dsl-screen=/tmp/ai386-dsl-rtc.png
```

**PASS — 1046.79 с (17 мин 27 с).** Клавиатура PS/2 и видимый VGA подтвердили:

```text
root@tty1[/]# echo ai386ready
ai386ready
root@tty1[/]# date +%Y
2026
root@tty1[/]#
```

[Снимок экрана](dsl-rtc.png) проверен визуально. Проверки отсутствия предупреждений tar о датах в будущем и `unknown type 15` успешны. Сообщения hwsetup/modprobe об отсутствующем floppy-модуле и lspci об отсутствии PCI остаются; они не являются неверными данными CMOS.

Полные `go test ./...` и `go test -race ./...`, `go vet ./...`, `go build ./...` успешны. После дополнительного исправления пустого CLI-параметра повторены полный обычный прогон, race CLI/main, vet/build. Спецификации синхронизированы в machine-core и config-cli.

Вне этапа: сохранение NVRAM между процессами, DST, NMI-источники, BIOS INT1A. Точная скорость DX4, frontend, другие ОС и установка на HD этим этапом не принимаются.
