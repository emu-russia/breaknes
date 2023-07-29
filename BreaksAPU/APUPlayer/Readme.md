# APU Player

About image by u/ThatPixelArtDude.

## How to get APU register dumps

You can use the main emulator (breaknes). In the settings you need to enable APURegump=True and select the folder where regdump will be saved.

Regdump entries format:

```c++
#pragma pack(push, 1)
struct RegDumpEntry
{
	uint32_t	clkDelta;	// Delta of previous CLK counter (CPU Core clock cycles) value at the time of accessing to the register
	uint8_t 	reg; 		// Register index + Flag (msb - 0: write, 1: read)
	uint8_t 	value;		// Written value. Not used for reading.
	uint16_t	padding;	// Not used (yet?)
};
#pragma pack(pop)
```
