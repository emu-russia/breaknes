using System.Text;
using System.Globalization;
using System.ComponentModel;
using System.Collections.Generic;
using System.Collections;

namespace Breaknes
{

    public class CpuDebugInfo_Commands
    {
        // Control commands

        [TypeConverter(typeof(ListConverter))]
        [Category("Control commands")]
        public List<string> commands { get; set; } = new List<string>();
        [Category("Control commands")]
        public byte n_ACIN { get; set; }
        [Category("Control commands")]
        public byte n_DAA { get; set; }
        [Category("Control commands")]
        public byte n_DSA { get; set; }
        [Category("Control commands")]
        public byte n_1PC { get; set; }

        public byte[] cmd = new byte[(int)ControlCommand.Max];
        public bool WR = false;
    }

    public enum ControlCommand
    {
        Y_SB = 0,
        SB_Y,
        X_SB,
        SB_X,
        S_ADL,
        S_SB,
        SB_S,
        S_S,
        NDB_ADD,
        DB_ADD,
        Z_ADD,
        SB_ADD,
        ADL_ADD,
        ANDS,
        EORS,
        ORS,
        SRS,
        SUMS,
        ADD_SB7,
        ADD_SB06,
        ADD_ADL,
        SB_AC,
        AC_SB,
        AC_DB,
        ADH_PCH,
        PCH_PCH,
        PCH_ADH,
        PCH_DB,
        ADL_PCL,
        PCL_PCL,
        PCL_ADL,
        PCL_DB,
        ADH_ABH,
        ADL_ABL,
        Z_ADL0,
        Z_ADL1,
        Z_ADL2,
        Z_ADH0,
        Z_ADH17,
        SB_DB,
        SB_ADH,
        DL_ADL,
        DL_ADH,
        DL_DB,
        P_DB,
        DB_P,
        DBZ_Z,
        DB_N,
        IR5_C,
        DB_C,
        ACR_C,
        IR5_D,
        IR5_I,
        DB_V,
        AVR_V,
        Z_V,
        Max,
    }

}
