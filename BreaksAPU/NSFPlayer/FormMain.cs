using Be.Windows.Forms;
using NSFPlayerCustomClass;
using System.Runtime.InteropServices;

namespace NSFPlayer
{
	public partial class FormMain : Form
	{
		[DllImport("kernel32")]
		static extern bool AllocConsole();

		private DSound? audio_backend;

		private int SourceSampleRate = 48000;
		private List<float> SampleBuf = new();

		private string DefaultTitle = "";

		public FormMain()
		{
			InitializeComponent();
		}

		private void FormMain_Load(object sender, EventArgs e)
		{
#if DEBUG
			AllocConsole();
#endif

			audio_backend = new DSound(Handle);

			DefaultTitle = this.Text;

			comboBox2.SelectedIndex = 0;
		}

		private void settingsToolStripMenuItem_Click(object sender, EventArgs e)
		{
			FormSettings formSettings = new();
			formSettings.ShowDialog();
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

		private void loadNSFToolStripMenuItem_Click(object sender, EventArgs e)
		{

		}

		private void playToolStripMenuItem_Click(object sender, EventArgs e)
		{

		}

		private void pauseToolStripMenuItem_Click(object sender, EventArgs e)
		{

		}

		private void stopToolStripMenuItem_Click(object sender, EventArgs e)
		{

		}

		private void previousTrackToolStripMenuItem_Click(object sender, EventArgs e)
		{

		}

		private void nextTrackToolStripMenuItem_Click(object sender, EventArgs e)
		{

		}

		private void nSFInfoToolStripMenuItem_Click(object sender, EventArgs e)
		{
			FormNSFInfo info = new FormNSFInfo();
			info.ShowDialog();
		}

		#endregion "NSF Controls"


		#region "Sample Buffer Playback Controls"

		private void toolStripButtonPlay_Click(object sender, EventArgs e)
		{
			if (audio_backend != null)
				audio_backend.PlaySampleBuf(SourceSampleRate, SampleBuf);
		}

		private void toolStripButtonDiscard_Click(object sender, EventArgs e)
		{
			if (audio_backend != null)
				audio_backend.StopSampleBuf();
			SampleBuf.Clear();
		}

		private void toolStripButtonStop_Click(object sender, EventArgs e)
		{
			if (audio_backend != null)
				audio_backend.StopSampleBuf();
		}

		#endregion "Sample Buffer Playback Controls"


		#region "APU Debug"

		private void loadSaveLogisimHexAsBinToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if (openFileDialogHEX.ShowDialog() == DialogResult.OK)
			{
				string hex_filename = openFileDialogHEX.FileName;
				byte[] arr = LogisimHEXConv.HEXToByteArray(File.ReadAllText(hex_filename));

				if (saveFileDialogBin.ShowDialog() == DialogResult.OK)
				{
					string bin_filename = saveFileDialogBin.FileName;
					File.WriteAllBytes(bin_filename, arr);
				}
			}
		}

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

		#endregion "APU Debug"
	}
}
