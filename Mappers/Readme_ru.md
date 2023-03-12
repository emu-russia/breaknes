# Mappers

Реализация мапперов сделана (в процессе) двумя способами:
- Типовой симуляцией схем распространенных мапперов на C++, используя BaseLogicLib
- Возможность симуляции необычных мапперов с применением микрокода 6502

## Микрокод мапперов

NES/Famicom славится своим большим количеством мапперов. К лицензионным мапперам добавилось ещё просто овер-10001 китайских мапперов, с минимальными вариациями, но для каждого приходится заводить свой "номер" в формате .NES.

А что если дать продвинутому пользователю самостоятельно говорить на "слепок" ромов - какой маппер нужно для них использовать. И не просто какой маппер, а самому определять поведение маппера.

У нас есть:
- Набор ромок CHR/PRG, возможно слепленных как .nes файл, а возможно и просто лежащими кучей файлов
- Схема маппера / вспомогательных чипов, которые есть на карике
- Эмулятор 6502
- Микропрограмма на 6502, которая симулирует обращение к пинам картриджа и реализует логику "дикого" маппера

![mappers_ucode1](mappers_ucode1.png)

![mappers_ucode2](mappers_ucode2.png)

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