# Proposal: SB16 — звуковая карта, DMA, бэкенды и совместимость Doom

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

## Why

Объединение работ по системе `sb16` (SoundBlaster16 + DMA 8237 + аудио-бэкенды):

1. **Базовая реализация** (`sb16-audio`): этап 7 маршрута — DMA 8237, DSP/Mixer
   CT1745, PCM playback/capture, хост-бэкенды (file/Linux/Windows/off),
   конфигурация и интеграция в машину.
2. **Совместимость и точность потока**:
   - `doom-sb16-mixer-defaults` — reset-значения master/voice CT1745;
   - `sb16-unsigned16-volume` — нейтральная точка unsigned16;
   - `sb16-stream-partitioning` — устойчивость к разбиению времени;
   - `audio-capture-silence` — корректная тишина записи в формате PCM.

## What Changes

- Контроллер DMA 8237 (master/slave, страницы, single-block/auto-init,
  терминальный счёт) до передачи звука (roadmap 7.5 → 7.3).
- SB16: DSP (сброс E1/E0, 40h/41h/42h/48h, D0h–D8h, DMA-команды 14h/24h/1Ch
  и B0h–CFh), микшер CT1745, IRQ на terminate count.
- Бэкенды: file (дамп PCM), Linux (aplay/arecord), Windows (winmm),
  generic/off; масштаб master×voice с учётом unsigned16 midpoint.
- Начальные уровни master/voice по CT1745 (modern 24, legacy 12).
- Поток сохраняет остаток DMA-слова и позицию stereo-канала при разбиении
  времени; capture дополняет недостающие байты тишиной формата.
- Конфиг: `sb16*`, `sb_backend*`, CLI `--sb16`, `--sb-backend`.

## Capabilities

### New Capabilities

- `sb16`: наблюдаемое поведение карты (DSP, DMA, mixer, IRQ).
- `audio`: контракт хост-бэкендов записи/воспроизведения.

### Modified Capabilities

- `config-cli`: параметры SB16 и выбор аудио-бэкенда.
- `machine-core`: DMA 8237 на шине, продвижение аудиопотока по виртуальному
  времени, прерывания SB16 и освобождение ресурсов.

## Impact

`internal/dma`, `internal/sb16`, `internal/audio`, `internal/machine`,
`internal/config`, гостевые прогоны DSL 4.4.10 / FreeBSD 4.11, FreeDOS/Doom.

## Источники

Объединены: `sb16-audio`, `sb16-unsigned16-volume`, `sb16-stream-partitioning`,
`audio-capture-silence`, `doom-sb16-mixer-defaults`.
