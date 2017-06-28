
#include "mbed.h"
#include "dcache-control.h"
#include "Wavp.h"

Wavp::Wavp() : audio(0x80, (AUDIO_WRITE_BUFF_NUM - 1), 0), _buff_index(0),
    _skip(false), _pause(false), _init_end(false)
{
    heep_buf = new uint8_t[AUDIO_WRITE_BUFF_SIZE * AUDIO_WRITE_BUFF_NUM + 31];
    audio_buf = (uint8_t (*)[AUDIO_WRITE_BUFF_SIZE])(((uint32_t)heep_buf + 31ul) & ~31ul);
}

Wavp::~Wavp()
{
    delete [] heep_buf;
}

bool Wavp::play(const char* filename)
{
    const rbsp_data_conf_t audio_write_async_ctl = {NULL, NULL};
    size_t audio_data_size = AUDIO_WRITE_BUFF_SIZE;
    FILE * fp;
    uint8_t * p_buf;
    size_t len = strlen(filename);

    if ((len <= 4) || (strncasecmp(&filename[len - 4], ".wav", 4) != 0)) {
        return false;
    }

    if (!_init_end) {
        audio.power();
        audio.outputVolume(1.0, 1.0);
        _init_end = true;
    }

     _skip = false;
    fp = fopen(filename, "r");
    if (wav_file.AnalyzeHeder(NULL, NULL, NULL, 0, fp) == false) {
        // do nothing
    } else if  ((wav_file.GetChannel() != 2)
            || (audio.format(wav_file.GetBlockSize()) == false)
            || (audio.frequency(wav_file.GetSamplingRate()) == false)) {
        // do nothing
    } else {
        while (audio_data_size == AUDIO_WRITE_BUFF_SIZE) {
            while ((_pause) && (!_skip)) {
                Thread::wait(100);
            }
            if (_skip) {
                break;
            }
            p_buf = audio_buf[_buff_index];
            audio_data_size = wav_file.GetNextData(p_buf, AUDIO_WRITE_BUFF_SIZE);
            if (audio_data_size > 0) {
                dcache_clean(p_buf, audio_data_size);
                audio.write(p_buf, audio_data_size, &audio_write_async_ctl);
                _buff_index = (_buff_index + 1) & AUDIO_MSK_RING_BUFF;
            }
        }
        Thread::wait(500);
    }
    fclose(fp);

    return true;
}

bool Wavp::is_paused(void)
{
    return _pause;
}

void Wavp::pause()
{
    _pause = !_pause;
}

void Wavp::pause(bool type)
{
    _pause = type;
}

void Wavp::skip(void)
{
    _skip = true;
}

bool Wavp::outputVolume(float leftVolumeOut, float rightVolumeOut)
{
    if (!_init_end) {
        audio.power();
        _init_end = true;
    }
    return audio.outputVolume(leftVolumeOut, rightVolumeOut);
}

