# Review: reset master/voice SB16 для Doom

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

Проверено 2026-10-14. Объём: `internal/sb16/mixer.go`, новые mixer-regression
тесты, обновление defaults в `sb16_test.go`, guest witness
`tools/doom/guest_sound_test.go` и change `doom-sb16-mixer-defaults`.
Это review рабочего дерева; scope change не закрывается этим отчётом.

## Standards

Блокирующих нарушений `AGENTS.md` в production-изменении не найдено.
Исправление локально: два legacy-регистра и четыре modern-регистра задаются
в существующем `resetMixer`. Добавлены тесты testify с реальным DMA/DSP
трактом; новых абстракций, параметров и изменений публичного API нет.

Комментарии `mixer.go` различают Creative master/voice defaults и прочие
значения хранения из QEMU. Комментарий `TestVolumeAttenuatesPlayback`
соответствует новой громкости: master×voice составляет 576/961, примерно
59.9% в принятой линейной модели. Открытых замечаний по standards нет.

## Spec и первичный источник

По [Creative Hardware Programming Guide](https://www.phatcode.net/res/243/files/sbhwpg.pdf#page=73)
проверены раздел CT1745 и печатная страница 4-15. Modern master/voice имеют
исходный уровень 24, legacy master/voice — 12. Размещение полей подтверждается
картой регистров на печатной 4-12. Значения C0h и CCh соответствуют этим полям.
В нумерации PDF это индекс 72 с нуля, страница 73 в viewer; design указывает
обе величины и печатную 4-15 правильно.

Production diff изменяет ровно 04h,22h,30h,31h,32h,33h. Цикл исходной
инициализации 30h–47h остаётся прежним, последующий цикл ограничен 30h–33h;
другие регистры не получают новых значений. Power-on и software mixer reset
вызывают один `resetMixer`. Последующая запись reset register00h после сброса
сохраняет прежнее поведение. Записи гостя, IRQ/DMA/backend и `applyVolume`
не изменены: максимальная modern-громкость даёт коэффициент 1, нулевая — 0,
unsigned тишина 128 остаётся 128. Аппаратная dB-кривая и legacy aliases
по-прежнему не реализуются этим исправлением и явно исключены из change.

`TestMixerMasterAndVoiceResetLevels` проверяет все шесть регистров после
power-on и software reset. Во втором случае каждый регистр предварительно
получает отличное от default значение, затем запись в register00h запускает
сброс и readback проверяет каждый канал. No-op reset больше не может
удовлетворить этой проверке. PCM-регрессия отдельно сохраняет обе полярности
70/186 и тишину 128 в действительном DMA/DSP Playback. Открытых замечаний
по scoped spec нет.

## Границы guest witness

`doomDMABufferTrace.Read` установлен через `Bus.RegisterMemory`; CPU и
`dma.Controller.ToDevice` через `transfer` читают диапазон общим `Bus.ReadMem`.
Поэтому `ring_reads` и `nonSilentReads` считают обращения обоих инициаторов.
Они подтверждают чтение сигнала из диапазона, но не доказывают отдельное
количество DMA-чтений. Design явно указывает общие CPU/DMA-чтения и отсутствие
разделения инициаторов, корректно ограничивая это свидетельство.

Для звукового эффекта сильнее совместное свидетельство: Ctrl уменьшает
патроны, гость пишет сигнал в DMA-кольцо, DSP выбирает проверенный U8 stereo
11025Hz формат, приходит IRQ5, а новые байты файлового backend содержат
отсчёты обеих полярностей. В unit-регрессии источник непосредственно задан
DMA-буфером, и наблюдается фактический Playback. Один агрегатный read counter
без остальных свидетельств мог бы быть удовлетворён чтениями CPU.
Эта диагностика не доказывает физическую слышимость Linux/Windows backend.

## Проверки

Выполнено на проверяемом дереве:

```sh
GOCACHE=/tmp/ai386-status-go-cache go test -race ./internal/sb16 ./internal/dma ./internal/audio
GOCACHE=/tmp/ai386-status-go-cache go vet ./internal/sb16 ./internal/dma ./internal/audio
```

Результат финальной повторной race-проверки после устранения замечаний:
52 теста в трёх пакетах прошли; race и vet без ошибок.
Полные guest результаты и остальные project gates фиксирует приёмка;
этот review не заменяет их и не отмечает change завершённым.
