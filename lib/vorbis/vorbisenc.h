#ifndef VORBIS_VORBISENC_H
#define VORBIS_VORBISENC_H
#include "codec.h"
#ifdef __cplusplus
extern "C" {
#endif

int vorbis_encode_init_vbr(vorbis_info *vi, long channels, long rate, float base_quality);
int vorbis_encode_setup_vbr(vorbis_info *vi, long channels, long rate, float quality);
int vorbis_encode_setup_init(vorbis_info *vi);

#ifdef __cplusplus
}
#endif
#endif // VORBIS_VORBISENC_H
