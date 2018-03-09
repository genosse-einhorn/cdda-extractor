#include "lame_backend.h"

#include <QLibrary>

namespace {

struct DllLameTable : public Encoder::LameFuncTable {
    QLibrary _lib;

    DllLameTable() : _lib(QStringLiteral("libmp3lame"), 0) {
#define RESOLVE_LAME(name) \
            do { \
                this->name = (decltype(this->name))_lib.resolve("lame_" #name); \
            } while (0)
#define RESOLVE_ID3(name) \
            do { \
                this->name = (decltype(this->name))_lib.resolve(#name); \
            } while (0)

        RESOLVE_LAME(init);
        RESOLVE_LAME(set_num_samples);
        RESOLVE_LAME(set_in_samplerate);
        RESOLVE_LAME(set_num_channels);
        RESOLVE_LAME(set_quality);
        RESOLVE_LAME(set_mode);
        RESOLVE_LAME(set_brate);
        RESOLVE_LAME(init_params);
        RESOLVE_LAME(encode_flush);
        RESOLVE_LAME(encode_buffer_interleaved);
        RESOLVE_LAME(close);

        // id3 stuff
        RESOLVE_ID3(id3tag_init);
        RESOLVE_ID3(id3tag_v2_only);
        RESOLVE_ID3(id3tag_pad_v2);
        RESOLVE_ID3(id3tag_set_albumart);
        RESOLVE_ID3(id3tag_set_textinfo_utf16);

        RESOLVE_LAME(get_id3v2_tag);
        RESOLVE_LAME(set_write_id3tag_automatic);

#undef RESOLVE_LAME
#undef RESOLVE_ID3
    }
};

} // anonymous Namespace

namespace Encoder {

LameFuncTable *
LameFuncTable::get() {
    static DllLameTable table;

    if (table.init)
        return &table;
    else
        return nullptr;
}

} // namespace Encoder
