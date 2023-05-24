using SharpTools;
using System.IO;
using System.Reflection.Metadata;
using System.Windows.Forms;

// Since the simulation is now very slow, the following strategy is used to reproduce the sound:
// Collect one second of sound. As it is recorded, play it back via DirectSound and clear the receive buffer.

namespace Breaknes
{
	public class AudioRender
	{
		private DSound? audio_backend;
		private int OutputSampleRate = 48000;
		private float OutputDC = 0.0f;
		private List<float> SampleBuf = new();
		public bool Dma = false;       // atomic

		private BreaksCore.AudioSignalFeatures aux_features;
		private int DecimateEach = 1;
		private int DecimateCounter = 0;

		private bool dump_audio = false;
		private string dump_audio_dir = "";
		private string dump_rom_name;

		private SimpleIIR filter = new SimpleIIR();
		private bool iir = true;

		public AudioRender(System.IntPtr handle, bool dump, string dump_dir, string rom_name, bool use_iir, int cutoff_freq)
		{
			dump_audio = dump;
			dump_audio_dir = dump_dir;
			dump_rom_name = rom_name;

			audio_backend = new DSound(handle);

			BreaksCore.GetApuSignalFeatures(out aux_features);

			iir = use_iir;
			if (iir)
			{
				filter.Reconfigure(aux_features.SampleRate, cutoff_freq);
			}

			// SRC
			Redecimate();
		}

		/// <summary>
		/// This sampler is used for SRC.
		/// The AUX output is sampled at a high frequency, which cannot be played by a ordinary sound card.
		/// Therefore, some of the samples are skipped to match the DSound playback frequency (SourceSampleRate variable).
		/// </summary>
		public void FeedSample()
		{
			if (iir)
			{
				float sample;
				BreaksCore.SampleAudioSignal(out sample);
				filter.Input(sample);
			}

			if (DecimateCounter >= DecimateEach)
			{
				float sample;
				if (iir)
				{
					sample = filter.Output();
				}
				else
				{
					BreaksCore.SampleAudioSignal(out sample);
				}
				SampleBuf.Add(sample);
				DecimateCounter = 0;

				if (SampleBuf.Count >= OutputSampleRate)
				{
					Playback();
				}
			}
			else
			{
				DecimateCounter++;
			}
		}

		/// <summary>
		/// If you change the sampling frequency of the sound source or output frequency, you must recalculate the decimation factor.
		/// </summary>
		private void Redecimate()
		{
			BreaksCore.GetApuSignalFeatures(out aux_features);
			DecimateEach = aux_features.SampleRate / OutputSampleRate;
			Console.WriteLine("APUSim sample rate: {0}, DSound sample rate: {1}, decimate factor: {2}", aux_features.SampleRate, OutputSampleRate, DecimateEach);
			DecimateCounter = 0;
		}

		private void Playback()
		{
			if (audio_backend != null)
			{
				Console.WriteLine("Play 1 second");
				Dma = true;
				DumpAudio();
				audio_backend.PlaySampleBuf(OutputSampleRate, SampleBuf, OutputDC);
				SampleBuf.Clear();
				Dma = false;
			}
		}

		private void DumpAudio()
		{
			if (dump_audio)
			{
				int numberOfSamples = SampleBuf.Count;
				byte[] bytes = new byte[numberOfSamples * 2];

				for (int i = 0; i < numberOfSamples; i++)
				{
					short value = (short)((SampleBuf[i]) * Int16.MaxValue);

					bytes[2 * i] = (byte)((value >> 0) & 0xff);
					bytes[2 * i + 1] = (byte)((value >> 8) & 0xff);
				}

				using (var stream = new FileStream(dump_audio_dir + "/" + dump_rom_name + "_aux.bin", FileMode.Append))
				{
					stream.Write(bytes, 0, bytes.Length);
				}
			}
		}
	}
}
