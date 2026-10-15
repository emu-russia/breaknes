#include "pch.h"

using namespace BaseLogic;

namespace M6502Core
{
	void DataBus::sim_SetExternalBus(uint8_t* data_bus)
	{
		sim_SetExternalBusImpl<0>(data_bus);
	}

	// RD = NOT(NOR(NOT(PHI2), rd_latch.nget())): One while PHI2 is Zero, rd_latch.nget() while PHI2 is One.
	template<int Phase> static inline TriState ReadStrobe(TriState PHI2, DLatch& rd_latch)
	{
		using P = Phi<Phase>;
		if (P::Generic) return NOT(NOR(NOT(PHI2), rd_latch.nget()));
		return P::Is1 ? TriState::One : rd_latch.nget();
	}

	template<int Phase> void DataBus::sim_SetExternalBusImpl(uint8_t* data_bus)
	{
		using P = Phi<Phase>;
		TriState PHI1 = core->wire.PHI1;
		TriState PHI2 = core->wire.PHI2;
		TriState WR = core->wire.WR;

		P::Set1(rd_latch, WR, PHI1);
		TriState RD = ReadStrobe<Phase>(PHI2, rd_latch);

		if (P::Phi1(PHI1))
		{
			DOR = ~(core->DB);
		}

		if (RD == TriState::Zero)
		{
			*data_bus = ~DOR;
		}
	}
	template void DataBus::sim_SetExternalBusImpl<0>(uint8_t*);
	template void DataBus::sim_SetExternalBusImpl<1>(uint8_t*);
	template void DataBus::sim_SetExternalBusImpl<2>(uint8_t*);

	void DataBus::sim_GetExternalBus(uint8_t* data_bus)
	{
		sim_GetExternalBusImpl<0>(data_bus);
	}

	template<int Phase> void DataBus::sim_GetExternalBusImpl(uint8_t* data_bus)
	{
		using P = Phi<Phase>;
		TriState PHI1 = core->wire.PHI1;
		TriState PHI2 = core->wire.PHI2;
		TriState WR = core->wire.WR;
		bool DL_ADL = core->cmd.DL_ADL;
		bool DL_ADH = core->cmd.DL_ADH;
		bool DL_DB = core->cmd.DL_DB;

		P::Set1(rd_latch, WR, PHI1);
		TriState RD = ReadStrobe<Phase>(PHI2, rd_latch);
		(void)RD;

		if (P::Phi2One(PHI2))
		{
			DL = ~(*data_bus);
		}

		if (P::Phi1One(PHI1))
		{
			if (DL_ADL)
			{
				if (core->ADL_Dirty)
				{
					core->ADL = core->ADL & ~DL;
				}
				else
				{
					core->ADL = ~DL;
					core->ADL_Dirty = true;
				}
			}

			if (DL_ADH)
			{
				if (core->ADH_Dirty)
				{
					core->ADH = core->ADH & ~DL;
				}
				else
				{
					core->ADH = ~DL;
					core->ADH_Dirty = true;
				}
			}

			if (DL_DB)
			{
				if (core->DB_Dirty)
				{
					core->DB = core->DB & ~DL;
				}
				else
				{
					core->DB = ~DL;
					core->DB_Dirty = true;
				}
			}
		}
	}

	template void DataBus::sim_GetExternalBusImpl<0>(uint8_t*);
	template void DataBus::sim_GetExternalBusImpl<1>(uint8_t*);
	template void DataBus::sim_GetExternalBusImpl<2>(uint8_t*);

	uint8_t DataBus::getDL()
	{
		return ~DL;
	}

	uint8_t DataBus::getDOR()
	{
		return ~DOR;
	}

	void DataBus::setDL(uint8_t val)
	{
		DL = ~val;
	}

	void DataBus::setDOR(uint8_t val)
	{
		DOR = ~val;
	}
}
