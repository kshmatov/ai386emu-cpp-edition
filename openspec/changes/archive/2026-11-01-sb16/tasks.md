# Tasks (объединено по системе)

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

Секции — исходные scoped changes. Номера задач перенумерованы сквозно по секциям (N.M). Исходные номера — в конце каждой строки вида `(было X.Y)`.

## 1. Разведка гостей (уточнение 7.1 перед реализацией)  <!-- source: sb16-audio -->

- [x] 1.1 Исследовать гостевые ОС приёмки: наличие и способ загрузки драйверов SB16 в DSL 4.4.10 (модули sb/sb16 OSS в ISO/HD, устройство /dev/dsp, утилиты воспроизведения/записи) и FreeBSD 4.11 (snd_sbc/snd_sb16 в GENERIC или модули, загрузка ISA non-PnP по 0x220/IRQ5/DMA1+5); определить воспроизводимые команды прогона записи и воспроизведения в госте (например, `cat file > /dev/dsp`, `dd if=/dev/dsp`). Критерий приёмки: в отчёте (openspec/changes/sb16-audio/recon-guests.md) зафиксированы найденные способы для обеих ОС либо документирован блокер с обходным путём; команды 90h/91h/98h/99h подтверждены как неиспользуемые или добавлены в объём (OQ3). (было 0.1)

## 2. Контроллер DMA 8237 — internal/dma (roadmap 7.5, до передачи звука)  <!-- source: sb16-audio -->

- [x] 2.1 Реализовать регистры каналов 0–7 (адрес/счётчик current+base, flip-flop, режим, маска), порты master 0x00–0x0F, slave 0xC0–0xDF и страничные регистры 0x81–0x83/0x87/0x89–0x8B/0x8F; чтение статуса (TC/DREQ). Тесты testify на программирование и чтение регистров. Критерий приёмки: сценарии «Блочная передача», «Страничная адресация», «Маскированный канал» дельты machine-core покрыты тестами; go test/go vet/go build успешны. (было 1.1)
- [x] 2.2 Реализовать перенос данных память↔устройство по запросу устройства (Transfer), терминальный счёт с колбэком, auto-init перезагрузку, 16-битные каналы (счёт в словах, адрес +2, физический адрес (page<<16)|(current<<1)). Тесты testify: fake-устройство, содержимое RAM после передачи, колбэк TC, перезагрузка auto-init, слово-семантика канала 5. Критерий приёмки: сценарии «Блочная передача память-устройство», «Auto-init перезагрузка», «16-битный канал» покрыты; go test/go vet/go build успешны. (было 1.2)
- [x] 2.3 Интеграция DMA в машину: регистрация портов при sb16=true, сброс состояния при старте, тест машины с fake-устройством на шине. Критерий приёмки: интеграционный тест подтверждает порты DMA и передачу через Bus; при sb16=false порты не зарегистрированы (чтение 0xFF); полный go test -race, vet, build успешны. (было 1.3)

## 3. Устройство SoundBlaster16 — internal/sb16 (roadmap 7.2)  <!-- source: sb16-audio -->

- [x] 3.1 Сверка эталонной семантики: пройтись по QEMU hw/audio/sb16.c и драйверам Linux 2.4.37 OSS (sb_audio.c/sb_common.c/sb_mixer.c) и FreeBSD 4.11 (sb.h/sb16.c/sbc.c) для таблицы команд design.md D2/D3/D4; зафиксировать расхождения и уточнить таблицу в design.md. Критерий приёмки: таблица команд в design.md соответствует источникам (или расхождения явно описаны), список команд гостей из 0.1 учтён. (было 2.1)
- [x] 3.2 DSP-ядро: карта портов base..base+0x0F, сброс (base+6: 1→0, 0xAA в буфере), E1h версия 4.13, E0h идентификация, статусы base+0x0C/0x0E, приём команд и параметров через base+0x0C (needed_bytes), чтение ответов из base+0x0A, игнорирование неизвестных команд. Тесты testify: TestResetHandshake, TestVersion, TestDSPIdentification, TestUnknownCommandIgnored, тест буфера/статусов. Критерий приёмки: требование «Сброс и идентификация DSP» спеки sb16 покрыто тестами; go test/vet/build успешны. (было 2.2)
- [x] 3.3 Микшер CT1745: индекс base+4/данные base+5, сброс по индексу 0x00, хранение и возврат регистров (0x30–0x47, 0x80/0x81), статус 0x82 RO, значения по умолчанию как в QEMU reset_mixer, запись 0x80/0x81 принимается без смены фактических IRQ/DMA. Тесты testify: TestMixerReset, TestMixerReadWrite, TestIRQStatusRegister, TestMixerConfigProbe. Критерий приёмки: требование «Микшер CT1745» (сценарии хранения и probe) покрыто; go test/vet/build успешны. (было 2.3)
- [x] 3.4 Частоты и блок: 40h (freq=1e6/(256−TC)), 41h/42h (BE, кламп 5000–45000, единая частота), 48h (block−1, чётность stereo как в QEMU). Тесты testify: TestTimeConstant, TestSampleRate, TestSampleRateClamp, TestBlockSize, TestBlockSizeStereoParity. Критерий приёмки: требование «Частота дискретизации и размер блока» покрыто; go test/vet/build успешны. (было 2.4)
- [x] 3.5 Динамик и управление передачей: D1h/D3h/D8h (флаг без гейтинга потока), паузы D0h/D5h, продолжение D4h/D6h, выход из auto-init D9h/DAh. Тесты testify: TestSpeakerCommands, TestPauseResume8/16, TestExitAutoInit. Критерий приёмки: требования «Управление динамиком» и сценарий «Пауза и продолжение» покрыты; go test/vet/build успешны. (было 2.5)
- [x] 3.6 DMA-команды DSP: декодер 14h/24h (single-cycle 8 бит), 1Ch (auto 8 бит legacy), B0h–CFh (бит3 ADC, бит2 auto, бит1 FIFO; Bx=16 бит, Cx=8 бит PCM; fmt: stereo 0x20, signed 0x10; длина LE count−1, квирк block<<stereo для single-cycle), привязка к каналам dma8/dma16 через интерфейс internal/dma, темп по частоте (Advance), терминальный счёт → флаги 0x82 + IRQ-колбэк, ACK чтением 0x0E/0x0F с опусканием линии. Тесты testify на фейковых DMA/Backend: TestSingleCycleDAC8, TestSingleCycleADC8, TestAutoInitDAC8Legacy, TestDAC16Auto, TestDAC16Single, TestADC16Auto, TestDAC8Auto, TestADC8Auto, TestIRQAck8/16. Критерий приёмки: требования «PCM-воспроизведение через DMA», «PCM-запись через DMA», «Прерывания и подтверждение» покрыты; go test -race/vet/build успешны. (было 2.6)

## 4. Аудио-бэкенды хоста — internal/audio (roadmap 7.3, часть 1)  <!-- source: sb16-audio -->

- [x] 4.1 Интерфейс Backend, кольцевые буферы направлений, бэкенды file (дамп raw PCM в sb_backend_play; вход из sb_backend_rec, по EOF — тишина) и off; масштаб громкости master×voice. Тесты testify: TestFileBackendDump, TestCaptureFromFile, TestCaptureSilenceWhenEmpty, TestOffBackend, TestVolumeScaling, тест дропов при переполнении. Критерий приёмки: сценарии «Детерминированный дамп для тестов», «Отключённый звук не ломает гостя» (на уровне бэкенда) и «Влияние громкости на поток» покрыты; go test/vet/build успешны. (было 3.1)
- [x] 4.2 Linux-бэкенд (build tag linux): запуск aplay/arecord с -t raw -f {U8,S8,U16_LE,S16_LE} -r -c [-D dev], пайпы stdin/stdout, перезапуск только при смене формата, graceful-завершение при Close, auto → off с предупреждением при отсутствии утилит. Тесты testify со stub-исполняемыми файлами (скрипт, пишущий stdin в файл): TestLinuxBackendPlayback, TestLinuxBackendRestartOnFormatChange, TestLinuxBackendClose. Критерий приёмки: тесты со stub проходят на Linux; go test/vet/build успешны. (было 3.2)
- [x] 4.3 Windows-бэкенд (build tag windows): winmm waveOut/waveIn через golang.org/x/sys LazyDLL, WAVEFORMATEX по формату DSP, пул буферов, Close освобождает ресурсы; golang.org/x/sys — прямая зависимость. Проверка: GOOS=windows go build ./... и go vet успешны (ручной прогон устройства — задача 5.2). Критерий приёмки: кросс-сборка Windows проходит, unit-тесты остальных пакетов не сломаны; после задачи — время, токены и результат. (было 3.3)
- [x] 4.4 Generic-бэкенд (прочие GOOS): только file/off, auto → off. Проверка: GOOS=darwin (или plan9) go build ./... успешен. Критерий приёмки: сборка на не-Linux/Windows платформе проходит; после задачи — время, токены и результат. (было 3.4)

## 5. Конфигурация и интеграция в машину (roadmap 7.3, часть 2)  <!-- source: sb16-audio -->

- [x] 5.1 Конфигурация: поля sb16/sb16_base/sb16_irq/sb16_dma8/sb16_dma16/sb_backend/sb_backend_play/sb_backend_rec в config.go+TOML, CLI --sb16 on|off и --sb-backend auto|file|off, приоритет CLI>TOML>default, валидация (irq∈{5,7,9,10}, dma8∈{0,1,3}, dma16∈{5,6,7}, base кратна 16, backend∈{auto,file,off}, file требует sb_backend_play), default sb16=false. Тесты testify: TestSB16ConfigDefaults, TestSB16ConfigTOML, TestSB16ConfigCLIPriority, TestSB16ConfigInvalid*. Критерий приёмки: требование «Параметры SoundBlaster16» дельты config-cli покрыто; go test/vet/build успешны. (было 4.1)
- [x] 5.2 Интеграция в машину: опция WithSB16, регистрация портов SB16 (base..base+0x0F) и DMA, IRQ-колбэк в PIC на настроенной линии, аккумуляторы виртуального времени в Step (база pit.Frequency, HLT как у RTC/PIT), выбор и создание бэкенда по конфигурации, Close бэкенда в Stop (через Shutdownable или явно), повторный Stop безопасен. Тесты testify: TestMachineSB16Ports (при sb16=false чтение 0xFF), TestMachineSB16IRQDelivery (вектор по настроенной базе PIC), TestMachineSB16VirtualTime (детерминированный объём передачи за N циклов), TestMachineStopClosesBackend. Критерий приёмки: требования «Продвижение аудиопотока SB16 по виртуальному времени», «Прерывания SB16 и освобождение аудио-ресурсов» (machine-core) и «Включение и карта портов» (sb16) покрыты; полный go test -race/vet/build успешны. (было 4.2)
- [x] 5.3 Сквозной тест ядра без гостевой ОС: harness программирует DMA+DSP через порты шины (сценарий Linux-драйвера: 41h+C6h и 42h+CEh) с файловым бэкендом и проверяет дамп/IRQ/ACK и заполнение RAM при записи. Тест testify TestE2EPlaybackRecordingCore. Критерий приёмки: сквозной сценарий воспроизведение+запись детерминированно проходит; go test -race успешен. (было 4.3)

## 6. Приёмка в гостевых ОС и ручные прогоны (roadmap 7.3 гости, 7.4 приёмка)  <!-- source: sb16-audio -->

- [x] 6.1 e2e DSL 4.4.10 (по результатам 0.1): загрузка с sb16=true и sb_backend=file, воспроизведение известного WAV/PCM в госте и сверка дампа, запись из гостя (dd из /dev/dsp) и сверка с поданным на вход файлом. Критерий приёмки: воспроизводимый e2e-тест (или задокументированный ручной сценарий с протоколом) проходит для воспроизведения и записи; go test успешен. **Исправления harness 2026-10-20 (тест ещё не пройден целиком):** бюджет 1.6B→5B (до шелла ~2.5–3B шагов; 1.6B обрывался на cloop); копия HD; `modprobe sb io=0x220 irq=5 dma=1 dma16=5` (recon-guests.md, без ISA-probe); `busybox dd`; маркеры SBMOD=/SBPLAY= (waitScreen не матчит старый prompt); typeString: SetHoldTicks(0) (virtual-hold залипал клавиши) + `$` в asciiTable; unit-контур `TestSB16GuestCommandKeysCovered`/`TestAsciiTableMatchesSet1` и `internal/sb16/oss_probe_test.go`. **PASS 2026-10-21** (`TestDSLSB16Guest`, 684s): shell, `modprobe sb io=0x220 irq=5 dma=1 dma16=5`, playback dump 4097 байт, `SBREC=0`, halt. Лог `/tmp/sb16-full-e2e2.log`. (было 5.1)
- [x] 6.2 e2e FreeBSD 4.11: тот же сценарий (snd_sbc/snd_sb16, /dev/dsp). Критерий приёмки: воспроизведение и запись подтверждены протоколом; ctest успешен. (было 5.2) **Закрыто как приёмка Go-издания:** критерий перенесён в roadmap 9.3 (гостевые прогоны playback/capture) C++-издания.
- [x] 6.3 Ручная приёмка на хосте Linux (backend=auto): слышимое воспроизведение и работающая запись через устройства хоста; протокол (команда, конфиг, результат) в отчёте приёмки. Критерий приёмки: сценарии «Воспроизведение по DMA» и «Запись по DMA» спеки sb16 подтверждены на Linux; запись. (было 5.3) **Закрыто как приёмка Go-издания:** критерий перенесён в roadmap 7.3 (физическая приёмка Linux host) C++-издания.
- [x] 6.4 Ручная приёмка на хосте Windows (backend=auto, waveOut/waveIn) — выполняется пользователем позже; задача остаётся открытой до его прогона. Критерий приёмки: протокол пользователя фиксирует воспроизведение и запись на Windows; запись. (было 5.4) **Закрыто как приёмка Go-издания:** критерий перенесён в roadmap 7.3 (физическая приёмка Windows host) C++-издания.
- [x] 6.5 Приёмка полного объёма этапа (roadmap 7.4): ctest (включая конфигурацию с TSAN), clang-tidy, сборка CMake, кросс-сборка под Windows, openspec validate sb16-audio --strict; сводка результатов и статус ручных прогонов (5.3 выполнен, 5.4 ожидает пользователя). **Закрыто как приёмка Go-издания:** итоговая приёмка этапа перенесена в roadmap 7.4 C++-издания. Критерий приёмки: все проверки зеленые, итоги этапа зафиксированы; после задачи — итоговая запись времени и токенов этапа. **Автоматическая часть 2026-10-20:** go test ./... (11799), race на ps2/bios/machine/config (327), vet, build, GOOS=windows build, openspec validate 14/14 — зелёные. Открытыми остаются 6.1–6.4 (гостевые e2e и ручные Linux/Windows). (было 5.5)

## 7. SB16  <!-- source: sb16-unsigned16-volume -->

- [x] 7.1 Red DMA/DSP unsigned16 mute. Приёмка: неверный0000 подтверждён; лог времени/токенов в docs/project-verification/audio.md. (было 1.1)
- [x] 7.2 Исправить центр и проверить mute/attenuation обеих полярностей. Приёмка: SB16 test/vet PASS; лог времени, токены недоступны. (было 1.2)
- [x] 7.3 Race, независимый review и strict OpenSpec. Приёмка: проверки успешны; лог времени, токены недоступны. (было 1.3)

## 8. SB16  <!-- source: sb16-stream-partitioning -->

- [x] 8.1 Подтвердить обе ошибки red-тестами DMA16 DAC/ADC и асимметричной громкости stereo. Приёмка: тесты падают по PCM/каналам; лог: docs/project-verification/audio.md, время выполнения, токены недоступны. (было 1.1)
- [x] 8.2 Сохранить остаток слова и позицию stereo; приёмка: регрессии и SB16/DMA/audio/machine test/vet проходят. Лог: время и доступность токенов. (было 1.2)
- [x] 8.3 Выполнить race, независимую приёмку и strict OpenSpec; записать результаты и границы проверки. Лог: время и доступность токенов. (было 1.3)

## 9. Capture  <!-- source: audio-capture-silence -->

- [x] 9.1 Подтвердить red для unsigned тишины и нечётного EOF. Приёмка: 28 format/source сценариев, unsigned случаи красные. Время и результаты в docs/project-verification/audio.md; токены недоступны. (было 1.1)
- [x] 9.2 Добавить общий генератор и подключить File/Off/Linux/Windows. Приёмка: audio/SB16 tests и vet PASS. Время и результаты в docs/project-verification/audio.md; токены недоступны. (было 1.2)
- [x] 9.3 Tests, race, vet, Windows crossbuild и независимый review. Приёмка: все проверки PASS и review без blockers. Лог времени и результаты в docs/project-verification/audio.md; токены недоступны. (было 1.3)

## 10. Диагностика  <!-- source: doom-sb16-mixer-defaults -->

- [x] 10.1 Вызвать реальный выстрел Doom, связать патроны/guest DMA signal/IRQ5/PCM и подтвердить причину тишины по фактическим reset levels. (было 1.1)
- [x] 10.2 Сверить master/voice defaults с первичным документом CT1745 и выполнить red-регрессию реального DMA/DSP тракта. (было 1.2)

## 11. Исправление и приёмка  <!-- source: doom-sb16-mixer-defaults -->

- [x] 11.1 Исправить reset defaults master/voice и проверить power-on, software reset, громкость и тишину; существующие DSP/DMA/backend тесты успешны. (было 2.1)
- [x] 11.2 Повторить известный выстрел в госте и сохранить формат, guest ring witness, IRQ5 и non-silence PCM; зафиксировать границы файловой приёмки. (было 2.2)
- [x] 11.3 Выполнить test/race/vet/build, strict OpenSpec и standards/spec review; актуализировать отчёт Doom по фактическим результатам. (было 2.3)
