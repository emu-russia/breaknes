// Thanks to @zeta0134

public class SimpleIIR
{
	float prev_output = 0;
	float delta = 0;
	float alpha;

	public void Reconfigure (float SampleRate, float cutoff_freq)
	{
		delta = 1.0f / SampleRate;
		float time_const = (float)(1.0 / (2.0 * Math.PI * cutoff_freq));
		alpha = delta / (time_const + delta);
		Console.WriteLine("IIR Reconfigured delta: {0}, alpha: {0}", delta, alpha);
	}

	public void Input (float sample)
	{
		prev_output = Output();
		delta = sample - prev_output;
	}

	public float Output ()
	{
		return prev_output + alpha * delta;
	}
}
