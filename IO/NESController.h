// NES Controller simulator
#pragma once

namespace IO
{
	enum class NESControllerState
	{
		Up = 0,
		Down,
		Left,
		Right,
		Select,
		Start,
		B,
		A,
		Max,
	};

	class NESController : public IODevice
	{
		CD4021 sr{};

		bool trace_enable = false;
		bool start_trace = false;
		size_t posedge_counter = 0;
		BaseLogic::TriState prev_clk = BaseLogic::TriState::X;

		void Trace(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[]);

	public:
		NESController();
		virtual ~NESController();

		virtual uint32_t GetID() override;
		virtual std::string GetName() override;
		virtual int GetIOStates() override;
		virtual std::string GetIOStateName(size_t io_state) override;

		virtual void SetState(size_t io_state, uint32_t value) override;
		virtual uint32_t GetState(size_t io_state) override;

		virtual void sim(BaseLogic::TriState inputs[], BaseLogic::TriState outputs[], float analog[]) override;
	};
}
