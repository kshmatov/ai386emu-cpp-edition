# Приёмка Debian VGA16 framebuffer

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

Дата: 2026-09-23. Ветка: `feature/guest-slitaz-debian`. Go: `go1.26.7 linux/amd64`.

## Образ и конфигурация

- ISO: `debian-31r8-i386-netinst.iso` (Debian 3.1r8 Sarge i386 netinst).
- SHA-256: `5cee250e89b2993cf9053c74602868f45b6d1b1386d41803dba42c6622a4558e`.
- RAM: 128 МиБ.
- Порядок загрузки: CD, затем HD.
- Команда ISOLINUX: ровно `linux`; параметр `debian-installer/framebuffer=false` не использовался.
- Лимит: 1 600 000 000 инструкций.

Воспроизводящий запуск:

```sh
go test ./internal/machine -run '^TestAdditionalLinuxDiagnostic$' -count=1 -timeout=45m -v -args \
  -additional-linux-kind=debian \
  -additional-linux-image=/tmp/ai386-guest-distros/debian-31r8-i386-netinst.iso \
  -additional-linux-boot-command=linux \
  -additional-linux-framebuffer-dir=/tmp/ai386-guest-distros/debian-vga16-accepted \
  -additional-linux-steps=1600000000
```

Перед стартом тест проверяет SHA-256 ISO. Обычный `go test ./...` не требует образ и пропускает этот длительный сценарий.

## Результат

Тест прошёл за 592,16 с. Все кадры имели разрешение 640×480, оставались читаемыми и принимали PS/2 Enter. Между этапами тест отвергал загрузочный лог, пустую панель, status bar и частично перерисованные кадры; следующий экран принимался только после устойчивого содержательного изменения.

| Этап | Инструкции | PNG | SHA-256 PNG |
|---|---:|---|---|
| `Choose a language` | 1 120 000 000 | `/tmp/ai386-guest-distros/debian-vga16-accepted/debian-framebuffer-language.png` | `53a01a9c5dee7bc667025b77e2104aaec0a3c90dcdcf06c1f36e454b8282065f` |
| `Choose a country, territory or area` | 1 268 000 000 | `/tmp/ai386-guest-distros/debian-vga16-accepted/debian-framebuffer-country.png` | `bb4c73541c91f734141eee2ef3e129a6defaec2c84b85f2b804953567a462ff7` |
| `Keymap to use` | 1 419 000 000 | `/tmp/ai386-guest-distros/debian-vga16-accepted/debian-framebuffer-keyboard.png` | `769b98cc0d2e82167c84085d505ae983b76c7b7132d1dafd31201c31d57dad72` |

Снимки просмотрены вручную после успешного прогона: выделены соответственно `English`, `United States` и `American English`. Этот сценарий подтверждает исправление framebuffer установщика, но не завершённую установку Debian на HD.

## Причина и проверяемое исправление

Linux `vga16fb` включает `Attribute Mode Control.P54S` и записывает в AC palette значения вроде `20h`, `30h` и `38h`. Рендерер раньше напрямую использовал их как индекс DAC, хотя при P54S биты 5–4 должны заменяться `Color Select[1:0]`. В результате VRAM содержал интерфейс, но большая часть цветов читалась из нулевых ячеек DAC и кадр выглядел чёрным.

Исправление реализует P54S/Color Select, а также полную адресацию графического кадра через CRTC Start Address/Offset и VGA Read Mode 1. Быстрые регрессионные тесты отдельно покрывают точное состояние палитры Linux VGA16, BOGL/fbcon write modes, CPU→VGA wide store и структурные критерии приёмки.
