using Be.Windows.Forms;
using SharpToolsCustomClass;
using System.Runtime.InteropServices;
using System.Text;
using SharpTools;

namespace NSFPlayer
{
	public partial class FormMain : Form
	{
		[DllImport("kernel32")]
		static extern bool AllocConsole();

		private DSound? audio_backend;
		private NSFSupport nsf = new();
		private bool nsf_loaded = false;
		private bool regdump_loaded = false;
		private bool auxdump_loaded = false;
		private byte current_song = 0;
		private bool Paused = true;         // atomic
		private bool Dma = false;       // atomic
		private bool InitRequired = false;  // atmoic

		// Stats
		private long timeStamp;
		private long aclkCounter = 0;

		private int OutputSampleRate = 48000;
		private float OutputDC = 0.0f;
		private List<float> SampleBuf = new();

		private string DefaultTitle = "";

		private bool fft = false;
		private float FurryIntensity = 1.0f;

		private BreaksCore.AudioSignalFeatures aux_features;
		private int DecimateEach = 1;
		private int DecimateCounter = 0;
		private long AclkToPlay = 0;
		private bool PreferPal = false;

		private float[] aux_dump = Array.Empty<float>();
		private long aux_dump_pointer = 0;

		public FormMain()
		{
			InitializeComponent();
		}

		private void FormMain_Load(object sender, EventArgs e)
		{
			audio_backend = new DSound(Handle);

			DefaultTitle = this.Text;

			comboBox2.SelectedIndex = 0;

			FormSettings.APUPlayerSettings settings = FormSettings.LoadSettings();
			OutputSampleRate = settings.OutputSampleRate;
			OutputDC = settings.DC;
			BreaksCore.Visual2A03Mapping = settings.Visual2A03Mapping;

			if (settings.AllocConsole)
			{
				AllocConsole();
			}

			SetPaused(true);

			backgroundWorker1.RunWorkerAsync();

			signalPlot1.ForceMinMax(true, -0.2f, +1.0f);

			nextTrackToolStripMenuItem.Enabled = false;
			previousTrackToolStripMenuItem.Enabled = false;

			button3.Enabled = false;
		}

		private void settingsToolStripMenuItem_Click(object sender, EventArgs e)
		{
			FormSettings formSettings = new();
			formSettings.FormClosed += FormSettings_FormClosed;
			formSettings.ShowDialog();
		}

		private void FormSettings_FormClosed(object? sender, FormClosedEventArgs e)
		{
			FormSettings form = (FormSettings)sender;
			if (form.UserAnswer == DialogResult.OK)
			{
				FormSettings.APUPlayerSettings settings = FormSettings.LoadSettings();
				OutputSampleRate = settings.OutputSampleRate;
				Redecimate();
				FurryIntensity = settings.FurryIntensity;
				PreferPal = settings.PreferPal;
				OutputDC = settings.DC;
				BreaksCore.Visual2A03Mapping = settings.Visual2A03Mapping;
			}
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

		private void InitBoard(string nsf_filename)
		{
			AclkToPlay = 0;

			byte[] data = File.ReadAllBytes(nsf_filename);
			nsf.LoadNSF(data);
			nsf_loaded = true;
			this.Text = DefaultTitle + " - " + nsf_filename;

			var settings = FormSettings.LoadSettings();
			BreaksCore.CreateBoard("NSFPlayer", settings.APU_Revision, "RP2C02G", "Fami");

			FurryIntensity = settings.FurryIntensity;
			PreferPal = settings.PreferPal;

			// Setup NSF

			bool bank_switching = false;
			for (int i = 0; i < 8; i++)
			{
				if (nsf.GetHead().Bankswitch[i] != 0)
				{
					bank_switching = true;
					break;
				}
			}

			BreaksCore.LoadNSFData(nsf.GetData(), nsf.GetData().Length, nsf.GetHead().LoadAddress);
			BreaksCore.EnableNSFBanking(bank_switching);

			var head = nsf.GetHead();
			for (int i = 0; i < 8; i++)
			{
				BreaksCore.SetDebugInfoByName(
					BreaksCore.DebugInfoType.DebugInfoType_Board,
					BreaksCore.BOARD_CATEGORY, "Bank" + i.ToString(), head.Bankswitch[i]);
			}

			BreaksCore.SetDebugInfoByName(BreaksCore.DebugInfoType.DebugInfoType_APURegs, BreaksCore.APU_REGS_CATEGORY, "Status", 0xf);

			UpdateMemLayout();

			// SRC
			Redecimate();

			// Autoplay
			SetSong(nsf.GetHead().StartingSong);
			SetPaused(!settings.AutoPlay);

			nextTrackToolStripMenuItem.Enabled = true;
			previousTrackToolStripMenuItem.Enabled = true;

			signalPlot1.EnableSelection(true);
		}

		private void DisposeBoard()
		{
			if (audio_backend != null)
				audio_backend.StopSampleBuf();

			SetPaused(true);
			BreaksCore.DestroyBoard();
			nsf_loaded = false;
			regdump_loaded = false;
			auxdump_loaded = false;
			nsf = new();
			this.Text = DefaultTitle;
			UpdateTrackStat();
			aclkCounter = 0;
			toolStripStatusLabelACLKCount.Text = aclkCounter.ToString();

			nextTrackToolStripMenuItem.Enabled = false;
			previousTrackToolStripMenuItem.Enabled = false;

			signalPlot1.PlotSignal(Array.Empty<float>());

			wavesControl1.EnableSelection(false);
			ResetWaves();
		}

		private void loadNSFToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if (openFileDialogNSF.ShowDialog() == DialogResult.OK)
			{
				DisposeBoard();
				InitBoard(openFileDialogNSF.FileName);
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
			InitRequired = true;
			UpdateTrackStat();
		}

		private void PrevSong()
		{
			if (!nsf_loaded)
				return;
			if (current_song <= 1)
				return;
			current_song--;
			SetSong(current_song);
		}

		private void NextSong()
		{
			if (!nsf_loaded)
				return;
			byte total = nsf.GetHead().TotalSongs;
			if (current_song >= total)
				return;
			current_song++;
			SetSong(current_song);
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
			signalPlot1.EnableSelection(paused);
			wavesControl1.EnableSelection(Paused);
			button3.Enabled = Paused;
		}

		private void ExecINIT()
		{
			if (nsf_loaded)
			{
				var head = nsf.GetHead();
				byte? x = (head.PalNtscBits & 2) == 0 ? (byte)(head.PalNtscBits & 1) : null;
				nsf.ExecuteUntilRTS(head.InitAddress, current_song, x, 0, false);
			}
		}

		private void ExecPLAY()
		{
			if (nsf_loaded)
			{
				var head = nsf.GetHead();
				if (nsf.IsCoreReady())
					return;
				nsf.ExecuteUntilRTS(head.PlayAddress, null, null, null, false);
			}
		}

		#endregion "NSF Controls"


		#region "Regdump Controls"

		// This section of code is intended to replace NSF -- reproducing operations on APU registers from their read/write history dump.

		private void loadAPURegisterDumpToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if (openFileDialogRegDump.ShowDialog() == DialogResult.OK)
			{
				DisposeBoard();
				InitBoardForRegDump(openFileDialogRegDump.FileName);
			}
		}

		private void InitBoardForRegDump(string regdump_filename)
		{
			byte[] regdump = File.ReadAllBytes(regdump_filename);
			regdump_loaded = true;
			this.Text = DefaultTitle + " - " + regdump_filename;

			var settings = FormSettings.LoadSettings();
			BreaksCore.CreateBoard("APUPlayer", settings.APU_Revision, "RP2C02G", "Fami");
			BreaksCore.Reset();

			FurryIntensity = settings.FurryIntensity;

			// Setup RegDump

			BreaksCore.LoadRegDump(regdump, regdump.Length);

			UpdateMemLayout();

			// SRC
			Redecimate();

			// Autoplay
			SetPaused(!settings.AutoPlay);
		}

		#endregion "Regdump Controls"


		#region "AUX Dump Controls"

		// This section of code does not simulate the APU at all, but simply outputs samples that have been saved in Logisim (AUX Dumper). The format used is .hex UInt32 as float.

		private void loadAUXDumpToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if (openFileDialogHEX.ShowDialog() == DialogResult.OK)
			{
				DisposeBoard();
				InitBoardForAuxDump(openFileDialogHEX.FileName);
			}
		}

		private void InitBoardForAuxDump(string hex_filename)
		{
			string text = File.ReadAllText(hex_filename);
			aux_dump = LogisimHEXConv.HEXToFloatArray(text);
			aux_dump_pointer = 0;
			auxdump_loaded = true;
			this.Text = DefaultTitle + " - " + hex_filename;

			var settings = FormSettings.LoadSettings();

			FurryIntensity = settings.FurryIntensity;

			// SRC
			Redecimate();

			// Autoplay
			SetPaused(!settings.AutoPlay);
		}

		#endregion "AUX Dump Controls"


		#region "Sample Buffer Playback Controls"

		private void toolStripButtonPlay_Click(object sender, EventArgs e)
		{
			if (audio_backend != null)
			{
				Dma = true;
				audio_backend.PlaySampleBuf(OutputSampleRate, SampleBuf, OutputDC);
				Dma = false;
			}
		}

		private void toolStripButtonDiscard_Click(object sender, EventArgs e)
		{
			if (audio_backend != null)
				audio_backend.StopSampleBuf();

			UpdateSampleBufStats();
			UpdateSignalPlot();

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
			long ms = OutputSampleRate != 0 ? (SampleBuf.Count * 1000) / (long)OutputSampleRate : 0;
			toolStripStatusLabelMsec.Text = ms.ToString() + " ms";
		}

		private void UpdateSignalPlot()
		{
			long numberOfSamples = SampleBuf.Count;
			float[] plot_samples = new float[numberOfSamples];

			long i = 0;
			foreach (var sample in SampleBuf)
			{
				plot_samples[i++] = sample;
			}

			signalPlot1.PlotSignal(plot_samples);
		}

		/// <summary>
		/// This sampler is used for SRC.
		/// The AUX output is sampled at a high frequency, which cannot be played by a ordinary sound card.
		/// Therefore, some of the samples are skipped to match the DSound playback frequency (SourceSampleRate variable).
		/// </summary>
		private void FeedSample()
		{
			if (DecimateCounter >= DecimateEach)
			{
				float sample;

				if (auxdump_loaded)
				{
					if (aux_dump_pointer < aux_dump.Length)
					{
						sample = aux_dump[aux_dump_pointer++];
					}
					else
					{
						SetPaused(true);
						UpdateSampleBufStats();
						UpdateSignalPlot();
						DecimateCounter = 0;
						return;
					}
				}
				else
				{
					BreaksCore.SampleAudioSignal(out sample);
				}

				SampleBuf.Add(sample);
				if (fft)
					furryPlot1.AddSample(sample * FurryIntensity);

				DecimateCounter = 0;
			}
			else
			{
				if (auxdump_loaded)
				{
					aux_dump_pointer++;
				}

				DecimateCounter++;
			}
		}

		/// <summary>
		/// If you change the sampling frequency of the sound source or output frequency, you must recalculate the decimation factor.
		/// </summary>
		private void Redecimate()
		{
			if (nsf_loaded || regdump_loaded)
			{
				BreaksCore.GetApuSignalFeatures(out aux_features);
				DecimateEach = aux_features.SampleRate / OutputSampleRate;
				Console.WriteLine("APUSim sample rate: {0}, DSound sample rate: {1}, decimate factor: {2}", aux_features.SampleRate, OutputSampleRate, DecimateEach);
			}
			else if (auxdump_loaded)
			{
				var settings = FormSettings.LoadSettings();
				DecimateEach = settings.AuxSampleRate / OutputSampleRate;
				Console.WriteLine("AUX sample rate: {0}, DSound sample rate: {1}, decimate factor: {2}", settings.AuxSampleRate, OutputSampleRate, DecimateEach);
			}
			DecimateCounter = 0;
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
		private void DoDebugStep()
		{
			if (Paused && (nsf_loaded || regdump_loaded))
			{
				BreaksCore.Step();
				//TraceCore();
				if (nsf_loaded)
					nsf.SyncExec();
				Button2Click();
				UpdateWaves();

				aclkCounter = BreaksCore.GetACLKCounter();
				toolStripStatusLabelACLKCount.Text = aclkCounter.ToString();
			}
		}

		private void button3_Click(object sender, EventArgs e)
		{
			DoDebugStep();
		}

		private void FormMain_KeyDown(object sender, KeyEventArgs e)
		{
			if (e.KeyCode == Keys.F11)
			{
				DoDebugStep();
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
			ExecINIT();
		}

		/// <summary>
		/// Exec PLAY
		/// </summary>
		private void executePLAYToolStripMenuItem_Click(object sender, EventArgs e)
		{
			ExecPLAY();
		}

		private void loadCPUMemoryDumpToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if (openFileDialogCpumem.ShowDialog() == DialogResult.OK)
			{
				byte[] cpumem = File.ReadAllBytes(openFileDialogCpumem.FileName);

				int descrID = -1;

				for (int i = 0; i < mem.Count; i++)
				{
					if (mem[i].name == "WRAM")
					{
						descrID = i;
						break;
					}
				}

				if (descrID >= 0)
				{
					BreaksCore.WriteMem(descrID, cpumem);
				}
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


		private void loadSaveLogisimAUXDumpForAudacityToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if (openFileDialogHEX.ShowDialog() == DialogResult.OK)
			{
				string hex_filename = openFileDialogHEX.FileName;
				float[] dump = LogisimHEXConv.HEXToFloatArray(File.ReadAllText(hex_filename));

				// Decimation

				List<float> decimated_dump = new();

				var settings = FormSettings.LoadSettings();

				int audacity_sample_rate = settings.OutputSampleRate;
				int decimate_each = settings.AuxSampleRate / audacity_sample_rate;
				int decimate_counter = 0;

				for (long i = 0; i < dump.Length; i++)
				{
					// Renormalize to the [-1.0; +1.0] range that Audacity uses

					float sample = dump[i];
					sample = (sample - 0.5f) * 2;

					if (decimate_counter >= decimate_each)
					{
						decimated_dump.Add(sample);
						decimate_counter = 0;
					}
					else
					{
						decimate_counter++;
					}
				}

				float[] ddump = decimated_dump.ToArray();

				if (saveFileDialogBin.ShowDialog() == DialogResult.OK)
				{
					string bin_filename = saveFileDialogBin.FileName;

					var byteArray = new byte[ddump.Length * 4];
					Buffer.BlockCopy(ddump, 0, byteArray, 0, byteArray.Length);

					File.WriteAllBytes(bin_filename, byteArray);
				}

				MessageBox.Show("Done.", "Message", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
			}
		}

		private void saveSampleBufferAsWAVToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if (saveFileDialogWAV.ShowDialog() == DialogResult.OK)
			{
				var settings = FormSettings.LoadSettings();
				Dma = true;
				SaveWav(saveFileDialogWAV.FileName, SampleBuf, settings.OutputSampleRate, settings.DC);
				Dma = false;
			}
		}

		// https://stackoverflow.com/questions/14659684/creating-a-wav-file-in-c-sharp

		private void SaveWav(string filename, List<float> samples, int samplerate, float DC)
		{
			int numsamples = samples.Count;
			ushort numchannels = 1;
			ushort samplelength = 2; // in bytes

			FileStream f = new FileStream(filename, FileMode.Create);
			BinaryWriter wr = new BinaryWriter(f);

			int data_blob_size = numsamples * numchannels * samplelength;

			wr.Write(Encoding.ASCII.GetBytes("RIFF"));
			wr.Write(0x2c + data_blob_size);
			wr.Write(Encoding.ASCII.GetBytes("WAVE"));
			wr.Write(Encoding.ASCII.GetBytes("fmt "));
			wr.Write(16);
			wr.Write((short)1); // Encoding (1: PCM)
			wr.Write((short)numchannels); // Channels
			wr.Write((int)(samplerate)); // Sample rate
			wr.Write((int)(samplerate * samplelength * numchannels) / 8); // Average bytes per second
			wr.Write((short)(samplelength * numchannels)); // block align
			wr.Write((short)(8 * samplelength)); // bits per sample
			wr.Write(Encoding.ASCII.GetBytes("data"));
			wr.Write((int)(data_blob_size));

			foreach (var sample in samples)
			{
				short value = (short)((sample + DC) * Int16.MaxValue);
				wr.Write(value);
			}

			f.Close();
		}

		private void toolStripButton2_Click(object sender, EventArgs e)
		{
			bool loaded_smth = (nsf_loaded || regdump_loaded || auxdump_loaded);

			if (Paused && loaded_smth)
			{
				if (signalPlot1.IsSelectedSomething())
				{
					float[] data = signalPlot1.SnatchSelection();
					signalPlot1.ClearSelection();
					FormSnatch snatch = new FormSnatch(data);
					snatch.Show();
				}
				else
				{
					MessageBox.Show("Select something first with the left mouse button. A box will appear, then click on Snatch.", "Message", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
				}
			}
			else
			{
				if (!loaded_smth)
				{
					MessageBox.Show("Nothing loaded", "Message", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
				}
				else
				{
					MessageBox.Show("I can't while the worker is running. Pause it first", "Message", MessageBoxButtons.OK, MessageBoxIcon.Exclamation);
				}
			}
		}
	}
}
