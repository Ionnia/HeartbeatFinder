#ifndef WAV_FILE_H
#define WAV_FILE_H

#include <QString>
#include <SDL2/SDL_audio.h>

struct WavFile {
    SDL_AudioSpec wav_spec;
    uint32_t wav_len;
    uint8_t *wav_buf;
    int16_t *wav_buf_16bit;
    bool isEmpty = true;

    WavFile(){}

    ~WavFile(){
        SDL_FreeWAV(wav_buf);
    }

    bool openWAV(QString fileName){
        // Открываем WAV файл
        if(SDL_LoadWAV(fileName.toStdString().c_str(), &wav_spec, &wav_buf, &wav_len) == nullptr){
            return false;
        } else {
            wav_buf_16bit = (int16_t*)wav_buf;
            isEmpty = false;
            return true;
        }
    }

    void free(){
        SDL_FreeWAV(wav_buf);
        isEmpty = true;
    }
};

#endif // WAV_FILE_H
