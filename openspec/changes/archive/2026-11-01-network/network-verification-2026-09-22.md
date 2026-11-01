# Проверка сети в установленных гостевых ОС, 22 сентября 2026

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

Ветка: `feature/ne2000-usernat`. Проверки выполнялись на рабочих копиях
установленных дисков в `/tmp/ai386-network-check/`; исходные образы не
изменялись. Команды вводились в гостевые консоли через PS/2 control-loop.

## FreeBSD 4.11

Запуск: `TestFreeBSDDiagnostic` с `-guest-network -freebsd-installed
-freebsd-control`, HD `/tmp/ai386-network-check/freebsd.img`. После входа
`root` выполнены:

```sh
dhclient ed0
ifconfig ed0
netstat -rn
cat /etc/resolv.conf
ping -c 1 10.0.2.2
ping -c 1 example.com
fetch -o /tmp/ai386-http-final.html http://example.com/
ls -l /tmp/ai386-http-final.html
fetch -o /tmp/ai386-https-final.html https://example.com/
sync
shutdown -h now
```

`ed0` поднялся с MAC `02:02:00:00:00:00`, адресом `10.0.2.15/24`, шлюзом
`10.0.2.2` и DNS `10.0.2.3`. Шлюз ответил на ping (1/1). DNS разрешил
`example.com` в `8.47.69.6`. Внешний ICMP не реализован в NAT, поэтому
ping этого адреса не получил ответа. HTTP загрузил файл размером 559 байт.
HTTPS завершился сообщениями гостевого `fetch`: `SSL support disabled` и
`Authentication error`; успешность HTTPS из этого гостя не подтверждена.
Гость завершил остановку строкой `The operating system has halted.`;
диагностический тест прошёл. Журнал:
`/tmp/ai386-network-check/freebsd-tx.log`.

После исправления коротких TCP-кадров FreeBSD повторно запущена на той же
сборке, что и DSL. Повторный прогон подтвердил `dhclient ed0`, адрес
`10.0.2.15/24`, DNS `10.0.2.3`, HTTP-файл 559 байт и штатную остановку.
Журнал финального прогона: `/tmp/ai386-network-check/freebsd-pad.log`.

## Damn Small Linux 4.4.10

Запуск: `TestDSLDiagnostic` с `-guest-network -dsl-installed
-dsl-require-shell -dsl-control-at-shell`, HD
`/tmp/ai386-network-check/dsl.img`. В гостевой оболочке выполнены:

```sh
modprobe ne io=0x300 irq=10
pump -i eth0
ifconfig eth0
cat /etc/resolv.conf
wget -t 1 -T 20 -O /tmp/ai386-http-pad.html http://example.com/
ls -l /tmp/ai386-http-pad.html
wget -t 1 -T 20 -O /tmp/ai386-https-pad.html https://example.com/
sync
shutdown -h now
```

Драйвер обнаружил NE2000 на `0x300`, IRQ 10. DHCP назначил `10.0.2.15/24`,
DNS — `10.0.2.3`; `ifconfig` показал 6 переданных пакетов без ошибок.
DNS разрешил `example.com`; HTTP соединение состоялось, сервер ответил
`200 OK`, файл содержит 559 байт. HTTPS соединился с портом 443, после
запроса получен `403 Forbidden`; успешная загрузка страницы по HTTPS не
подтверждена. До дополнения коротких TCP-кадров тот же `wget` завершался
тайм-аутом соединения с обоими адресами `example.com`.
Гость завершил остановку строкой `DSL halted.`; диагностический тест прошёл.
Журнал: `/tmp/ai386-network-check/dsl-pad.log`.

## Обнаруженные и исправленные дефекты

- ARP отвечал только за шлюз, а не за DNS; ответы ARP не дополнялись до
  минимальной длины Ethernet-кадра.
- ICMP echo-ответ сохранял неверное направление IP-адресов и считал checksum
  по дополненным байтам.
- 16-битный remote DMA через системную шину записывал старший байт в соседний
  порт вместо того же data port.
- Уже ожидающее прерывание PTX/PRX не выставлялось при снятии маски IMR.
- Бит TXP не очищался по завершении передачи, а чтение TSR возвращало TPSR.
- TCP SYN-ACK и другие короткие TCP-ответы имели длину 54 байта вместо
  минимальных 60 байт Ethernet-кадра. Checksum TCP вычисляется только по
  длине IP-пакета, без байтов дополнения.

Каждый дефект воспроизведён тестом до исправления. Проверка DSL после
последнего исправления подтверждает исходящий HTTP.

## Автоматические проверки

После последнего исправления прошли `go test ./...`, `go test -race ./...`,
`go vet ./...`, `go build ./...` и
`openspec validate ne2000-usernat --strict`. Сборка выдала предупреждение о
невозможности записи stat cache в системный Go module cache, но завершилась
с кодом 0.
