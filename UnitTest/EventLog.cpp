// Event log support

#include "pch.h"

namespace Debug
{
	EventLog::EventLog()
	{
		traceEvents = eventHistory.root.AddArray(nullptr);
		assert(traceEvents);
	}

	EventLog::~EventLog()
	{
		// The memory will be cleared along with json root (eventHistory).
	}

	void EventLog::TraceBegin(int chan, char * s, uint64_t stamp)
	{
		Json::Value* entry = traceEvents->AddObject(nullptr);
		assert(entry);

		Json::Value* pid = entry->AddInt("pid", 1);
		assert(pid);

		Json::Value* tid = entry->AddInt("pid", chan);
		assert(tid);

		Json::Value* ts = entry->AddUInt64("ts", stamp);
		assert(ts);

		Json::Value* ph = entry->AddAnsiString("ph", "B");
		assert(ph);

		Json::Value* name = entry->AddAnsiString("name", s);
		assert(name);
	}

	void EventLog::TraceEnd(int chan, uint64_t stamp)
	{
		Json::Value* entry = traceEvents->AddObject(nullptr);
		assert(entry);

		Json::Value* pid = entry->AddInt("pid", 1);
		assert(pid);

		Json::Value* tid = entry->AddInt("pid", chan);
		assert(tid);

		Json::Value* ts = entry->AddUInt64("ts", stamp);
		assert(ts);

		Json::Value* ph = entry->AddAnsiString("ph", "E");
		assert(ph);
	}

	void EventLog::TraceEvent(int chan, char * text, uint64_t stamp)
	{
		Json::Value* entry = traceEvents->AddObject(nullptr);
		assert(entry);

		Json::Value* pid = entry->AddInt("pid", 1);
		assert(pid);

		Json::Value* tid = entry->AddInt("pid", chan);
		assert(tid);

		Json::Value* ts = entry->AddUInt64("ts", stamp);
		assert(ts);

		Json::Value* ph = entry->AddAnsiString("ph", "I");
		assert(ph);

		Json::Value* name = entry->AddAnsiString("name", text);
		assert(name);
	}

	void EventLog::ToString(std::string & jsonText)
	{
		size_t actualTextSize = 0;
		eventHistory.GetSerializedTextSize((void *)jsonText.data(), -1, actualTextSize);
		jsonText.resize(actualTextSize);
		eventHistory.Serialize((void*)jsonText.data(), actualTextSize, actualTextSize);
	}
}
