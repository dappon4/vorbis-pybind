// Control macros (from vorbis/codec.h in libvorbis)
#define OV_ECTL_RATEMANAGE_GET    0x10
#define OV_ECTL_RATEMANAGE_SET    0x11
#define OV_ECTL_RATEMANAGE_AVG    0x12
#define OV_ECTL_RATEMANAGE_HARD   0x13
#define OV_ECTL_RATEMANAGE2_GET   0x14
#define OV_ECTL_RATEMANAGE2_SET   0x15
#define OV_ECTL_LOWPASS_GET       0x20
#define OV_ECTL_LOWPASS_SET       0x21
#define OV_ECTL_IBLOCK_GET        0x30
#define OV_ECTL_IBLOCK_SET        0x31
#define OV_ECTL_COUPLING_GET      0x40
#define OV_ECTL_COUPLING_SET      0x41

#ifndef VORBIS_OVECTL_STRUCTS_DEFINED
#define VORBIS_OVECTL_STRUCTS_DEFINED
typedef struct ovectl_ratemanage_arg {
    int management_active;
    long bitrate_hard_min;
    long bitrate_hard_max;
    double bitrate_hard_window;
    long bitrate_av_lo;
    long bitrate_av_hi;
    double bitrate_av_window;
    double bitrate_av_window_center;
} ovectl_ratemanage_arg;

typedef struct ovectl_ratemanage2_arg {
    int management_active;
    long bitrate_limit_min_kbps;
    long bitrate_limit_max_kbps;
    long bitrate_average_kbps;
    double bitrate_average_damping;
    long bitrate_limit_reservoir_bits;
    double bitrate_limit_reservoir_bias;
} ovectl_ratemanage2_arg;
#endif

#ifndef VORBIS_CODEC_H
#define VORBIS_CODEC_H
#include <ogg/ogg.h>
#ifdef __cplusplus
extern "C" {
#endif


// Error codes (from vorbis/codec.h in libvorbis)
#define OV_FALSE        -1
#define OV_EOF          -2
#define OV_HOLE         -3
#define OV_EREAD        -128
#define OV_EFAULT       -129
#define OV_EIMPL        -130
#define OV_EINVAL       -131
#define OV_ENOTVORBIS   -132
#define OV_EBADHEADER   -133
#define OV_EVERSION     -134
#define OV_ENOTAUDIO    -135
#define OV_EBADPACKET   -136
#define OV_EBADLINK     -137
#define OV_ENOSEEK      -138


// Full struct definitions (from codec_internal.h and public API)

typedef struct vorbis_info {
    int version;
    int channels;
    long rate;
    long bitrate_upper;
    long bitrate_nominal;
    long bitrate_lower;
    long bitrate_window;
    void *codec_setup;
} vorbis_info;

typedef struct vorbis_comment {
    char **user_comments;
    int   *comment_lengths;
    int    comments;
    char  *vendor;
} vorbis_comment;


typedef struct alloc_chain {
    void *ptr;
    struct alloc_chain *next;
} alloc_chain;

typedef struct vorbis_block {
    struct vorbis_dsp_state *vd;
    void *internal; // vorbis_block_internal*
    float **pcm;
    float **pcmret;
    long pcm_storage;
    long pcm_current;
    int  pcm_returned;
    int  eofflag;
    long sequence;
    long granulepos;
    long glue_bits;
    long time_bits;
    long floor_bits;
    long res_bits;
    int mode; // Added missing member
    oggpack_buffer opb;
    long W;
    long lW;
    long nW;
    long centerW;
    long pcmend;
    long localalloc;
    long localtop;
    long totaluse;
    void *localstore;
    alloc_chain *reap;
} vorbis_block;

typedef struct vorbis_dsp_state {
    int analysisp;
    vorbis_info *vi;
    float **pcm;
    float **pcmret;
    int pcm_storage;
    int pcm_current;
    int pcm_returned;
    int preextrapolate;
    int eofflag;
    long lW;
    long W;
    long nW;
    long centerW;
    ogg_int64_t granulepos;
    ogg_int64_t sequence;
    ogg_int64_t glue_bits;
    ogg_int64_t time_bits;
    ogg_int64_t floor_bits;
    ogg_int64_t res_bits;
    void *backend_state;
} vorbis_dsp_state;

// API functions
void vorbis_info_init(vorbis_info *vi);
void vorbis_info_clear(vorbis_info *vi);
void vorbis_comment_init(vorbis_comment *vc);
void vorbis_comment_add_tag(vorbis_comment *vc, const char *tag, const char *contents);
void vorbis_comment_clear(vorbis_comment *vc);
int vorbis_analysis_init(vorbis_dsp_state *v, vorbis_info *vi);
int vorbis_block_init(vorbis_dsp_state *v, vorbis_block *vb);
int vorbis_block_clear(vorbis_block *vb);
float **vorbis_analysis_buffer(vorbis_dsp_state *v, int vals);
int vorbis_analysis_wrote(vorbis_dsp_state *v, int vals);
int vorbis_analysis_blockout(vorbis_dsp_state *v, vorbis_block *vb);
int vorbis_analysis(vorbis_block *vb, ogg_packet *op);

int vorbis_analysis_headerout(vorbis_dsp_state *v, vorbis_comment *vc,
                              ogg_packet *op, ogg_packet *op_comm, ogg_packet *op_code);
void vorbis_dsp_clear(vorbis_dsp_state *v);
int vorbis_bitrate_flushpacket(vorbis_dsp_state *vd, ogg_packet *op);

#ifdef __cplusplus
}
#endif
#endif // VORBIS_CODEC_H
