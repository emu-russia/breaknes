// Event log support

#pragma once

namespace Debug
{
	class EventLog
	{
		Json eventHistory;
		Json::Value* traceEvents;

	public:
		EventLog();
		~EventLog();

		void TraceBegin(int chan, char * s, uint64_t ts);

		void TraceEnd(int chan, uint64_t ts);

		void TraceEvent(int chan, char * text, uint64_t ts);

		/// @brief Get event history as serialized Json text. Then you can save the text to a file or transfer it to the utility to display the history.
		void ToString(std::string & jsonText);
	};
}
