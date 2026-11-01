## Decisions

Сохранить math/big.Float и extended80 сериализацию. Состояние изменяется только после проверки памяти. Операции должны быть общими для всех адресов/программ. Не заявлять полный набор transcendental и точную модель всех немаскированных исключений.

## Sources

[Intel Vol.2 opcode tables FSUB/FSUBR](https://cdrdv2-public.intel.com/782156/325383-sdm-vol-2abcd.pdf). DE E0+i означает FSUBRP, DE E8+i — FSUBP; прежняя трактовка objdump AT&T была неверна.
