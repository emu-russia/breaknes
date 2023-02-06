// https://www.nesdev.org/wiki/NSF

namespace NSFPlayer
{
	public class NSFHeader
	{
		public byte[] Signature { get; set; } = new byte[5];
		public byte Version { get; set; }
		public byte TotalSongs { get; set; }
		public byte StartingSong { get; set; }
		public UInt16 LoadAddress { get; set; }
		public UInt16 InitAddress { get; set; }
		public UInt16 PlayAddress { get; set; }
		public string Name { get; set; } = "";
		public string Artist { get; set; } = "";
		public string Copyright { get; set; } = "";
		public UInt16 PlaySpeedNtsc { get; set; }
		public byte[] Bankswitch { get; set; } = new byte[8];
		public UInt16 PlaySpeedPal { get; set; }
		public byte PalNtscBits { get; set; }
		public byte ExtraChips { get; set; }
		public byte ReservedNSF2 { get; set; }
		public int DataLength { get; set; }
	}

	public class NSFLoader
	{
		private NSFHeader head = new NSFHeader();
		private byte[] nsf_data = Array.Empty<byte>();
		private const int data_offset = 0x80;

		public NSFHeader GetHead()
		{
			return head;
		}

		public byte[] GetData()
		{
			return nsf_data;
		}

		public void LoadNSF(byte[] data)
		{
			LoadHeader(data);
			nsf_data = new byte[head.DataLength];
			Array.Copy(data, data_offset, nsf_data, 0, head.DataLength);
		}

		private void LoadHeader(byte[] data)
		{
			for (int i=0; i<5; i++)
			{
				head.Signature[i] = data[i];
			}

			head.Version = data[5];
			head.TotalSongs = data[6];
			head.StartingSong = data[7];
			head.LoadAddress = (UInt16)((data[9] << 8) | data[8]);
			head.InitAddress = (UInt16)((data[11] << 8) | data[10]);
			head.PlayAddress = (UInt16)((data[13] << 8) | data[12]);

			head.Name = "";
			for (int i = 0; i < 32; i++)
			{
				if (data[i + 0xe] == 0)
					break;
				head.Name += (char)data[i + 0xe];
			}

			head.Artist = "";
			for (int i = 0; i < 32; i++)
			{
				if (data[i + 0x2e] == 0)
					break;
				head.Artist += (char)data[i + 0x2e];
			}

			head.Copyright = "";
			for (int i = 0; i < 32; i++)
			{
				if (data[i + 0x4e] == 0)
					break;
				head.Copyright += (char)data[i + 0x4e];
			}

			head.PlaySpeedNtsc = (UInt16)((data[0x6f] << 8) | data[0x6e]);
			head.PlaySpeedPal = (UInt16)((data[0x79] << 8) | data[0x78]);

			for (int i = 0; i < 8; i++)
			{
				head.Bankswitch[i] = data[i + 0x70];
			}

			head.PalNtscBits = data[0x7a];
			head.ExtraChips = data[0x7b];
			head.ReservedNSF2 = data[0x7c];

			head.DataLength = (int)((data[0x7f] << 16) | (data[0x7e] << 8) | data[0x7d]);

			if (head.DataLength == 0)
			{
				head.DataLength = data.Length - data_offset;
			}
		}
	}
}
