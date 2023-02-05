// Sample rate converter

namespace DSoundDemo
{
	class SRC
	{
		static public void SampleRateConv (List<float> src_samples, int src_sample_rate, List<float> dst_samples, int dst_sample_rate)
		{
			// TODO

			dst_samples.Clear();

			foreach (float src_sample in src_samples)
			{
				dst_samples.Add(src_sample);
			}
		}
	}
}
