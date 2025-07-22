#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// Simple WAV header struct for PCM 16-bit LE
#pragma pack(push, 1)
typedef struct {
    char riff[4];
    uint32_t chunk_size;
    char wave[4];
    char fmt[4];
    uint32_t subchunk1_size;
    uint16_t audio_format;
    uint16_t num_channels;
    uint32_t sample_rate;
    uint32_t byte_rate;
    uint16_t block_align;
    uint16_t bits_per_sample;
    char data[4];
    uint32_t subchunk2_size;
} wav_header_t;
#pragma pack(pop)

// Reads a WAV file and outputs float PCM (normalized -1..1), returns number of samples per channel
size_t wav_to_pcm(const char* filename, float** out_pcm, int* out_channels, int* out_rate, int* out_frames) {
    FILE* f = fopen(filename, "rb");
    if (!f) return 0;
    wav_header_t header;
    if (fread(&header, sizeof(header), 1, f) != 1) { fclose(f); return 0; }
    if (memcmp(header.riff, "RIFF", 4) != 0 || memcmp(header.wave, "WAVE", 4) != 0) { fclose(f); return 0; }
    if (header.audio_format != 1 || header.bits_per_sample != 16) { fclose(f); return 0; }
    int channels = header.num_channels;
    int rate = header.sample_rate;
    int frames = header.subchunk2_size / (channels * 2);
    float* pcm = (float*)malloc(sizeof(float) * frames * channels);
    for (int i = 0; i < frames * channels; ++i) {
        int16_t sample;
        if (fread(&sample, sizeof(sample), 1, f) != 1) { pcm[i] = 0.0f; }
        else { pcm[i] = sample / 32768.0f; }
    }
    fclose(f);
    *out_pcm = pcm;
    *out_channels = channels;
    *out_rate = rate;
    *out_frames = frames;
    return frames;
}
