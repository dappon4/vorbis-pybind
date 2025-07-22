#pragma once
#include <stddef.h>
size_t wav_to_pcm(const char* filename, float** out_pcm, int* out_channels, int* out_rate, int* out_frames);
