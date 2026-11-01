# Диагностика установки FreeDOS

Исходный сбой: Linux, `go run -tags desktop ./cmd/ai386 --config=build/freedos/freedos-install.toml`, выбор Install to harddisk, `.Error!`.

Факты:
- Воспроизведение на отдельном пустом HD: debug-install/repro.sh, исходный результат FAIL `.Error!`.
- IVT INT13 заменён MEMDISK на 9f80:000a; native hook игнорирует замену и возвращает AX=8001 CF=1. Журнал /tmp/ai386-freedos-int13-probe.log.
- После исправления векторов kernel загрузился, но PANIC MCB corrupted. INT12 возвращает 640 при BDA 638. Журнал /tmp/ai386-freedos-memory-probe.log.
- После исправления INT12 оба первых сбоя отсутствуют; выполнение дошло до HimemX и HMA buffers. Дальнейшая установка ещё не подтверждена.
- Целевые BIOS-тесты 88 PASS. Полный make check выполняется.

Неизвестное: причина длительного выполнения кода HimemX с LastException=13 после размещения буферов HMA.


Дополнительные воспроизведённые дефекты и результаты:
- HIMEMX #GP handler temporarily loads flat DS/ES, clears CR0.PE and POPs original selectors. CPU must preserve hidden real-mode data limits; prior reset and forced64K checks repeated #GP indefinitely. Exact guest-instruction regression RED→GREEN.
- FDI window repaint calls BIOS INT10 AH08 to read chars/attributes. Prior no-op destroyed text during repaint. Read/repaint regression RED→GREEN; language/welcome window now readable.
- FreeFDISK V1.4.3 /AUTO actual probe: AH43 AL0 DL80, DAP one block LBA63, writable HD. BIOS advertised EDD but lacked write, giving Error writing partition tables. AH43 regression RED→GREEN. Actual FreeFDISK /AUTO now writes MBR55AA and primary active FAT16 partition start63, sectors524097 on256MiB HD.
- Local config hd_size=256M was ignored (default1GiB). Explicit TOML key and legacy hdsize fallback now covered RED→GREEN.
- Full makecheck and Windows/Linux builds pass after EDD/config fixes; independent reviewers found no open functional issues.
