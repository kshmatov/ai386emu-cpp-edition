# Design: SB16

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

Объединение design-решений по системе звука. Мотивация — proposal.md.
Исходные scoped changes указаны в разделах; полные задачи — tasks.md.

## 1. Базовая реализация (sb16-audio)

### Context

Гостевые ОС приёмки (DSL 4.4.10 / OSS, FreeBSD 4.11 / snd_sb16) не находят
SB16; без 8237 DMA невозможна передача аудиоданных. Разведка гостей —
`recon-guests.md`.

### Decisions

- **DMA 8237** (`internal/dma`): пара каскадированных микросхем, каналы 0–7,
  flip-flop, страничные регистры, single-cycle/block/auto-init, TC-callback.
  16-битные каналы: счёт в словах, адрес `+2`, физадрес `(page<<16)|(current<<1)`.
  Интеграция только при `sb16=true`.
- **DSP** (`internal/sb16`): порты base..base+0x0F; сброс (base+6 1→0 → 0xAA);
  E1h версия 4.13; команды 40h/41h/42h/48h; D0h/D5h/D4h/D6h/D8h/D9h/DAh;
  DMA-команды 14h/24h/1Ch и семейство B0h–CFh (бит3 ADC, бит2 auto, бит1 FIFO;
  fmt stereo 0x20, signed 0x10). Квирк: `block<<stereo` для single-cycle.
  Темп по `Advance` виртуального времени; TC → IRQ + флаги 0x82; ACK чтением
  0x0E/0x0F.
- **Mixer CT1745**: индекс base+4 / данные base+5; сброс 0x00; регистры
  0x30–0x47, 0x80/0x81; статус 0x82 RO. IRQ/DMA-конфиг — probe без смены
  фактических линий.
- **Бэкенды** (`internal/audio`): интерфейс Backend + кольцевые буферы;
  file/off всегда; Linux (aplay/arecord, build tag linux); Windows
  (winmm waveOut/waveIn, build tag windows, без cgo); generic → file/off.
  auto → off с предупреждением при отсутствии утилит.
- **Конфиг/интеграция**: `WithSB16`, PIC IRQ, виртуальное время (база
  `pit.Frequency`), `Stop` закрывает бэкенд.

### Risks

- [Расхождение команд с реальной картой] → сверка с QEMU hw/audio/sb16.c и
  драйверами Linux 2.4.37 / FreeBSD 4.11 (задача 2.1).
- [Потактовая точность DSP] → вне объёма; детерминированный виртуальный темп.

## 2. Начальные уровни микшера (doom-sb16-mixer-defaults)

### Корень

`resetMixer` задавал master/voice = 4/31 (из 31). Произведение громкостей
16/961 обнуляло амплитуды до 58 → unsigned8 PCM оставался 80h (тишина),
хотя DMA-кольцо содержало отсчёты 70–186. Doom доверяет сбросу и не пишет
громкость.

### Решение

Master/voice после создания карты и mixer reset по CT1745: modern level 24,
legacy level 12. Регрессия на реальном DMA/DSP-тракте с диапазоном отсчётов
гостевого эффекта.

## 3. Громкость unsigned16 (sb16-unsigned16-volume)

Mute unsigned16 выдавал 0000 вместо нейтрального 8000. Решение: масштабирование
относительно midpoint 32768 (8000h); attenuation уменьшает амплитуды выше и
ниже 8000h симметрично.

## 4. Разбиение времени потока (sb16-stream-partitioning)

Малые шаги времени теряли 16-битные отсчёты DMA; разбиение stereo меняло
выбор громкости правого канала. Решение: сохранять кредит неполного
DMA-слова и позицию канала stereo-потока между порциями `Advance`.

## 5. Тишина записи (audio-capture-silence)

Capture без источника и после EOF заполнял буфер нулями — для unsigned PCM
это отрицательная предельная амплитуда. Решение: общий генератор тишины
учитывает разрядность, знак и смещение неполного отсчёта; File, Off, Linux
и Windows используют его при отсутствии входных данных.

## Non-goals

FM-синтез OPL2/OPL3, MIDI/MPU-401, CSP/ASP, ADPCM, игровой порт,
потактовая точность DSP/DMA, аналоговая dB-шкала, входящий микс сверх
хранения регистров и масштабирования.

## Источники

`sb16-audio` (design 21.8K, задачи 0–5), `doom-sb16-mixer-defaults`,
`sb16-unsigned16-volume`, `sb16-stream-partitioning`, `audio-capture-silence`.
