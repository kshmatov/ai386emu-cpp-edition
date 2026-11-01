# Проверка TB-корректности, device-clock и JIT-флагов, 2026-10-17

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

## Контекст

T13 (RunBatch/native JIT) сломал загрузку FreeDOS/ISOLINUX: «Loading spec
packet failed, Extremely broken BIOS detected» (INT 13h AH=4B El Torito) и
«Invalid Opcode at 3D23…» на экране выбора языка. Бисект (TB off → PASS,
TB on + JIT off → Invalid Opcode, TB on + JIT on → ISOLINUX fail) показал
три независимых дефекта.

## Бисект и диагноз

`TestFreeDOSInstallMenu` (LiveCD ISO `FD14LIVE.iso`, SHA-256 в
`build/freedos/`):

| Конфигурация | Результат | Симптом |
|---|---|---|
| TB off (`canUseTB()=false`) | PASS 9.76s | — |
| TB on, JIT off | FAIL 11.24s | «Invalid Opcode at 3D23 12BA…» |
| TB on, JIT on | FAIL 2.28s | «ISOLINUX: Loading spec packet failed / Extremely broken BIOS» |

Диагностика stored vs fresh decode (в `stepTB`: `dispatch(op.insn)` vs
`decodeStream` от `op.eip` + `dispatch`):

```
TB-DECODE-MISMATCH eip= 522  saved.op= 7  fresh.op= 94  saved.b= 80  fresh.b= 140
  saved.len= 1  fresh.len= 2  savedNext= 523  freshNext= 524  bytes= 140 214 140  cs= 65535
```

- `saved`: `PUSH AX` (op=7, b=0x50), len=1
- `fresh`: `MOV SI, SS` (op=94, b=0x8C), len=2; байты `8C D6 8C`
- CS=0xFFFF (физадрес 0x1001FA) vs fillTB-диагностика того же eip=522 в
  другом CS (phys=588954): stored `MOV SI,SS` — верно.

**Вывод:** `op.insn` битый (из другого CS), `op.eip`/`op.next`/handlers
корректны. Ключ блока — физадрес, не логический EIP.

## Исправления и проверки

### 1. TB: физадрес блока (`tbOp.phys`)

`stepTB` сверяет `tbPhys(op.eip)==op.phys` на входе в блок; `tbGo`
(chain-link + lookup) сверяет физадрес. Регрессии
`internal/cpu/tb_cs_phys_test.go`:

- `TestTBAfterFarJumpUsesNewCSCode`: CS=0x1000 `MOV AX,0x1111; JMP FAR
  0x2000:0000` → CS=0x2000 `MOV AX,0x2222; HLT`. После `Run(2)` AX=0x1111,
  CS=0x2000; после `Run(4)` AX=0x2222, Halted.
- `TestTBAfterFarJumpDifferentCodeAtSameEIP`: INC/DEC AX на одном EIP в
  разных сегментах. PASS.

`TestFreeDOSInstallMenu` после фикса: **PASS 11.92s** (Welcome, Install
menu, «What is your preferred language?», Russian/English).

### 2. Phantom-execution

`stepTB`/`fillTB`/`tbValidate` возвращают `false` на отказе. Suite:
**11800 passed / 27 packages** (`go test ./... -count=1`).

### 3. Device-clock floor (`RunBatch`)

`cycles = max(delta, max(executed, 1))`. `TestFreeDOSInstallProfile`
(boot→Welcome, `clock_hz=1e6`):

```
PROFILE total_wall=37.62s steps=211600072 halted=0.0% steps/s=5624663
        clock_cycles=708551580 cum_MIPS=18.83
```

- boot-to-welcome **reached=true** (11M steps, 766ms)
- boot-menu **reached=true** (600K steps, 99ms)
- enter-installer / language-list — budget-limited (нет KeyEvent, ожидаемо)

До фикса (max(delta,1) на батч): `clock_cycles=304M`, гость зависал на
ISOLINUX (0/4 фаз). После: 708M ≈ историч. 704M.

### 4. Batch vs Step (вывод T13)

| Конфигурация | wall | cycles | MIPS | Welcome |
|---|---:|---:|---:|---|
| T12 историч. (phantom steps) | 18.6 s | ~704M | 37.9* | reached |
| `RunBatch(32)` | 38.1 s | 708M | 18.6 | reached |
| `Step()` | 35.0 s | 708M | 20.2 | reached |

*37.9 MIPS завышены: ~половина «шагов» T12 была phantom (счётчик рос без
`op.fn`). Реальная скорость ~5.6M insn/s в обоих случаях.

**Вывод:** batch не выигрывает → `machine.run()` → `Step()`.

### 5. JIT INC/DEC CF

`emitSetFlagsKeepCF` (маска `0x8D4`, `and ecx, 0xFFFFF72B`) для INC/DEC.
Регрессии `internal/cpu/jit_flags_test.go`:

- `TestJITIncDecPreservesCF`: STC; MOV AX,0xFFFF; INC AX → CF=1, AX=0.
  JIT и интерпретатор совпадают.
- `TestJITDecPreservesCF`: STC; MOV AX,1; DEC AX → CF=1, AX=0.

### 6. JIT выключен по умолчанию

`cpu.New()` → `disableJIT=true`. `CPUReset` сохраняет флаг (иначе
`m.cpu.Reset()` в `Start()` включал JIT обратно — причина повторного
ISOLINUX-падения после «отключения» JIT). После фикса `TestFreeDOSInstallMenu`
PASS.

### 7. Кросс-сборка Windows

`syscall.Mmap` — Unix-only. `jit.go` / `jitcall_amd64.s` / `jit_raw_test.go`
получили `//go:build unix && amd64`; `jit_stub.go` (`!unix || !amd64`)
возвращает `false` из `jitCompile`/`jitExec`.

```
GOOS=windows GOARCH=amd64 go build ./...                 # OK
GOOS=windows GOARCH=amd64 go build -tags desktop ./cmd/ai386  # OK
GOOS=windows GOARCH=amd64 go vet ./...                   # OK
```

`TestJITCompileTB` до правки падал (`jitOps=0`): real mode в `newRunCPU`
даёт `opsize32=false`, `jitCanNative` отклоняет. После setup
`cr0|=1` + `cache[SegCS].ar=0x409b` (D=1) тест PASS.

## Итоговые команды

```sh
go test ./... -count=1          # 11800 passed / 27 packages
go vet ./...                    # no issues
go test ./internal/cpu -run 'TestTBAfterFarJump|TestJITIncDec|TestJITDec' -v
go test ./internal/machine -run '^TestFreeDOSInstallMenu$' -count=1 \
  -timeout=10m -args -freedos-image=$PWD/build/freedos/FD14LIVE.iso
go test ./internal/machine -run '^TestFreeDOSInstallProfile$' -count=1 \
  -timeout=20m -args -freedos-image=$PWD/build/freedos/FD14LIVE.iso
```

Длительности и exit-коды зафиксированы выше; точный токен-счётчик
инструментам недоступен. Гостевые/физические прогоны (SB16, сеть, полная
установка) остаются в [остатке приёмки](../../../docs/doom-debug/remaining-acceptance.md).
