using System.Text;
using Newtonsoft.Json;

namespace Breaknes
{
    public class BoardDescription
    {
        public List<Board> boards = new List<Board>();
    }

    public class Board
    {
        public string name = "";
        public string apu = "";
        public string ppu = "";
        public string p1 = "";
    }

    public class BoardDescriptionLoader
    {
		static public BoardDescription Load ()
        {
			string json = File.ReadAllText ("BoardDescription.json", Encoding.UTF8);
            return JsonConvert.DeserializeObject<BoardDescription>(json);
		}
	}
}
