# Slooow Plaaayer

![SlooowPlaaayer](/UserManual/imgstore/SlooowPlaaayer.png)

Utility for playing audio samples that were obtained from a slow source (such as APUSim).

Sample sources:
- Debug vibrato sound (Debug menu)
- Data from a .wav file

## Principle of operation

The Background Worker runs in the background and collects samples from the source (`SourceSamples`) into an intermediate buffer (`SampleBuf`) with a slight delay.

Once every (real) second, statistics are updated and a signal graph is plotted.

If the source samples run out, the worker falls asleep until it gets a new source.

## TODO

Currently only 44100 Hz for audio sources is supported. You need to implement the SRC.
