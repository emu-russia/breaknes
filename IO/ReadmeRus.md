# IO Устройства

Данный раздел содержит реализацию логики различных устройств ввода/вывода для NES/Famicom/Dendy, портов расширения и сопроводительных чипов.

TBD: порты расширения пока не реализованы и не рассматриваются.

## Введение

Как может показаться - что может быть сложного в обычном контроллере? Однако...

- Соединение контроллеров различается у NES/Famicom/Dendy. NES/Dendy использует соединение через порты. В Famicom контроллеры неразъёмно соединены с основным юнитом.
- На втором контроллере Famicom пристутствует микрофон (да, такой вот wtf из 90-х)
- На Dendy контроллерах присутствуют Turbo-кнопки
- Количество портов, их тип и реализация I/O на материнской плате различается между всеми консолями

И как вот это всё генерализовать красиво? :-)

## Архитектура IO подсистемы

- Реализация логики устройств находится в нативном коде
- Каждое устройство ввода снабжается уникальным идентификатором, полностью определяющим его модель и реализацию (DeviceID); список всех идентификаторов в io.h
- Описание Motherboard в BoardDescription.json содержит список портов (IOPort). Каждый порт содержит список ID устройств, которые можно к нему подключить (Attach / Detach)
- IO подсистема содержит фабрику для создания устройств по его DeviceID. Созданное устройство определяется описателем, целым числом >= 0 (Handle)
- Каждое устройство предоставляет список своих контролов ввода/вывода (IOState). Каждый IOState определяется целым числом >= 0
- Устройство содержит метод SetState, которым может пользоваться потребитель, для установки состояний IOState контролов

## Настройка IO устройств

Со стороны эмулятора настройка подразумевает просто привязку IOState к какому-то API для работы с устройствами ввода на стороне ПК. 
После чего ассоциативная связь используется для установки SetState подключенному устройству.

Эмулятор содержит пул зарегистрированных устройств с указанием DeviceID (устройства добавляет и удаляет пользователь).

Далее каждое устройство можно настроить (грубо говоря сделать привязку IOState к DirectInput/XInput или ещё какому-то API). У каждого типа устройств свой диалог биндинга кнопок/прочих актуаторов.

После настройки устройство можно подключить или отключить к указанной модели материнской платы (Attach / Detach). После запуска эмуляции для IO подсистемы будут созданы инстанции подключенных устройств, подключены к портам и далее эмулятор может вызывать со своей стороны SetState, чтобы нативная реализация преобразовывала IOState в конкретные сигналы для портов ввода/вывода.

Опять же описание весьма абстрактное, по ходу работ оно кристаллизуется в конкретные классы/методы.

## Абстрактное устройство

DeviceID: 0x00000000

Не содержит никаких IOState.

## Famicom Controller (Port1)

![famicom_controller1](/Wiki/imgstore/famicom_controller1.png)

DeviceID: 0x00000001

|IOState|Actuator|Values|
|---|---|---|
|0|Up|0/1|
|1|Down|0/1|
|2|Left|0/1|
|3|Right|0/1|
|4|Select|0/1|
|5|Start|0/1|
|6|B|0/1|
|7|A|0/1|

Контроллеры Famicom хотя внешне и не съёмные, но внутри корпуса их таки можно отсоединить, поэтому для унификации кода будем считать их съёмными.

## Famicom Controller (Port2)

![famicom_controller2](/Wiki/imgstore/famicom_controller2.png)

DeviceID: 0x00000002

|IOState|Actuator|Values|
|---|---|---|
|0|Up|0/1|
|1|Down|0/1|
|2|Left|0/1|
|3|Right|0/1|
|4|B|0/1|
|5|A|0/1|
|6|Volume|0...255|
|7|MicLevel|0...255|

Значения громкости и уровня микрофона пока в диапазоне UInt8, посмотрим насколько эта модель хорошая в процессе.

## NES Controller (Port1/2)

![nes_controller](/Wiki/imgstore/nes_controller.png)

DeviceID: 0x00000003

|IOState|Actuator|Values|
|---|---|---|
|0|Up|0/1|
|1|Down|0/1|
|2|Left|0/1|
|3|Right|0/1|
|4|Select|0/1|
|5|Start|0/1|
|6|B|0/1|
|7|A|0/1|

## Dendy Turbo Controller (Port1/2)

![dendy_controller](/Wiki/imgstore/dendy_controller.png)

DeviceID: 0x00000004

|IOState|Actuator|Values|
|---|---|---|
|0|Up|0/1|
|1|Down|0/1|
|2|Left|0/1|
|3|Right|0/1|
|4|Select|0/1|
|5|Start|0/1|
|6|TurboB|0/1|
|7|TurboA|0/1|
|8|B|0/1|
|9|A|0/1|

## Virtual Famicom Controller (Port1)

![virt_famicom_controller1](/Wiki/imgstore/virt_famicom_controller1.png)

DeviceID: 0x00010001

|IOState|Actuator|Values|
|---|---|---|
|0|Up|0/1|
|1|Down|0/1|
|2|Left|0/1|
|3|Right|0/1|
|4|Select|0/1|
|5|Start|0/1|
|6|B|0/1|
|7|A|0/1|

## Virtual Famicom Controller (Port2)

![virt_famicom_controller2](/Wiki/imgstore/virt_famicom_controller2.png)

DeviceID: 0x00010002

|IOState|Actuator|Values|
|---|---|---|
|0|Up|0/1|
|1|Down|0/1|
|2|Left|0/1|
|3|Right|0/1|
|4|B|0/1|
|5|A|0/1|
|6|Volume|0...255|
|7|MicLevel|0...255|

## Virtual NES Controller (Port1/2)

![virt_nes_controller](/Wiki/imgstore/virt_nes_controller.png)

DeviceID: 0x00010003

|IOState|Actuator|Values|
|---|---|---|
|0|Up|0/1|
|1|Down|0/1|
|2|Left|0/1|
|3|Right|0/1|
|4|Select|0/1|
|5|Start|0/1|
|6|B|0/1|
|7|A|0/1|

## Virtual Dendy Turbo Controller (Port1/2)

![virt_dendy_controller](/Wiki/imgstore/virt_dendy_controller.png)

DeviceID: 0x00010004

|IOState|Actuator|Values|
|---|---|---|
|0|Up|0/1|
|1|Down|0/1|
|2|Left|0/1|
|3|Right|0/1|
|4|Select|0/1|
|5|Start|0/1|
|6|TurboB|0/1|
|7|TurboA|0/1|
|8|B|0/1|
|9|A|0/1|
