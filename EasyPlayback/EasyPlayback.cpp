/* mbed EasyPlayback Library
 * Copyright (C) 2017 dkato
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mbed.h"
#include "dcache-control.h"
#include "EasyPlayback.h"

EasyPlayback::EasyPlayback(audio_type_t type) : 
     _buff_index(0), _type(type), _skip(false), _pause(false), _init_end(false)
{
    _audio_ssif = NULL;
    if (_type == AUDIO_TPYE_SSIF) {
        _audio_buff_size = 4096;
        _audio_ssif = new AUDIO_GRBoard(0x80, (AUDIO_WRITE_BUFF_NUM - 1), 0);
        _audio = _audio_ssif;
    } else if (_type == AUDIO_TPYE_SPDIF) {
        MBED_ASSERT(false);
    } else {
        MBED_ASSERT(false);
    }
    _heap_buf = new uint8_t[_audio_buff_size * AUDIO_WRITE_BUFF_NUM + 31];
    _audio_buf = (uint8_t *)(((uint32_t)_heap_buf + 31ul) & ~31ul);
}

EasyPlayback::~EasyPlayback()
{
    if (_audio_ssif != NULL) {
        delete _audio_ssif;
    }
#if (R_BSP_SPDIF_ENABLE == 1)
    if (_audio_spdif != NULL) {
        delete _audio_spdif;
    }
#endif
    delete [] _heap_buf;
}

bool EasyPlayback::get_tag(const char* filename, char* p_title, char* p_artist, char* p_album, uint16_t tag_size)
{
    FILE * fp = NULL;
    EasyDecoder * decoder;
    bool ret = false;

    decoder = create_decoer_class(filename);
    if (decoder == NULL) {
        return false;
    }

    fp = fopen(filename, "r");
    if (fp == NULL) {
        // do nothing
    } else if (decoder->AnalyzeHeder(p_title, p_artist, p_album, tag_size, fp) != false) {
        ret = true;
    }
    delete decoder;
    if (fp != NULL) {
        fclose(fp);
    }

    return ret;
}

bool EasyPlayback::play(const char* filename)
{
    const rbsp_data_conf_t audio_write_async_ctl = {NULL, NULL};
    size_t audio_data_size;
    FILE * fp = NULL;
    uint8_t * p_buf;
    uint32_t read_size;
    uint32_t i;
    EasyDecoder * decoder;
    bool ret = false;
    uint32_t padding_size;

    decoder = create_decoer_class(filename);
    if (decoder == NULL) {
        return false;
    }

    if (!_init_end) {
        _audio->power();
        _audio->outputVolume(1.0, 1.0);
        _init_end = true;
    }

     _skip = false;
    fp = fopen(filename, "r");
    if (fp == NULL) {
        // do nothing
    } else if (decoder->AnalyzeHeder(NULL, NULL, NULL, 0, fp) == false) {
        // do nothing
    } else if  ((decoder->GetChannel() != 2)
            || (_audio->format(decoder->GetBlockSize()) == false)
            || (_audio->frequency(decoder->GetSamplingRate()) == false)) {
        // do nothing
    } else {
        if ((_type == AUDIO_TPYE_SPDIF) && (decoder->GetBlockSize() == 16)) {
            padding_size = 2;
            read_size = _audio_buff_size / 2;
        } else if ((decoder->GetBlockSize() == 20) || (decoder->GetBlockSize() == 24)) {
            padding_size = 1;
            read_size = _audio_buff_size * 3 / 4;
        } else {
            padding_size = 0;
            read_size = _audio_buff_size;
        }
        audio_data_size = read_size;
        setvbuf(fp, NULL, _IONBF, 0); // unbuffered

        while (audio_data_size == read_size) {
            while ((_pause) && (!_skip)) {
                Thread::wait(100);
            }
            if (_skip) {
                break;
            }
            p_buf = &_audio_buf[_audio_buff_size * _buff_index];
            audio_data_size = decoder->GetNextData(p_buf, read_size);
            if (audio_data_size > 0) {
                // fill the shortfall with 0
                for (i = audio_data_size; i < read_size; i++) {
                    p_buf[i] = 0;
                }

                if (padding_size != 0) {
                    int idx_w = _audio_buff_size - 1;
                    int idx_r = read_size - 1;
                    uint32_t block_byte = (decoder->GetBlockSize() + 7) / 8;

                    while (idx_w >= 0) {
                        // padding
                        for (i = 0; i < padding_size; i++) {
                            p_buf[idx_w--] = 0x00;
                        }
                        for (i = 0; i < block_byte; i++) {
                            p_buf[idx_w--] = p_buf[idx_r--];
                        }
                    }
                }
                dcache_clean(p_buf, _audio_buff_size);
                _audio->write(p_buf, _audio_buff_size, &audio_write_async_ctl);
                _buff_index = (_buff_index + 1) & AUDIO_MSK_RING_BUFF;
            }
        }
        Thread::wait(500);
        ret = true;
    }
    delete decoder;
    if (fp != NULL) {
        fclose(fp);
    }

    return ret;
}

bool EasyPlayback::is_paused(void)
{
    return _pause;
}

void EasyPlayback::pause()
{
    _pause = !_pause;
}

void EasyPlayback::pause(bool type)
{
    _pause = type;
}

void EasyPlayback::skip(void)
{
    _skip = true;
}

bool EasyPlayback::outputVolume(float VolumeOut)
{
    if (!_init_end) {
        _audio->power();
        _init_end = true;
    }
    return _audio->outputVolume(VolumeOut, VolumeOut);
}

EasyDecoder * EasyPlayback::create_decoer_class(const char* filename)
{
    std::map<std::string, EasyDecoder*(*)()>::iterator itr;
    char *extension = strstr(filename, ".");

    if (extension == NULL) {
        return NULL;
    }

    itr = m_lpDecoders.find(extension);
    if (itr == m_lpDecoders.end()) {
        return NULL;
    }

    return (*itr).second();
}

