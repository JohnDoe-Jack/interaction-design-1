#include "sound.hpp"
#include <AudioFileSourcePROGMEM.h>
#include <AudioGeneratorWAV.h>
#include <AudioOutputI2S.h>

AudioFileSourcePROGMEM *file = nullptr;
AudioGeneratorWAV *wav = nullptr;
AudioOutputI2S *out = nullptr;

bool initSound() {
    file = new AudioFileSourcePROGMEM(sound, sound_len);
    out = new AudioOutputI2S();
    wav = new AudioGeneratorWAV();
    return wav->begin(file, out);
}

void playSound() {
    if (wav->isRunning()) {
        wav->loop();
    }
}

void loopSound() {
    if (wav->isRunning()) {
        wav->loop();
    }
}