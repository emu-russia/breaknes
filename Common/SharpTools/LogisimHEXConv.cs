﻿// Converts Logisim .hex dump to byte/UInt16/UInt32/float array.

using System.Text.RegularExpressions;

namespace SharpTools
{
	public class LogisimHEXConv
	{
		public static byte[] HEXToByteArray(string text)
		{
			List<byte> res = new List<byte>();

			Regex regex = new Regex(@"^v2.0 raw[\r\n]+([\da-fA-F\*]+\s*)*");

			var matches = regex.Matches(text);

			foreach (Match match in matches)
			{
				foreach (var cap in match.Groups[1].Captures)
				{
					string entry = cap.ToString();

					if (entry.Contains('*'))
					{
						string[] ab = entry.Trim().Split('*');
						int counter = Convert.ToInt32(ab[0], 10);
						byte val = Convert.ToByte(ab[1], 16);
						for (int i = 0; i < counter; i++)
						{
							res.Add(val);
						}
					}
					else
					{
						byte val = Convert.ToByte(entry.Trim(), 16);
						res.Add(val);
					}
				}
			}

			return res.ToArray();
		}

		public static UInt16 [] HEXToShortArray(string text)
		{
			List<UInt16> res = new List<UInt16>();

			Regex regex = new Regex(@"^v2.0 raw[\r\n]+([\da-fA-F\*]+\s*)*");

			var matches = regex.Matches(text);

			foreach (Match match in matches)
			{
				foreach (var cap in match.Groups[1].Captures)
				{
					string entry = cap.ToString();

					if (entry.Contains('*'))
					{
						string[] ab = entry.Trim().Split('*');
						int counter = Convert.ToInt32(ab[0], 10);
						UInt16 val = Convert.ToUInt16(ab[1], 16);
						for (int i=0; i<counter; i++)
						{
							res.Add(val);
						}
					}
					else
					{
						UInt16 val = Convert.ToUInt16(entry.Trim(), 16);
						res.Add(val);
					}
				}
			}

			return res.ToArray();
		}

		public static UInt32[] HEXToUIntArray(string text)
		{
			List<UInt32> res = new List<UInt32>();

			Regex regex = new Regex(@"^v2.0 raw[\r\n]+([\da-fA-F\*]+\s*)*");

			var matches = regex.Matches(text);

			foreach (Match match in matches)
			{
				foreach (var cap in match.Groups[1].Captures)
				{
					string entry = cap.ToString();

					if (entry.Contains('*'))
					{
						string[] ab = entry.Trim().Split('*');
						int counter = Convert.ToInt32(ab[0], 10);
						UInt32 val = Convert.ToUInt32(ab[1], 16);
						for (int i = 0; i < counter; i++)
						{
							res.Add(val);
						}
					}
					else
					{
						UInt32 val = Convert.ToUInt32(entry.Trim(), 16);
						res.Add(val);
					}
				}
			}

			return res.ToArray();
		}

		public static float[] HEXToFloatArray(string text)
		{
			List<float> res = new List<float>();

			Regex regex = new Regex(@"^v2.0 raw[\r\n]+([\da-fA-F\*]+\s*)*");

			var matches = regex.Matches(text);

			foreach (Match match in matches)
			{
				foreach (var cap in match.Groups[1].Captures)
				{
					string entry = cap.ToString();

					if (entry.Contains('*'))
					{
						string[] ab = entry.Trim().Split('*');
						int counter = Convert.ToInt32(ab[0], 10);
						UInt32 val = Convert.ToUInt32(ab[1], 16);
						byte[] b = BitConverter.GetBytes(val);
						for (int i = 0; i < counter; i++)
						{
							res.Add(BitConverter.ToSingle(b, 0));
						}
					}
					else
					{
						UInt32 val = Convert.ToUInt32(entry.Trim(), 16);
						byte[] b = BitConverter.GetBytes(val);
						res.Add(BitConverter.ToSingle(b, 0));
					}
				}
			}

			return res.ToArray();
		}
	}
}
