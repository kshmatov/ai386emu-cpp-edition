# Диагностика FreeBSD

Установленная FreeBSD4.11 достигла login, но csh после root login упала SIGSEGV. Core прочитан из UFS без изменения HD: `/tmp/ai386-installed-csh.core`,532480 байт. ELF NT_PRSTATUS: EIP=bfbfffba, EAX=bfbf1430; память[EAX+14h]=002Fh. По sigtramp это `mov 0x14(%eax),%gs`, восстановление GS после SIGCHLD.

`readDescriptor` уже использовал readSystem; `markAccessed` повторно читал access byte через обычный read8 с CPL3 и затем пытался писать. Даже A=1 поэтому приводил к user page fault на supervisor LDT.

Регрессия восьми сочетаний GDT/LDT,A=0/1,RW=0/1: FAIL до исправления, PASS после. При WP=1,A=0,RW=0 ожидается supervisor write PF(code3); уже установленный A не требует записи. Прямое user чтение по-прежнему PF(code5). Дополнительная отсутствующая страница даёт supervisor read PF(code0).

Гостевой повтор с `/tmp/ai386-descriptor-test`: ядро с HD, npx INT16, службы запущены без прежних signal11 sendmail. Вход root достиг shell `#`, падение csh устранено. Снимок `/tmp/ai386-freebsd-descriptor-root.png`; общий test/vet/build и CPU race44.768 с PASS. Дисковая приёмка продолжается отдельно.
