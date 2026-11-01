# Верификация fix-bda-cursor, 2026-10-17

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

## Все найденные проблемы

| # | Проблема | Статус | Область |
|---|---|---|---|
| 1 | INT 10h не писал BDA-курсор 0x450/0x451; backspace COMMAND.COM из (0,0) уводил курсор в (255,79) → CRTC 20479, ввод зависал после `dir` | **исправлена** | `bios` INT 10h |
| 2 | Симптом: `crtcCur=20479`, `CursorPosition()=(51,15)`, `WriteChar` молча игнорирует row≥25, echo исчезает | устранён вместе с #1 | vga/наблюдение |
| 3 | Free-space пауза `dir` (~2.5 s / 16.7M Step) — гость не читает INT 16h, `KeyLen` растёт | **не баг эмулятора**, задокументирована | гость COMMAND.COM |
| 4 | Headless-критерий «dir готов»: `Contains("C:\\>")` ложно матчит эхо `C:\>dir` | задокументирован в design | тестовая методика |
| 5 | При сэмплировании сразу после `KeyLen==0` echo символа может опоздать до break | не воспроизводится как зависание после #1 | наблюдение |

## Корень проблемы 1

COMMAND.COM/DOS при backspace читают позицию из BDA (0040:0050 col,
0040:0051 row), **не** через INT 10h AH=03h. BIOS обязан хранить там
экранную позицию. До фикса там всегда было (0,0):

```
backspace из (0,0): col==0 → row-- = 255 (uint8), col = 79
→ AH=02h DH=255 DL=79 → CRTC 255*80+79 = 20479 = 0x4FFF
→ (20479 % 16384) = 4095 → row=51, col=15  // за пределами 25 строк
```

Hardware scroll исключён: `crtcStart=0` на всех шагах репро.

## Исправление

`internal/bios/int10.go`: `writeBDACursor` при AH=00h (→0,0), AH=02h
(DH/DL), AH=0Eh (итоговые row/col после переноса/прокрутки).

## Проверки

### Unit-регрессии (`internal/bios/int10_bda_cursor_test.go`)

| Тест | До фикса | После |
|---|---|---|
| `TestInt10SetCursorUpdatesBDA` | FAIL (BDA 0) | PASS (row=5,col=12) |
| `TestInt10TeletypeUpdatesBDA` | FAIL (BDA 0) | PASS (col сдвигается) |
| `TestInt10GetCursorMatchesBDA` | FAIL | PASS (AH=03h ↔ BDA) |
| `TestInt10SetModeResetsBDACursor` | FAIL | PASS (0,0) |
| `TestInt10BackspaceFromSyncedBDACursor` | FAIL (row=0≠24) | PASS (24,4) |

### Живой FreeDOS (`freedos-ready.img`, `clock_hz=1e6`, копия HD)

Сценарий пользователя: `dir` → `x` → Backspace → `echo hi`.

| Точка | pos | BDA | Экран (хвост) |
|---|---|---|---|
| после dir | (24,0) | (24,0) | `… bytes free` |
| `x` потреблён | (24,4) | (24,4) | `C:\>` |
| break `x` | (24,5) | (24,5) | `C:\>x` |
| break Backspace | **(24,4)** | **(24,4)** | `C:\>` (x стёрт) |
| `echo hi` + Enter | (24,11) | (24,11) | `C:\>echo hi` |

Нет `crtcCur=20479`, нет `pos=(51,15)`, echo и backspace корректны.
До фикса на break Backspace: `crtcCur=20479`, `pos=(51,15)`, далее `cho hi`
со сдвигом.

### Общие проверки

- `go test ./...` — 11798 passed / 27 packages
- `go test -race ./internal/bios ./internal/ps2 ./internal/vga` — 255 passed
- `go vet ./...` — чисто
- `openspec validate --all --strict` — 23/23

### Временные артефакты диагностики

Throwaway `build/freedos/dump_dir*.go` (`//go:build ignore`) удалены после
верификации. Повторное воспроизведение: копия `freedos-ready.img`, `machine.New`
+ `Step`/`KeyEvent`, замер `Display().CursorPosition()`, CRTC 0x0C–0x0F и
BDA 0x450/0x451.

## Границы проверки

- Страницы 1–7 BDA-курсора не трогались (COMMAND.COM использует страницу 0).
- Прямая запись гостём CRTC 0x0E/0x0F в обход INT 10h не синхронизируется.
- Free-space пауза `dir` — поведение гостя, не оптимизировалась.
- Оконный frontend при этом сценарии отдельно не гонялся; headless-путь
  `Machine.KeyEvent` → INT 10h/BDA покрыт.
