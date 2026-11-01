# Статус

- BIOS guest vectors + conventional BDA: исправлены и покрыты регрессиями.
- ROM chaining FLAGS: IRQ0 и VM86 paging независимый review PASS.
- CPU: HIMEMX unreal-mode retry regression RED→GREEN; полный CPU и независимые boundaries PASS.
- Полные проверки после всех исправлений: make check PASS, Windows headless/desktop и Linux desktop builds PASS. FreeDOS fixture TestFreeDOSInstallMenu PASS (19.532s).
- Приёмка: FreeDOS kernel + HIMEMX + FreeCom + UDVD2 + SHSUCDX загружены. Оригинальная .Error! не появляется.
- INT10 AH08: исправлен, BIOS/VGA tests и независимый review PASS; реальный экран языка и подтверждения читаемый.
- AH43 EDD write и hd_size исправлены; независимый review + make check + builds PASS. FreeFDISK /AUTO реально создал active FAT16 раздел на временном HD256MiB. Форматирование FAT16 подтверждено по BPB/OEM FRDOS4.1/signature55AA. После FORMAT installer попросил reboot для чтенияC:, выполняется новый boot с LiveCD. Копирование FreeDOS и bootHD ещё проверяются.

- AH15 disk type: регрессии RED→GREEN, VINFO C: fixed после reboot; independent review PASS.
- INT4B VDS: trace подтвердил переход UDVD2 INT4B AX8103 в null IVT. CF1 unavailable fallback исправлен, regression RED→GREEN, independent review PASS.
- Финальные gates после VDS: make check PASS; Linux desktop/Windows headless+desktop PASS; debug instrumentation удалена.
- Полная приёмка: official installer прошёл STAGE600/700 и STAGE800, реально копирует BASE пакеты на изолированный HD256MiB (ядро, FreeCom и первые utility packages записаны). Ожидается завершение и проверка HD boot.

- Полная установка завершена: official BASE 65 packages, MBR/VBR/kernel/shell; screenshot ai386-install-complete.png.
- HD default JEMMEX boot обнаружил live NT неочищенный interrupt/trap gates. Regression VM86 INT2F/IRETD RED→GREEN; независимый review PASS (16/32 bit, same/outer privilege, task gates).
- Default JEMMEX + UDVD2/SHSUCDX HD boot PASS до C:\>; screenshot ai386-hd-boot.png.
- Готовый образ freedos-ready.img с отдельной конфигурацией freedos-ready.toml. Doom скопирован гостевой COPY с E: на C:\DOOM; все 20 файлов проверены SHA-256. Original freedos.img untouched. Игра не запускалась.

- Итоговые gates: make check PASS (lint, unit, race, build, OpenSpec 10/10); Windows headless+desktop и Linux desktop build PASS; real ISO regression TestFreeDOSInstallMenu PASS 21.736s.
