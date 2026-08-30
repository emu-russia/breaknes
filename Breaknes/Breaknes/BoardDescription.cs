using System.Text;
using Newtonsoft.Json;

namespace Breaknes
{
	public class BoardDescription
	{
		public List<Board> boards = new List<Board>();
	}

	public class IOPort
	{
		public string name = "";
		public UInt32 [] devices = Array.Empty<UInt32>();
	}

	public class Board
	{
		public string name = "";
		public string apu = "";

		/// <summary>
		/// PPU revision (legacy single-PPU form). When `ppus` is empty, this value
		/// is used as the single PPU of the board.
		/// </summary>
		public string ppu = "";

		/// <summary>
		/// PPU revisions of the board (up to 2). When empty, the `ppu` field is used.
		/// </summary>
		public List<string> ppus = new List<string>();

		/// <summary>
		/// Physical layout of the TV Sets in the emulator window: "horizontal" (side by side) or "vertical" (one above the other).
		/// </summary>
		public string tv_layout = "horizontal";

		/// <summary>
		/// TV Set binding: tvs[i] = index of the PPU whose video signal is displayed
		/// on the i-th TV. When empty, TV[i] shows PPU[i]. For debugging, the same PPU
		/// can be bound to both TVs (e.g. [0, 0]).
		/// </summary>
		public List<int> tvs = new List<int>();

		public string p1 = "";
		public IOPort [] io = Array.Empty<IOPort>();

		/// <summary>
		/// The effective list of PPU revisions of the board.
		/// </summary>
		public List<string> GetEffectivePpus()
		{
			if (ppus.Count > 0)
			{
				return ppus;
			}
			if (!string.IsNullOrEmpty(ppu))
			{
				return new List<string> { ppu };
			}
			return new List<string>();
		}

		/// <summary>
		/// The effective TV->PPU binding list. When `tvs` is not specified, the
		/// default identity binding is used (TV[i] shows PPU[i], capped at 2 TVs).
		/// </summary>
		public List<int> GetEffectiveTvBinding(int ppuCount)
		{
			List<int> binding = new List<int>();
			if (tvs.Count > 0)
			{
				foreach (var ppu_index in tvs)
				{
					binding.Add(ppu_index);
				}
			}
			else
			{
				int tv_count = Math.Min(ppuCount, 2);
				for (int i = 0; i < tv_count; i++)
				{
					binding.Add(i);
				}
			}
			return binding;
		}
	}

	public class BoardDescriptionLoader
	{
		static public BoardDescription Load ()
		{
			string json = File.ReadAllText ("BoardDescription.json", Encoding.UTF8);
			var descr = JsonConvert.DeserializeObject<BoardDescription>(json);
			if (descr == null)
			{
				descr = new BoardDescription();
			}
			//Console.Write(JsonConvert.SerializeObject (descr));
			return descr;
		}
	}
}
