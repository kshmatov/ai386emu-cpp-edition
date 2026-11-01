# Проверка SB16 в реальных гостях

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

Дата: 2026-10-14. Состояние: **DSL и FreeBSD U8 mono file playback/capture PASS**.
Физические host audio устройства недоступны; этот пункт не закрыт.

| Гость | Playback | Capture | Завершение |
|---|---|---|---|
| DSL 4.4.10 | Полные 8192 байта + только U8-тишина | 8192 байта, guest memcmp и сохранённый файл cmp | Штатный halt, exit 0 |
| FreeBSD 4.11 | Полные 8192 байта + только U8-тишина, целый новый dump | 8192 байта, guest memcmp/cmp и host UFS extraction | Штатный halt, exit 0 |

Проверка использует новые копии исходных HD в
`/tmp/ai386-audio-verification/dsl.img` и `freebsd-full.img`. Оригиналы
`/tmp/ai386-{dsl,freebsd}-final-hd.img` открываются только для копирования
и SHA256. Эмулятор собран в `machine.test` в том же каталоге.

Строгий сценарий: известные 8192 ненулевых байта `share/source.pcm`,
file backend; гостевой OSS probe выбирает U8, mono, 8000 Hz и проверяет
readback всех трёх параметров. Master и PCM устанавливаются в 100%.
Playback выводит весь источник (финальный FreeBSD probe добавляет хвост U8-тишины)
и выполняет DSP SYNC, host dump сравнивается
с источником. Capture читает 8192 байта из `/dev/dsp`, сохраняет их на HD
и делает побайтовое сравнение с источником. Маркер принимается только
отдельной строкой экрана после запуска probe, затем требуется штатный halt.

Тестовая поддержка: `guest_audio_diagnostic_test.go` добавляет opt-in
file backend к существующим DSL/FreeBSD diagnostics; по умолчанию SB16
выключен. `TestGuestAudioOptions` проверяет наличие DSP handshake и файла
при включении, отсутствие порта при выключении. Проверки:
`go test ./internal/machine -run '^TestGuestAudioOptions$' -count=1` и
`go vet ./internal/machine` прошли с `GOCACHE=/tmp/ai386-status-go-cache`.

Уточнение разведки: наличие `snd_sbc.ko`, `snd_sb16.ko`, `snd_pcm.ko`
и узлов `/dev/dsp`, `/dev/mixer` на FreeBSD HD подтверждено чтением UFS.
Но прежний совет `device.hints` для FreeBSD 4.11 нельзя считать проверенным:
официальные исходники [ISA bus](https://github.com/freebsd/freebsd-src/blob/releng/4.11/sys/isa/isa_common.c)
и [snd_sbc](https://github.com/freebsd/freebsd-src/blob/releng/4.11/sys/dev/sound/isa/sbc.c)
используют старую схему создания ISA children; у sbc нет identify callback.
Штатная загрузка драйвера подтверждена итоговым прогоном ниже.

Физический host Linux: `/dev/snd` отсутствует. File backend не является
подтверждением слышимого вывода или записи с физического устройства.

## Разведочные запуски и исправления сценария

- Первая FreeBSD-сессия (`freebsd.log`) подтвердила наличие cc/gcc и пустой
  `Installed devices` после `kldload snd_sb16`. Временный Python controller
  затем завершился из-за превышения лимита канонической PTY-строки при
  передаче длинного JSON. Этот запуск не является приёмкой.
- На свежей `freebsd2.img` обе C-программы успешно скомпилированы штатным cc:
  82.9 млн инструкций для probe, 42.6 млн для KLD. Однако простая линковка
  `ld -Bshareable sbc.o` не создала SYSINIT set. Гостевой probe честно
  завершился `mixer: Device not configured`, `AI386_AUDIO_FAIL`.
  Это ошибка подготовки KLD, а не доказательство дефекта эмуляции.
- Исправление следует [штатному kmod.mk FreeBSD 4.11](https://github.com/freebsd/freebsd-src/blob/releng/4.11/sys/conf/kmod.mk):
  `ld -r`, `gensetdefs`, компиляция `setdef0.c`/`setdef1.c`, затем
  `ld -Bshareable setdef0.o sbc.kld setdef1.o`. Финальный повтор подготовлен
  в `freebsd3.img`, `share-final/`.

Повторяемые исходники сохранены в `tools/sb16-guest/`. Host verifier
сравнивает весь источник и допускает только U8-тишину 0x80 до/после него.
Пять проверок (точный поток, padding, тишина/нули, искажение/обрезка/повтор,
непригодный постоянный источник) прошли. Host-компиляция `probe.c`
с `gcc -Wall -Wextra -Werror -c` также прошла. Гостевые файлы исходного
прогона сохранены отдельно: `artifacts.sha256` фиксирует точные версии.
`aplay -l` и `arecord -l` оба ответили `no soundcards found`.

## DSL 4.4.10 — PASS U8 mono

`dsl.log`: `PASS 1370.58s`, процесс завершился с exit code 0.
Артефакты: `dsl.log.jsonl`, `dsl-audio.png`, `dsl-halt.png`, `dsl-play.pcm`.
Бинарник: `41b8c426668309825c00620eca90076493444e9c92d2141b2cf4d04f8a0599a3`.
Исходный PCM: `e54a7fde6e60678057fd18a8aa1eb703601e6e0d63df7ce8affe9184ae384044`.

Гость загрузился только с HD; strict shell check `echo ai386ready` прошёл.
Read-only shared CD смонтирован `mount -t iso9660 /dev/sr0 /mnt`.
Штатный OSS-драйвер загружен `modprobe sb io=0x220 irq=5 dma=1 dma16=5`.
Живой гость подтвердил отсутствие cc/gcc и наличие tcc. На этой копии
`/dev/mixer` уже существовал; попытка `mknod` вернула File exists, что
не затронуло действующий узел. C probe запущен через tcc с константами OSS,
как в README; текст фактической команды сохранён в логе и PNG.

Probe проверил U8/mono/8000 Hz readback, вывел `AI386_AUDIO_PLAY_DONE`,
`AI386_AUDIO_CAPTURE_EXACT_8192`, `AI386_AUDIO_OK`. Host control отдельно
проверил строку `\nAI386_AUDIO_OK `, затем гостевой `cmp` сохранённого
`/root/ai386-capture.pcm` с исходником на CD дал `AI386_AUDIO_FILE_OK`.
Этот маркер также проверен как отдельная строка, а не эхо команды.

Host verifier подтвердил **полное совпадение 8192 байт источника**,
без префикса; последующие 4101 байт дампа — исключительно U8-тишина 0x80.
SHA256 полного дампа:
`454fdafb3261b8a8e25397c213088762d3477153e5dd2fe0a0f5d1f641f47000`.
Затем выполнены `sync`, `shutdown -h now`; проверены `/dev/scd0 umounted`,
`/dev/hda1 umounted`, `DSL halted.`. Только после halt передан explicit quit.
Оба PNG визуально просмотрены. Это приёмка именно U8 mono, не физических
аудиоустройств и не остальных форматов SB16.

## Независимый review sb16-stream-partitioning

Подтверждены тестами две исходные проблемы: потеря времени неполного DMA16
слова при Advance(1), включая чтение/потерю ADC-источника без передачи;
и повторный выбор левого канала громкости на каждой части stereo PCM.
Root исправил сохранение времени полного слова и хранение следующего канала.

Первый review выявил blocker к scoped design: dmaCmd переиспользует stream,
поэтому новая DSP-команда сохраняла nextRight предыдущего нечётного блока.
Исправление: beginStream сбрасывает nextRight. Добавлена red/green регрессия
через реальный DMA: нечётный auto-init блок → pause → новая команда.
Повторный review подтверждает нужный сброс и сохранение фазы при pause,
auto-init и разбиении PCM; открытых high/blocking findings к этому изменению нет.
DSL U8 mono выполнен до этого исправления; обе изменённые ветви его формат
не затрагивают. Финальный FreeBSD U8 прогон использует новую сборку
`db9cd8254c6797cdb4d040e9a9f27c97819432ff64357b83f5501e418f88b18b`.

Дополнительно статический review обнаружил старую неправильную обработку
unsigned16 в applyVolume: ветка всегда трактовала sample как int16.
Root подтвердил это отдельным red-тестом реального DMA; исправление и
дополнительный scoped review выполняются отдельно от гостевого U8 сценария.

Unsigned16 fix также прошёл независимый review: перед масштабированием
вычитается 32768, после него центр возвращается; signed16 сохраняет прежнюю
семантику. Произведение амплитуды и двух коэффициентов 0..31 помещается в
int32; преобразование обратно в uint16 сохраняет нужные биты. Реальные
DMA/DSP-тесты проверяют mute для минимального/среднего/максимального sample
и известные значения 16384/32768/49152 при громкости 16/31:
24312/32768/41224. Замечаний high/blocking не осталось.
Независимый focused run пяти новых test-функций (word timing, stereo chunks,
new command, unsigned16 mute и attenuation) прошёл с `-count=1 -v`.

FreeBSD U8 mono final run уже шёл на сборке db9cd825… до unsigned16 fix;
новая правка касается только ветви is16 и не меняет его U8 mono контракт.
Это явно не гостевая приёмка unsigned16: тот формат проверен DMA-тестами.
Повторная SHA256-проверка обоих исходных HD после DSL дала OK.

## audio-capture-silence — red → green

2026-10-14, завершение проверок 16:41 UTC; время этапа около 5 минут.

Подтверждён отдельный дефект capture: File при EOF/отсутствующем источнике,
Off и пустая очередь Linux выдавали 00h для unsigned PCM. Это предельная
отрицательная амплитуда, а не тишина. Red сохранён в
`/tmp/ai386-audio-verification/capture-silence-red.log`: ожидаемые 80h для U8
и 00h,80h для U16 отличаются от фактических нулей.

Scoped change: `openspec/changes/audio-capture-silence`. Общий метод Format
заполняет тишину по знаку/разрядности и учитывает byte offset хвоста после
нечётного префикса источника. File сохраняет реальные байты; Off хранит
формат под mutex (factory и SB16 nil fallback теперь *Off); Linux получает
ring и format одним snapshot; Windows использует существующий mutex.

28 сценариев охватывают File/Off, отсутствующий/пустой источник, нечётный
ненулевой префикс и Linux ring без внешнего процесса, во всех четырёх
форматах. Повторный Capture после EOF тоже проверен. Старый Linux process
тест больше не распознаёт данные по признаку «байт не ноль»: он ждёт реальные
четыре входных байта и сравнивает их вместе с правильной U8-тишиной хвоста.

Проверки: `go test ./internal/audio ./internal/sb16 -count=1`, те же пакеты
с `-race`, `go vet`, `GOOS=windows GOARCH=amd64 go build` — PASS.
Логи `capture-silence-green.log`, `capture-silence-race.log`; OpenSpec strict
PASS. Независимый root review: форматная тишина, нечётный хвост File,
Off pointer/mutex/factories и snapshots Linux/Windows корректны; high/blocking
замечаний нет. Дополнительно Windows 386 и generic FreeBSD 386 build PASS.
Затраченные токены недоступны.
Реальные host audio устройства по-прежнему не проверены.

## FreeBSD: локализация ошибки ISA harness

Прогон `freebsd-final.log` после корректного gensetdefs действительно запустил
модуль, но получил kernel panic в kldload. Offline ELF исходного `/kernel`
точно сопоставляет EIP c02462ea с `resource_list_find+0x1a`, инструкцией
`mov (%eax),%eax` при нулевом адресе. Официальный
[isa_common.c FreeBSD 4.11](https://github.com/freebsd/freebsd-src/blob/releng/4.11/sys/isa/isa_common.c)
показывает причину: plain device_add_child не создаёт ISA ivars/resource list;
их создаёт isa_add_child через BUS_ADD_CHILD. Наш shim ошибочно обходил этот
вызов. Это локализованный дефект тестовой подготовки, а не доказательство
дефекта CPU или SB16. Прогон остановлен после panic и не засчитан.

Исправленный shim вызывает штатный KOBJ dispatch bus_add_child из
[bus_if.m](https://github.com/freebsd/freebsd-src/blob/releng/4.11/sys/kern/bus_if.m),
затем назначает ресурсы и запускает штатный драйвер. Начат повтор на новой
`freebsd-accepted.img`, shared CD `share-accepted`, журнал
`freebsd-accepted.log`. Бинарник включает все текущие SB16 и capture fixes:
`18b5c949e4f81f5bbe688c510dbee736b3c6a12af009bc6b6ce6fef7262c28cc`.
Приёмка FreeBSD до завершения этого повтора остаётся открытой.

Повтор `freebsd-accepted.log` выявил вторую ошибку harness, также до PCM:
EIP c0b774db соответствует add_isa_child+0x2b в скомпилированном sbc.ko,
при попытке KOBJ dispatch (read 0x60). В FreeBSD 4.11 device_t использует
старый device_ops ABI; наличие sys/kobj.h не означает совместимость этих
объектов. Подтверждение: bus.h из releng/4.11, символ BUS_ADD_CHILD
в исходном `/kernel` c0136ac0, disasm с чтением device_ops по device+0x14.
Исправленный shim вызывает экспортированную функцию BUS_ADD_CHILD напрямую
по сигнатуре bus_if.m. Прогон остановлен после panic, PCM acceptance не дан.
Следующий повтор: `freebsd-verified.img`, `share-verified`,
`freebsd-verified.log`; используется тот же бинарник 18b5c949….

## FreeBSD: полный сигнал и контракт закрытия PCM

Исправление BUS_ADD_CHILD подтверждено в `freebsd-verified.log`: штатный
sbc0/pcm0 обнаружил SB16 DSP 4.13, io 0x220, IRQ 5, DMA 1:5; строгий setup
marker прошёл после 167700000 инструкций. Probe подтвердил format readback,
запись точных 8192 байт, сохранение файла и отдельный guest cmp (strict
AI386_AUDIO_FILE_OK). Однако исходное воспроизведение отдало лишь первые
4098 байт: это сохранено как **FAIL** в `freebsd-verified-play-initial.pcm`.

Первичный источник объясняет результат: FreeBSD 4.11
[dsp.c](https://github.com/freebsd/freebsd-src/blob/releng/4.11/sys/dev/sound/pcm/dsp.c)
для DSP_SYNC ждёт только bufsoft; в
[channel.c](https://github.com/freebsd/freebsd-src/blob/releng/4.11/sys/dev/sound/pcm/channel.c)
chn_flush ждёт лишь пока resid > размер bufhard, затем прекращает DMA.
То есть close может отбросить последний аппаратный буфер 4096 байт.

Probe теперь записывает после полного источника 8192 байта явной U8-тишины
с проверкой полного write. Повтор playback в том же госте подтвердил
source8192 byte-exact + 4097 байт только 80h; сегмент сохранён отдельно
`freebsd-verified-pad-segment.pcm` с offset 4098 исходного общего dump.
Этот диагностический сегмент не подменяет полную приёмку. Production для
этого поведения не менялся. Host C compile -Wall -Wextra -Werror PASS.
Новый полный прогон на свежей `freebsd-full.img`, `share-full`,
`freebsd-full.log` использует финальный repo probe.c и новый File capture
source с начала; проверяется целый output dump, без выделения сегментов.


## FreeBSD 4.11 — окончательный PASS U8 mono

`freebsd-full.log`: PASS; wrapper exit status 0, 477 секунд. Штатный halt
подтверждён отдельной строкой `The operating system has halted`, после чего
передан explicit quit. `freebsd-full-audio.png` и `freebsd-full-halt.png`
визуально просмотрены. Финальная сборка эмулятора SHA:
`18b5c949e4f81f5bbe688c510dbee736b3c6a12af009bc6bce6fef7262c28cc`.

Свежий HD `freebsd-full.img` создан из неизменённого исходного образа.
Read-only CD `share-full` содержит финальные repo `probe.c`, `freebsd-sbc.c`
(как sbc.c), source.pcm и setup.sh. Исходники скомпилированы штатным гостевым
cc. После успешного setup выполнено:

```sh
/root/probe /mnt/source.pcm && cmp /root/ai386-capture.pcm /mnt/source.pcm && echo AI386_AUDIO_FILE_OK
sync
shutdown -h now
```

Strict control проверил отдельные строки setup и AI386_AUDIO_FILE_OK;
промежуточные AI386_AUDIO_PLAY_DONE, AI386_AUDIO_CAPTURE_EXACT_8192,
AI386_AUDIO_OK присутствуют. Формат U8/mono/8000 Hz проверяется ioctl
readback, master/PCM заданы 100%.

**Целый** `freebsd-full-play.pcm` содержит полный источник 8192 байта,
нулевой префикс и 4102 байта только 80h после него. Host verifier:
`PCM_EXACT_8192 silence_before=0 silence_after=4102`.
SHA256 dump: `2f60eb3c546227bdffc034debe54f123b29ba24d2e33056365b016f4fcbf3655`.

После halt реальный `/root/ai386-capture.pcm` извлечён из UFS рабочей копии
в `freebsd-full-capture.pcm`; длина 8192 и полное host-сравнение с source PASS.
Его SHA совпадает с source:
`e54a7fde6e60678057fd18a8aa1eb703601e6e0d63df7ce8affe9184ae384044`.
Это дополнительно к двум независимым сравнениям внутри гостя.

Точные источники закреплены в `full-share.sha256`:
probe.c `c727295afaf2474bfe509ed6007a0ac8425bfac41308ead1edcb15565bfa3105`,
sbc.c `0abf0991725cb17dc8addddfd33d2ee1c66785871909ca788201ef9ef4658d48`.
JSON-команды и полный guest output: `freebsd-full.log.jsonl`,
`freebsd-full.log`, wrapper lifecycle: `freebsd-full-wrapper.log`.
Входные артефакты и capture/playback сохранены в
`/tmp/ai386-audio-verification/`. Ни один частичный/неудачный прогон не заменяет
этот финальный whole-file результат. Исходный DSL PASS относится к ранней
версии probe без padding; там весь источник вышел точно и без padding.

Запуск финальной ВМ:

```sh
sh tools/fpu-reference/guest/run-freebsd-session.sh \
  --log /tmp/ai386-audio-verification/freebsd-full-wrapper.log \
  --lock /tmp/ai386-audio-verification/freebsd-full.lock -- \
  python3 /tmp/ai386-audio-verification/control.py \
  /tmp/ai386-audio-verification/freebsd-full.log \
  /tmp/ai386-audio-verification/machine-accepted.test \
  -test.run '^TestFreeBSDDiagnostic$' -test.v -test.timeout=2h \
  -freebsd-image=/tmp/ai386-freebsd-4.11-miniinst.iso \
  -freebsd-hd=/tmp/ai386-audio-verification/freebsd-full.img \
  -freebsd-installed -freebsd-control -freebsd-steps=2000000000 \
  -freebsd-shared=/tmp/ai386-audio-verification/share-full \
  -guest-audio-play=/tmp/ai386-audio-verification/freebsd-full-play.pcm \
  -guest-audio-rec=/tmp/ai386-audio-verification/share-full/source.pcm
```

Завершение приёмки 2026-10-14 около 17:04 UTC; токены недоступны.

Итоговый `sha256sum -c bases-before.sha256` после всех прогонов:
DSL source HD OK, FreeBSD source HD OK. `git diff --check`, повторный
vet audio/SB16/machine, OpenSpec strict и 5 host verifier tests PASS.
