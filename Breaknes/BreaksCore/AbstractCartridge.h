#pragma once

namespace Breaknes
{
	enum class ConnectorType
	{
		None = 0,
		FamicomStyle,
		NESStyle,
		Max,
	};

	class AbstractCartridge
	{
	public:
		AbstractCartridge();
		virtual ~AbstractCartridge();

		virtual void sim() = 0;
	};
}
