# Исследование: дополнительные гостевые Linux

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

- Запрос: добавить SliTaz 4.0 Base и Debian 3.1 Sarge i386 к проверяемым гостевым ОС.
- Текущие гостевые проверки находятся в `internal/machine/dsl_diagnostic_test.go` и `freebsd_diagnostic_test.go`. Они используют локальные ISO только при явном флаге, поэтому обычный `go test ./...` не зависит от больших образов.
- Эмулятор принимает ISO как CD, HD как файл и параметр RAM через `machine.New`; пример CLI есть в `README.md`.
- SliTaz 4.0 официально поддерживает i486 и текстовый режим при 48 МиБ RAM: <https://www.slitaz.org/en/doc/releases/4.0/relnotes.en.html>. Официальный Base ISO: <https://mirror.slitaz.org/i486/stable/4.0/iso/flavors/slitaz-4.0-base.iso>.
- Debian 3.1 Sarge поддерживает i386; официальный архив CD: <https://cloudfront.debian.net/cdimage/archive/3.1_r8/i386/iso-cd/>. `netinst` образ 106 MiB; руководство описывает CD-загрузку установщика: <https://archive.debian.org/debian/dists/sarge/main/installer-i386/current/doc/manual/en/ch05s01.html>.
- Загруженные проверочные ISO лежат в `/tmp/ai386-guest-distros/`, вне репозитория. SHA-256: SliTaz `7f669e323fe19fc7521de7e4c48fc08a4906050a0ed6c477a2d7e7f4f8c2020c`; Debian `5cee250e89b2993cf9053c74602868f45b6d1b1386d41803dba42c6622a4558e`.
- В официальном файле `SHA256SUMS` для полного Debian CD `netinst` не перечислен; образ скачан по HTTPS с официального архива, его локальный SHA-256 записан для воспроизведения.

## Открытые вопросы

- Фактический рубеж загрузки каждого ISO в ai386 выясняется гостевым прогоном.
- Поддержка ISA NE2000 в конкретных образах выясняется внутри гостя; загрузка системы не считается доказательством работоспособности сети.
