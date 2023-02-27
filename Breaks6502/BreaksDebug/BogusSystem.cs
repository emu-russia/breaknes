// A dummy device that uses the 6502 as a processor.
// Contains 64 Kbytes of memory and nothing else.

using System;
using System.Text;
using System.Globalization;
using System.ComponentModel;
using System.Runtime.InteropServices;
using System.Collections.Generic;
using System.Collections;
using System.Linq;

using Be.Windows.Forms;

namespace BreaksDebug
{
    public class BogusSystem
    {
        public int Cycle = 0;

        byte CLK = 0;
        IByteProvider mem;

        public void Step(bool trace)
        {
            if (trace)
            {
                Console.WriteLine("Step");
            }

#if false
            cpu_pads.PHI0 = CLK;

            // Execute M6502Core::sim

            // Reading must be done before the processor simulation, so that data is already on the data bus,
            // and writing must be done after the simulation, because the processor must first put something on the data bus.

            if (cpu_pads.PHI1 == 1)
            {
                // Although all memory operations are performed only during PHI2, a check is made here on PHI1, since the output of PHI2 is not yet set by the simulator.
                MemRead(trace);
            }

            CpuPadsRaw pads = SerializePads (cpu_pads);

            Sim(ref pads, ref info);

            cpu_pads = DeserializePads(pads);

            if (cpu_pads.PHI2 == 1)
            {
                MemWrite(trace);
            }
#endif

            // Clockgen

            if (CLK != 0)
            {
                CLK = 0;
                Cycle++;
            }
            else
            {
                CLK = 1;
            }
        }
  
        public void AttatchMemory (IByteProvider prov)
        {
            mem = prov;
        }

        public class MemoryMapping
        {
            public int RamStart = 0;
            public int RamSize = 0x800;
            public int RomStart = 0xc000;
            public int RomSize = 0x4000;
        }

        MemoryMapping memMap = new MemoryMapping();

        public void SetMemoryMapping(MemoryMapping map)
        {
            memMap = map;
        }

        void MemRead(bool trace)
        {
#if false
            if (cpu_pads.RnW == 1)
            {
                long address = cpu_pads.A;

                // CPU Read

                if ( (address >= memMap.RamStart && address < (memMap.RamStart + memMap.RamSize)) || 
                    (address >= memMap.RomStart && address < (memMap.RomStart + memMap.RomSize)) )
                {
                    cpu_pads.D = mem.ReadByte(address);
                    if (trace)
                    {
                        Console.WriteLine("CPU Read " + address.ToString("X4") + " " + cpu_pads.D.ToString("X2"));
                    }
                }
                else
                {
                    if (trace)
                    {
                        Console.WriteLine("CPU Read " + address.ToString("X4") + " ignored");
                    }
                }
            }
#endif
        }

        void MemWrite(bool trace)
        {
#if false
            if (cpu_pads.RnW == 0)
            {
                long address = cpu_pads.A;

                // CPU Write

                if (address >= memMap.RamStart && address < (memMap.RamStart + memMap.RamSize))
                {
                    mem.WriteByte(address, cpu_pads.D);
                    if (trace)
                    {
                        Console.WriteLine("CPU Write " + address.ToString("X4") + " " + cpu_pads.D.ToString("X2"));
                    }
                }
                else
                {
                    if (trace)
                    {
                        Console.WriteLine("CPU Write " + address.ToString("X4") + " ignored");
                    }
                }
            }
#endif
        }


        //[DllImport("M6502CoreInterop.dll", CallingConvention = CallingConvention.Cdecl)]
        //static extern void Sim(ref CpuPadsRaw pads, ref CpuDebugInfoRaw debugInfo);

    }

}
