# GR-Boards_Audio_WAV
GR-PEACH、および、GR-LYCHEEで動作するサンプルプログラムです。  
GR-LYCHEEの開発環境については、[GR-LYCHEE用オフライン開発環境の手順](https://developer.mbed.org/users/dkato/notebook/offline-development-lychee-langja/)を参照ください。


## 概要
USB、または、SDのルートフォルダにあるWAVファイル(.wav)を再生するサンプルです。  
USER_BUTTON0を押すと次の曲を再生します。


### 補足：キャッシュメモリについて
Audioデータの転送にはDMAを使用するため、Audio用バッファは非キャッシュに配置しています。  
キャッシュ制御についての詳細は[こちら](https://github.com/d-kato/mbed-gr-libs)を参照ください。  

非キャッシュメモリをを使用しない場合は、以下のようなキャッシュ制御が必要となります。  

```cpp
#include "dcache-control.h" //★キャッシュ制御用のヘッダを追加

//★通常メモリに配置 キャッシュ制御を行うメモリは必ず32byteにアライン、32byteの倍数にする
//32 bytes aligned!
#if defined(__ICCARM__)
#pragma data_alignment=32
static uint8_t audio_write_buff[AUDIO_WRITE_BUFF_NUM][AUDIO_WRITE_BUFF_SIZE];
#else
static uint8_t audio_write_buff[AUDIO_WRITE_BUFF_NUM][AUDIO_WRITE_BUFF_SIZE]__attribute((aligned(32)));
#endif

int main() {
・・・省略・・・
      dcache_clean(p_buf, AUDIO_WRITE_BUFF_SIZE); //★キャッシュ内のデータを実メモリに書き込む
      audio.write(p_buf, audio_data_size, &audio_write_async_ctl);
```
