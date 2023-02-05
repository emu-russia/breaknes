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

			PaintSound();
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

			int plot_samples_count = Math.Min(numberOfSamples, 4096);
			float[] plot_samples = new float[plot_samples_count];

			for (int i = 0; i < numberOfSamples; i++)
			{
				double vibrato = Math.Cos(2 * Math.PI * 10.0 * i / waveFormat.SampleRate);
				short value = (short)(Math.Cos(2 * Math.PI * (220.0 + 4.0 * vibrato) * i / waveFormat.SampleRate) * 16384); // Not too loud
				
				if (i < plot_samples_count)
				{
					plot_samples[i] = value;
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
			float[] L;
			float[] R;

			bool res = ReadWav(wav_name, out L, out R);
			if (!res)
			{
				MessageBox.Show("ReadWav cant");
				return;
			}

			// Create SecondarySoundBuffer
			var secondaryBufferDesc = new SoundBufferDescription();
			secondaryBufferDesc.BufferBytes = L.Length * 4;
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
			int numberOfSamples = L.Length;

			for (int i = 0; i < numberOfSamples; i++)
			{
				short value_l = (short)(L[i] * Int16.MaxValue);
				short value_r = (short)(L[i] * Int16.MaxValue);

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

		// https://stackoverflow.com/questions/8754111/how-to-read-the-data-in-a-wav-file-to-an-array

		bool ReadWav(string filename, out float[] L, out float[] R)
		{
			L = R = null;

			try
			{
				using (FileStream fs = File.Open(filename, FileMode.Open))
				{
					BinaryReader reader = new BinaryReader(fs);

					// chunk 0
					int chunkID = reader.ReadInt32();
					int fileSize = reader.ReadInt32();
					int riffType = reader.ReadInt32();


					// chunk 1
					int fmtID = reader.ReadInt32();
					int fmtSize = reader.ReadInt32(); // bytes for this chunk (expect 16 or 18)

					// 16 bytes coming...
					int fmtCode = reader.ReadInt16();
					int channels = reader.ReadInt16();
					int sampleRate = reader.ReadInt32();
					int byteRate = reader.ReadInt32();
					int fmtBlockAlign = reader.ReadInt16();
					int bitDepth = reader.ReadInt16();

					if (fmtSize == 18)
					{
						// Read any extra values
						int fmtExtraSize = reader.ReadInt16();
						reader.ReadBytes(fmtExtraSize);
					}

					// chunk 2
					int dataID = reader.ReadInt32();
					int bytes = reader.ReadInt32();

					// DATA!
					byte[] byteArray = reader.ReadBytes(bytes);

					int bytesForSamp = bitDepth / 8;
					int nValues = bytes / bytesForSamp;

					float[] asFloat = null;
					switch (bitDepth)
					{
						case 64:
							double[]
								asDouble = new double[nValues];
							Buffer.BlockCopy(byteArray, 0, asDouble, 0, bytes);
							asFloat = Array.ConvertAll(asDouble, e => (float)e);
							break;
						case 32:
							asFloat = new float[nValues];
							Buffer.BlockCopy(byteArray, 0, asFloat, 0, bytes);
							break;
						case 16:
							Int16[]
								asInt16 = new Int16[nValues];
							Buffer.BlockCopy(byteArray, 0, asInt16, 0, bytes);
							asFloat = Array.ConvertAll(asInt16, e => e / (float)(Int16.MaxValue + 1));
							break;
						default:
							return false;
					}

					switch (channels)
					{
						case 1:
							L = asFloat;
							R = null;
							return true;
						case 2:
							// de-interleave
							int nSamps = nValues / 2;
							L = new float[nSamps];
							R = new float[nSamps];
							for (int s = 0, v = 0; s < nSamps; s++)
							{
								L[s] = asFloat[v++];
								R[s] = asFloat[v++];
							}
							return true;
						default:
							return false;
					}
				}
			}
			catch
			{
				return false;
			}

			return false;
		}
	}
}
