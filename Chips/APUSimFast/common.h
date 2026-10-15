// Common elements of APU circuitry

#pragma once

#include <array>
#include <cstring>
#include <cstdio>
#include <type_traits>

namespace APUSimUnitTest { class UnitTest; }

namespace APUSimFast
{
	// Evaluate a phase-sensitive block once per changed dependency, then on
	// subsequent API calls until its stored state stops changing. Never settle
	// multiple passes in one call: a delayed latch transition belongs to the
	// next master half-call in the reference simulator.
	template <typename State, typename Key>
	class StablePhase
	{
		static_assert(std::is_trivially_copyable<State>::value, "phase state must be copyable");
		Key key{};
		std::array<unsigned char, sizeof(State)> before{};
		uint64_t epoch = 0;
		bool stable = false;
#ifdef APUSIMFAST_PHASE_STATS
		uint64_t calls = 0, evaluations = 0;
#endif
	public:
#ifdef APUSIMFAST_PHASE_STATS
		~StablePhase() { std::fprintf(stderr, "phase state_bytes=%zu key_bytes=%zu calls=%llu evaluations=%llu\n", sizeof(State), sizeof(Key), (unsigned long long)calls, (unsigned long long)evaluations); }
#endif
		bool begin(const State& state, Key next, uint64_t generation)
		{
#ifdef APUSIMFAST_PHASE_STATS
			++calls;
#endif
			#ifndef APUSIMFAST_DISABLE_PHASE
			if (stable && epoch == generation && key == next) return false;
#endif
#ifdef APUSIMFAST_PHASE_STATS
			++evaluations;
#endif
			key = next;
			epoch = generation;
			std::memcpy(before.data(), &state, sizeof(State));
			return true;
		}
		void end(const State& state)
		{
			stable = std::memcmp(before.data(), &state, sizeof(State)) == 0;
		}
		// `fixed_point`: the block proved that a replay of this evaluation with the same inputs
		// would leave its state and outputs unchanged (round 2), so the next call with the same
		// key may skip it even though this evaluation changed the state.
		void end(const State& state, bool fixed_point)
		{
			stable = fixed_point || std::memcmp(before.data(), &state, sizeof(State)) == 0;
		}
		// True when the last evaluation left the stored state unchanged (no settling pending).
		bool IsStable() const { return stable; }
	};

	// These counters retain both banks of the scalar bit cells. Main is updated
	// first; the clocked carry bank prepares the next value from that new Main.
	// A debug write changes Main alone, just like CounterBit::set.
	template <unsigned Width, bool CountUp>
	class PackedCounter
	{
		friend APUSimUnitTest::UnitTest;
		static_assert(Width > 0 && Width < 16, "counter width must fit uint16_t");
		static constexpr uint16_t Mask = (uint16_t)((1u << Width) - 1u);
		uint16_t main = 0;
		uint16_t carry = 0;

	public:
		void sim(BaseLogic::TriState clear, BaseLogic::TriState load,
			BaseLogic::TriState step, BaseLogic::TriState clock, uint16_t value, bool carry_in = true)
		{
			// MUX uses the selector's low bit; DLatch's enable instead tests One.
			if ((load & 1) != 0) main = value & Mask;
			else if ((clear & 1) != 0) main = 0;
			else if ((step & 1) != 0) main = (uint16_t)(~carry) & Mask;

			if (clock == BaseLogic::TriState::One)
			{
				const unsigned amount = carry_in ? 1u : 0u;
				const unsigned next = CountUp ? (unsigned)main + amount : (unsigned)main - amount;
				carry = (uint16_t)(~next) & Mask;
			}
		}

		uint16_t get() const { return main; }
		void set(uint16_t value) { main = value & Mask; }
		BaseLogic::TriState get_carry(bool carry_in = true) const
		{
			return carry_in && (CountUp ? main == Mask : main == 0) ? BaseLogic::One : BaseLogic::Zero;
		}
	};

	template <unsigned Width>
	class PackedReversibleCounter
	{
		friend APUSimUnitTest::UnitTest;
		static constexpr uint16_t Mask = (1u << Width) - 1u;
		uint16_t main = 0, carry = 0;
	public:
		void sim(BaseLogic::TriState down, BaseLogic::TriState clear, BaseLogic::TriState load,
			BaseLogic::TriState step, BaseLogic::TriState clock, uint16_t value)
		{
			if (load & 1) main = value & Mask;
			else if (clear & 1) main = 0;
			else if (step & 1) main = uint16_t(~carry) & Mask;
			if (clock == BaseLogic::One)
				carry = uint16_t(~((down & 1) ? unsigned(main) - 1u : unsigned(main) + 1u)) & Mask;
		}
		uint16_t get() const { return main; }
		void set(uint16_t value) { main = value & Mask; }
		BaseLogic::TriState get_carry(BaseLogic::TriState down) const
		{
			return ((down & 1) ? main == 0 : main == Mask) ? BaseLogic::One : BaseLogic::Zero;
		}
	};

	class RegisterBit
	{
		BaseLogic::DLatch transp_latch{};

	public:
		void sim(BaseLogic::TriState ACLK1, BaseLogic::TriState Enable, BaseLogic::TriState Value);
		BaseLogic::TriState get();
		BaseLogic::TriState nget();
		void set(BaseLogic::TriState val);
	};

	class RegisterBitRes
	{
		BaseLogic::DLatch transp_latch{};

	public:
		void sim(BaseLogic::TriState ACLK1, BaseLogic::TriState Enable, BaseLogic::TriState Value, BaseLogic::TriState Res);
		BaseLogic::TriState get();
		BaseLogic::TriState nget();
		void set(BaseLogic::TriState val);
	};

	class RegisterBitRes2
	{
		BaseLogic::DLatch transp_latch{};

	public:
		void sim(BaseLogic::TriState ACLK1, BaseLogic::TriState Enable, BaseLogic::TriState Value, BaseLogic::TriState Res1, BaseLogic::TriState Res2);
		BaseLogic::TriState get();
		BaseLogic::TriState nget();
		void set(BaseLogic::TriState val);
	};

	class CounterBit
	{
		BaseLogic::DLatch transp_latch{};
		BaseLogic::DLatch cg_latch{};

	public:
		BaseLogic::TriState sim(
			BaseLogic::TriState Carry,
			BaseLogic::TriState Clear,
			BaseLogic::TriState Load,
			BaseLogic::TriState Step,
			BaseLogic::TriState ACLK1,
			BaseLogic::TriState val);
		BaseLogic::TriState get();
		BaseLogic::TriState nget();
		void set(BaseLogic::TriState val);
	};

	class DownCounterBit
	{
		BaseLogic::DLatch transp_latch{};
		BaseLogic::DLatch cg_latch{};

	public:
		BaseLogic::TriState sim(
			BaseLogic::TriState Carry,
			BaseLogic::TriState Clear,
			BaseLogic::TriState Load,
			BaseLogic::TriState Step,
			BaseLogic::TriState ACLK1,
			BaseLogic::TriState val);
		BaseLogic::TriState get();
		BaseLogic::TriState nget();
		void set(BaseLogic::TriState val);
	};

	class RevCounterBit
	{
		BaseLogic::DLatch transp_latch{};
		BaseLogic::DLatch cg_latch{};

	public:
		BaseLogic::TriState sim(
			BaseLogic::TriState Carry,
			BaseLogic::TriState Dec,
			BaseLogic::TriState Clear,
			BaseLogic::TriState Load,
			BaseLogic::TriState Step,
			BaseLogic::TriState ACLK1,
			BaseLogic::TriState val);
		BaseLogic::TriState get();
		BaseLogic::TriState nget();
		void set(BaseLogic::TriState val);
	};
}
