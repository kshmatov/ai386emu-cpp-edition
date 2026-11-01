# Реестр целочисленных команд и системных механизмов DX4

> История Go-издания: упоминания Go-команд, путей и цифр ниже относятся к Go-реализации (ai386).

Срез 2026-09-11 после cpu-dx4-profile. Реестр закрывает инвентаризацию 1.2, а не реализацию CPU. Все ссылки к Go-файлам ниже относятся к `internal/cpu/` от корня репозитория. Для каждой строки действуют общие оси M; отдельный тест подтверждает только свои случаи.

## Источники и правила развёртки

- [Intel 273021-001](https://www.ardent-tool.com/CPU/docs/Intel/486/embedded/manuals/273021-001.pdf), гл.12 — кодировки и группы; гл.4 — регистры; гл.5–6 — режимы/защита; гл.11 — debug; приложение B — test registers. Это источник всех строк, с ограничением модели из [паспорта](../../cpu-functional-completion/cpu-profile.md).
- [Intel AP-485 241618-004](https://www.ardent-tool.com/CPU/docs/Intel/CPUID/241618-004.pdf), §3.2–3.3 — CPUID. Прямые ссылки также сохранены в [базовой спеке CPU](../../../specs/cpu/spec.md).
- Реализация форм: `decode_table.go`, `decode_opf.go`, `decode_exec.go`, затем `runstep.go`. Комментарии о поздних CPU в декодере не служат источником допустимости.
- `v` означает **два отдельных варианта 16 и 32**, `r/m` — **отдельные register и memory формы**; `r` в opcode — каждый допустимый регистр. Списки через `;`, `/` между мнемониками и `×` разворачиваются в отдельные случаи. Memory-only формы исключают Mod=3. `cc` разворачивается в O, NO, B/C/NAE, AE/NB/NC, E/Z, NE/NZ, BE/NA, A/NBE, S, NS, P/PE, NP/PO, L/NGE, GE/NL, LE/NG, G/NLE.
- R/P/V: real / protected / VM86. Если строка не ограничивает режим, проверяются все три. В P — CPL0–3 и все применимые DPL/RPL/IOPL. Для privileged форм R допускает исполнение, P требует CPL0, V проверяет отказ; protected-only отдельно отвергает R/V.
- **M (открытая общая матрица каждой формы):** O16/O32 с 66 и без него; A16/A32 с 67 и без него (у регистровых форм отсутствие изменения семантики); CS.D и SS.B независимо; все допустимые ModRM, SIB, displacement и segment override; wrap эффективного адреса, сегментные границы, null/not-present/read-only, paging R/W/U/WP/A/D, #AC; fault на fetch/операнде/stack и restart без преждевременных изменений. Не применимые оси не создают новых команд.
- LOCK: только разрешённые memory read-modify-write назначения, включая направление; прочие сочетания #UD. REP/REPE/REPNE: строковые формы, нулевой count, DF±, прерывание между элементами и повтор. Префиксы 26/2E/36/3E/64/65/66/67/F0/F2/F3, повторы, длина 15 байт, ошибки fetch требуют общей проверки в 4.6. Допустимость нестандартных сочетаний не выводится из современного CPU.
- Флаги указаны символическими масками определённых битов; `preserved` означает отсутствие изменения EFLAGS. Undefined не сравниваются. Сохранение не перечисленных флагов также проверяется, исключая явно описанные системные переходы. Для count-dependent флагов нужен отдельный случай на каждый класс count, а не одна общая маска.
- Статус M — **частично, не принято целиком**. Каждый элемент декартова произведения наследует этот явный пробел, пока отдельная запись приёмки не укажет его тест, исходные условия и маску. Conformance (`conformance_test.go`, `testdata/80386/README.md`) дополняет строки, но не доказывает защищённый режим или особенности DX4.

## Команды

| Команда | Opcode/group | Отдельные формы | Определённые флаги / ошибки | Код | Имеющиеся тесты | Статус и остаток |
|---|---|---|---|---|---|---|
| ADD | 00–05; 80/81/83 /0 | r/m8,r8; r/mv,rv; r8,r/m8; rv,r/mv; AL,i8; Av,iv; r/m8,i8; r/mv,iv; r/mv,sx(i8) | CF PF AF ZF SF OF | alu.go | alu_test.go, arithmetic_regression_test.go | M; полная матрица форм не принята |
| OR | 08–0D; 80/81/83 /1 | r/m8,r8; r/mv,rv; r8,r/m8; rv,r/mv; AL,i8; Av,iv; r/m8,i8; r/mv,iv; r/mv,sx(i8) | CF=OF=0; PF ZF SF; AF undefined | alu.go | alu_test.go, arithmetic_regression_test.go | M; полная матрица форм не принята |
| ADC | 10–15; 80/81/83 /2 | r/m8,r8; r/mv,rv; r8,r/m8; rv,r/mv; AL,i8; Av,iv; r/m8,i8; r/mv,iv; r/mv,sx(i8) | CF PF AF ZF SF OF | alu.go | alu_test.go, arithmetic_regression_test.go | M; полная матрица форм не принята |
| SBB | 18–1D; 80/81/83 /3 | r/m8,r8; r/mv,rv; r8,r/m8; rv,r/mv; AL,i8; Av,iv; r/m8,i8; r/mv,iv; r/mv,sx(i8) | CF PF AF ZF SF OF | alu.go | alu_test.go, arithmetic_regression_test.go | M; полная матрица форм не принята |
| AND | 20–25; 80/81/83 /4 | r/m8,r8; r/mv,rv; r8,r/m8; rv,r/mv; AL,i8; Av,iv; r/m8,i8; r/mv,iv; r/mv,sx(i8) | CF=OF=0; PF ZF SF; AF undefined | alu.go | alu_test.go, arithmetic_regression_test.go | M; полная матрица форм не принята |
| SUB | 28–2D; 80/81/83 /5 | r/m8,r8; r/mv,rv; r8,r/m8; rv,r/mv; AL,i8; Av,iv; r/m8,i8; r/mv,iv; r/mv,sx(i8) | CF PF AF ZF SF OF | alu.go | alu_test.go, arithmetic_regression_test.go | M; полная матрица форм не принята |
| XOR | 30–35; 80/81/83 /6 | r/m8,r8; r/mv,rv; r8,r/m8; rv,r/mv; AL,i8; Av,iv; r/m8,i8; r/mv,iv; r/mv,sx(i8) | CF=OF=0; PF ZF SF; AF undefined | alu.go | alu_test.go, arithmetic_regression_test.go | M; полная матрица форм не принята |
| CMP | 38–3D; 80/81/83 /7 | r/m8,r8; r/mv,rv; r8,r/m8; rv,r/mv; AL,i8; Av,iv; r/m8,i8; r/mv,iv; r/mv,sx(i8) | CF PF AF ZF SF OF | alu.go | alu_test.go, arithmetic_regression_test.go | M; полная матрица форм не принята |
| TEST | 84/85; A8/A9; F6/F7 /0 | r/m8,r8; r/mv,rv; AL,i8; Av,iv; r/m8,i8; r/mv,iv | CF=OF=0; PF ZF SF; AF undefined | alu.go | alu_test.go | M; полная матрица форм не принята |
| INC / DEC | 40+r/48+r; FE /0,/1; FF /0,/1 | rv; r/m8; r/mv | PF AF ZF SF OF; CF preserved | alu.go | alu_test.go | M; полная матрица форм не принята |
| NEG / NOT | F6/F7 /3,/2 | r/m8; r/mv | NEG: CF PF AF ZF SF OF; NOT: preserved | alu.go | alu_test.go | M; полная матрица форм не принята |
| MUL / IMUL unary | F6/F7 /4,/5 | r/m8 → AX; r/mv → Dv:Av | CF OF; other arithmetic undefined | alu.go | alu_test.go | M; полная матрица форм не принята |
| IMUL binary/ternary | 0F AF; 69; 6B | rv,r/mv; rv,r/mv,iv; rv,r/mv,sx(i8) | CF OF; other arithmetic undefined | alu.go | alu_test.go | M; полная матрица форм не принята |
| DIV / IDIV | F6/F7 /6,/7 | AX ÷ r/m8; Dv:Av ÷ r/mv | arithmetic undefined; #DE zero/quotient overflow | alu.go | alu_test.go | M; полная матрица форм не принята |
| CBW / CWDE / CWD / CDQ | 98; 99 | AL→AX; AX→EAX; AX→DX:AX; EAX→EDX:EAX | preserved | alu.go | alu_test.go | M; полная матрица форм не принята |
| DAA / DAS / AAA / AAS | 27; 2F; 37; 3F | implicit AL/AX | DAA/DAS: CF AF SF ZF PF; AAA/AAS: CF AF; others undefined | bcd.go | bcd_test.go | M; полная матрица форм не принята |
| AAM / AAD | D4 ib; D5 ib | implicit AX; documented decimal base 0A | SF ZF PF; other arithmetic undefined; AAM #DE base0 | bcd.go | bcd_test.go | M; other bases separately audited as extension |
| ROL / ROR / RCL / RCR / SHL(SAL) / SHR / SAR | C0/C1; D0–D3; /0,/1,/2,/3,/4,/5,/7 | r/m8 or r/mv × count 1/CL/i8 | count0 preserved; rotate CF, OF for count1; shifts SF ZF PF, CF/OF depend count; AF undefined | shift.go | shift_test.go, arithmetic_regression_test.go | M; полная матрица форм не принята |
| SHLD / SHRD | 0F A4/A5; 0F AC/AD | r/mv,rv,i8; r/mv,rv,CL | count0 preserved; SF ZF PF CF for defined counts; OF count1; AF undefined | shift.go | shift_test.go | M; полная матрица форм не принята |
| BT / BTS / BTR / BTC | 0F A3/AB/B3/BB; 0F BA /4–/7 | r/mv,rv; r/mv,i8 | CF; ZF preserved; OF SF AF PF undefined | bitops.go | shift_test.go, arithmetic_regression_test.go | M; полная матрица форм не принята |
| BSF / BSR | 0F BC/BD | rv,r/mv | ZF; other arithmetic undefined; destination undefined on zero | bitops.go | shift_test.go | M; полная матрица форм не принята |
| MOV | 88–8B; A0–A3; B0+r/B8+r; C6/C7 /0 | r/m8,r8; r/mv,rv; reverse; AL/Av,moffs and reverse; r8,i8; rv,iv; r/m8,i8; r/mv,iv | preserved | data.go | data_test.go, decode_test.go | M; полная матрица форм не принята |
| MOVSX / MOVZX | 0F BE/BF; 0F B6/B7 | r16,r/m8; r32,r/m8; r32,r/m16 | preserved | data.go | data_test.go | M; полная матрица форм не принята |
| MOV segment | 8C; 8E | r/m16,Sreg; Sreg,r/m16 (CS load forbidden); register destination with O32 requires generation-specific upper bits | preserved; SS shadow; #UD/#GP/#SS/#NP | data.go | segment_load_fault_test.go, interrupt_shadow_test.go | M; полная матрица форм не принята |
| LEA | 8D | rv,m (address only; Mod=3 forbidden) | preserved | data.go | data_test.go, arithmetic_regression_test.go | M; полная матрица форм не принята |
| LES / LDS / LSS / LFS / LGS | C4/C5; 0F B2/B4/B5 | rv,m16:16 or m16:32 (memory only) | preserved; #GP/#SS/#NP; LSS shadow | call.go | far_load_fault_test.go | M; полная матрица форм не принята |
| XCHG / NOP | 86/87; 90+r | r/m8,r8; r/mv,rv; Av,rv; 90 NOP | preserved; memory XCHG implicit lock | data.go, runstep.go | data_test.go | M; полная матрица форм не принята |
| XADD | 0F C0/C1 | r/m8,r8; r/mv,rv | CF PF AF ZF SF OF | i486.go | xadd_regression_test.go, i486_test.go | M; полная матрица форм не принята |
| CMPXCHG | 0F B0/B1 | r/m8,r8; r/mv,rv | CF PF AF ZF SF OF | i486.go | cmpxchg_fault_test.go, i486_operand_test.go | M; полная матрица форм не принята |
| BSWAP | 0F C8+r | r32; O16 undefined, not accepted as swap16 | preserved | bitops.go | shift_test.go | M; полная матрица форм не принята |
| XLAT | D7 | DS:[BX/EBX+AL] → AL, segment override | preserved | alu.go | alu_test.go | M; полная матрица форм не принята |
| PUSH / POP | 50+r/58+r; 68/6A; FF /6; 8F /0 | rv; r/mv; PUSH iv/sx(i8); stack B independent of O/A | preserved | data.go | data_test.go, stack_limit_regression_test.go, arithmetic_regression_test.go | M; полная матрица форм не принята |
| PUSH / POP segment | 06/07; 0E; 16/17; 1E/1F; 0F A0/A1/A8/A9 | ES/SS/DS/FS/GS; PUSH CS only; O16/O32 stack slots | preserved; POP SS shadow; selector faults | data.go | segment_load_fault_test.go, shift_test.go | M; полная матрица форм не принята |
| PUSHA(D) / POPA(D) | 60/61 | O16/O32; independent stack B; original SP/ESP saved, skipped on POPA | preserved | data.go | data_test.go, stack_limit_regression_test.go | M; полная матрица форм не принята |
| PUSHF(D) / POPF(D) | 9C/9D | O16/O32 × R/P/V, CPL/IOPL restrictions | EFLAGS field/privilege masks; RF/VM image rules; ID profile | call.go | call_test.go, v86_test.go, dx4_profile_test.go | M; полная матрица форм не принята |
| LAHF / SAHF | 9F/9E | AH↔SF ZF AF PF CF (LAHF reserved image) | listed five flags on SAHF; otherwise preserved | runstep.go | call_test.go, conformance_test.go | M; полная матрица форм не принята |
| CLC / STC / CMC / CLD / STD | F8/F9/F5/FC/FD | implicit | CF clear/set/complement; DF clear/set | runstep.go | conformance_test.go | M; полная матрица форм не принята |
| CLI / STI | FA/FB | R/P/V; IOPL/CPL checks; STI IRQ shadow | IF only if allowed; #GP before change | runstep.go, io.go | interrupt_shadow_test.go, v86_monitor_test.go | M; IOPL separated from TSS bitmap in task4.1 |
| MOVS / CMPS / STOS / LODS / SCAS | A4/A5; A6/A7; AA/AB; AC/AD; AE/AF | byte/word/dword × A16/A32; implicit SI/DI/CX; REP/REPE/REPNE as applicable | CMPS/SCAS arithmetic flags; others preserved; DF controls progress | string.go | string_test.go, review_regression_test.go | M; полная матрица форм не принята |
| IN / OUT | E4–E7; EC–EF | AL/AX/EAX × port i8/DX | preserved; permission before bus side effect | io.go | io_test.go | M; TSS bitmap absent:4.1; boundary port audit |
| INS / OUTS | 6C–6F | byte/word/dword × A16/A32 × REP/no REP, port DX | preserved; DF; partial REP progress on fault | io.go | io_test.go, io_fault_test.go | M; TSS bitmap absent:4.1; faults and irreversible bus ordering |
| JMP near/far | EB/E9; FF /4; EA; FF /5 | rel8/relv; r/mv; ptr16:v; m16:v (far indirect memory only) | preserved except task switch | control.go, protmode.go | control_test.go, protmode_test.go | M; far privilege/task faults4.5–4.6 |
| Jcc | 70+cc; 0F 80+cc | 16 conditions × rel8/relv | preserved; reads OF/CF/ZF/SF/PF as condition | control.go | control_test.go | M; полная матрица форм не принята |
| SETcc | 0F 90+cc | 16 conditions × r/m8 | preserved | bitops.go | shift_test.go | M; полная матрица форм не принята |
| LOOP / LOOPE / LOOPNE / JCXZ / JECXZ | E2/E1/E0/E3 | rel8; CX/ECX selected by A16/A32 | preserved; LOOPE/NE read ZF | control.go | control_test.go | M; полная матрица форм не принята |
| CALL near/far | E8; FF /2; 9A; FF /3 | relv; r/mv; ptr16:v; m16:v | preserved except task switch | call.go, protmode.go | call_test.go, protmode_test.go, stack_limit_regression_test.go | M; gates/tasks4.5, stack/target fault matrix4.6 |
| RET / RETF | C3/C2; CB/CA | O16/O32 × no immediate/i16 stack adjustment | preserved except privilege segment invalidation | call.go, protmode.go | call_test.go, retf_width_test.go | M; outer privilege/stack fault matrix4.6 |
| ENTER / LEAVE | C8 iw ib; C9 | O16/O32 × independent stack B; nesting 0–31 | preserved | call.go | call_test.go, arithmetic_regression_test.go, stack_limit_regression_test.go | M; полная матрица форм не принята |
| INT3 / INT / INTO / IRET(D) | CC/CD ib/CE/CF | real IVT; protected gates; VM86; O16/O32 IRET; NT task return | gate clears TF/RF/NT/VM as applicable, IF for interrupt gate; IRET privilege masks | interrupt.go, stubs.go, protmode.go | softint_test.go, interrupt_test.go, idt_test.go, v86_monitor_test.go, dx4_profile_test.go | M; nested faults4.2, tasks4.5, flags/AC4.6 |
| BOUND | 62 | rv,m pair v:v; memory only | preserved; #BR signed lower/upper limits | alu.go | arithmetic_regression_test.go | M; полная матрица форм не принята |
| ARPL | 63 | r/m16,r16; protected only, not VM86 | ZF | selector_instructions.go | selector_instructions_test.go | M; полная матрица форм не принята |
| LAR / LSL | 0F 02/03 | rv,r/m16 selector; protected only, not VM86 | ZF; destination unchanged on invalid selector | protmode.go | protmode_test.go | M; полная матрица форм не принята |
| VERR / VERW | 0F 00 /4,/5 | r/m16 selector; protected only, not VM86 | ZF | selector_instructions.go | selector_instructions_test.go | M; полная матрица форм не принята |
| SLDT / STR / LLDT / LTR | 0F 00 /0–/3 | selector r/m16; protected only; LLDT/LTR CPL0; O32 register store upper bits audit | preserved; #UD/#GP/#NP; LTR busy update | selector_instructions.go, protmode.go | selector_instructions_test.go, protmode_test.go | M; TSS/descriptor write fault stages4.5–4.6 |
| SGDT / SIDT / LGDT / LIDT | 0F 01 /0–/3 | memory6 only; O16/O32 base formatting; loads CPL0 in P/V | preserved; privilege and memory faults | protmode.go | protmode_test.go, system_access_test.go | M; полная матрица форм не принята |
| SMSW / LMSW / CLTS | 0F 01 /4,/6; 0F 06 | SMSW r/m16 or reg O32; LMSW r/m16; CLTS implicit; writes privileged | preserved; CR0 fields, PE cannot clear via LMSW | protmode.go, runstep.go | smsw_test.go, lmsw_test.go | M; полная матрица форм не принята |
| MOV CR | 0F 20/22 | r32↔CR0/CR2/CR3, CPL0; CR1/CR4+ profile/reserved audit | preserved; CR0 illegal combinations #GP; unsupported registers #UD | protmode.go | protmode_test.go, dx4_profile_test.go | M; reserved/read-write masks and transitions4.6 |
| MOV DR | 0F 21/23 | r32↔DR0–DR7, CPL0; DR4/5 alias audit | preserved except debug exception machinery | debug_registers.go | debug_registers_test.go | M; state storage exists; GD/DR6/DR7 and breakpoints4.3 |
| MOV TR | 0F 24/26 | r32↔TR3–TR7, CPL0; cache/TLB test operations | preserved; invalid index/privilege faults | decode_opf.go | — | ABSENT: 24 placeholder, 26 not wired; test state/semantics4.6; 25 is not reverse MOV TR |
| INVD / WBINVD / INVLPG | 0F 08/09; 0F 01 /7 | implicit / m effective address only; CPL0; INVLPG no data load | preserved; #GP privilege, INVLPG Mod3 #UD | i486.go | i486_operand_test.go, i486_test.go | M; functional uncached memory and TLB invalidation audit4.6 |
| CPUID | 0F A2 | EAX leaf → EAX/EBX/ECX/EDX, all modes | preserved; LOCK #UD | i486.go | dx4_profile_test.go | Accepted scoped profile; general fetch/fault/prefix matrix M still applies |
| HLT | F4 | R/P CPL0; VM86 #GP; wake signals | preserved | runstep.go | requestirq_test.go, cpu_test.go | M; NMI wake4.4, debug/IRQ priorities4.3–4.4 |

## Системные механизмы: отдельные строки приёмки

Каждая строка также включает fetch/operand/descriptor/stack faults до и после разрешённых изменений, приоритет векторов, error code, saved EIP/EFLAGS и повтор. Источник — соответствующие главы 4–6/11 Intel 273021-001.

| Механизм и варианты | Код / существующие тесты | Пробел / задача |
|---|---|---|
| Reset, CPUID/ID, 4 КиБ paging без PSE | reset.go, i486.go, paging.go; dx4_profile_test.go | Профиль принят; общий набор переходов CR0/CR3 остаётся 4.6 |
| Сегменты: code/data/system, conforming, expand-down, null, accessed, RPL/CPL/DPL, hidden cache | protmode.go; descriptor_accessed_test.go, segment_fault_regression_test.go, segment_load_fault_test.go | Полная матрица 4.6 |
| Paging: present/U/S/R/W/WP, PDE/PTE A/D, CR2, cross-page, supervisor system access, CR3 reload | paging.go; paging_permissions_test.go, paging_test.go, system_access_test.go | Все взаимодействия с fault priority и TLB/cache/test registers 4.6 |
| TSS I/O bitmap: IN/OUT/INS/OUTS, ширина1/2/4, VM86, limit и sentinel | io.go проверяет IOPL; io_test.go, io_fault_test.go | Bitmap отсутствует; 4.1 |
| IRQ, IF, STI/MOV SS/POP SS/LSS shadow, HLT | runstep.go; interrupt_shadow_test.go, requestirq_test.go | IRQ случаи частично приняты; совмещение с debug/NMI 4.3–4.4 |
| NMI: IF0, latch/blocking, повтор, IRET, HLT, priority | interrupt.go, runstep.go: точки интеграции; отдельного полного теста нет | Отсутствует принятый механизм; 4.4 |
| Debug: TF single-step, RF restart, DR execution/data, GD, DR6/7, SS shadow, task trap | debug_registers.go, runstep.go; debug_registers_test.go | Хранение DR не реализует механизм; 4.3 |
| Вложенные исключения, #DF, ошибка доставки #DF, изоляция CPU | stubs.go; interrupt_regression_test.go, idt_test.go | delivering глобальный; таблица сочетаний и параллельные CPU 4.2 |
| Task switch: CALL/JMP/INT/IRET, task gate/direct TSS, 16/32 TSS, busy/backlink/NT/TS, CPL/VM86 | protmode.go, stubs.go; protmode_test.go | Все стадии fault до/после точки переключения и descriptor writes 4.5 |
| #AC: CR0.AM × EFLAGS.AC × CPL, ширины, implicit stack/system access и PF priority | paging.go, runstep.go: пути доступа; полной матрицы нет | Проверить фактическую доставку и POPFD/IRETD AC; 4.6 |
| #NM/#MF, WAIT/no-wait, CR0.EM/TS/MP/NE и pending exception | fpu.go, runstep.go; fpu_control_test.go, fpu_native_error_test.go | Реестр x87 1.3 и полная матрица 4.7 |
| Legacy FERR/IRQ13 при NE0, PIC mask, clear/retrigger | CPU/Machine: точки интеграции; сквозного теста нет | Отсутствует; 4.8 |
| SMI/SMM/RSM, save-state и SMRAM профиля DX4 | 0F AA сейчас opUnd; обработчика и тестов нет | Требуется сверка model-specific разделов перед реализацией в 4.6; не исключать из полноты по отсутствию handler |

## Кодировки, которые нельзя считать готовыми обычными командами

| Кодировки | Состояние и основание дальнейшей проверки |
|---|---|
| 9B, D8–DF | WAIT и x87 вынесены в отдельный полный реестр 1.3; это обязательная часть CPU |
| F1 ICEBP | opUnimplemented; явный пробел/model-specific сверка 4.3, не принят как #UD по умолчанию |
| D6 SALC | Реализована и есть 386 conformance; недокументированное расширение, не доказательство принадлежности DX4 |
| 82 group1; F6/F7 /1 TEST; shift /6 | Алиасы в декодере; отдельная проверка принадлежности и undefined flags, не новые документированные команды |
| AAM/AAD base !=10 | Декодируются; дополнительные значения основания требуют отдельного model-specific подтверждения |
| 0F 04/05/07; 0F A6/A7 | LOADALL/STOREALL/старые варианты и зарезервированные формы: не считать современными SYSCALL/SYSENTER; проверить отказ по профилю |
| 0F 25 | Placeholder в коде; обратный MOV TR имеет 0F26; нельзя закреплять ошибочный комментарий как контракт |
| 0F 30/31/32, CMOV 0F40–4F, CMPXCHG8B 0FC7 /1, SIMD | Расширения поздних поколений, отсутствующие в выбранном профиле; проверка #UD, а не добавление реализации по имени placeholder |
| 0F AA RSM | Отдельный открытый системный механизм выше; не смешивать с поздними расширениями |
| Остальные reserved entries, неверные group/ModRM, MOV CS, invalid LOCK | Негативная матрица #UD и fault priority в 4.6; default opUnd не доказывает корректность длины/порядка fault |

## Проверка полноты инвентаризации

Первичная таблица сверена по диапазонам 00–3F (8 ALU семейств, сегменты, BCD, escape), 40–5F (регистры/стек), 60–6F (stack/BOUND/ARPL/prefix/IMUL/string I/O), 70–BF (conditions/data/flags/strings), C0–DF (shift/stack/control/BCD/x87), E0–FF (flow/I/O/prefix/ALU groups). Группы1–8 развёрнуты в строках; group9 относится к позднему CMPXCHG8B. В 0F отдельно сверены 00–09, 20–26, 80–9F, A0–AF, B0–BF, C0–CF; прочие слоты учтены как reserved/поздние. Проверка не ограничивалась перечнем реализованных opID: MOV TR и SMM оставлены явными пробелами.

Приёмка 1.2 означает наличие реестра. Для закрытия 4.6 и 5.4 необходимо заменить каждый открытый элемент M конкретным доказательством или исправлением, включая ранее неизвестные пробелы. Общий статус функциональной совместимости остаётся открытым.
