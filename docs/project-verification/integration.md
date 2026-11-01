# Интеграционные проверки

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

2026-10-14, feature/freedos-doom. Точное потребление токенов недоступно.

## SB16 fixes
Подтверждены red до production: DMA16 малые шаги DAC/ADC; stereo выбор громкости; новая команда после нечётного блока; unsigned16 mute. Последние SB1638 tests/vet PASS. Дополнительный end-to-end DMA stereo8/16 tiny/bulk проходит через нечётные auto-init блоки с асимметричной громкостью и двумя IRQ.

SB16/DMA/audio/machine test+vet PASS171 до unsigned16; race PASS174 после unsigned16 (добавленная stereoodd-block регрессия позже, итоговый fullgate ещё впереди). Первоначальный machine run в sandbox не смог открыть localhost socket; повтор с разрешёнными сокетами прошёл, отказ не засчитывался. Scoped review audioagent обнаружил и проверил устранение stream restart blocker; unsigned16 review также PASS.

OpenSpec validate --all --strict:16/16 PASS в16:33UTC; формат новых SB16/helper/inbound файлов чистый. Два scoped SB16 changes завершены3/3 каждый. Время этапа примерно16:20–16:33UTC, токены недоступны.

## Общий gate
Production CPU/audio/network settled: общий go test ./... -count=1 и go test -race ./... -count=1 PASS, оба runner exit0. Полный go vet ./... PASS. Linux/Windows amd64 headless/desktop go build ./... и go build -tags desktop ./... PASS. Журналы /tmp/ai386-project-{full-test,full-race,vet,build-linux,build-linux-desktop,build-windows,build-windows-desktop}.log.

Кросс-сборки дали предупреждение о read-only metadata cache в /home/kis/go/pkg/mod; итоговые exit0, сборка завершена. CPU unit/race отдельно PASS50.351/206.831с, native/reference/conformance и реальные post-fix guests в cpu.md.

Тестовая проверка заголовков Debian добавлена после обычного полного gate: focused tests8PASS/vet; полный race уже включает новую проверку. Последующие helper-only изменения не меняют production snapshots.
