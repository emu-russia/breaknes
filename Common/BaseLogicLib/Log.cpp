// Breaknes logging facility implementation. See Log.h.

#include "pch.h"

#include "Log.h"

#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <atomic>
#include <mutex>
#include <vector>

namespace Log
{
	struct SourceInfo
	{
		const char* name = nullptr;
		std::vector<LogCategoryDesc> cats;
	};

	// Runtime state. The masks are lock-free (read on the simulation thread,
	// written from the UI thread); the sinks are guarded by the mutex.

	static std::atomic<bool> g_enabled{ false };
	static std::atomic<uint64_t> g_source_mask{ 0 };
	static std::atomic<uint64_t> g_cat_masks[Source_Count]{};

	static SourceInfo g_sources[Source_Count];

	static std::mutex g_mutex;
	static FILE* g_file = nullptr;
	static bool g_to_stdout = false;

	void SetEnabled(bool enabled)
	{
		g_enabled.store(enabled, std::memory_order_relaxed);
	}

	bool IsEnabled()
	{
		return g_enabled.load(std::memory_order_relaxed);
	}

	void SetSourceMask(uint64_t mask)
	{
		g_source_mask.store(mask, std::memory_order_relaxed);
	}

	uint64_t GetSourceMask()
	{
		return g_source_mask.load(std::memory_order_relaxed);
	}

	void SetCategoryMask(Source src, uint64_t mask)
	{
		if (src < 0 || src >= Source_Count)
			return;

		g_cat_masks[(uint32_t)src].store(mask, std::memory_order_relaxed);
	}

	uint64_t GetCategoryMask(Source src)
	{
		if (src < 0 || src >= Source_Count)
			return 0;

		return g_cat_masks[(uint32_t)src].load(std::memory_order_relaxed);
	}

	void SetOutputFile(const char* path)
	{
		std::lock_guard<std::mutex> lock(g_mutex);

		if (g_file != nullptr)
		{
			fclose(g_file);
			g_file = nullptr;
		}

		if (path != nullptr && path[0] != 0)
		{
			g_file = fopen(path, "w");
		}
	}

	void SetOutputStdout(bool enable)
	{
		std::lock_guard<std::mutex> lock(g_mutex);
		g_to_stdout = enable;
	}

	bool IsCategoryEnabled(Source src, uint64_t cat)
	{
		if (!g_enabled.load(std::memory_order_relaxed))
			return false;

		if (src < 0 || src >= Source_Count)
			return false;

		if (!(g_source_mask.load(std::memory_order_relaxed) & (1ULL << (uint32_t)src)))
			return false;

		return (g_cat_masks[(uint32_t)src].load(std::memory_order_relaxed) & cat) != 0;
	}

	void Write(Source src, uint64_t cat, const char* fmt, ...)
	{
		if (!IsCategoryEnabled(src, cat))
			return;

		char buf[1024];
		va_list args;
		va_start(args, fmt);
		vsnprintf(buf, sizeof(buf), fmt, args);
		va_end(args);

		const char* src_name = GetSourceName(src);
		const char* cat_name = GetCategoryNameByBit(src, cat);

		std::lock_guard<std::mutex> lock(g_mutex);

		if (g_to_stdout)
		{
			printf("[%s][%s] %s\n", src_name, cat_name, buf);
		}

		if (g_file != nullptr)
		{
			fprintf(g_file, "[%s][%s] %s\n", src_name, cat_name, buf);
			fflush(g_file);
		}
	}

	void RegisterSource(Source src, const char* name, const LogCategoryDesc* cats, size_t count)
	{
		if (src < 0 || src >= Source_Count)
			return;

		SourceInfo& info = g_sources[(uint32_t)src];

		// Idempotent: keep the first registration.
		if (info.name != nullptr)
			return;

		info.name = name;

		if (cats != nullptr)
		{
			info.cats.reserve(count);
			for (size_t n = 0; n < count; n++)
			{
				info.cats.push_back(cats[n]);
			}
		}
	}

	size_t GetSourceCount()
	{
		return Source_Count;
	}

	const char* GetSourceName(Source src)
	{
		if (src < 0 || src >= Source_Count)
			return "?";

		const char* name = g_sources[(uint32_t)src].name;
		return name != nullptr ? name : "?";
	}

	size_t GetCategoryCount(Source src)
	{
		if (src < 0 || src >= Source_Count)
			return 0;

		return g_sources[(uint32_t)src].cats.size();
	}

	uint64_t GetCategoryBit(Source src, size_t index)
	{
		if (src < 0 || src >= Source_Count)
			return 0;

		const std::vector<LogCategoryDesc>& cats = g_sources[(uint32_t)src].cats;
		if (index >= cats.size())
			return 0;

		return cats[index].bit;
	}

	const char* GetCategoryName(Source src, size_t index)
	{
		if (src < 0 || src >= Source_Count)
			return "?";

		const std::vector<LogCategoryDesc>& cats = g_sources[(uint32_t)src].cats;
		if (index >= cats.size())
			return "?";

		return cats[index].name;
	}

	const char* GetCategoryNameByBit(Source src, uint64_t bit)
	{
		if (src < 0 || src >= Source_Count)
			return "?";

		const std::vector<LogCategoryDesc>& cats = g_sources[(uint32_t)src].cats;
		for (size_t n = 0; n < cats.size(); n++)
		{
			if (cats[n].bit == bit)
				return cats[n].name;
		}

		return "?";
	}
}
