# DSL 4.4.10: сетевая приёмка

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

## Результат

Прогон 2026-09-21 выявил блокирующий дефект эмуляции NE2000 до получения
DHCP lease. DSL загрузилась с отдельной копии HD, строгая проверка shell
выполнила `echo ai386ready`, после чего через эмулируемую PS/2-клавиатуру была
запущена штатная загрузка Linux-драйвера:

```sh
modprobe ne io=0x300 irq=10
```

При чтении драйвером 32-байтного NE2000 PROM процесс эмулятора аварийно
завершился:

```text
panic: runtime error: index out of range [6] with length 6
gitverse.ru/kshmatov/ai386/internal/ne2000.(*Device).readDMA
    internal/ne2000/ne2000.go:179
gitverse.ru/kshmatov/ai386/internal/ne2000.(*Device).In
    internal/ne2000/ne2000.go:65
```

Причина локализована без изменения production-кода: ветка PROM использует
`d.mac[d.rsar/2]` для любого `d.rsar < 32`, хотя MAC содержит только шесть
байт. Реальный Linux-драйвер продолжает remote-DMA чтение PROM после первых
12 байт и достигает индекса 6.

Из-за panic не были достигнуты обнаружение `eth0`, DHCP, route,
`resolv.conf`, DNS, HTTP и HTTPS. Задача 4.3 этим прогоном не закрыта.

## Подготовка и команда

Базовый HD `/tmp/ai386-dsl-final-hd.img` не использовался напрямую. Рабочая
копия создана проверяющим скриптом в уникальном каталоге:

```sh
bash tools/freebsd-hd-prepare/prepare-freebsd-working-hd.sh \
  --expected-size 2147483648 --output-root /tmp \
  /tmp/ai386-dsl-final-hd.img

GOCACHE=/tmp/ai386-go-cache-dsl-network \
  go test -c -o /tmp/ai386-dsl-network.test ./internal/machine

sh tools/fpu-reference/guest/run-freebsd-session.sh \
  --log /tmp/ai386-freebsd-5.2a.MBfVoH/dsl-network.log \
  --lock /tmp/ai386-freebsd-5.2a.MBfVoH/freebsd-working.img.lock -- \
  /tmp/ai386-dsl-network.test \
  -test.run '^TestDSLDiagnostic$' -test.v -test.timeout=35m \
  -dsl-image=/home/kis/go/src/gitverse.ru/kshmatov/ai386/dsl-4.4.10.iso \
  -dsl-hd=/tmp/ai386-freebsd-5.2a.MBfVoH/freebsd-working.img \
  -dsl-installed -dsl-boot-steps=500000000 \
  -dsl-extra-steps=4000000000 -dsl-continue-faults \
  -dsl-require-shell -dsl-control-at-shell -guest-network
```

Сессия работала в интерактивном PTY. Команда `modprobe` была введена JSON
`events` control-loop, то есть каждое нажатие прошло через гостевой PS/2.
Процесс завершился status 2 через 991 секунду; runner записал окончательный
`AI386_FREEBSD_RUN_EXIT` и снял lock.

## Заранее выбранные внешние точки

До запуска были зафиксированы единые точки для DNS, HTTP и HTTPS:

- DNS: `example.com`;
- HTTP: `http://example.com/`;
- HTTPS: `https://example.com/`.

До обращений к ним прогон не дошёл. Для HTTPS планировался честный запуск
гостевого `/usr/bin/wget`: он связан с OpenSSL 0.9.7 и поддерживает максимум
TLS 1.0. Поэтому возможный отказ современного сервера по версии TLS должен
фиксироваться отдельно от прохождения DNS/TCP; отключение проверки
сертификата не устраняет несовместимость протокола.

## Целостность и свидетельства

Основа сборки: commit `febd10a0f9810cd6f11b00f74410ff32be446492` плюс
изменения рабочего дерева change `ne2000-usernat`.

| Артефакт | SHA-256 |
|---|---|
| DSL ISO `dsl-4.4.10.iso` | `6d9369c7b8fb0f23880c5ec44f5a4a2d9d628d7a1407ea83bf0b3249eda33537` |
| базовый HD `/tmp/ai386-dsl-final-hd.img` после прогона | `b629394156e45d59ecd1fc30442bb4282743d45a8140d2321bee5fc213c9f4af` |
| исходная рабочая копия (manifest) | `b629394156e45d59ecd1fc30442bb4282743d45a8140d2321bee5fc213c9f4af` |
| рабочая копия после аварии | `047f949c42926b1d4a4aa918985eaeeddc90dbaed38bb4fdd1c614fe2bdcae1b` |
| `/tmp/ai386-dsl-network.test` | `fabaa4a3d10c7eba18be041c1c5929858a7ab30d82775ef21c4840918670f0e0` |
| полный `dsl-network.log` | `a18810135ee597bdecb5bde56b03a62f60c0bfdaefeb032360f92b0b01dee9d1` |
| runner | `882af207f7f83a34c9751ecebea23b2bf44db32ae56240caf2fe42d18fae4102` |
| HD preparation script | `ca23f71a40602c7b2105285006ada0811befbd50b3f8bc20054badc0bae54b29` |

Manifest подготовки:
`/tmp/ai386-freebsd-5.2a.MBfVoH/preparation.manifest`.
Полный журнал:
`/tmp/ai386-freebsd-5.2a.MBfVoH/dsl-network.log`.

## Остановка и состояние дисков

Требуемый штатный `shutdown` и последующий `{"quit":true}` выполнить было
невозможно: panic завершил процесс эмулятора внутри исполнения гостевой
инструкции. Рабочая файловая система могла оставаться смонтированной на запись,
поэтому `/tmp/ai386-freebsd-5.2a.MBfVoH/freebsd-working.img` нельзя повторно
использовать как чистую основу. Базовый HD не менялся и сохранил исходный hash.

Для повторной приёмки после отдельного исправления NE2000 нужно создать новую
копию базового HD и повторить весь сценарий со штатным гостевым shutdown.
