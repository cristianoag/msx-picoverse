#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/audio_i2s.h"
#include "mp3dec.h"

// Define I2S pins
#define I2S_WSEL_PIN  27  // GPIO pin for I2S left-right clock
#define I2S_DATA_PIN  28  // GPIO pin for I2S data
#define I2S_BCLK_PIN  29  // GPIO pin for I2S bit clock

// Global audio buffer pool
struct audio_buffer_pool *audio_buffer_pool;

void init_audio() 
{
    static audio_format_t audio_format = {
        .sample_freq = 44100,
        .format = AUDIO_BUFFER_FORMAT_PCM_S16,
        .channel_count = 2,
    };

    static struct audio_buffer_format producer_format = {
        .format = &audio_format,
        .sample_stride = 4,
    };

    // Initialize the audio buffer pool with 3 buffers, each capable of holding 576 samples
    audio_buffer_pool = audio_new_producer_pool(&producer_format, 3, 576);
    if (!audio_buffer_pool) {
        printf("Failed to create audio buffer pool.\n");
        while (1);
    }

    static const struct audio_i2s_config config = {
        .data_pin = I2S_DATA_PIN,
        .clock_pin_base = I2S_BCLK_PIN,
        .dma_channel = 0,
        .pio_sm = 0,
    };

    const struct audio_format *output_format;
    output_format = audio_i2s_setup(&audio_format, &config);
    if (!output_format) {
        printf("Failed to initialize I2S audio.\n");
        while (1);
    }

    bool connected = audio_i2s_connect(audio_buffer_pool);
    if (!connected) {
        printf("Failed to connect audio buffer pool to I2S.\n");
        while (1);
    }

    audio_i2s_set_enabled(true);
}

void play_mp3( uint8_t *mp3_data, size_t mp3_size) {
    HMP3Decoder hMP3Decoder;
    MP3FrameInfo mp3FrameInfo;
    int offset, err;
    short pcm_buffer[1152 * 2];  // Max samples per frame * number of channels

    hMP3Decoder = MP3InitDecoder();
    if (!hMP3Decoder) {
        printf("Failed to initialize MP3 decoder.\n");
        return;
    }

    offset = MP3FindSyncWord(mp3_data, mp3_size);
    while (offset >= 0 && mp3_size > 0) {
        mp3_data += offset;
        mp3_size -= offset;

        err = MP3Decode(hMP3Decoder, &mp3_data, (int *)&mp3_size, pcm_buffer, 0);
        if (err) {
            printf("MP3 decode error: %d\n", err);
            break;
        }

        MP3GetLastFrameInfo(hMP3Decoder, &mp3FrameInfo);

        // Output PCM data to I2S
        struct audio_buffer *buffer = take_audio_buffer(audio_buffer_pool, true);
        int16_t *samples = (int16_t *)buffer->buffer->bytes;
        for (int i = 0; i < mp3FrameInfo.outputSamps; i++) {
            samples[i] = pcm_buffer[i];
        }
        buffer->sample_count = mp3FrameInfo.outputSamps / mp3FrameInfo.nChans;
        give_audio_buffer(audio_buffer_pool, buffer);

        offset = MP3FindSyncWord(mp3_data, mp3_size);
    }

    MP3FreeDecoder(hMP3Decoder);
}

int main()
{
    stdio_init_all();

    // Initialize I2S audio
    init_audio();

    // Load MP3 data from flash (replace with actual data and size)
    extern const uint8_t mp3_file_data[]; //get from flash
    extern const size_t mp3_file_size; //get from flash

    // Play MP3
   // play_mp3(mp3_file_data, mp3_file_size);

}
