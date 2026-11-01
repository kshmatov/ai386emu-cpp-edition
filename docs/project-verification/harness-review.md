# Независимая проверка диагностик

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

Root рассмотрел audio helper, probe.c, verify_pcm.py, freebsd-sbc.c и inbound_test.go.

- Audio flags подключают настоящий file backend/SB16, unit test проверяет DSP handshake и создание файла. Гость делает ioctl с readback U8/mono/8000, реальный write/read через OSS, memcmp8192 и сохранение capture на HD. Host verifier требует полный точный источник, вне него допускает только80h и отвергает иной хвост.
- FreeBSD KLD добавляет только ISAchild/resources; используется установленный штатный snd_sbc/snd_sb16. Предварительная ошибка linker gensetdefs честно сохранена как FAIL. Не используется в production и запрещён unload вне disposable guest.
- Inbound UDP действительно доставляет чужой payload на host endpoint активного flow, затем получает точный легитимный ответ. Existing timeout/TCP negative tests дополняют его. Это не внешний портscan.

Blocking/high замечаний к данным helper/test файлам нет. TLS сценарии оцениваются по окончательным журналам и negative CA control, а не по exit0 s_client. Точное потребление токенов недоступно.
