## Why
FreeBSD npx_probe1 включает CR0.NE и размаскирует ZE, но эмулятор не доставляет16 после FDIV/FNOP. Ядро пишет error reporting broken.

## What Changes
Отложенная доставка #MF при pending unmasked status и NE, FWAIT с проверкой TS/MP, корректное подавление результата/попа при размаскированном делении на ноль.

## Capabilities
### Modified Capabilities
- `cpu`: native x87 exception delivery.

## Impact
Декодирование WAIT и x87; регрессии; повтор npx probe. Остальная полнота исключений/IRQ13 остаётся в2.7–2.8.
