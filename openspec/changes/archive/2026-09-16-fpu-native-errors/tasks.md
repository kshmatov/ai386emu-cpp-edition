## 1. Native x87

- [x] 1.1 Реализовать pending/#MF, FWAIT и ZE suppression с регрессиями. Приёмка: masked/unmasked, pop, no-wait, TS/MP и адрес fault покрыты; test/vet/build/race; журнал времени/токенов.

## 2. Гость

- [x] 2.1 Повторить FreeBSD npx probe. Приёмка: ядро использует exception16 без error reporting broken; результат и журнал записаны.
