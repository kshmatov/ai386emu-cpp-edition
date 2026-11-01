## ADDED Requirements

### Requirement: Жизненный цикл сетевого устройства


Машина SHALL продвигать сетевое устройство по виртуальному времени и при
остановке освобождать его ресурсы. Повторный вызов Stop SHALL быть безопасен.

#### Scenario: Остановка с активным соединением
- **WHEN** VM останавливается при активном исходящем TCP-соединении
- **THEN** сетевое устройство прекращает обработку и освобождает сокеты без
  утечки goroutine или блокировки остановки

### Requirement: RX не повреждает непрочитанные кадры

NE2000 SHALL отбрасывать входящий пакет, который достигает BNRY при связывании любой из его страниц, с OVW и без изменения CURR/непрочитанной RAM.

#### Scenario: Многостраничный пакет пересекает BNRY
- **WHEN** конец пакета находится за BNRY в линейном или wrapped кольце
- **THEN** пакет отбрасывается, непрочитанные данные остаются целыми

#### Scenario: Буфер освобождён
- **WHEN** гость продвинул BNRY и места достаточно
- **THEN** следующий пакет принимается с правильным next-page и PRX

### Requirement: TCP timeout retransmits the earliest unacknowledged segment

The user-mode NAT SHALL retransmit only the earliest unacknowledged segment
of a TCP flow when that segment reaches its configured retransmission timeout.
Later segments SHALL NOT bypass the earliest segment's unexpired timer.

#### Scenario: Several segments remain unacknowledged after receiver overflow
- **WHEN** multiple TCP segments are outstanding and their timers have expired
- **THEN** one timer advance retransmits only the earliest outstanding segment
- **AND** a cumulative acknowledgement allows the next unacknowledged segment
  to become eligible for retransmission
