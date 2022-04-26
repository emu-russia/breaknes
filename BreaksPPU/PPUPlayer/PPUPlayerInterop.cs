using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using System.Runtime.InteropServices;

namespace PPUPlayer
{
    internal class PPUPlayerInterop
    {

        [DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void CreateBoard();


        [DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void DestroyBoard();


        [DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void Step();


        [DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void CPUWrite(int ppuReg, byte val);


        [DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void CPURead(int ppuReg);


        [DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern int GetPCLKCounter();


        [DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void InsertCartridge(byte[] nesImage, int nesImageSize);


        [DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void EjectCartridge();


        [DllImport("PPUPlayerInterop.dll", CallingConvention = CallingConvention.Cdecl)]
        public static extern void SampleVideoSignal(out float sample);
    }
}
