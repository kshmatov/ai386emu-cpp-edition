FreeBSD4.11-RELEASE i386 с официального miniinst ISO,64MiB RAM. После исправления: `npx0: <math processor> on motherboard`, `npx0: INT 16 interface`. Достигнут sysinstall, диагностический тест PASS за167.24с.

Команда: `/tmp/ai386-fpu-native-test -test.run TestFreeBSDDiagnostic -test.v -test.timeout=15m -freebsd-image=/tmp/ai386-freebsd-4.11-miniinst.iso -freebsd-hd=/tmp/ai386-freebsd-fpu-probe.img -freebsd-require-installer -freebsd-control -freebsd-control-at-step=250000000 -freebsd-steps=600000000`. В обоих CONTROL подан `{"quit":true}` для продолжения/выхода.

Test/vet/build и race CPU/Machine/BIOS успешны; IRQ13 и полный набор x87 этим сценарием не принимаются.
