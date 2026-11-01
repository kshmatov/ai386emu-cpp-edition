# FreeBSD4.11: HD и сохранность данных

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

Принято 2026-09-11. Оригинальный miniinst ISO из scenarios.md, RAM64МиБ, отдельный raw-HD2147483648 байт `/tmp/ai386-freebsd-io-hd.img`. Standard/Minimal installation: Standard MBR, автоматический disklabel, bin+crypto, без дополнительных пакетов. Исходные неудачные образы не используются и сохранены отдельно.

## Воспроизведение

```sh
go test -c -o /tmp/ai386-descriptor-test ./internal/machine
/tmp/ai386-descriptor-test -test.run TestFreeBSDDiagnostic -test.v -test.timeout=2h -freebsd-image=/tmp/ai386-freebsd-4.11-miniinst.iso -freebsd-hd=/tmp/ai386-freebsd-io-hd.img -freebsd-installed -freebsd-control -freebsd-steps=2000000000 -freebsd-shared=/tmp/ai386-cd-acceptance-1 -freebsd-shared=/tmp/ai386-cd-acceptance-2
```

`-freebsd-installed` проверяет BIOS DL=80h, затем вывод ядра и login: загрузка с CD вместо HD не засчитывается. На `CONTROL READY` JSON keys/events подаёт set1 scan codes через PS/2; steps исполняет гостя. root имеет пустой пароль в созданной тестовой установке. После входа:

```sh
echo ai386persist > /root/ai386-persist.txt
cat /root/ai386-persist.txt
sync
shutdown -h now
```

Увидены `ai386persist`, `syncing disks... done`, `The operating system has halted`. Только после halt отправлен `{"quit":true}`. Первый тест PASS428.14с.

Второй независимый запуск того же бинарника и HD: все UFS-разделы `FILESYSTEM CLEAN; SKIPPING CHECKS`. Вход root, команды:

```sh
cat /root/ai386-persist.txt
echo ai386ready
```

Вывод `ai386persist`, `ai386ready` и повторное приглашение `#`. Снимок `/tmp/ai386-freebsd-persistence.png` проверен визуально. Это подтверждает рабочую сессию: HD boot, ввод PS/2, VGA, запись/чтение и сохранность после перезапуска. После проверки снова выполнены sync/shutdown, гостевой halt подтверждён; повторный тест PASS265.84с.

## Исправления и границы

E820, restart INS/OUTS после page fault, ATA DSC, BDA475, x87 #MF и supervisor accessed-bit GDT/LDT приняты отдельными changes. Последнее исправление устранило SIGSEGV csh в signal trampoline; sendmail на принятом запуске не повторяет прежние падения.

Три CD дополнительно прочитаны, shared read-only подтверждён в attach-all-cds/acceptance.md. Это не подменяет ещё открытую полноту ISA/x87, звук, сеть и общую приёмку проекта. Test/vet/build успешны; CPU race44.768с, config/machine race1.314с, OpenSpec strict55/55.
