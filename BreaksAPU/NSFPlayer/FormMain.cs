using Be.Windows.Forms;
using NSFPlayerCustomClass;
using System.Runtime.InteropServices;
using System.Windows.Forms;

namespace NSFPlayer
{
	public partial class FormMain : Form
	{
		[DllImport("kernel32")]
		static extern bool AllocConsole();

		private DSound? audio_backend;
		private NSFSupport nsf = new();
		private bool nsf_loaded = false;
		private byte current_song = 0;
		private bool Paused = true;         // atomic
		private bool Dma = false;		// atomic

		// Stats
		private long timeStamp;
		private int aclkCounter = 0;

		private int SourceSampleRate = 48000;
		private List<float> SampleBuf = new();

		private string DefaultTitle = "";

		private bool fft = false;

		public FormMain()
		{
			InitializeComponent();
			AllocConsole();
		}

		private void FormMain_Load(object sender, EventArgs e)
		{
			audio_backend = new DSound(Handle);

			DefaultTitle = this.Text;

			comboBox2.SelectedIndex = 0;

			FormSettings.APUPlayerSettings settings = FormSettings.LoadSettings();
			SourceSampleRate = settings.OutputSampleRate;

			SetPaused(true);

			backgroundWorker1.RunWorkerAsync();

			signalPlot1.ForceMinMax(true, -0.5f, +2.0f);
		}

		private void settingsToolStripMenuItem_Click(object sender, EventArgs e)
		{
			FormSettings formSettings = new();
			formSettings.FormClosed += FormSettings_FormClosed;
			formSettings.ShowDialog();
		}

		private void FormSettings_FormClosed(object? sender, FormClosedEventArgs e)
		{
			FormSettings.APUPlayerSettings settings = FormSettings.LoadSettings();
			SourceSampleRate = settings.OutputSampleRate;
		}

		private void exitToolStripMenuItem_Click(object sender, EventArgs e)
		{
			Close();
		}

		private void aboutToolStripMenuItem_Click(object sender, EventArgs e)
		{
			FormAbout dlg = new FormAbout();
			dlg.ShowDialog();
		}


		#region "NSF Controls"

		private void backgroundWorker1_DoWork(object sender, System.ComponentModel.DoWorkEventArgs e)
		{
			while (!backgroundWorker1.CancellationPending)
			{
				if (Paused || !nsf_loaded || Dma)
				{
					Thread.Sleep(10);
					continue;
				}

				// Simulate APU

				NSFPlayerInterop.Step();
				nsf.SyncExec();

				// Add audio sample

				float next_sample;
				NSFPlayerInterop.SampleAudioSignal(out next_sample);
				FeedSample(next_sample);
				if (fft)
					furryPlot1.AddSample(next_sample);

				// Show statistics that are updated once every 1 second.

				long now = DateTime.Now.Ticks / TimeSpan.TicksPerMillisecond;
				if (now > (timeStamp + 1000))
				{
					timeStamp = now;

					UpdateSampleBufStats();
					UpdateSignalPlot();

					int aclk_per_sec = NSFPlayerInterop.GetACLKCounter() - aclkCounter;
					toolStripStatusLabelACLK.Text = aclk_per_sec.ToString();

					aclkCounter = NSFPlayerInterop.GetACLKCounter();
				}
			}
		}

		private void InitBoard(string nsf_filename)
		{
			byte[] data = File.ReadAllBytes(nsf_filename);
			nsf.LoadNSF(data);
			nsf_loaded = true;
			SetSong(nsf.GetHead().StartingSong);
			this.Text = DefaultTitle + " - " + nsf_filename;

			var settings = FormSettings.LoadSettings();
			NSFPlayerInterop.CreateBoard("NSFPlayer", settings.APU_Revision, "None", "None");

			// Setup NSF

			bool bank_switching = false;
			for (int i=0; i<8; i++)
			{
				if (nsf.GetHead().Bankswitch[i] != 0)
				{
					bank_switching = true;
					break;
				}
			}

			NSFPlayerInterop.LoadNSFData(nsf.GetData(), nsf.GetData().Length, nsf.GetHead().LoadAddress);
			NSFPlayerInterop.EnableNSFBanking(bank_switching);

			var head = nsf.GetHead();
			for (int i = 0; i < 8; i++)
			{
				BreaksCore.SetDebugInfoByName(
					BreaksCore.DebugInfoType.DebugInfoType_Board, 
					BreaksCore.BOARD_CATEGORY, "Bank" + i.ToString(), head.Bankswitch[i]);
			}

			BreaksCore.SetDebugInfoByName(BreaksCore.DebugInfoType.DebugInfoType_APURegs, BreaksCore.APU_REGS_CATEGORY, "Status", 0xf);

			UpdateMemLayout();

			// Autoplay
			SetPaused(!settings.AutoPlay);
		}

		private void DisposeBoard()
		{
			SetPaused(true);
			NSFPlayerInterop.DestroyBoard();
			nsf_loaded = false;
			nsf = new();
			this.Text = DefaultTitle;
			UpdateTrackStat();
			aclkCounter = 0;
		}

		private void loadNSFToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if (openFileDialog1.ShowDialog() == DialogResult.OK)
			{
				string nsf_filename = openFileDialog1.FileName;
				DisposeBoard();
				InitBoard(nsf_filename);
			}
		}

		private void playToolStripMenuItem_Click(object sender, EventArgs e)
		{
			SetPaused(false);
		}

		private void pauseToolStripMenuItem_Click(object sender, EventArgs e)
		{
			SetPaused(true);
		}

		private void stopToolStripMenuItem_Click(object sender, EventArgs e)
		{
			DisposeBoard();
		}

		private void previousTrackToolStripMenuItem_Click(object sender, EventArgs e)
		{
			PrevSong();
		}

		private void nextTrackToolStripMenuItem_Click(object sender, EventArgs e)
		{
			NextSong();
		}

		private void nSFInfoToolStripMenuItem_Click(object sender, EventArgs e)
		{
			FormNSFInfo info = new FormNSFInfo(nsf_loaded ? nsf.GetHead() : null);
			info.ShowDialog();
		}

		private void SetSong(byte n)
		{
			current_song = n;
			UpdateTrackStat();
		}

		private void PrevSong()
		{
			if (!nsf_loaded)
				return;
			byte total = nsf.GetHead().TotalSongs;
			if (current_song <= 1)
				return;
			current_song--;
			UpdateTrackStat();
		}

		private void NextSong()
		{
			if (!nsf_loaded)
				return;
			byte total = nsf.GetHead().TotalSongs;
			if (current_song >= total)
				return;
			current_song++;
			UpdateTrackStat();
		}

		private void UpdateTrackStat()
		{
			if (nsf_loaded)
				toolStripStatusLabel4.Text = current_song.ToString() + " / " + nsf.GetHead().TotalSongs.ToString();
			else
				toolStripStatusLabel4.Text = "Not loaded";
		}

		private void SetPaused(bool paused)
		{
			Paused = paused;
			toolStripStatusLabelState.Text = paused ? "Paused" : "Running";
			if (paused)
			{
				playToolStripMenuItem.Checked = false;
				pauseToolStripMenuItem.Checked = true;
			}
			else
			{
				playToolStripMenuItem.Checked = true;
				pauseToolStripMenuItem.Checked = false;
			}
		}

		#endregion "NSF Controls"


		#region "Sample Buffer Playback Controls"

		private void toolStripButtonPlay_Click(object sender, EventArgs e)
		{
			if (audio_backend != null)
			{
				Dma = true;
				audio_backend.PlaySampleBuf(SourceSampleRate, SampleBuf);
				Dma = false;
			}
		}

		private void toolStripButtonDiscard_Click(object sender, EventArgs e)
		{
			if (audio_backend != null)
				audio_backend.StopSampleBuf();
			Dma = true;
			SampleBuf.Clear();
			Dma = false;
		}

		private void toolStripButtonStop_Click(object sender, EventArgs e)
		{
			if (audio_backend != null)
				audio_backend.StopSampleBuf();
		}

		private void UpdateSampleBufStats()
		{
			toolStripStatusLabelSamples.Text = SampleBuf.Count.ToString();
			long ms = SourceSampleRate != 0 ? (SampleBuf.Count * 1000) / (long)SourceSampleRate : 0;
			toolStripStatusLabelMsec.Text = ms.ToString() + " ms";
		}

		private void UpdateSignalPlot()
		{
			int numberOfSamples = SampleBuf.Count;
			float[] plot_samples = new float[numberOfSamples];

			for (int i = 0; i < numberOfSamples; i++)
			{
				plot_samples[i] = SampleBuf[i];
			}

			signalPlot1.PlotSignal(plot_samples);
		}

		/// <summary>
		/// This sampler is used for SRC.
		/// The AUX output is sampled at a high frequency, which cannot be played by a ordinary sound card.
		/// Therefore, some of the samples are skipped to match the DSound playback frequency (SourceSampleRate variable).
		/// </summary>
		/// <param name="sample"></param>
		private void FeedSample (float sample)
		{
			// TODO: Skip some samples :)

			SampleBuf.Add(sample);
		}

		#endregion "Sample Buffer Playback Controls"


		#region "APU Debug"

		List<BreaksCore.MemDesciptor> mem = new();

		private void comboBox2_SelectedIndexChanged(object sender, EventArgs e)
		{
			Button2Click();
		}

		private void comboBox1_SelectedIndexChanged(object sender, EventArgs e)
		{
			if (UpdateMemLayoutInProgress)
				return;

			Button1Click();
		}

		/// <summary>
		/// Update DebugInfo
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void button2_Click(object sender, EventArgs e)
		{
			Button2Click();
		}

		/// <summary>
		/// Dump Mem
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void button1_Click(object sender, EventArgs e)
		{
			Button1Click();
		}

		void Button2Click()
		{
			List<BreaksCore.DebugInfoEntry> entries = BreaksCore.GetDebugInfo(ComboBoxToDebugInfoType());
			UpdateDebugInfo(entries);
		}

		BreaksCore.DebugInfoType ComboBoxToDebugInfoType()
		{
			switch (comboBox2.SelectedIndex)
			{
				case 0:
					return BreaksCore.DebugInfoType.DebugInfoType_Core;
				case 1:
					return BreaksCore.DebugInfoType.DebugInfoType_CoreRegs;
				case 2:
					return BreaksCore.DebugInfoType.DebugInfoType_APU;
				case 3:
					return BreaksCore.DebugInfoType.DebugInfoType_APURegs;
				case 4:
					return BreaksCore.DebugInfoType.DebugInfoType_Board;
			}

			return BreaksCore.DebugInfoType.DebugInfoType_Test;
		}

		void UpdateDebugInfo(List<BreaksCore.DebugInfoEntry> entries)
		{
			// Construct an object to show in PropertyGrid

			CustomClass myProperties = new CustomClass();

			foreach (BreaksCore.DebugInfoEntry entry in entries)
			{
				CustomProperty myProp = new CustomProperty();
				myProp.Name = entry.name;
				myProp.Category = entry.category;
				myProp.Value = entry.value;
				myProperties.Add(myProp);
			}

			propertyGrid1.SelectedObject = myProperties;
		}

		void Button1Click()
		{
			if (mem.Count == 0)
			{
				hexBox1.ByteProvider = new DynamicByteProvider(new byte[0]);
				hexBox1.Refresh();
				return;
			}

			int descrID = comboBox1.SelectedIndex;

			{
				byte[] buf = new byte[mem[descrID].size];

				BreaksCore.DumpMem(descrID, buf);
				hexBox1.ByteProvider = new DynamicByteProvider(buf);
				hexBox1.Refresh();
			}
		}

		bool UpdateMemLayoutInProgress = false;

		/// <summary>
		/// Get a set of memory regions from the debugger and fill the ComboBox.
		/// </summary>
		void UpdateMemLayout()
		{
			UpdateMemLayoutInProgress = true;

			comboBox1.Items.Clear();

			mem = BreaksCore.GetMemoryLayout();

			foreach (var descr in mem)
			{
				comboBox1.Items.Add(descr.name);
			}

			if (mem.Count != 0)
			{
				comboBox1.SelectedIndex = 0;
			}

			UpdateMemLayoutInProgress = false;
		}

		/// <summary>
		/// A tool for short-range debugging.
		/// Step-by-step execution of the simulation. Available only if the worker is stopped.
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void button3_Click(object sender, EventArgs e)
		{
			if (Paused && nsf_loaded)
			{
				NSFPlayerInterop.Step();
				TraceCore();
				nsf.SyncExec();
				Button2Click();
			}
		}

		private void TraceCore()
		{
			string text = "";
			List<BreaksCore.DebugInfoEntry> entries = BreaksCore.GetDebugInfo(BreaksCore.DebugInfoType.DebugInfoType_CoreRegs);
			foreach (var entry in entries)
			{
				text += entry.name + " = " + entry.value.ToString("X2") + "; ";
			}
			Console.WriteLine(text);
		}

		/// <summary>
		/// Exec INIT.
		/// </summary>
		private void executeINITToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if (nsf_loaded)
			{
				var head = nsf.GetHead();
				byte? x = (head.PalNtscBits & 2) != 0 ? (byte)(head.PalNtscBits & 1) : null;
				nsf.ExecuteUntilRTS(head.InitAddress, current_song, x, 0);
			}
		}

		/// <summary>
		/// Exec PLAY
		/// </summary>
		private void executePLAYToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if (nsf_loaded)
			{
				var head = nsf.GetHead();
				nsf.ExecuteUntilRTS(head.PlayAddress, null, null, null);
			}
		}

		#endregion "APU Debug"


		#region "What's that for?"

		private void sendFeedbackToolStripMenuItem_Click(object sender, EventArgs e)
		{
			OpenUrl("https://github.com/emu-russia/breaknes/issues");
		}

		private void checkForUpdatesToolStripMenuItem_Click(object sender, EventArgs e)
		{
			OpenUrl("https://github.com/emu-russia/breaknes/releases");
		}

		private void OpenUrl(string url)
		{
			// https://stackoverflow.com/questions/4580263/how-to-open-in-default-browser-in-c-sharp

			try
			{
				System.Diagnostics.Process.Start(url);
			}
			catch
			{
				// hack because of this: https://github.com/dotnet/corefx/issues/10361
				if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
				{
					url = url.Replace("&", "^&");
					System.Diagnostics.Process.Start(new System.Diagnostics.ProcessStartInfo(url) { UseShellExecute = true });
				}
				else if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
				{
					System.Diagnostics.Process.Start("xdg-open", url);
				}
				else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
				{
					System.Diagnostics.Process.Start("open", url);
				}
				else
				{
					throw;
				}
			}
		}

		
		/// <summary>
		/// FFT enable button.
		/// </summary>
		/// <param name="sender"></param>
		/// <param name="e"></param>
		private void toolStripButton1_Click(object sender, EventArgs e)
		{
			if (toolStripButton1.Checked)
			{
				toolStripButton1.Checked = false;
				fft = false;
			}
			else
			{
				toolStripButton1.Checked = true;
				fft = true;
			}
		}


		#endregion "What's that for?"

	}
}
