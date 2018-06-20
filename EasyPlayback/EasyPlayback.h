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

#ifndef __EASY_PLAYBACK_H__
#define __EASY_PLAYBACK_H__

#include <string>
#include <map>
#include "EasyDecoder.h"
#include "AUDIO_GRBoard.h"
#include "FATFileSystem.h"

class EasyPlayback
{
public:
    typedef enum {
        AUDIO_TPYE_SSIF,
        AUDIO_TPYE_SPDIF
    } audio_type_t;

    EasyPlayback(audio_type_t type = AUDIO_TPYE_SSIF);
    ~EasyPlayback();
    bool get_tag(const char* filename, char* p_title, char* p_artist, char* p_album, uint16_t tag_size, FATFileSystem* pfs = NULL);
    bool play(const char* filename, FATFileSystem* pfs = NULL);
    bool is_paused(void);
    void pause(void);
    void pause(bool type);
    void skip(void);
    bool outputVolume(float VolumeOut);

    template<typename T>
    void add_decoder(const string& extension) {
        m_lpDecoders[extension] = &T::inst;
    }

private:
    #define AUDIO_WRITE_BUFF_NUM   (8)
    #define AUDIO_MSK_RING_BUFF    (AUDIO_WRITE_BUFF_NUM - 1)
    AUDIO_GRBoard * _audio_ssif;
    AUDIO_RBSP * _audio;
    int _buff_index;
    audio_type_t _type;
    bool _skip;
    bool _pause;
    bool _init_end;
    uint32_t _audio_buff_size;
    uint8_t *_heap_buf;
    uint8_t *_audio_buf;
    std::map<std::string, EasyDecoder*(*)()> m_lpDecoders;

    EasyDecoder * create_decoer_class(const char* filename);
};

#endif
