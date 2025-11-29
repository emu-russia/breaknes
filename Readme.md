
![logo](/Wiki/imgstore/157481692-2ecd4e71-2599-4050-9ce0-815c0336ad27.png)

NES/Famicom/Dendy emulator at the gate level.

## Progress

![progress](/Wiki/imgstore/progress.png)

## Build for Windows

Use Windows and VS2026. Open Breaknes.sln and click the Build button with your left heel. Dotnet 6.0 **Desktop** Runtime is also required.

For those who like minimalism - see Breaknes build using SDL2 (`Breaknes/BreaknesSDL`).

## Build for Linux

In general, the build process is typical for Linux. First you get all the sources from Git. Then you call CMake/make

```
# Get source
# Choose a suitable folder to store a clone of the repository, cd there and then
git clone https://github.com/emu-russia/breaknes.git
cd breaknes

# Preliminary squats
mkdir build
cd build
cmake ..
make

# Find the executable file in the depths of the build folder
./breaknes bomber.nes
```

If something doesn't work, you do it. You have red eyes for a reason. :penguin:
