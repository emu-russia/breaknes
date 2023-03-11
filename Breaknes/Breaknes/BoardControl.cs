// Trivial control of the instance of the emulated system. Create a board and plug in a cartridge.

using SharpTools;

namespace Breaknes
{
	public class BoardControl
	{
		public bool Paused = true;			// atomic

		public void CreateBoard(BoardDescription db, string name)
		{
			foreach (var board in db.boards)
			{
				if (board.name == name)
				{
					BreaksCore.CreateBoard(board.name, board.apu, board.ppu, board.p1);
					BreaksCore.Reset();

					// Make additional settings for emulation in the Breaknes casual environment

					BreaksCore.SetOamDecayBehavior(BreaksCore.OAMDecayBehavior.Keep);
					BreaksCore.SetRAWColorMode(true);

					break;
				}
			}
		}

		public void DisposeBoard()
		{
			BreaksCore.DestroyBoard();
		}

		public void InsertCartridge(string nes_file)
		{
			byte[] nes_image = File.ReadAllBytes(nes_file);
			BreaksCore.InsertCartridge(nes_image, nes_image.Length);
		}

		public void EjectCartridge()
		{
			BreaksCore.EjectCartridge();
		}
	}
}
