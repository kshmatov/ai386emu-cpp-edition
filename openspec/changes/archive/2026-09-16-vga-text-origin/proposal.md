## Why

Linux использует CRTC Start Address для аппаратной прокрутки; текущий Frame читает только начало окна и может показывать старый или пустой экран.

## What Changes

Отображать текстовые клетки и курсор относительно CRTC Start Address; дать наблюдателю TextCell без чтения/изменения port index.

## Capabilities

### Modified Capabilities
- vga: начало видимого текстового экрана.

## Impact

VGA Frame/text helpers, регрессии прокрутки, диагностика DSL.
