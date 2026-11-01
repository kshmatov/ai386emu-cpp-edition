# Подготовка FreeDOS 1.4 для ai386

Исходная задача: подготовить образ для загрузки и установки DOS 6.22 или аналогичной системы.

## Факты

- ai386 принимает raw-образ HD через `--hd`, размер нового диска через `--hd-size`, ISO через `--cd` и порядок загрузки через `--boot` (`README.md`, `internal/cli/cli.go`).
- BIOS ai386 поддерживает El Torito no-emulation и floppy-emulation (`internal/bios/eltorito.go`).
- Официальный сайт FreeDOS рекомендует FreeDOS 1.4 LiveCD для установки в виртуальной машине: <https://www.freedos.org/download/>.
- Официальная SHA-256 сумма архива `FD14-LiveCD.zip`: `2020ff6bb681967fd6eff8f51ad2e5cd5ab4421165948cef4246e4f7fcaf6339`: <https://www.freedos.org/download/verify.txt>.
- FreeDOS требует не менее 20 МБ HD; выбран диск 256 МиБ с запасом.

## Открытые вопросы

- Полная интерактивная установка внутри ai386 может выявить ещё не покрытые особенности CPU/BIOS. Это проверяется отдельным загрузочным прогоном после проверки носителя в QEMU.
