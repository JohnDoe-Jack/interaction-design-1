# wav_to_c_array.py
import sys

def wav_to_c_array(wav_path, out_path="sound.h"):
    with open(wav_path, "rb") as f:
        data = f.read()

    with open(out_path, "w") as f:
        f.write('#pragma once\n#include <pgmspace.h>\n\n')
        f.write('const unsigned char sound[] PROGMEM = {\n')
        for i in range(0, len(data), 12):
            line = ', '.join(f'0x{b:02x}' for b in data[i:i+12])
            f.write(f'  {line},\n')
        f.write('};\n')
        f.write(f'const unsigned int sound_len = {len(data)};\n')

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python wav_to_c_array.py input.wav")
        sys.exit(1)
    wav_to_c_array(sys.argv[1])
