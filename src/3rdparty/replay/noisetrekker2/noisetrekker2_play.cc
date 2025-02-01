// SPDX-License-Identifier: LicenseRef-NoiseTrekker2
#include <algorithm>
#include <cassert>
#include <cstddef>

#include "noisetrekker2.h"

using namespace std;

namespace replay::noisetrekker2 {

size_t fread(void *ptr, size_t size, size_t n, FILE *stream) {
    ssize_t bytes = min((size_t)(size * n), stream->size - stream->pos);
    if (bytes <= 0) return 0;
    assert(bytes > 0);
    assert(stream->pos <= stream->size);
    assert(stream->pos + bytes <= stream->size);
    memcpy(ptr, stream->data + stream->pos, bytes);
    stream->pos += bytes;
    return bytes;
}

}

namespace replay::noisetrekker2::play {

long SamplesPerTick = 0;
#include "NtkSourceCode/Alphatrack.cpp"
#include "NtkSourceCode/tb303.cpp"
#include "NtkSourceCode/cubicspline.cpp"
#include "NtkSourceCode/main.cpp"

void reset() noexcept {
    ped_line = 0;
    cPosition = 0;
    memset(buf0, 0, sizeof(buf0));
    memset(buf1, 0, sizeof(buf1));
    memset(buf024, 0, sizeof(buf024));
    memset(buf124, 0, sizeof(buf124));
    memset(FLANGE_LEFTBUFFER, 0, sizeof(FLANGE_LEFTBUFFER));
    memset(FLANGE_RIGHTBUFFER, 0, sizeof(FLANGE_RIGHTBUFFER));
    memset(delay_left_buffer, 0, sizeof(delay_left_buffer));
    memset(delay_right_buffer, 0, sizeof(delay_right_buffer));
    memset(allBuffer_L, 0, sizeof(allBuffer_L));
    memset(allBuffer_L2, 0, sizeof(allBuffer_L2));
    memset(allBuffer_L3, 0, sizeof(allBuffer_L3));
    memset(allBuffer_L4, 0, sizeof(allBuffer_L4));
    memset(allBuffer_L5, 0, sizeof(allBuffer_L5));
    memset(allBuffer_L6, 0, sizeof(allBuffer_L6));
    for (int i = 0; i < MAX_TRACKS; ++i) {
        Synthesizer[i].ENV1_STAGE = 0;
        Synthesizer[i].ENV2_STAGE = 0;
    }
}

}
