## ADDED Requirements

### Requirement: Atomic far data loads

LDS/LES/LSS/LFS/LGS SHALL читать весь указатель и успешно проверять сегмент до изменения GPR. POP sreg SHALL сохранять прежний SP/ESP при ошибке операнда или сегмента. POP SS SHALL вычислять новое значение указателя по старому SS.B, независимо от нового. Page fault SHALL сохранять первый CR2 и соответствующий код; selector/cache также остаются прежними.

#### Scenario: Fault during operand or descriptor load
- **WHEN** pointer/selector/accessed недоступен либо дескриптор недопустим
- **THEN** GPR и указатель стека не меняются до доставки исключения.

#### Scenario: Successful stack segment switch
- **WHEN** POP SS меняет ширину адреса стека
- **THEN** приращение SP вычисляется по предыдущей ширине, с учётом operand size.
