# Mappers

Данный компонент объединяет в себе всё что связано с портом картриджа и мапперами. Просьба рассматривать в данном контексте термин "Mappers" как синоним картриджа.

Реализация мапперов сделана (в процессе) двумя способами:
- Типовой симуляцией схем распространенных мапперов на C++, используя BaseLogicLib
- Возможность симуляции необычных мапперов с применением микрокода 6502 + метаинформация JSON

## Список проблемных мест

Список того, что сейчас проблемное в этом компоненте:
- Не очень понятна сделана поддержка работы с дампами памяти (PRG/CHR). Регион CHR называется "CHR-ROM" со времен ранних стадий разработки. PRG вообще не поддерживается
- Очень хаотична сделана эмуляция мапперов формата .nes, в смысле трансляции номера маппера в компоненты платы картриджа. Это ещё усложняется тем, что один и тот же маппер (например MMC1) может быть реализован в разных картриджах уймой вариаций и как формировать и соединять компоненты картриджа для его симуляции не очень понятно
- Нет поддержки ROM чипов. Сейчас сделано всё обычных byte массивом. Нужно сделать по аналогии с чипом `Generic SRAM` в BaseBoardLib
- Эмуляция MMC1 требует отладки и проверки. Скорее всего что-то не так с делителем.

## Абстрактный картридж

В основе эмуляции мапперов лежит такой же принцип, как и для остальных частей эмулятора: берём устройство или разъем, пихаем туда сигналы и что-то происходит.

В случае с картриджем используется "абстрактный порт", который обобщает в себе все интерфейсы реального разъёма картриджа NES/Famicom (звук, Expansion Port), но в конкретной инстанции используются только необходимые (например если создана материнская плата NES - звука с картриджа не будет).

Фабрика (CartridgeFactory) создаёт инстанцию картриджа для основной части эмулятора на базе мета-информационных признаков (заголовок NES, метаинформация из JSONES).

## Микрокод мапперов

NES/Famicom славится своим большим количеством мапперов. К лицензионным мапперам добавилось ещё просто овер-10001 китайских мапперов, с минимальными вариациями, но для каждого приходится заводить свой "номер" в формате .NES.

А что если дать продвинутому пользователю самостоятельно говорить на "слепок" ромов - какой маппер нужно для них использовать. И не просто какой маппер, а самому определять поведение маппера.

У нас есть:
- Набор ромок CHR/PRG, возможно слепленных как .nes файл, а возможно и просто лежащими кучей файлов
- Схема маппера / вспомогательных чипов, которые есть на карике
- Эмулятор 6502
- Микропрограмма на 6502, которая симулирует обращение к пинам картриджа и реализует логику "дикого" маппера

|![mappers_ucode1](mappers_ucode1.png)|![mappers_ucode2](mappers_ucode2.png)|
|---|---|

Конечно, большую часть мапперов проще симулировать на C++, но для универсальности добавим возможность написания симуляторов своих мапперов.

Как это будет работать:
- Рядом с .nes или просто так лежит JSON с мета-информацией (вроде как все проголосовали называть этот формат `JSONES`)
- В JSON написан путь к .asm файлу с микрокодом
- В наш эмулятор встроен примитивный/продвинутый ассемблер, который компилирует микрокод в 64 Кбайт BLOB
- Создаётся ещё одна инстанция M6502Core, которая тупо работает с микрокодом и больше ни с чем
- Когда основной эмулятор симулирует порт картриджа с микрокодом - то в память 6502 микрокода заносятся значения сигналов разъема картриджа и управление передаётся на точку входа микрокода
- Микрокод симулирует там что-то, делает bbrrrr и выдаёт в память значения выходных сигналов разъема карика
- Основная часть эмулятора забирает сигналы и радуется

Более подробные детали, типа карты памяти UCode 6502 и проч. в процессе реализации. Также ещё нужно придумать как микрокод будет получать доступ к ромкам (очевидно их все предварительно нужно загрузить в память, а со стороны 6502 как-то туда стучаться). Что-нибудь придумаем.

## Почему ассемблер

У читателя может возникнуть резонный вопрос: почему вместо популярного скриптового языка (LUA, Python) используется такой извращённый подход (микрокод на ассемблере 6502)?

Причины тут следующие:
- У нас уже есть готовый ассемблер для 6502 и эмулятор. Не нужно тащить какие-то ещё зависимости в наш маленький проект
- Скриптовые движки в результате своей деятельности генерируют байт-код. Ассемблер в результате своей деятельности генерирует микрокод. Принципиально между ними нет различий.
- У LUA кривой синтаксис не для людей
- У Python кодовая база как боинг
- Тот кто хоть сталкивался с муками выбора интеграции скриптового движка в свой C++ проект может всё это не читать (но уже прочитал)
