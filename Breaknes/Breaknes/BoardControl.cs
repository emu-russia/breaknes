// Trivial control of the instance of the emulated system. Create a board and plug in a cartridge.

using System.IO;
using SharpTools;

namespace Breaknes
{
	public class BoardControl
	{
		public bool Paused = true;          // atomic
		public bool SimulationStarted = false;

		public OnUpdateWaves? onUpdateWaves = null;
		public delegate void OnUpdateWaves();

		public void CreateBoard(BoardDescription db, string name)
		{
			foreach (var board in db.boards)
			{
				if (board.name == name)
				{
					// Point the CartPcb path at the Nescartdb data (copied into the
					// Nescartdb/ subfolder next to the executable at build time) and
					// at the user board dir.
					string dataDir = System.AppDomain.CurrentDomain.BaseDirectory;
					BreaksCore.SetNescartdbDir(Path.Combine(dataDir, "Nescartdb"));
					BreaksCore.SetUserBoardsDir(Path.Combine(dataDir, "CustomBoards"));

					// The board may contain several PPUs (up to 2). Each PPU is bound
					// to its own virtual TV Set by default; the binding can be changed
					// below according to the BoardDescription.json ("tvs" array).
					var ppus = board.GetEffectivePpus();
					BreaksCore.CreateBoardEx(board.name, board.apu, ppus.ToArray(), ppus.Count, board.p1);
					BreaksCore.Reset();

					// Apply the TV Set binding from the board description (or the default identity binding).
					int ppuCount = BreaksCore.GetPPUCount();
					var tvBinding = board.GetEffectiveTvBinding(ppuCount);
					for (int tv = 0; tv < tvBinding.Count && tv < 2; tv++)
					{
						BreaksCore.BindPPUToTV(tvBinding[tv], tv, true);
					}

					// Make additional settings for emulation in the Breaknes casual environment

					BreaksCore.SetOamDecayBehavior(BreaksCore.OAMDecayBehavior.Keep);
					BreaksCore.SetRAWColorMode(true);

					SimulationStarted = true;

					break;
				}
			}
		}

		public void DisposeBoard()
		{
			BreaksCore.DestroyBoard();
			SimulationStarted = false;
		}

		public int InsertCartridge(string nes_file)
		{
			byte[] nes_image = File.ReadAllBytes(nes_file);
			return BreaksCore.InsertCartridge(nes_image, nes_image.Length);
		}

		public void EjectCartridge()
		{
			BreaksCore.EjectCartridge();
		}
	}
}
