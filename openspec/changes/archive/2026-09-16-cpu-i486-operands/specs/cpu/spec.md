## ADDED Requirements

### Requirement: Ширина и привилегии команд i486

CPU SHALL исполнять CMPXCHG byte с байтовыми операндами, сохраняя остальные биты регистров. INVD, WBINVD и INVLPG SHALL вызывать #GP при CPL>0 в protected/v86 режиме; INVLPG с регистровым операндом SHALL вызывать #UD.

#### Scenario: Байтовое сравнение

- **WHEN** CMPXCHG BL,CL выполняется при EAX=1111AA07h, EBX=2222BB07h и ECX=3333CC09h
- **THEN** ZF=1, EAX не меняется, EBX=2222BB09h

#### Scenario: Привилегированные команды

- **WHEN** INVD, WBINVD или INVLPG исполняется при CPL=3
- **THEN** возникает #GP(0)

#### Scenario: Форма INVLPG

- **WHEN** INVLPG кодируется с ModRM.mod=3
- **THEN** возникает #UD
