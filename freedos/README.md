# FreeDOS 1.4 для ai386

Комплект подготовлен из официального `FD14-LiveCD.zip`.

## Состав

- `FD14LIVE.iso` — загрузочный LiveCD и установщик;
- `FD14BOOT.img` — официальная загрузочная дискета 1,44 МБ, запасной вариант загрузки;
- `freedos.img` — новый пустой sparse raw-HD, 256 МиБ;
- `freedos-install.toml` — конфигурация установки для ai386;
- `doom-cd/` — готовый каталог Doom 1.9 Shareware, подключённый как CD;
- `doom19s.zip` — исходный shareware-дистрибутив.

## Запуск установки

Из корня репозитория:

```sh
go run -tags desktop ./cmd/ai386 --config=build/freedos/freedos-install.toml
```

В меню FreeDOS выберите установку на жёсткий диск и следуйте подсказкам. При запросе перезагрузки остановите VM и запустите её с загрузкой с HD:

```sh
go run -tags desktop ./cmd/ai386 \
  --config=build/freedos/freedos-install.toml --boot=hd
```

Чтобы снова начать с чистого диска, удалите только `build/freedos/freedos.img` и создайте новый файл командой `truncate -s 256M build/freedos/freedos.img`. Не используйте для этого существующий корневой `vm.img`.

## Контрольные суммы

```text
2020ff6bb681967fd6eff8f51ad2e5cd5ab4421165948cef4246e4f7fcaf6339  FD14-LiveCD.zip
c48a9dcf4b8e22f44e268a9879745f0bd88c061195ac584e6ef2deb0477f81fb  FD14LIVE.iso
ab6882e068ec2a47cf2f60bf0260d247310f2a4ed0050bcf749c54e80a08dd9e  FD14BOOT.img
```

Контрольная сумма ZIP опубликована FreeDOS: <https://www.freedos.org/download/verify.txt>.

## Doom 1.9 Shareware

Конфигурация автоматически подключает `doom-cd/` через `shared` как второй
CD-ROM (Secondary Master). Официальные игровые файлы извлечены из
`doom19s.zip`: объединены `DOOMS_19.1` и `DOOMS_19.2`, затем распакован
полученный самораспаковывающийся ZIP. Исходные документы игры сохранены.

Источник: <https://www.gamers.org/games/doom/>.
Архив: <https://www.gamers.org/pub/idgames/idstuff/doom/doom19s.zip>.

После установки FreeDOS найдите CD по наличию `INSTALL.BAT` и каталога
`DOOM` (`DIR D:\`, `DIR E:\` и далее). Перейдите на найденный диск,
выполните `CD \`, затем `INSTALL.BAT`. Он скопирует игру в `C:\DOOM`.

```dos
C:
CD \DOOM
SET BLASTER=A220 I5 D1 H5 T6
SETUP
DOOM
```

SoundBlaster16 включён с backend `auto`. В `SETUP.EXE` выберите
Sound Blaster для эффектов, порт `220`, IRQ `5`, DMA `1`. Для музыки
выберите `None`: OPL/MIDI пока не реализованы. На Linux backend
использует утилиты `aplay` и `arecord`.
Запуск самой игры пока не проверен: текущий `freedos.img` ещё пустой.

Контрольные суммы SHA-256:

```text
cacf0142b31ca1af00796b4a0339e07992ac5f21bc3f81e7532fe1b5e1b486e6  doom19s.zip
b8020523561a5ad9706e009a52d61c578f37faafd85ac471962308406292ce27  doom-cd/DOOM/DOOM.EXE
1d7d43be501e67d927e415e0b8f3e29c3bf33075e859721816f652a526cac771  doom-cd/DOOM/DOOM1.WAD
```

Проверка комплекта из корня репозитория:

```sh
go test ./build/freedos -run TestFreeDOSDoomCD -count=1
```

Тест читает конфигурацию, проверяет подключение Secondary Master, обходит
файловую систему виртуального ISO и сравнивает каждый файл с оригиналом
на хосте, включая DOS-исполняемый файл и IWAD.
