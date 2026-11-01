## ADDED Requirements

### Requirement: Large disk geometry

BIOS SHALL возвращать AH08 maximum cylinder не более1023 без оборачивания, сохраняяheads/sectors. AH48 SHALL заполнять26 байт по DS:SI: фактический размер, flags,32-битныеCHS,64-битное число секторов и16-битный sector size. Буфер меньше26 SHALL отклоняться с CF/AH error без изменения; байты за26 SHALL сохраняться.

Основание: GNU GRUB0.97, официальный [архив исходников](https://alpha.gnu.org/gnu/grub/grub-0.97.tar.gz), stage2/bios.c get_diskinfo. Полный диск доступен через LBA независимо от legacy CHS-предела.

#### Scenario: 2GiB HD
- **WHEN** загрузчик спрашивает параметры2ГиБ диска
- **THEN** AH08 сообщает максимальный цилиндр1023, AH48 —4194304 секторов по512 байт, без потери ёмкости.

#### Scenario: Buffer bounds
- **WHEN** размер входного буфера меньше26 либо больше26
- **THEN** слишком малый буфер не меняется; успешный вызов пишет только26 байт.
