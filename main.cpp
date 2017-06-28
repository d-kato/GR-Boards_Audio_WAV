
#include "mbed.h"
#include "SdUsbConnect.h"
#include "Wavp.h"

#define FILE_NAME_LEN          (64)
#define MOUNT_NAME             "storage"

static InterruptIn skip_btn(USER_BUTTON0);
static Wavp WavPlayer;

static void skip_btn_fall(void) {
    WavPlayer.skip();
}

int main() {
    DIR  * d;
    struct dirent * p;
    char file_path[sizeof("/"MOUNT_NAME"/") + FILE_NAME_LEN];
    SdUsbConnect storage(MOUNT_NAME);

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
                sprintf(file_path, "/%s/%s", MOUNT_NAME, p->d_name);
                printf("%s\r\n", file_path);
                WavPlayer.play(file_path);
            }
        }
        closedir(d);
    }
}

