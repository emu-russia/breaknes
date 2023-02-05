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
		DirectSound? directSound;
		PrimarySoundBuffer? primarySoundBuffer;
		SecondarySoundBuffer? secondarySoundBuffer;
		WaveFormat? waveFormat;

		float[] SourceSamples = Array.Empty<float>();
		long SourceSamplesPtr = 0;
		int SourceSampleRate;
		int DestSampleRate = 48000;

		List<float> SampleBuf = new();

		float FurryIntensity = 4.0f;

		bool Paused = true;			// atomic
		long timeStamp;
		int workerDelayCounter = 0;
		int workerDelay = 10000;
		bool Plotting = false;		// atomic

		public FormMain()
		{
			InitializeComponent();
		}

		private void FormMain_Load(object sender, EventArgs e)
		{
			InitDSound();
			PauseWorker(true);
			backgroundWorker1.RunWorkerAsync();
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
		}

		private void PaintSound()
		{
			int numberOfSamples = 100000;
			SourceSampleRate = DestSampleRate;
			SourceSamples = new float[numberOfSamples];

			for (int i = 0; i < numberOfSamples; i++)
			{
				int si = 10000 + i;
				double vibrato = Math.Cos(2 * Math.PI * 10.0 * si / SourceSampleRate);
				float value = (float)(Math.Cos(2 * Math.PI * (220.0 + 4.0 * vibrato) * si / SourceSampleRate) );
				SourceSamples[i] = value;
			}

			SourceSamplesPtr = 0;
			PauseWorker(false);
		}

		private void PaintWavFile (string wav_name)
		{
			float[] smono;

			bool res = WavLoader.Load(wav_name, out smono, out SourceSampleRate);
			if (!res)
			{
				MessageBox.Show("ReadWav cant");
				return;
			}

			int numberOfSamples = smono.Length;
			SourceSamples = new float[numberOfSamples];

			for (int i = 0; i < numberOfSamples; i++)
			{
				SourceSamples[i] = smono[i];
			}

			SourceSamplesPtr = 0;
			PauseWorker(false);
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
			PlaySampleBuf();
		}

		private void toolStripButton2_Click(object sender, EventArgs e)
		{
			StopSampleBuf();
			Plotting = true;
			SampleBuf.Clear();
			Plotting = false;
			UpdateSampleBufStats();
			UpdateSignalPlot();
		}

		private void toolStripButton3_Click(object sender, EventArgs e)
		{
			StopSampleBuf();
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

		private void backgroundWorker1_DoWork(object sender, System.ComponentModel.DoWorkEventArgs e)
		{
			while (!backgroundWorker1.CancellationPending)
			{
				if (Paused || Plotting)
				{
					Thread.Sleep(10);
					continue;
				}

				if (SourceSamplesPtr >= SourceSamples.Length)
				{
					PauseWorker(true);
					continue;
				}

				// Delay

				workerDelayCounter++;
				if (workerDelayCounter >= workerDelay)
				{
					workerDelayCounter = 0;
				}
				else continue;

				// Advance

				float next_sample = SourceSamples[SourceSamplesPtr];
				SourceSamplesPtr++;
				SampleBuf.Add(next_sample);
				furryPlot1.AddSample(next_sample * FurryIntensity);

				// Show statistics that are updated once every 1 second.

				long now = DateTime.Now.Ticks / TimeSpan.TicksPerMillisecond;
				if (now > (timeStamp + 1000))
				{
					UpdateSampleBufStats();
					UpdateSignalPlot();
					timeStamp = now;
				}
			}

			Console.WriteLine("Background Worker canceled.");
		}

		private void UpdateSampleBufStats()
		{
			toolStripStatusLabel2.Text = SampleBuf.Count.ToString();
			long ms = SourceSampleRate != 0 ? (SampleBuf.Count * 1000) / (long)SourceSampleRate : 0;
			toolStripStatusLabel4.Text = ms.ToString() + " ms";
			long hz = Paused ? 0 : (long)SourceSampleRate;
			toolStripStatusLabel8.Text = hz.ToString() + " Hz";
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

		private void PlaySampleBuf()
		{
			if (SampleBuf.Count == 0)
				return;

			// SRC

			// HACK: Putting resampling on the shoulders of DSound
			DestSampleRate = SourceSampleRate;

			List<float> SampleBufPrepared = new();
			Plotting = true;
			SRC.SampleRateConv(SampleBuf, SourceSampleRate, SampleBufPrepared, DestSampleRate);
			Plotting = false;

			// WaveFormat Mono, DestSampleRate Hz, 16 bit
			waveFormat = new WaveFormat(DestSampleRate, 16, 1);

			// Create SecondarySoundBuffer
			var secondaryBufferDesc = new SoundBufferDescription();
			secondaryBufferDesc.BufferBytes = SampleBufPrepared.Count * 2;
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
			int numberOfSamples = SampleBufPrepared.Count;

			for (int i = 0; i < numberOfSamples; i++)
			{
				short value = (short)(SampleBufPrepared[i] * Int16.MaxValue);
				dataPart1.Write(value);
			}

			// Unlock the buffer
			secondarySoundBuffer.Unlock(dataPart1, dataPart2);

			// Play the song
			secondarySoundBuffer.Play(0, PlayFlags.None);
		}

		private void StopSampleBuf()
		{
			if (secondarySoundBuffer != null)
				secondarySoundBuffer.Stop();
		}

		private void PauseWorker (bool pause)
		{
			Paused = pause;
			toolStripStatusLabel6.Text = pause ? "Paused" : "Running";
		}
	}
}
