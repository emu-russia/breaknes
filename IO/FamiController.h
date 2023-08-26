// Famicom Controllers (1/2) simulator
#pragma once

namespace IO
{
	enum class FamiController1State
	{
		Up = 0,
		Down,
		Left,
		Right,
		Select,			// P1 only
		Start,			// P1 only
		B,
		A,
		Max,
	};

	enum class FamiController2State
	{
		Up = 0,
		Down,
		Left,
		Right,
		B,
		A,
		Volume,			// P2 only
		MicLevel,		// P2 only
		Max,
	};

	class FamiController1 : public IODevice
	{
		CD4021 sr{};

	public:
		FamiController1();
		virtual ~FamiController1();

		virtual uint32_t GetID() override;
		virtual std::string GetName() override;
		virtual int GetIOStates() override;
		virtual std::string GetIOStateName(size_t io_state) override;

		virtual void SetState(size_t io_state, uint32_t value) override;
		virtual uint32_t GetState(size_t io_state) override;

		virtual void sim(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[], float analog[]) override;
	};

	class FamiController2 : public IODevice
	{
		CD4021 sr{};

	public:
		FamiController2();
		virtual ~FamiController2();

		virtual uint32_t GetID() override;
		virtual std::string GetName() override;
		virtual int GetIOStates() override;
		virtual std::string GetIOStateName(size_t io_state) override;

		virtual void SetState(size_t io_state, uint32_t value) override;
		virtual uint32_t GetState(size_t io_state) override;

		virtual void sim(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[], float analog[]) override;
	};
}
