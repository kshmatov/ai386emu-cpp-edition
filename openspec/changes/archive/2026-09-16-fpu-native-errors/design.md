## Контракт
При CR0.NE=1 pending=(status & ~control & 3Fh) вызывает #MF на следующей waiting x87 или FWAIT до обращения к памяти/изменения стека. FNINIT/FNCLEX/FNSTSW/FNSTCW/FNSAVE не ждут. FWAIT при MP=1 и TS=1 вызывает #NM; EM сам по себе не запрещает FWAIT. NE=0 не доставляет native16; legacy FERR/IRQ13 отдельно.

Размаскированное ZE устанавливает ZE/ES/B, сохраняет операнды и TOP; masked ZE оставляет бесконечность и обычный pop. Полный контракт других арифметических исключений не заявляется.

Источники: Intel SDM Vol3A exception16; FreeBSDreleng/4.11 sys/i386/isa/npx.c fp_divide_by_0.

Источники: https://www.intel.com/content/dam/www/public/us/en/documents/manuals/64-ia-32-architectures-software-developer-vol-3a-part-1-manual.pdf ; https://github.com/freebsd/freebsd-src/blob/releng/4.11/sys/i386/isa/npx.c .
