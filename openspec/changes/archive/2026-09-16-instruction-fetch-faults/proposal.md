## Why

DSL получает #UD по 4003CFFF: первый байт 0F находится в памяти, следующая страница ещё отсутствует. Декодер подставляет нули после #PF, а Step проверяет opcode/исполняет раньше доставки fault.

## What Changes

Доставлять fault выборки до интерпретации opcode и исполнения; сохранять CR2 первого недоступного байта.

## Capabilities

### Modified Capabilities
- cpu: атомарность instruction fetch.

## Impact

Step, fetchCode, регрессии demand paging.
