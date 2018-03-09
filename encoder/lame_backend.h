#ifndef LAME_BACKEND_H
#define LAME_BACKEND_H

#include <string.h>

#ifdef WIN32
#define LAME_CDECL __cdecl
#else
#define LAME_CDECL
#endif

namespace Encoder {

struct LameGlobalFlags;
typedef struct LameGlobalFlags *LameHandle;

struct LameFuncTable {
    enum LameMpegMode {
        MODE_STEREO = 0,
        MODE_JOINT_STEREO,
        MODE_DUAL_CHANNEL,
        MODE_MONO,
        MODE_NOT_SET
    };

    LameHandle (LAME_CDECL *init)(void);
    int (LAME_CDECL *set_num_samples)(LameHandle, unsigned long);
    int (LAME_CDECL *set_in_samplerate)(LameHandle, int);
    int (LAME_CDECL *set_num_channels)(LameHandle, int);
    int (LAME_CDECL *set_quality)(LameHandle, int);
    int (LAME_CDECL *set_mode)(LameHandle, LameMpegMode);
    int (LAME_CDECL *set_brate)(LameHandle, int);
    int (LAME_CDECL *init_params)(LameHandle);
    int (LAME_CDECL *encode_flush)(LameHandle, unsigned char *, int);
    int (LAME_CDECL *encode_buffer_interleaved)(LameHandle, short[], int, unsigned char *, int);
    int (LAME_CDECL *close)(LameHandle);

    // id3 stuff
    void (LAME_CDECL *id3tag_init)(LameHandle);
    void (LAME_CDECL *id3tag_v2_only)(LameHandle);
    void (LAME_CDECL *id3tag_pad_v2)(LameHandle);
    int (LAME_CDECL *id3tag_set_albumart)(LameHandle, const char *, size_t);
    int (LAME_CDECL *id3tag_set_textinfo_utf16)(LameHandle, const char *, const unsigned short *);

    size_t (LAME_CDECL *get_id3v2_tag)(LameHandle, unsigned char *, size_t);
    void (LAME_CDECL *set_write_id3tag_automatic)(LameHandle, int);

    // the lame function table is allocated statically and must not be freed
    // returns nullptr if libmp3lame is not available
    static LameFuncTable *get();
};
} // namespace Encoder

#endif // LAME_BACKEND_H
