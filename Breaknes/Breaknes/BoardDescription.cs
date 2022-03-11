using System.Text;
using System.Globalization;
using System.ComponentModel;
using System.Collections.Generic;
using System.Collections;

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
}
