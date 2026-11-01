# Сетевая проверка 14 октября 2026

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

Рабочий каталог `/tmp/ai386-network-20261014`; исходные HD копируются
через `cp --reflink=auto --sparse=always`. SHA256 баз записан в
`bases-before.sha256`, диагностическая сборка — `machine.test`.
Каждая ОС использует отдельную копию и PS/2 control-loop, завершение
подтверждается guest halt и итоговым exit foreground runner.

## План и границы

1. Прочитать версии TLS-клиентов в живых DSL 4.4.10 / FreeBSD 4.11.
2. Получить DHCP, DNS, внешний HTTP, фактический HTTPS с HTTP 200 и
   известным телом, проверить сертификат и сохранить серверный журнал.
3. Проверить входящие пакеты от постороннего UDP endpoint в активный поток,
   поздний ответ после timeout и TCP сегменты без handshake.
4. Штатно остановить обе ОС; проверить неизменность исходных дисков.

TLS endpoint — контролируемый сервер на фактическом адресе хоста
`192.168.0.127`, HTTP 18080 / HTTPS 18443. Используются TLS 1.0 и AES128-SHA
для совместимости со старыми гостевыми клиентами. Собственный CA RSA2048/SHA1, CN `ai386-network-test`, и отдельный
leaf RSA2048/SHA1, CN `ai386-network-endpoint`, SAN IP `192.168.0.127`
и DNS `ai386-network-test`. Leaf действителен 14 октября 00:00 UTC —
16 октября 00:00 UTC; начало задано явно, чтобы учитывать виртуальное
время уже запущенных VM. Это временная тестовая конфигурация.
SHA1 fingerprint CA:
`F6:05:C1:85:ED:4E:3E:24:A8:05:77:03:6D:14:22:80:7B:32:45:47`.
CA загружается гостем с локального endpoint. FreeBSD сравнивает явный
SHA1 fingerprint; DSL сравнивает MD5 самого PEM-файла
`e5d49bf7e2a6a45b652e681b7cc0cb62` как контроль неизменности доставленного
тестового артефакта. TLS проверяет цепочку; ошибки не обходятся.

Файлы endpoint, ключ/сертификат и журналы находятся в подкаталоге
`endpoint`. Host preflight подтвердил `Verification: OK`,
`Verify return code: 0 (ok)`, `HTTP/1.0 200 OK`, тело
`AI386_NETWORK_OK host 20261014`. Первый preflight до готовности listener
получил connection refused; повтор после READY прошёл.

Выбор TLS1/шифра и CAfile основан на официальной документации
[s_server](https://docs.openssl.org/3.0/man1/openssl-s_server/) и
[s_client](https://docs.openssl.org/1.0.2/man1/s_client/).
`s_client` может продолжать работу после ошибки verify; поэтому один
exit 0 не принимается, нужен проверенный сертификат и реальный HTTP payload.

Внешний контрольный узел
[`tls-v1-0.badssl.com:1010`](https://badssl.com/) доступен с хоста:
TLS1/AES128-SHA с SNI проходит цепочку доверия. Без SNI узел отдаёт
другой, истёкший в 2018 году fallback сертификат; такая попытка не является
приёмкой HTTPS. Совместимость старых гостей с SNI проверяется отдельно.
Локальный HTTPS доказывает путь guest → NE2000 → NAT → host socket →
TLS → HTTP; он не заменяет отдельное подтверждение интернет-HTTPS.

## Автоматическая проверка входящих пакетов

Добавлен `TestUDPNATRejectsUnsolicitedSourceOnActiveFlow`: гость создаёт
реальный UDP flow; отдельный сокет посылает посторонний payload на его
host endpoint. За окно наблюдения 50 мс в guest callback нет кадров.
Затем разрешённый endpoint посылает ответ и его точное тело получено.
Так положительный контроль исключает ложный успех из-за неработающего RX.

Команда:

```sh
GOCACHE=/tmp/ai386-status-go-cache go test -race ./internal/usernat \
  -run 'TestUDPNATRejectsUnsolicitedSourceOnActiveFlow|TestUDPNATDiscardsResponseAfterFlowTimeout|TestTCPNATDiscardsUnsolicitedSegments' \
  -count=1 -v
```

Все три теста PASS (1.170 с после добавления cleanup NAT), журнал `inbound-tests.log`. Timeout-тест
посылает реальный поздний UDP ответ после удаления flow. TCP-тест
проверяет отсутствие dial/flow/frame для ACK/PSH/RST без handshake;
это пакетная проверка, а не внешний TCP port scan гостя.

## Подготовка FreeBSD и ошибка сценария

Свежая FreeBSD HD-копия не содержала `/boot/kernel.conf`, карта `ed0`
по умолчанию была отключена. Первый boot прочитал настоящую версию
`OpenSSL 0.9.7d 17 Mar 2004` и через PS/2 записал:

```text
enable ed0
port ed0 0x300
irq ed0 10
quit
```

Файл прочитан обратно, затем `sync; shutdown -h now` достиг настоящего
halt. `freebsd.log`: PASS 428.76 с, runner status 0.

Следующий запуск подтвердил загрузку kernel.conf и ed0. Его ожидание
`bound to 10.0.2.15` было ошибкой сценария: FreeBSD dhclient работает
молча и вернул `#`, поэтому ожидаемая строка не появилась. Это не отказ
NAT; `freebsd-network.log` сохранён как FAIL 628.73 с / runner status 1.
Повтор `freebsd-final.log` использует вывод `ifconfig ed0` и
`cat /etc/resolv.conf`; ожидаемые строки сверены с прежним настоящим
FreeBSD журналом. Ошибочный запуск не засчитывается как приёмка.

## DSL 4.4.10

Живой inventory: GNU Wget 1.9, динамические libssl/libcrypto.so.0.9.7,
исполняемой команды openssl нет. Версия установлена в госте, не угадана
по возрасту дистрибутива. По [точному исходнику GNU Wget 1.9](https://ftp.gnu.org/gnu/wget/wget-1.9.tar.gz)
выбраны `--sslprotocol=3 --sslcheckcert=1 --sslcafile=ca.pem`.
`gen_sslfunc.c` содержит исключение для self-signed leaf, поэтому
использована отдельная CA→leaf цепочка и отрицательный контроль:
тот же запрос с другим CA должен завершиться ошибкой SSL.

Получены DHCP `10.0.2.15/24`, DNS `10.0.2.3`; NE2000 найден на 0x300 / IRQ 10.
DNS разрешил example.com в `8.6.112.5`, `8.47.69.5`. Внешний HTTP вернул
`200 OK`, файл 577 байт. HTTPS к выбранному локальному узлу прошёл:
wrong CA отклонён, правильный CA дал `HTTP/1.0 200 OK`, Content-Length 30,
точное тело `AI386_NETWORK_OK dsl 20261014` и `AI386_HTTPS_dsl_PASS`.
`endpoint/server.log` в 16:30:02 UTC подтверждает `/dsl`, AES128-SHA и
тело 30 байт. Listener допускает только TLS1; строка `SSLv3` внутри
`SSLSocket.cipher()` обозначает семейство cipher suite, не согласованную
версию протокола. `dsl-https.png` визуально проверен, тело/HTTP 200 видны.
`cat /tmp/response.txt` проверен строгим until с началом строки.

Попытка интернет-HTTPS `www.howsmyssl.com/a/check` использовала TLS1,
включённую проверку сертификата и цепочку CA, предварительно проверенную
современным клиентом хоста. DNS вернул 34.71.45.200, TCP 443 установился,
но гостевой Wget ответил `Unable to establish SSL connection`.
Debug support в этой сборке не включён. Это отрицательный результат
интернет-HTTPS; нельзя считать его успешной приёмкой. Хост на том же
узле без SNI получил HTTP 200 и JSON `tls_version: TLS1.0`.
[API выбранного узла](https://www.howsmyssl.com/s/api.html) предназначен
для проверки собственных TLS-клиентов.

Локальный строгий скрипт сохранён как `endpoint/dsl-local.sh`, внешний —
`endpoint/dsl.sh`; запускались через PS/2 после HTTP-доставки в `/tmp` гостя.
Штатный shutdown подтвердил `/dev/hda1 umounted`, `DSL halted.`;
`dsl-halt.png` визуально проверен. `dsl.log`: PASS 1766.17 с,
runner status 0, duration 1766 с. После halt передан explicit quit.

## FreeBSD 4.11

В финальном запуске DHCP `10.0.2.15/24`, DNS `10.0.2.3`, внешний HTTP
example.com 577 байт и фактический HTML прочитаны в консоли; снимок DHCP
`freebsd-dhcp.png`. TLS получил проверенный ответ, как описано ниже. В первой попытке скрипт
корректно остановился на fingerprint: OpenSSL 0.9.7d по умолчанию печатает
MD5, поэтому добавлен явный `-sha1` для сравнения с ожидаемым SHA1.
Ни эта остановка, ни прежние DSL403 / fetch `SSL support disabled`
успешным HTTPS не считаются.

После явного `-sha1` фактический guest OpenSSL подтвердил fingerprint CA,
`ca.pem: OK`, depth 1 CA и depth 0 leaf с `verify return:1`,
`HTTP/1.0 200 OK`, Content-Length 34 и тело
`AI386_NETWORK_OK freebsd 20261014`; скрипт выдал
`AI386_HTTPS_freebsd_PASS`. Серверный журнал фиксирует запрос в 16:35:48 UTC.
Скрипт: `endpoint/freebsd-local.sh`, снимок `freebsd-https.png` визуально проверен.
SHA1 окончательного leaf:
`31:61:86:40:75:34:54:CD:31:A0:10:28:05:D5:3B:30:3C:7A:0C:5E`.

Внешний запрос с доставленным минимальным CA bundle 2729 байт получил
HTTP 200 / JSON `tls_version: TLS 1.0`, но проверка цепочки завершилась
`verify error:num=7:certificate signature failure` на depth 3,
ISRG Root X1. Гостевой `openssl x509 -text` показывает алгоритм подписи
числовым OID `1.2.840.113549.1.1.11`; это SHA256-with-RSA по
[RFC 7518, приложение A.2](https://www.rfc-editor.org/rfc/inline-errata/rfc7518.html).
Гостевой `openssl dgst -sha256` отвечает `unknown option '-sha256'`.
Это свидетельства отсутствующей поддержки SHA256 в старом клиенте,
а не вывод только по возрасту ОС. Интернет-HTTPS с проверенным доверием
не принят; реальные HTTP/TLS байты доставлены, но криптографическая
проверка внешнего сертификата не прошла.

`freebsd-final.log` завершён штатным `sync; shutdown -h now`,
`The operating system has halted.`, explicit quit, PASS 1423.37 с и
runner status 0 / duration 1424 с. Отдельные отрицательные проверки
внутри сеанса не превращаются в положительную приёмку из-за exit 0.

## Обнаруженная ошибка больших передач и повтор после исправления

FreeBSD дважды не получила полный CA bundle 8457 байт: fetch выдал
`transfer timed out` и удалил неполный файл. Первая внешняя попытка
s_client поэтому имела точную ошибку `fopen public-ca.pem: No such file`
и `unable to get local issuer certificate`; она всё же получила
реальный внешний HTTP 200 / JSON TLS 1.0, но проверку сертификата не прошла
и не засчитана. Это не доказательство несовместимости TLS клиента.

Контроль независимым телом из повторяемых `0123456789abcdef`:

| FreeBSD HTTP body | Результат |
| --- | --- |
| 1024 байта | PASS, MD5 `7dd7e68f1907b4665684355797725b8e` |
| 8457 байт | timeout, файл удалён |
| 32768 байт | timeout; kernel `ed0: NIC memory corrupt - invalid packet length 13667` |
| 2729 байт, минимальный ISRG X1/X2 CA bundle | PASS, MD5 `a5fb2015473f32fec4fc946a23cf7d87` |

Контроль HTTPS на той же старой сборке: 1024 байта прошли с проверенным
CA и точным MD5; 8457 байт зависли, выходной файл имел размер 0,
процесс остановлен через PS/2 Ctrl-C. Полный RAM snapshot сохранён в
`freebsd-network-ram.bin`; диагностический control-loop не предоставляет
чтение NIC-регистров, поэтому конфигурация ring не выдумывается.

Ошибка локализована в NE2000 ReceiveFrame: проверялось совпадение BNRY
только с концом пакета, но пакет мог пересечь занятую страницу между
началом и концом. Исправление и red/green тесты описаны в
`openspec/changes/ne2000-rx-boundary/`. Независимый review подтвердил
проверку всех страниц до записи RAM/CURR, линейный и циклический случаи,
попадание концом точно в BNRY и повтор после освобождения места.

Повтор запущен на финальной production-сборке `machine-fixed.test`, SHA256
`4fc90f17702ca47fa87329cdbc626b03e7d4922dfe527b5b457f2e7553600bf7`.
Скрипт `endpoint/freebsd-large-fixed.sh` проверяет HTTP и HTTPS тела
1024/8457/32768 байт, размеры, HTTP 200, контрольные MD5 и отсутствие
ошибок TLS verify. Первый повтор `freebsd-fixed.log` остановился до payload: команда
DHCP не вывела ожидаемый результат за 100M инструкций. Strict harness
завершился FAIL 331.99 с / runner status 1. Это не принимается за
сетевой PASS. Второй повтор `freebsd-fixed-retry.log` использует прежний
лимит DHCP 500M. DHCP прошёл за 25.8M инструкций, IP и DNS правильные;
единичный отказ предыдущего запуска не воспроизведён.
На исходной сборке DHCP ранее завершился за 23.7M инструкций, поэтому
причина задержки не приписывается одному бюджету без наблюдения.
HTTP 1024 и 8457 байт прошли с точными MD5; HTTP 32768 байт остались на
0% за 600M инструкций, без прежнего NIC memory corrupt. Control Until
обязателен сам по себе, поэтому harness остановился FAIL 767.62 с / runner
status 1 до поставленных в очередь Ctrl-C/shutdown. HTTPS часть общего
скрипта в этом запуске не достигнута.

Дополнительная ошибка повторной отправки TCP воспроизведена быстрым
тестом: один timeout повторял всё неподтверждённое окно, снова создавая
большую серию кадров. Scoped изменение, red/green и первичный контракт:
`openspec/changes/ne2000-tcp-retransmit-recovery/`. Исправление повторяет
только первый неподтверждённый сегмент. Финальный прогон ниже прошёл.

### Итоговый повтор после RX и TCP исправлений

Журнал `freebsd-tcp-fixed.log`: **PASS 452.70 с**, runner status 0,
duration 453 с. Использован `machine-tcp-inspect.test`, SHA256
`1884bab9af5194103d8a593b477dc1011e85690cfb7c204cfa65924f578cd36e`.
Это финальные production-исходники плюс read-only Go overlay
`/tmp/ai386-network-control-overlay.json`: команда Inspect читает
числовые поля NIC и RAM через reflect. Записи в устройство не добавляются.
Обычная сборка тех же production-исходников `machine-tcp-fixed.test` имеет
SHA256 `2aa9b8ee6a13814a66f67adb0e2a202bf0abc2b3cd113760be5b2c4697d50342`.

DHCP прошёл за 47.5M инструкций: `10.0.2.15/24`, DNS `10.0.2.3`.
Фактический ed0 — NE1000 (8 bit), MAC `02:02:00:00:00:00`.
Read-only Inspect подтвердил RX ring `PSTART=0x26`, `PSTOP=0x40`:
после DHCP `BNRY=0x26`, `CURR=0x27`, после передач `BNRY=0x37`,
`CURR=0x38`; ISR 0 в обоих снимках. Отдельные события OVW во время
предыдущего неудачного прогона не трассировались.

| Тело | MD5 | HTTP | HTTPS с проверкой CA |
| --- | --- | --- | --- |
| 1024 байта | `7dd7e68f1907b4665684355797725b8e` | PASS | PASS |
| 8457 байт | `ecd6b856a99de2ca9e46445ebf2568f2` | PASS | PASS |
| 32768 байт | `0049e62277755a4477ce6b6ca37498b0` | PASS | PASS |

Скрипт проверил HTTP 200, Content-Length, точный размер и MD5 сохранённого
тела, `verify return:1` и отсутствие любого `error` в TLS журнале.
Строгая проверка экрана подтвердила `AI386_LARGE_NETWORK_PASS`;
`freebsd-tcp-pass.png` визуально проверен. Endpoint подтвердил все три
HTTPS ответа с согласованным TLSv1 в 17:15:33–17:16:03 UTC.

Независимый negative control с `wrong-ca.pem` получил `verify error`
20 (unable to get local issuer certificate), 27 (certificate not trusted)
и 21 (unable to verify the first certificate). `s_client` при этом
возвращает exit 0 и `verify return:1`: это разрешение callback продолжать
диагностику, не успешное доверие. Поэтому положительный скрипт обязательно
отвергает любой `error`; один exit 0 или verify return 1 не принимается.
`freebsd-negative-ca.png` визуально проверен, скрипт сохранён как
`endpoint/freebsd-negative-ca.sh`; внешняя попытка — `external-original.sh`.

После `sync; shutdown -h now` прочитано `The operating system has halted.`,
снимок `freebsd-tcp-fixed-halt.png` проверен, затем отправлен explicit quit.
Временный endpoint остановлен после приёмки (SIGTERM, серверный exit 143).

Контроль до/после TCP правки: при уже исправленной целостности RX кольца
32 КиБ оставались на 0% за 600M инструкций; после единственной production
правки выбора retransmit сегмента HTTP и HTTPS 32 КиБ прошли. Это
подтверждает исправление гостевого зависания. Точная последовательность
OVW/ACK старого отказа отдельно не записана.

DSL HTTPS проверен на первоначальной сборке этого отчёта; финальная
повторная матрица больших передач выполнена для FreeBSD. Интернет-HTTPS
с проверенным публичным сертификатом по-прежнему не принят у обоих
старых клиентов; локальная приёмка не выдаётся за этот результат.

## Входящие соединения в живых VM

`guest-listeners.txt` фиксирует socket inodes принадлежащих именно
процессам двух VM и отсутствие TCP LISTEN до запросов и после реального
сетевого обмена (14 октября 19:32:45 MSK). Это дополняет пакетные тесты
NAT, но не выдаётся за внешний port scan гостевого IP.

## Сохранность исходных образов

После всех запусков SHA256 исходных HD совпали с `bases-before.sha256`;
`bases-after.sha256` сохранён в рабочем каталоге; `diff -u` завершился с кодом 0.

- FreeBSD: `6aa54f7333f12c5998f6baf68ff1d0c1313cd1096a95b4c90988013fd3438831`.
- DSL: `b629394156e45d59ecd1fc30442bb4282743d45a8140d2321bee5fc213c9f4af`.

Гостевые изменения пишутся только в `freebsd.img` и `dsl.img` рабочего
каталога. Исходные файлы не подключались к диагностическим VM.

## Проверки после последней TCP правки

- `go test ./... -count=1` — PASS, итоговый exit 0.
- `go test -race ./internal/usernat -count=1 -timeout=2m` — PASS 1.410 с.
- `go test -race ./internal/machine -run '^Test(Network|GuestNetwork)' -count=1 -timeout=2m` — PASS 1.107 с.
- `go vet ./...` — PASS.
- Linux/Windows amd64 `go build ./...` и `go build -tags desktop ./...` — все четыре exit 0.
- `openspec validate ne2000-tcp-retransmit-recovery --strict` и scoped diff-check — PASS.

Все команды использовали GOCACHE=/tmp/ai386-status-go-cache. Журналы
`final-full-test.log`, `final-vet.log`, `final-build-*.log` и
`tcp-recovery-*-race.log` лежат в рабочем каталоге. Сборки сообщили
предупреждение о read-only metadata cache в /home/kis/go/pkg/mod;
сборка завершилась успешно. Полный race проекта был выполнен root до
последней TCP правки; после неё повторены затронутые сетевые пакеты.
