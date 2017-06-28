#ifndef __WAVP_H__
#define __WAVP_H__

#include "AUDIO_GRBoard.h"
#include "dec_wav_cnv2ch.h"

class Wavp
{
public:
    Wavp();
    ~Wavp();
    bool play(const char* filename);
    bool is_paused(void);
    void pause(void);
    void pause(bool type);
    void skip(void);
    bool outputVolume(float leftVolumeOut, float rightVolumeOut);

private:
    #define AUDIO_WRITE_BUFF_SIZE  (4096)
    #define AUDIO_WRITE_BUFF_NUM   (8)
    #define AUDIO_MSK_RING_BUFF    (AUDIO_WRITE_BUFF_NUM - 1)
    AUDIO_GRBoard audio;
    int _buff_index;
    bool _skip;
    bool _pause;
    bool _init_end;
    dec_wav_cnv2ch wav_file;
    uint8_t *heep_buf;
    uint8_t (*audio_buf)[AUDIO_WRITE_BUFF_SIZE];
};

#endif
