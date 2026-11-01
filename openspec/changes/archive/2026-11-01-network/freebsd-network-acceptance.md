# FreeBSD 4.11: приёмка сети

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

Дата: 2026-09-21, часовой пояс Europe/Moscow. Результат: **BLOCKED**, задача
4.3 не принята. Реальный probe NE2000 аварийно завершает эмулятор до создания
`ed0`; DHCP, DNS, HTTP и HTTPS из гостя поэтому не достигнуты.

## Среда и изоляция диска

Проверка выполнена на рабочем дереве с HEAD
`febd10a0f9810cd6f11b00f74410ff32be446492`. Актуальный test binary собран
непосредственно перед запуском:

```sh
GOCACHE=/tmp/ai386-go-cache-freebsd-network \
  go test -c -o /tmp/ai386-freebsd-network.LSLQvk/ai386-machine-test \
  ./internal/machine
```

Базовый установленный диск не передавался VM на запись. Для проверки создана
отдельная copy-on-write/обычная копия:

```sh
mkdir /tmp/ai386-freebsd-network.LSLQvk
cp --reflink=auto /tmp/ai386-freebsd-final-hd.img \
  /tmp/ai386-freebsd-network.LSLQvk/freebsd-working.img
```

SHA-256 базы до и после запусков одинаков:
`6aa54f7333f12c5998f6baf68ff1d0c1313cd1096a95b4c90988013fd3438831`.
Начальный SHA-256 рабочей копии совпадал с базой. После записи гостем
`/boot/kernel.conf` рабочая копия имеет SHA-256
`9598f364abf409cef2f8f3e6cdec2f83eb77290048c0c8b5b59b1b5a9ec0c98b`.

## Заранее выбранные внешние проверки

До сетевых команд зафиксированы следующие конечные точки:

- DNS: A-запрос `example.com` через полученный по DHCP resolver;
- HTTP: `http://example.com/`;
- современный HTTPS: `https://example.com/`;
- контроль старого TLS: `https://tls-v1-0.badssl.com:1010/`.

Последние две проверки должны были различить исправность TCP-пути и ожидаемое
ограничение TLS/корневых сертификатов FreeBSD 4.11. Ни один запрос к этим
узлам фактически не отправлен: VM упала раньше DHCP.

## Команда зачётного запуска

```sh
sh tools/fpu-reference/guest/run-freebsd-session.sh \
  --log /tmp/ai386-freebsd-network.LSLQvk/freebsd-network-final.log \
  --lock /tmp/ai386-freebsd-network.LSLQvk/freebsd-working.img.lock -- \
  /tmp/ai386-freebsd-network.LSLQvk/ai386-machine-test \
  -test.run '^TestFreeBSDDiagnostic$' -test.v -test.timeout=2h \
  -freebsd-image=/tmp/ai386-freebsd-4.11-miniinst.iso \
  -freebsd-hd=/tmp/ai386-freebsd-network.LSLQvk/freebsd-working.img \
  -freebsd-installed -freebsd-control -freebsd-steps=2000000000 \
  -guest-network
```

FreeBSD 4.11 GENERIC содержит `ed0`, но оставляет его выключенным и задаёт
port `0x280`, тогда как диагностический harness подключает карту на `0x300`.
Это подтверждено официальным исходником
`sys/i386/conf/GENERIC` ветки `releng/4.11`:
<https://github.com/freebsd/freebsd-src/blob/releng/4.11/sys/i386/conf/GENERIC>.
Поэтому на рабочей копии штатными средствами гостя создан
`/boot/kernel.conf`:

```text
enable ed0
port ed0 0x300
irq ed0 10
iomem ed0 0xd8000
```

При повторной загрузке журнал подтвердил применение файла:

```text
config> enable ed0
config> port ed0 0x300
config> irq ed0 10
config> iomem ed0 0xd8000
Preloaded userconfig_script "/boot/kernel.conf" at 0xc055c09c.
```

## Воспроизводимый дефект

Первое чтение PROM драйвером `ed0` приводит к panic хоста:

```text
panic: runtime error: index out of range [6] with length 6
gitverse.ru/kshmatov/ai386/internal/ne2000.(*Device).readDMA
    internal/ne2000/ne2000.go:179
gitverse.ru/kshmatov/ai386/internal/ne2000.(*Device).In
    internal/ne2000/ne2000.go:65
gitverse.ru/kshmatov/ai386/internal/cpu.(*CPU).execIOString
    internal/cpu/io.go:166
```

`readDMA` рассматривает весь диапазон PROM `rsar < 32` как дублированные
байты MAC и индексирует шестибайтный `d.mac` выражением `d.mac[d.rsar/2]`.
При `rsar == 12` получается индекс 6 и процесс падает. Это блокирующий дефект
эмулятора, а не ограничение TLS или внешней сети.

Зачётный процесс завершился со status 2 через 63 секунды. Полный неизменённый
журнал: `/tmp/ai386-freebsd-network.LSLQvk/freebsd-network-final.log`.

## Подготовительный запуск

До включения legacy-устройства FreeBSD штатно загрузился с HD и достиг login.
После входа root команда `ifconfig -a` показала только `faith0`, `lo0`, `ppp0`
и `sl0`; `grep ed0 /var/run/dmesg.boot` ничего не вывел. Это соответствует
выключенному `ed0` в GENERIC. Попытка `kldload if_ed` завершилась:

```text
module_register: module miibus/ukphy already exists!
linker_file_sysinit "miibus.ko" failed to register! 17
kldload: can't load if_ed: Exec format error
```

В этой подготовительной сессии `/boot/kernel.conf` был записан и выполнен
`sync`. Затем из-за опечатки PS/2 была введена команда `shutdown -h naw`, а
не `shutdown -h now`; control-проверка закономерно завершилась FAIL через
759 секунд. Этот запуск не считается приёмочным. Полный журнал сохранён в
`/tmp/ai386-freebsd-network.LSLQvk/freebsd-network.log`.

## Shutdown и ограничения результата

Требование «штатный guest shutdown, затем explicit quit» выполнить в зачётном
запуске невозможно: panic процесса эмулятора произошёл во время kernel probe,
до login и control-loop. После panic отправить гостю shutdown или JSON
`{"quit":true}` уже некому. Runner завершился, lock удалён; базовый HD не
изменён. Production-код в рамках этой приёмки намеренно не исправлялся.

Не подтверждены:

- создание `ed0` и MAC;
- DHCP-адрес, default route и `/etc/resolv.conf`;
- DNS lookup;
- исходящий HTTP и HTTPS;
- отбрасывание незатребованного входящего трафика;
- штатный shutdown после сетевой сессии.

После исправления PROM необходимо повторить ту же команду, проверить все
перечисленные пункты и завершить гостя `sync; shutdown -h now`; только после
строки `The operating system has halted` отправить `{"quit":true}`.

## Артефакты и хеши

| Артефакт | SHA-256 |
|---|---|
| `/tmp/ai386-freebsd-4.11-miniinst.iso` | `8c7b5c8aca08fd0ba0a0fdc06a0de4f275efa1bbffab7763d205d1fc3a942e04` |
| базовый `/tmp/ai386-freebsd-final-hd.img` | `6aa54f7333f12c5998f6baf68ff1d0c1313cd1096a95b4c90988013fd3438831` |
| рабочий HD после запусков | `9598f364abf409cef2f8f3e6cdec2f83eb77290048c0c8b5b59b1b5a9ec0c98b` |
| `ai386-machine-test` | `fabaa4a3d10c7eba18be041c1c5929858a7ab30d82775ef21c4840918670f0e0` |
| подготовительный лог | `d4c1aacfa1b0e4b4a241e53d9b33989de85334e0535b8cbff7361435e76d6085` |
| зачётный лог | `a375b218bed5cf62c2505695b72ddfff5c4dee727df1c68ce4ff8c915de3403e` |
| официальный `releng/4.11` GENERIC | `2e6671b66b502bea22c07c5dd4239c64c7d336bb5c766c72444edc3182e1a06a` |

