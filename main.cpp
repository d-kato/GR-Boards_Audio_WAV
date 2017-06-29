
#include "mbed.h"
#include "SdUsbConnect.h"
#include "EasyPlayback.h"
#include "EasyDec_WavCnv2ch.h"

#define FILE_NAME_LEN          (64)
#define MOUNT_NAME             "storage"
#define TAG_BUFF_SIZE          (64 + 1) //null-terminated

static InterruptIn skip_btn(USER_BUTTON0);
static EasyPlayback AudioPlayer;

static char title_tag[TAG_BUFF_SIZE];
static char artist_tag[TAG_BUFF_SIZE];
static char album_tag[TAG_BUFF_SIZE];

static void skip_btn_fall(void) {
    AudioPlayer.skip();
}

int main() {
    DIR  * d;
    struct dirent * p;
    char file_path[sizeof("/"MOUNT_NAME"/") + FILE_NAME_LEN];
    SdUsbConnect storage(MOUNT_NAME);

    // decoder setting
    AudioPlayer.add_decoder<EasyDec_WavCnv2ch>(".wav");
    AudioPlayer.add_decoder<EasyDec_WavCnv2ch>(".WAV");

    // volume control
    AudioPlayer.outputVolume(0.7, 0.7);

    // button setting
    skip_btn.fall(&skip_btn_fall);

    // wait for the storage device to be connected
    printf("Finding a storage...\r\n");
    storage.wait_connect();
    printf("done\r\n");

    while(1) {
        // file search
        d = opendir("/"MOUNT_NAME"/");
        while ((p = readdir(d)) != NULL) {
            size_t len = strlen(p->d_name);
            if (len < FILE_NAME_LEN) {
                // make file path
                sprintf(file_path, "/%s/%s", MOUNT_NAME, p->d_name);
                printf("%s\r\n", file_path);

                // get tag info
                if (AudioPlayer.get_tag(file_path, title_tag, artist_tag, album_tag, TAG_BUFF_SIZE)) {
                    printf("Title :%s\r\n", title_tag);
                    printf("Artist:%s\r\n", artist_tag);
                    printf("Album :%s\r\n", album_tag);
                    printf("\r\n");
                }

                // playback
                AudioPlayer.play(file_path);
            }
        }
        closedir(d);
    }
}

