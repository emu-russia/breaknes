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
		public string ppu = "";
		public string p1 = "";
		public IOPort [] io = Array.Empty<IOPort>();
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
