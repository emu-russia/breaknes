# Руководство по дампу видеосигнала в PPUPlayer

Требования:
- PPUPlayer 2.2+ (https://github.com/emu-russia/breaknes/releases/)
- Nintendulator (https://www.qmtpro.com/~nes/nintendulator/)

## Первый запуск PPUPlayer

Запустить PPUPlayer:

![dump_video_001](/UserManual/imgstore/dump_video_001.png)

Выбрать в настройках ревизию PPU, для которого нужно получить дамп видеосигнала:

![dump_video_002](/UserManual/imgstore/dump_video_002.png)

Запустить PPUPlayer в "свободный полёт" (Run PPU). Перейти на вкладку Current Scan и убедиться что PPU живой:

![dump_video_003](/UserManual/imgstore/dump_video_003.png)

## Дамп состояния PPU в Nintendulator

Запустить какую-то игру в Nintendulator, желательно без сложного маппера.

Открыть отладачик и сдампить всю память PPU:

![dump_video_004](/UserManual/imgstore/dump_video_004.png)

Получится файл с расширением .ppumem, который можно найти где-то в недрах вашей папки Users/AppData.

## Загружаем состояние PPU в PPUPlayer

В меню `PPU Dumps` выбрать пункт `Load Nintendulator PPU Dump` и загрузить полученный в Nintendulator дамп памяти PPU.

Получится примерно такое:

![dump_video_005](/UserManual/imgstore/dump_video_005.png)

Но упс. Что-то не так.

Дело в том, что Nintendulator сохраняет только память PPU, но не сохраняет текущие значения регистров $2000 и $2001.

Особенно важен регистр $2000, в котором задаются адреса Pattern table и размер спрайтов.

Чтобы это исправить, переходим на вкладку Debug и меняем значение регистра $2000 (для Contra нужно установить значение 0x30):

![dump_video_006](/UserManual/imgstore/dump_video_006.png)

Получилось такое:

![dump_video_007](/UserManual/imgstore/dump_video_007.png)

## Включить дамп видеосигнала

Осталось за малым.

В меню `PPU Dumps` нужно выбрать пункт `Start video signal dump`, указать файл и после этого автоматически начнётся дамп в файл видеовыхода микросхемы PPU.

- Для "композитных" PPU (2C02/2C07) производится дамп массива float, каждое значение представляет собой напряжение (уровень сигнала)
- Для RGB PPU дампится 4 байта (в очередности R, G, B, SYNC), которые соответствуют выходам RGB PPU

## RAW Dump

Есть возможность вместо видеосигнала PPU получать значение пикселей до схемы ЦАП:

![dump_video_008](/UserManual/imgstore/dump_video_008.png)

В этом случае будут дампиться значения uint16_t следующего формата:

```c++
		/// <summary>
		/// Raw PPU color, which is obtained from the PPU circuits BEFORE the video signal generator.
		/// The user can switch the PPUSim video output to use "raw" color, instead of the original (Composite/RGB).
		/// </summary>
		union RAWOut
		{
			struct
			{
				unsigned CC0 : 1;	// Chroma (CB[0-3])
				unsigned CC1 : 1;
				unsigned CC2 : 1;
				unsigned CC3 : 1;
				unsigned LL0 : 1;	// Luma (CB[4-5])
				unsigned LL1 : 1;
				unsigned TR : 1;	// "Tint Red", $2001[5]
				unsigned TG : 1;	// "Tint Green", $2001[6]
				unsigned TB : 1;	// "Tint Blue", $2001[7]
				unsigned Sync : 1;	// 1: Sync level
			};
			uint16_t raw;
		} RAW;
```

## Заключение

PPUPlayer стал мощным инструментов в изучении PPU. Внутри него живёт настоящий живой PPU.

Не стесняйтесь тыкать разные кнопочки в PPUPlayer, экспериментируйте. Если что, имеется также презентация (тут рядом).
