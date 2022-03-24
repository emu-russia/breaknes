#pragma once

namespace Breaknes
{
	struct DebugInfoProvider
	{
		DebugInfoEntry* entry;
		uint32_t(*GetValue)(void* opaque, DebugInfoEntry* entry);
		void* opaque;
	};

	struct MemProvider
	{
		MemDesciptor* descr;
		uint8_t(*ReadByte)(void* opaque, size_t addr);
		void* opaque;
	};

	class Core
	{
		static uint8_t ReadTestMem(void* opaque, size_t addr);
		static uint32_t GetTestInfo(void* opaque, DebugInfoEntry* entry);

	public:
		// TBD: Make std::map[DebugInfoType] of std::list?
		std::list<DebugInfoProvider> testInfo;
		std::list<DebugInfoProvider> coreInfo;
		std::list<DebugInfoProvider> coreRegsInfo;
		std::list<DebugInfoProvider> apuInfo;
		std::list<DebugInfoProvider> apuRegsInfo;
		std::list<DebugInfoProvider> ppuInfo;
		std::list<DebugInfoProvider> ppuRegsInfo;
		std::list<DebugInfoProvider> boardInfo;
		std::list<DebugInfoProvider> cartInfo;

		std::list<MemProvider> memMap;

		Core();
		~Core();

		void AddDebugInfo(DebugInfoType type, DebugInfoEntry* entry, uint32_t(*GetValue)(void* opaque, DebugInfoEntry* entry), void* opaque);

		void DisposeDebugInfo();

		void AddMemRegion(MemDesciptor* descr, uint8_t (*ReadByte)(void* opaque, size_t addr), void* opaque);

		void DisposeMemMap();
	};
}
