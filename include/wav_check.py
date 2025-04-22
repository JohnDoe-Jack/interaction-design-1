import wave
import sys

filename = sys.argv[1]
with wave.open(filename, 'rb') as wav:
    print("Channels:", wav.getnchannels())
    print("Sample width:", wav.getsampwidth(), "bytes")
    print("Frame rate (sample rate):", wav.getframerate())
    print("Number of frames:", wav.getnframes())
