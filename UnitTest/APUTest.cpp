#include "pch.h"

using namespace BaseLogic;
using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace APUSimUnitTest
{
	UnitTest::UnitTest(APUSim::Revision rev)
	{
		core = new M6502Core::M6502(false, true);
		apu = new APUSim::APU(core, rev);
	}

	UnitTest::~UnitTest()
	{
		delete apu;
		delete core;
	}

	bool UnitTest::VerifyRegOpByAddress(uint16_t addr, bool read)
	{
		switch (addr)
		{
			case 0x4000:
				if ((apu->wire.W4000 != TriState::One && !read))
					return false;
				break;
			case 0x4001:
				if ((apu->wire.W4001 != TriState::One && !read))
					return false;
				break;
			case 0x4002:
				if ((apu->wire.W4002 != TriState::One && !read))
					return false;
				break;
			case 0x4003:
				if ((apu->wire.W4003 != TriState::One && !read))
					return false;
				break;
			case 0x4004:
				if ((apu->wire.W4004 != TriState::One && !read))
					return false;
				break;
			case 0x4005:
				if ((apu->wire.W4005 != TriState::One && !read))
					return false;
				break;
			case 0x4006:
				if ((apu->wire.W4006 != TriState::One && !read))
					return false;
				break;
			case 0x4007:
				if ((apu->wire.W4007 != TriState::One && !read))
					return false;
				break;
			case 0x4008:
				if ((apu->wire.W4008 != TriState::One && !read))
					return false;
				break;
			case 0x400a:
				if ((apu->wire.W400A != TriState::One && !read))
					return false;
				break;
			case 0x400b:
				if ((apu->wire.W400B != TriState::One && !read))
					return false;
				break;
			case 0x400c:
				if ((apu->wire.W400C != TriState::One && !read))
					return false;
				break;
			case 0x400e:
				if ((apu->wire.W400E != TriState::One && !read))
					return false;
				break;
			case 0x400f:
				if ((apu->wire.W400F != TriState::One && !read))
					return false;
				break;
			case 0x4010:
				if ((apu->wire.W4010 != TriState::One && !read))
					return false;
				break;
			case 0x4011:
				if ((apu->wire.W4011 != TriState::One && !read))
					return false;
				break;
			case 0x4012:
				if ((apu->wire.W4012 != TriState::One && !read))
					return false;
				break;
			case 0x4013:
				if ((apu->wire.W4013 != TriState::One && !read))
					return false;
				break;
			case 0x4014:
				if ((apu->wire.W4014 != TriState::One && !read))
					return false;
				break;
			case 0x4015:
				if ((apu->wire.W4015 != TriState::One && !read))
					return false;
				if ((apu->wire.n_R4015 != TriState::Zero && read))
					return false;
				break;
			case 0x4016:
				if ((apu->wire.W4016 != TriState::One && !read))
					return false;
				if ((apu->wire.n_R4016 != TriState::Zero && read))
					return false;
				break;
			case 0x4017:
				if ((apu->wire.W4017 != TriState::One && !read))
					return false;
				if ((apu->wire.n_R4017 != TriState::Zero && read))
					return false;
				break;
			case 0x4018:
				if ((apu->wire.n_R4018 != TriState::Zero && read))
					return false;
				break;
			case 0x4019:
				if ((apu->wire.n_R4019 != TriState::Zero && read))
					return false;
				break;
			case 0x401a:
				if ((apu->wire.W401A != TriState::One && !read))
					return false;
				if ((apu->wire.n_R401A != TriState::Zero && read))
					return false;
				break;

			default:
				break;
		}

		return true;
	}

	bool UnitTest::TestRegOps()
	{
		apu->wire.DBG = TriState::One;
		apu->wire.PHI1 = TriState::Zero;
		
		// Read
		
		apu->Ax = 0x4000;
		apu->CPU_Addr = 0x4000;
		apu->wire.RnW = TriState::One;

		for (size_t n = 0; n < 0x20; n++)
		{
			apu->regs->sim();

			if (!VerifyRegOpByAddress(apu->Ax, true))
			{
				Logger::WriteMessage(("RegOp failed! Idx: " + std::to_string(n) + ", Read").c_str());
				return false;
			}

			apu->Ax++;
			apu->CPU_Addr++;
		}

		// Write

		apu->Ax = 0x4000;
		apu->CPU_Addr = 0x4000;
		apu->wire.RnW = TriState::Zero;

		for (size_t n = 0; n < 0x20; n++)
		{
			apu->regs->sim();

			if (!VerifyRegOpByAddress(apu->Ax, false))
			{
				Logger::WriteMessage(("RegOp failed! Idx: " + std::to_string(n) + ", Write").c_str());
				return false;
			}

			apu->Ax++;
			apu->CPU_Addr++;
		}

		return true;
	}
}
