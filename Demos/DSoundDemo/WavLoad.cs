// https://stackoverflow.com/questions/8754111/how-to-read-the-data-in-a-wav-file-to-an-array

namespace DSoundDemo
{
	class WavLoader
	{
		/// <summary>
		/// Load a .wav file. Convert all samples to normalized float.
		/// If the .wav is stereo, turn it into mono.
		/// </summary>
		/// <param name="filename">Name and path of the .wav file</param>
		/// <param name="smono">Normalized float output buffer</param>
		/// <returns></returns>
		static public bool Load (string filename, out float[] smono)
		{
			smono = Array.Empty<float>();

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

					float[] asFloat = Array.Empty<float>();
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
							smono = asFloat;
							return true;
						case 2:
							// de-interleave
							int nSamps = nValues / 2;
							smono = new float[nSamps];
							for (int s = 0, v = 0; s < nSamps; s++)
							{
								float left_chan = asFloat[v++];
								float right_chan = asFloat[v++];
								smono[s] = (left_chan + right_chan) / 2;
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
		}
	}
}
