// DirectSound backend

using SharpDX;
using SharpDX.DirectSound;
using SharpDX.Multimedia;
using System.Reflection.Metadata;

// https://github.com/sharpdx/SharpDX-Samples/blob/master/Desktop/DirectSound/PlaySound/Program.cs

namespace SharpTools
{
	public class DSound
	{
		DirectSound? directSound;
		PrimarySoundBuffer? primarySoundBuffer;
		SecondarySoundBuffer? secondarySoundBuffer;
		WaveFormat? waveFormat;

		int DestSampleRate = 48000;

		public DSound(IntPtr Handle)
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

		public void PlaySampleBuf(int SourceSampleRate, List<float> SampleBuf, float DC)
		{
			if (SampleBuf.Count == 0)
				return;

			// Putting resampling on the shoulders of DSound
			
			DestSampleRate = SourceSampleRate;

			// WaveFormat Mono, DestSampleRate Hz, 16 bit
			waveFormat = new WaveFormat(DestSampleRate, 16, 1);

			// Create SecondarySoundBuffer
			var secondaryBufferDesc = new SoundBufferDescription();
			secondaryBufferDesc.BufferBytes = SampleBuf.Count * 2;
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
			int numberOfSamples = SampleBuf.Count;

			for (int i = 0; i < numberOfSamples; i++)
			{
				short value = (short)((SampleBuf[i] + DC) * Int16.MaxValue);
				dataPart1.Write(value);
			}

			// Unlock the buffer
			secondarySoundBuffer.Unlock(dataPart1, dataPart2);

			// Play the song
			secondarySoundBuffer.Play(0, PlayFlags.None);
		}

		public void StopSampleBuf()
		{
			if (secondarySoundBuffer != null)
				secondarySoundBuffer.Stop();
		}
	}
}
