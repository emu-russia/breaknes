// Trivial control of the instance of the emulated system. Create a board and plug in a cartridge.

using SharpTools;

namespace Breaknes
{
	class BoardControl
	{
		public void CreateBoardAndStickCartridge(BoardDescription db, string name, string nes_file)
		{
			foreach (var board in db.boards)
			{
				if (board.name == name)
				{
					BreaksCore.CreateBoard(board.name, board.apu, board.ppu, board.p1);
					byte[] nes_image = File.ReadAllBytes(nes_file);
					BreaksCore.InsertCartridge(nes_image, nes_image.Length);
					break;
				}
			}
		}

		public void DisposeBoard()
		{
			BreaksCore.DestroyBoard();
		}
	}
}
