# Review: BIOS video mode в BDA

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

Проверено 2026-10-14 для `doom-bios-video-mode`. Объём production — одна
запись BDA449 внутри успешного INT10 AH00. Прочие изменения int10 рабочего
дерева, включая AH08, происходили до этого исправления и сюда не относятся.
Код не редактировался reviewer; изменён только этот отчёт.

## Standards

Блокирующих нарушений `AGENTS.md` не найдено. Исправление находится у
существующего BIOS mode switch и использует переданный Bus, не вводит
параметров, обходного изменения Doom или host-restore графики после выхода.
Regression tests используют testify и исполняемый гостевой код.

## Spec

После успешного `display.SetMode(AL)` локальный mode получает AL, затем
`bus.WriteMem(0x449, mode)` синхронизирует BDA. Ошибка SetMode не входит
в эту ветку: closure, BDA и VGA остаются прежними. В `VGA.SetMode` неизвестный
режим возвращает error до изменения состояния. Scope соответствует spec:
текущий BIOS mode в BDA обновляется при успешном AH00, неизвестный сохраняется.

VBE path AH4F и прямое программирование регистров VGA не менялись.
Их BDA-семантика этим исправлением не принимается. Также не заявлена
полнота BDA cursor/page/geometry, дополнительных видеорежимов или API.

Guest test воспроизводит фактическое условие Doom: INT10 mode13,
CMP byte[449h],13h, JNE к HLT, иначе INT10 mode3 и teletype `D`.
Раскладка инструкции проверена: JNE находится на offsets10–11, следующий
IP=12, displacement0Ah даёт offset22 — финальный HLT. При старом BIOS
branch обходит обе INT10, оставляя графику. При исправленном выполняются
restore и видимая запись D. Тест проверяет halt, TextGeometry25×80,
BDA03 и символ в первой клетке. Он не принуждает VGA/гостевое поле снаружи
и не может пройти при прежней реализации.

Accepted/rejected test проверяет последовательность3,13h,7,12h, затем14h:
каждый принятый mode читается из BDA, неизвестный сохраняет12h и Frame.
Frame является самостоятельным свежим снимком, не alias mutable framebuffer.
После review fixture усилен: перед неизвестным запросом VGA получает
FFh по A0000, и `NotZero(before.Pix[0])` подтверждает видимые данные.
Последующее равенство Frame теперь обнаружит также случайную очистку VRAM,
а не только изменение геометрии. При текущем invalid path VGA не меняется.

## Независимый red/green и проверки

Через временный Go overlay в `/tmp` удалена только новая BDA write из
копии `int10.go`; исходное дерево не менялось. Запуск обоих новых тестов
дал два ожидаемых FAIL: guest rows0 вместо25, accepted mode BDA0 вместо3.
Журнал: `/tmp/ai386-bda-review-red.log`; overlay:
`/tmp/ai386-bda-review-red-overlay.json`. Это независимая проверка red,
а не только принятие сообщения другого агента.

На текущем production выполнены:

```sh
GOCACHE=/tmp/ai386-status-go-cache go test -race ./internal/bios ./internal/vga
GOCACHE=/tmp/ai386-status-go-cache go vet ./internal/bios ./internal/vga
```

219 тестов в двух пакетах прошли; race/vet без ошибок. После усиления VRAM
fixture оба целевых теста отдельно повторены с race и прошли, vet также
прошёл. Открытых блокирующих
standards/spec замечаний нет. Свежие rendered timedemo с видимым итогом и
отдельной строкой DOS должны подтвердить реальный выход; этот review
не объявляет их PASS и не закрывает scope или project acceptance.
