#include "mbed.h"
#include "SdUsbConnect.h"
#include "EasyPlayback.h"
#include "EasyDec_WavCnv2ch.h"

#define FILE_NAME_LEN          (64)
#define MOUNT_NAME             "storage"
#define TAG_BUFF_SIZE          (64 + 1) //null-terminated

static InterruptIn skip_btn(USER_BUTTON0);
static EasyPlayback AudioPlayer;

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
    AudioPlayer.outputVolume(0.5);  // Volume control (min:0.0 max:1.0)

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

                // playback
                AudioPlayer.play(file_path);
            }
        }
        closedir(d);
    }
}

