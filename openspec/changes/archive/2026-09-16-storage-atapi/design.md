## Context

Ядро уже смонтировало initrd, но драйверу CD нужен аппаратный ATAPI. BIOS продолжает читать ISO через BlockDevice.

## Decisions

CD определяется размером сектора 2048. Сигнатура 14/EB, IDENTIFY PACKET A1h, PACKET A0h принимает 12 байт. Передачи data-in ограничены host byte count, каждый блок и завершение имеют interrupt reason и IRQ. READ(10/12), INQUIRY, REQUEST SENSE, READ CAPACITY, READ TOC, MODE SENSE(10), TEST UNIT READY, PREVENT/ALLOW и START STOP. Неподдержанные команды дают ILLEGAL REQUEST, readonly сохраняется. DMA не объявляется. IRQ6/14/15 идут через PIC и соблюдают remap/mask.

## Sources

[QEMU ATAPI implementation](https://github.com/qemu/qemu/blob/master/hw/ide/atapi.c) используется для сверки протокола, код не копируется.
