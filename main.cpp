
#include "mbed.h"
#include "AUDIO_GRBoard.h"
#include "dec_wav.h"
#include "SdUsbConnect.h"

#define AUDIO_WRITE_BUFF_SIZE  (4096)
#define AUDIO_WRITE_BUFF_NUM   (9)
#define FILE_NAME_LEN          (64)
#define TEXT_SIZE              (64 + 1) //null-terminated
#define MOUNT_NAME             "storage"

//32 bytes aligned! No cache memory
#if defined(__ICCARM__)
#pragma data_alignment=32
static uint8_t audio_write_buff[AUDIO_WRITE_BUFF_NUM][AUDIO_WRITE_BUFF_SIZE]@ ".mirrorram";
#else
static uint8_t audio_write_buff[AUDIO_WRITE_BUFF_NUM][AUDIO_WRITE_BUFF_SIZE]
                __attribute((section("NC_BSS"),aligned(32)));
#endif
//Tag buffer
static uint8_t title_buf[TEXT_SIZE];
static uint8_t artist_buf[TEXT_SIZE];
static uint8_t album_buf[TEXT_SIZE];
static bool    file_skip = false;
static InterruptIn  button(USER_BUTTON0);

AUDIO_GRBoard audio(0x80, (AUDIO_WRITE_BUFF_NUM - 1), 0);

static void callback_audio_write_end(void * p_data, int32_t result, void * p_app_data) {
    if (result < 0) {
        printf("audio write callback error %ld\r\n", result);
    }
}

static void button_fall(void) {
    file_skip = true;
}

int main() {
    rbsp_data_conf_t audio_write_async_ctl = {&callback_audio_write_end, NULL};
    FILE * fp = NULL;
    DIR  * d = NULL;
    char file_path[sizeof("/"MOUNT_NAME"/") + FILE_NAME_LEN];
    int buff_index = 0;
    size_t audio_data_size;
    dec_wav wav_file;

    button.fall(&button_fall);
    SdUsbConnect storage(MOUNT_NAME);
    audio.power();
    audio.outputVolume(1.0, 1.0);

    while(1) {
        // wait for the storage device to be connected
        storage.wait_connect();

        // in a loop, append a file
        while(1) {
            // if the device is disconnected, we try to connect it again
            if (!storage.connected()) {
                break;
            }
            if (fp == NULL) {
                // file search
                if (d == NULL) {
                    d = opendir("/"MOUNT_NAME"/");
                }
                struct dirent * p;
                while ((p = readdir(d)) != NULL) {
                    size_t len = strlen(p->d_name);
                    if ((len > 4) && (len < FILE_NAME_LEN)
                        && (strncasecmp(&p->d_name[len - 4], ".wav", 4) == 0)) {
                        strcpy(file_path, "/"MOUNT_NAME"/");
                        strcat(file_path, p->d_name);
                        fp = fopen(file_path, "r");
                        if (wav_file.AnalyzeHeder(title_buf, artist_buf, album_buf,
                                                   TEXT_SIZE, fp) == false) {
                            fclose(fp);
                            fp = NULL;
                        } else if ((wav_file.GetChannel() != 2)
                                || (audio.format(wav_file.GetBlockSize()) == false)
                                || (audio.frequency(wav_file.GetSamplingRate()) == false)) {
                            printf("Error File  :%s\r\n", p->d_name);
                            printf("Audio Info  :%dch, %dbit, %dHz\r\n", wav_file.GetChannel(),
                                    wav_file.GetBlockSize(), (int)wav_file.GetSamplingRate());
                            printf("\r\n");
                            fclose(fp);
                            fp = NULL;
                        } else {
                            printf("File        :%s\r\n", p->d_name);
                            printf("Audio Info  :%dch, %dbit, %dHz\r\n", wav_file.GetChannel(),
                                    wav_file.GetBlockSize(), (int)wav_file.GetSamplingRate());
                            printf("Title       :%s\r\n", title_buf);
                            printf("Artist      :%s\r\n", artist_buf);
                            printf("Album       :%s\r\n", album_buf);
                            printf("\r\n");
                            break;
                        }
                    }
                }
                if (p == NULL) {
                    closedir(d);
                    d = NULL;
                }
            } else {
                // file read
                uint8_t * p_buf = audio_write_buff[buff_index];

                audio_data_size = wav_file.GetNextData(p_buf, AUDIO_WRITE_BUFF_SIZE);
                if (audio_data_size > 0) {
                    audio.write(p_buf, audio_data_size, &audio_write_async_ctl);
                    buff_index++;
                    if (buff_index >= AUDIO_WRITE_BUFF_NUM) {
                        buff_index = 0;
                    }
                }

                // file close
                if ((audio_data_size < AUDIO_WRITE_BUFF_SIZE) || (file_skip == true)) {
                    file_skip = false;
                    fclose(fp);
                    fp = NULL;
                    Thread::wait(500);
                }
            }
        }
        // close check
        if (fp != NULL) {
            fclose(fp);
            fp = NULL;
        }
        if (d != NULL) {
            closedir(d);
            d = NULL;
        }
    }
}

