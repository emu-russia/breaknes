#pragma once

#pragma pack(push, 1)
struct NESHeader
{
	uint8_t Sign[4];
	uint8_t PRGSize;
	uint8_t CHRSize;
	uint8_t Flags_6;
	uint8_t Flags_7;
	uint8_t Flags_8;
	uint8_t Flags_9;
	uint8_t Flags_10;
	uint8_t padding[5];
};
#pragma pack(pop)

#define NES_TRAINER_SIZE 512
