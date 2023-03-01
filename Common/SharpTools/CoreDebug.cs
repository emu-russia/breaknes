// This module is used in the Breaks Debugger for a more "human" output of debugging information received from the core.

using System;
using System.Text;
using System.Globalization;
using System.ComponentModel;
using System.Runtime.InteropServices;
using System.Collections;

namespace SharpTools
{
	public class CoreDebug
	{
		public CpuPads cpu_pads = new CpuPads();
		public BreaksCoreInterop.CpuDebugInfoRaw info = new BreaksCoreInterop.CpuDebugInfoRaw();

		public class CpuPads
		{
			[Category("Inputs")]
			public byte n_NMI { get; set; }
			[Category("Inputs")]
			public byte n_IRQ { get; set; }
			[Category("Inputs")]
			public byte n_RES { get; set; }
			[Category("Inputs")]
			public byte PHI0 { get; set; }
			[Category("Inputs")]
			public byte RDY { get; set; }
			[Category("Inputs")]
			public byte SO { get; set; }
			[Category("Outputs")]
			public byte PHI1 { get; set; }
			[Category("Outputs")]
			public byte PHI2 { get; set; }
			[Category("Outputs")]
			public byte RnW { get; set; }
			[Category("Outputs")]
			public byte SYNC { get; set; }
			[Category("Address Bus")]
			public string A { get; set; }
			[Category("Data Bus")]
			public byte D { get; set; }
		}

		public class CpuDebugInfo_RegsBuses
		{
			[Category("Internal buses")]
			public string SB { get; set; }
			[Category("Internal buses")]
			public string DB { get; set; }
			[Category("Internal buses")]
			public string ADL { get; set; }
			[Category("Internal buses")]
			public string ADH { get; set; }

			[Category("Regs")]
			public string IR { get; set; }
			[Category("Regs")]
			public string PD { get; set; }
			[Category("Regs")]
			public string Y { get; set; }
			[Category("Regs")]
			public string X { get; set; }
			[Category("Regs")]
			public string S { get; set; }
			[Category("Regs")]
			public string AI { get; set; }
			[Category("Regs")]
			public string BI { get; set; }
			[Category("Regs")]
			public string ADD { get; set; }
			[Category("Regs")]
			public string AC { get; set; }
			[Category("Regs")]
			public string PCL { get; set; }
			[Category("Regs")]
			public string PCH { get; set; }
			[Category("Regs")]
			public string PCLS { get; set; }
			[Category("Regs")]
			public string PCHS { get; set; }
			[Category("Regs")]
			public string ABL { get; set; }
			[Category("Regs")]
			public string ABH { get; set; }
			[Category("Regs")]
			public string DL { get; set; }
			[Category("Regs")]
			public string DOR { get; set; }

			[Category("Flags")]
			public byte C_OUT { get; set; }
			[Category("Flags")]
			public byte Z_OUT { get; set; }
			[Category("Flags")]
			public byte I_OUT { get; set; }
			[Category("Flags")]
			public byte D_OUT { get; set; }
			[Category("Flags")]
			public byte B_OUT { get; set; }
			[Category("Flags")]
			public byte V_OUT { get; set; }
			[Category("Flags")]
			public byte N_OUT { get; set; }

			public byte IRForDisasm;
			public byte PDForDisasm;
			public UInt16 PCForUnitTest;
		}

		public class CpuDebugInfo_Internals
		{
			[Category("Prev Ready")]
			public byte n_PRDY { get; set; }
			[Category("Interrupts")]
			public byte n_NMIP { get; set; }
			[Category("Interrupts")]
			public byte n_IRQP { get; set; }
			[Category("Interrupts")]
			public byte RESP { get; set; }
			[Category("Interrupts")]
			public byte BRK6E { get; set; }
			[Category("Interrupts")]
			public byte BRK7 { get; set; }
			[Category("Interrupts")]
			public byte DORES { get; set; }
			[Category("Interrupts")]
			public byte n_DONMI { get; set; }
			[Category("Extra Cycle Counter")]
			public byte n_T2 { get; set; }
			[Category("Extra Cycle Counter")]
			public byte n_T3 { get; set; }
			[Category("Extra Cycle Counter")]
			public byte n_T4 { get; set; }
			[Category("Extra Cycle Counter")]
			public byte n_T5 { get; set; }
			[Category("Dispatcher")]
			public byte T0 { get; set; }
			[Category("Dispatcher")]
			public byte n_T0 { get; set; }
			[Category("Dispatcher")]
			public byte n_T1X { get; set; }
			[Category("Dispatcher")]
			public byte Z_IR { get; set; }
			[Category("Dispatcher")]
			public byte FETCH { get; set; }
			[Category("Dispatcher")]
			public byte n_ready { get; set; }
			[Category("Dispatcher")]
			public byte WR { get; set; }
			[Category("Dispatcher")]
			public byte TRES2 { get; set; }
			[Category("Dispatcher")]
			public byte ACRL1 { get; set; }
			[Category("Dispatcher")]
			public byte ACRL2 { get; set; }
			[Category("Dispatcher")]
			public byte T1 { get; set; }
			[Category("Dispatcher")]
			public byte RMW_T6 { get; set; }
			[Category("Dispatcher")]
			public byte RMW_T7 { get; set; }
			[Category("Dispatcher")]
			public byte ENDS { get; set; }
			[Category("Dispatcher")]
			public byte ENDX { get; set; }
			[Category("Dispatcher")]
			public byte TRES1 { get; set; }
			[Category("Dispatcher")]
			public byte n_TRESX { get; set; }
			[Category("Branch Logic")]
			public byte BRFW { get; set; }
			[Category("Branch Logic")]
			public byte n_BRTAKEN { get; set; }
			[Category("ALU Output")]
			public byte ACR { get; set; }
			[Category("ALU Output")]
			public byte AVR { get; set; }
		}

		public class CpuDebugInfo_Decoder
		{
			// Decoder

			[TypeConverter(typeof(ListConverter))]
			[Category("Decoder")]
			public List<string> decoder_out { get; set; } = new List<string>();
		}

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

		public CpuDebugInfo_RegsBuses GetRegsBuses()
		{
			CpuDebugInfo_RegsBuses res = new CpuDebugInfo_RegsBuses();

			res.SB = "0x" + info.SB.ToString("X2");
			res.DB = "0x" + info.DB.ToString("X2");
			res.ADL = "0x" + info.ADL.ToString("X2");
			res.ADH = "0x" + info.ADH.ToString("X2");

			res.IR = "0x" + info.IR.ToString("X2");
			res.PD = "0x" + info.PD.ToString("X2");
			res.Y = "0x" + info.Y.ToString("X2");
			res.X = "0x" + info.X.ToString("X2");
			res.S = "0x" + info.S.ToString("X2");
			res.AI = "0x" + info.AI.ToString("X2");
			res.BI = "0x" + info.BI.ToString("X2");
			res.ADD = "0x" + info.ADD.ToString("X2");
			res.AC = "0x" + info.AC.ToString("X2");
			res.PCL = "0x" + info.PCL.ToString("X2");
			res.PCH = "0x" + info.PCH.ToString("X2");
			res.PCLS = "0x" + info.PCLS.ToString("X2");
			res.PCHS = "0x" + info.PCHS.ToString("X2");
			res.ABL = "0x" + info.ABL.ToString("X2");
			res.ABH = "0x" + info.ABH.ToString("X2");
			res.DL = "0x" + info.DL.ToString("X2");
			res.DOR = "0x" + info.DOR.ToString("X2");

			res.C_OUT = info.C_OUT;
			res.Z_OUT = info.Z_OUT;
			res.I_OUT = info.I_OUT;
			res.D_OUT = info.D_OUT;
			res.B_OUT = info.B_OUT;
			res.V_OUT = info.V_OUT;
			res.N_OUT = info.N_OUT;

			res.IRForDisasm = info.IR;
			res.PDForDisasm = info.PD;
			res.PCForUnitTest = (UInt16)(((UInt16)info.PCH << 8) | (UInt16)info.PCL);

			return res;
		}

		public CpuDebugInfo_Internals GetInternals()
		{
			CpuDebugInfo_Internals res = new CpuDebugInfo_Internals();

			res.n_PRDY = info.n_PRDY;
			res.n_NMIP = info.n_NMIP;
			res.n_IRQP = info.n_IRQP;
			res.RESP = info.RESP;
			res.BRK6E = info.BRK6E;
			res.BRK7 = info.BRK7;
			res.DORES = info.DORES;
			res.n_DONMI = info.n_DONMI;
			res.n_T2 = info.n_T2;
			res.n_T3 = info.n_T3;
			res.n_T4 = info.n_T4;
			res.n_T5 = info.n_T5;
			res.T0 = info.T0;
			res.n_T0 = info.n_T0;
			res.n_T1X = info.n_T1X;
			res.Z_IR = info.Z_IR;
			res.FETCH = info.FETCH;
			res.n_ready = info.n_ready;
			res.WR = info.WR;
			res.TRES2 = info.TRES2;
			res.ACRL1 = info.ACRL1;
			res.ACRL2 = info.ACRL2;
			res.T1 = info.T1;
			res.RMW_T6 = info.RMW_T6;
			res.RMW_T7 = info.RMW_T7;
			res.ENDS = info.ENDS;
			res.ENDX = info.ENDX;
			res.TRES1 = info.TRES1;
			res.n_TRESX = info.n_TRESX;
			res.BRFW = info.BRFW;
			res.n_BRTAKEN = info.n_BRTAKEN;
			res.ACR = info.ACR;
			res.AVR = info.AVR;

			return res;
		}

		public CpuDebugInfo_Decoder GetDecoder()
		{
			CpuDebugInfo_Decoder res = new CpuDebugInfo_Decoder();

			for (int n = 0; n < 130; n++)
			{
				unsafe
				{
					if (info.decoder_out[n] != 0)
					{
						string text = n.ToString() + ": " + DecoderDecoder.GetDecoderOutName(n);
						res.decoder_out.Add(text);
					}
				}
			}

			return res;
		}

		public CpuDebugInfo_Commands GetCommands()
		{
			CpuDebugInfo_Commands res = new CpuDebugInfo_Commands();

			for (int n = 0; n < (int)ControlCommand.Max; n++)
			{
				unsafe
				{
					if (info.cmd[n] != 0)
					{
						res.commands.Add(((ControlCommand)n).ToString());
					}
					res.cmd[n] = info.cmd[n];
				}
			}

			res.n_ACIN = info.n_ACIN;
			res.n_DAA = info.n_DAA;
			res.n_DSA = info.n_DSA;
			res.n_1PC = info.n_1PC;
			res.WR = info.WR != 0 ? true : false;

			return res;
		}

		// https://stackoverflow.com/questions/32582504/propertygrid-expandable-collection

		public class ListConverter : CollectionConverter
		{
			public override bool GetPropertiesSupported(ITypeDescriptorContext context)
			{
				return true;
			}

			public override PropertyDescriptorCollection GetProperties(ITypeDescriptorContext context, object value, Attribute[] attributes)
			{
				IList list = value as IList;
				if (list == null || list.Count == 0)
					return base.GetProperties(context, value, attributes);

				var items = new PropertyDescriptorCollection(null);
				for (int i = 0; i < list.Count; i++)
				{
					object item = list[i];
					items.Add(new ExpandableCollectionPropertyDescriptor(list, i));
				}
				return items;
			}

			public class ExpandableCollectionPropertyDescriptor : PropertyDescriptor
			{
				private IList collection;
				private readonly int _index;

				public ExpandableCollectionPropertyDescriptor(IList coll, int idx)
					: base(GetDisplayName(coll, idx), null)
				{
					collection = coll;
					_index = idx;
				}

				private static string GetDisplayName(IList list, int index)
				{
					return "[" + index + "]  " + CSharpName(list[index].GetType());
				}

				private static string CSharpName(Type type)
				{
					var sb = new StringBuilder();
					var name = type.Name;
					if (!type.IsGenericType)
						return name;
					sb.Append(name.Substring(0, name.IndexOf('`')));
					sb.Append("<");
					sb.Append(string.Join(", ", type.GetGenericArguments().Select(CSharpName)));
					sb.Append(">");
					return sb.ToString();
				}

				public override bool CanResetValue(object component)
				{
					return true;
				}

				public override Type ComponentType
				{
					get { return this.collection.GetType(); }
				}

				public override object GetValue(object component)
				{
					return collection[_index];
				}

				public override bool IsReadOnly
				{
					get { return false; }
				}

				public override string Name
				{
					get { return _index.ToString(CultureInfo.InvariantCulture); }
				}

				public override Type PropertyType
				{
					get { return collection[_index].GetType(); }
				}

				public override void ResetValue(object component)
				{
				}

				public override bool ShouldSerializeValue(object component)
				{
					return true;
				}

				public override void SetValue(object component, object value)
				{
					collection[_index] = value;
				}
			}
		}

	} // CoreDebug

	class DecoderDecoder
	{
		static string[] decoder_out_name = new string[130] {
			"STY (TX)",
			"OP ind, Y (T3)",
			"OP abs, Y (T2)",
			"DEY INY (T0)",
			"TYA (T0)",
			"CPY INY (T0)",

			"OP zpg, X/Y & OP abs, X/Y (T2)",
			"LDX STX A<->X S<->X (TX)",
			"OP ind, X (T2)",
			"TXA (T0)",
			"DEX (T0)",
			"CPX INX (T0)",
			"STX TXA TXS (TX)",
			"TXS (T0)",
			"LDX TAX TSX (T0)",
			"DEX (T1)",
			"INX (T1)",
			"TSX (T0)",
			"DEY INY (T1)",
			"LDY (T0)",
			"LDY TAY (T0)",

			"JSR (T0)",
			"BRK5",
			"Push (T0)",
			"RTS (T4)",
			"Pull (T3)",
			"RTI/5",
			"ROR (TX)",
			"T2",
			"EOR (T0)",
			"JMP (excluder for D31) (TX)",
			"ALU absolute (T2)",
			"ORA (T0)",
			"LEFT_ALL (T2)",
			"T0 ANY",
			"STK2",
			"BRK JSR RTI RTS Push/pull + BIT JMP (T3)",

			"BRK JSR (T4)",
			"RTI (T4)",
			"OP X, ind (T3)",
			"OP ind, Y (T4)",
			"OP ind, Y (T2)",
			"RIGHT ODD (T3)",
			"Pull (TX)",
			"INC NOP (TX)",
			"OP X, ind (T4)",
			"OP ind, Y (T3)",
			"RET (TX)",
			"JSR2",
			"CPY CPX INY INX (T0)",
			"CMP (T0)",
			"SBC0 (T0)",
			"ADC SBC (T0)",
			"ROL (TX)",

			"JMP ind (T3)",
			"ASL ROL (TX)",
			"JSR/5",
			"BRK JSR RTI RTS Push/pull (T2)",
			"TYA (T0)",
			"UPPER ODD (T1)",
			"ADC SBC (T1)",
			"ASL ROL LSR ROR (T1)",
			"TXA (T0)",
			"PLA (T0)",
			"LDA (T0)",
			"ALL ODD (T0)",
			"TAY (T0)",
			"ASL ROL LSR ROR (T0)",
			"TAX (T0)",
			"BIT0",
			"AND0",
			"OP abs,XY (T4)",
			"OP ind,Y (T5)",

			"BR0",
			"PHA (T2)",
			"LSR ROR (T0)",
			"LSR ROR (TX)",
			"BRK (T2)",
			"JSR (T3)",
			"STA (TX)",
			"BR2",
			"zero page (T2)",
			"ALU indirect (T2)",
			"ABS/2",
			"RTS/5",
			"T4 ANY",
			"T3 ANY",
			"BRK RTI (T0)",
			"JMP (T0)",
			"OP X, ind (T5)",
			"RIGHT_ALL (T3)",

			"OP ind, Y (T4)",
			"RIGHT ODD (T3)",
			"BR3",
			"BRK RTI (TX)",
			"JSR (TX)",
			"JMP (TX)",
			"STORE (TX)",
			"BRK (T4)",
			"PHP (T2)",
			"Push (T2)",
			"JMP/4",
			"RTI RTS (T5)",
			"JSR (T5)",

			"JMP abs (T2)",
			"Pull (T3)",
			"LSR ROR DEC INC DEX NOP (4x4 bottom right) (TX)",
			"ASL ROL (TX)",
			"CLI SEI (T0)",
			"BIT (T1)",
			"CLC SEC (T0)",
			"Memory zero page X/Y (T3)",
			"ADC SBC (T1)",
			"BIT (T0)",
			"PLP (T0)",
			"RTI (T4)",
			"CMP (T1)",
			"CPY CPX abs (T1)",
			"ASL ROL (T1)",
			"CPY CPX zpg/immed (T1)",

			"CLD SED (T0)",
			"/IR6",
			"Memory absolute (T3)",
			"Memory zero page (T2)",
			"Memory indirect (T5)",
			"Memory absolute X/Y (T4)",
			"/IR7",
			"CLV",
			"IMPL",
			"pp",
		};

		public static string GetDecoderOutName(int n)
		{
			return decoder_out_name[n];
		}
	}

}
