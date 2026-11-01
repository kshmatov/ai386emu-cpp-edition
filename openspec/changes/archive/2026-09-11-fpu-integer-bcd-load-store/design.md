## Context

Integer store округляет значение по RC, а BCD store требует 18 десятичных цифр. При unmasked invalid или precision запись и FISTP pop должны быть подавлены.

## Decisions

Память полностью транслируется до вычисления и записи. Integer conversion возвращает indefinite только при masked IE; unmasked IE/PE завершает инструкцию без записи. FBLD валидирует каждый nibble 0–9; invalid masked загружает indefinite, unmasked сохраняет стек.

## Validation

Проверены знаки, пределы 16/32/64, четыре RC, все маски IE/PE, BCD round-trip, overflow и недопустимые цифры, включая page boundary atomicity.
