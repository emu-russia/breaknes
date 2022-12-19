# Breaknes

![logo](/Breaknes/Breaknes/157481692-2ecd4e71-2599-4050-9ce0-815c0336ad27.png)

NES/Famicom/Dendy emulator at the gate level.

The last significant milestone:

![mariwa](/UserManual/imgstore/mariwa.png)

## Build

Use Windows and VS2022. Dotnet 6.0 **Desktop** Runtime is also required.

Builds for Linux will be available as soon as everything settles down.

## Что будет и чего не будет в эмуляторе

Эмулятор ещё не готов, поэтому можно сразу написать чего не будет.

- Save States. Не нужно, играйте без них. Используйте виртуальную машину и песочницу, в которой запущен эмулятор.
- AVI Record. Нах. Используйте программы захвата
- TAS. Есть уже дофига эмуляторов для TAS
- Удобный отладчик. Используйте Mesen. В эмуляторе будут базовые отладочные механизмы, чтобы проверить что там что-то шевелится. Всё остальное мы проверяем специализированными тулзами (Breaks Debugger, PPU Player) и юнит-тестами
- Frame Skip. Это что?
- Перемотка. Используйте виртуальную машину и песочницу, в которой запущен эмулятор.
- Скорости. Пока на первое время 1-2 FPS будет достаточно
- Запись аудио. Используйте программы захвата
- Game Genie. Нах нужно.

Короче видно что эмулятор будет максимально казуальным и примерно соответствует реальной консоли. Что умеет консоль, то умеет и эмулятор. Примерно так.

Киллер фичи.

- Точная симуляция основных чипов (собственно основная суть всей затеи)
- Настраиваемые борды. Можно слепить свою борду.
- @eugene-s-nesdev попросил также сделать возможность лепить химеру из чипов, т.к. нет ни одного полностью нормального денди-чипа, поэтому лучше всего будет взять по кускам из разных и слепить свой

## Procrastination Chart

- Breaknes GUI (managed part): 30%
- BreaksCore (native part): 50%
- 6502 Core: 100%
- APU: 3/12
- PPU: 100%
- Boards: 1/5
- Mappers: 0.5 (Only NROM partially)

The values reflect mostly a subjective feeling about reaching a critical mass of code. Bugs and improvements can be made ad infinitum.
