#include "lame_backend.h"

#include <lame.h>
#include <utility>

namespace {

// lots of template hackery to cast Encoder::LameHandle to lame_t and back
// any self-respecting compiler should of course just emit a jump to the original function
template<typename T> struct converted
{
    using type = T;
};
template<> struct converted<lame_t>
{
    using type = Encoder::LameHandle;
};
template<> struct converted<Encoder::LameHandle>
{
    using type = lame_t;
};
template<> struct converted<Encoder::LameFuncTable::LameMpegMode>
{
    using type = MPEG_mode;
};
template<> struct converted<MPEG_mode>
{
    using type = Encoder::LameFuncTable::LameMpegMode;
};

template<typename F, F f> struct caller;

template<typename R, typename... Args, R(*f)(Args...)>
struct caller<R(*)(Args...), f>
{
    static typename converted<R>::type call(typename converted<Args>::type... a) {
        return (typename converted<R>::type)(f(Args(a)...));
    }
};

#define WRAP_LAME(name) .name = &caller<decltype(&lame_##name), lame_##name>::call
#define WRAP_ID3TAG(name) .id3tag_##name = &caller<decltype(&id3tag_##name), id3tag_##name>::call

Encoder::LameFuncTable table = {
    WRAP_LAME(init),
    WRAP_LAME(set_num_samples),
    WRAP_LAME(set_in_samplerate),
    WRAP_LAME(set_num_channels),
    WRAP_LAME(set_quality),
    WRAP_LAME(set_mode),
    WRAP_LAME(set_brate),
    WRAP_LAME(init_params),
    WRAP_LAME(encode_flush),
    WRAP_LAME(encode_buffer_interleaved),
    WRAP_LAME(close),
    WRAP_ID3TAG(init),
    WRAP_ID3TAG(v2_only),
    WRAP_ID3TAG(pad_v2),
    WRAP_ID3TAG(set_albumart),
    WRAP_ID3TAG(set_textinfo_utf16),
    WRAP_LAME(get_id3v2_tag),
    WRAP_LAME(set_write_id3tag_automatic)
};

} // anonymous namespace

Encoder::LameFuncTable *
Encoder::LameFuncTable::get()
{
    return &table;
}
