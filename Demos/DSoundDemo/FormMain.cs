using Microsoft.VisualBasic;
using SharpDX;
using SharpDX.DirectSound;
using SharpDX.Multimedia;
using System.Diagnostics;
using System.Windows.Forms;

namespace DSoundDemo
{
	public partial class FormMain : Form
	{
		DirectSound directSound;
		PrimarySoundBuffer primarySoundBuffer;
		SecondarySoundBuffer secondarySoundBuffer;
		WaveFormat waveFormat;

		public FormMain()
		{
			InitializeComponent();
		}

		private void FormMain_Load(object sender, EventArgs e)
		{
			InitDSound();
		}

		// https://github.com/sharpdx/SharpDX-Samples/blob/master/Desktop/DirectSound/PlaySound/Program.cs

		private void InitDSound()
		{
			directSound = new DirectSound();

			directSound.SetCooperativeLevel(Handle, CooperativeLevel.Priority);

			// Create PrimarySoundBuffer
			var primaryBufferDesc = new SoundBufferDescription();
			primaryBufferDesc.Flags = BufferFlags.PrimaryBuffer;
			primaryBufferDesc.AlgorithmFor3D = Guid.Empty;

			primarySoundBuffer = new PrimarySoundBuffer(directSound, primaryBufferDesc);

			// Play the PrimarySound Buffer
			primarySoundBuffer.Play(0, PlayFlags.Looping);

			// Default WaveFormat Stereo 44100 16 bit
			waveFormat = new WaveFormat();

			// Create SecondarySoundBuffer
			var secondaryBufferDesc = new SoundBufferDescription();
			secondaryBufferDesc.BufferBytes = waveFormat.ConvertLatencyToByteSize(60000);
			secondaryBufferDesc.Format = waveFormat;
			secondaryBufferDesc.Flags = BufferFlags.GetCurrentPosition2 | BufferFlags.ControlPositionNotify | BufferFlags.GlobalFocus |
										BufferFlags.ControlVolume | BufferFlags.StickyFocus;
			secondaryBufferDesc.AlgorithmFor3D = Guid.Empty;
			secondarySoundBuffer = new SecondarySoundBuffer(directSound, secondaryBufferDesc);
		}

		private void PaintSound()
		{
			// Get Capabilties from secondary sound buffer
			var capabilities = secondarySoundBuffer.Capabilities;

			// Lock the buffer
			DataStream dataPart2;
			var dataPart1 = secondarySoundBuffer.Lock(0, capabilities.BufferBytes, LockFlags.EntireBuffer, out dataPart2);

			// Fill the buffer with some sound
			int numberOfSamples = capabilities.BufferBytes / waveFormat.BlockAlign;

			int plot_samples_count = Math.Min(numberOfSamples, 4096 * 16);
			float[] plot_samples = new float[plot_samples_count];

			for (int i = 0; i < numberOfSamples; i++)
			{
				double vibrato = Math.Cos(2 * Math.PI * 10.0 * i / waveFormat.SampleRate);
				short value = (short)(Math.Cos(2 * Math.PI * (220.0 + 4.0 * vibrato) * i / waveFormat.SampleRate) * 16384); // Not too loud
				
				if (i < plot_samples_count)
				{
					plot_samples[i] = value;
					furryPlot1.AddSample(value);
				}

				dataPart1.Write(value);
				dataPart1.Write(value);
			}

			signalPlot1.PlotSignal(plot_samples);

			// Unlock the buffer
			secondarySoundBuffer.Unlock(dataPart1, dataPart2);
		}

		private void PaintWavFile (string wav_name)
		{
			float[] smono;

			bool res = WavLoader.Load(wav_name, out smono);
			if (!res)
			{
				MessageBox.Show("ReadWav cant");
				return;
			}

			// Create SecondarySoundBuffer
			var secondaryBufferDesc = new SoundBufferDescription();
			secondaryBufferDesc.BufferBytes = smono.Length * 4;
			secondaryBufferDesc.Format = waveFormat;
			secondaryBufferDesc.Flags = BufferFlags.GetCurrentPosition2 | BufferFlags.ControlPositionNotify | BufferFlags.GlobalFocus |
										BufferFlags.ControlVolume | BufferFlags.StickyFocus;
			secondaryBufferDesc.AlgorithmFor3D = Guid.Empty;
			secondarySoundBuffer = new SecondarySoundBuffer(directSound, secondaryBufferDesc);

			// Get Capabilties from secondary sound buffer
			var capabilities = secondarySoundBuffer.Capabilities;

			// Lock the buffer
			DataStream dataPart2;
			var dataPart1 = secondarySoundBuffer.Lock(0, capabilities.BufferBytes, LockFlags.EntireBuffer, out dataPart2);

			// Fill the buffer with some sound
			int numberOfSamples = smono.Length;

			for (int i = 0; i < numberOfSamples; i++)
			{
				short value_l = (short)(smono[i] * Int16.MaxValue);
				short value_r = (short)(smono[i] * Int16.MaxValue);

				dataPart1.Write(value_l);
				dataPart1.Write(value_r);
			}

			// Unlock the buffer
			secondarySoundBuffer.Unlock(dataPart1, dataPart2);
		}

		private void exitToolStripMenuItem_Click(object sender, EventArgs e)
		{
			Close();
		}

		private void loadwavFileToolStripMenuItem_Click(object sender, EventArgs e)
		{
			if (openFileDialog1.ShowDialog() == DialogResult.OK)
			{
				string wavFile = openFileDialog1.FileName;
				PaintWavFile(wavFile);
			}
		}

		private void toolStripButton1_Click(object sender, EventArgs e)
		{
			// Play the song
			secondarySoundBuffer.Play(0, PlayFlags.Looping);
		}

		private void toolStripButton2_Click(object sender, EventArgs e)
		{

		}

		private void toolStripButton3_Click(object sender, EventArgs e)
		{
			secondarySoundBuffer.Stop();
		}

		private void aboutToolStripMenuItem_Click(object sender, EventArgs e)
		{
			FormAbout about = new FormAbout();
			about.ShowDialog();
		}

		private void generateVibratoSoundToolStripMenuItem_Click(object sender, EventArgs e)
		{
			PaintSound();
		}
	}
}
