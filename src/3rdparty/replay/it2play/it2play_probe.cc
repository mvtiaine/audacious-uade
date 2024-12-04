// SPDX-License-Identifier: BSD-3-Clause
#include "it2play.h"

namespace replay::it2play::probe {
using namespace replay::it2play;
namespace hq {
#include "it2drivers/hq_m.h"
#include "it2drivers/hq_m.c"
#include "it2drivers/hq.h"
#include "it2drivers/hq.c"
#undef Get8BitWaveForm
#undef Get16BitWaveForm
#undef FilterSample
#undef MixSample
#undef MixSampleSurround
#undef RampCurrVolumeL
#undef RampCurrVolumeR
#undef RAMPSPEED
#undef RAMPCOMPENSATE
#undef UpdatePos
}
namespace sb16 {
#include "it2drivers/sb16_m.h"
#include "it2drivers/sb16_m.c"
#include "it2drivers/sb16.h"
#include "it2drivers/sb16.c"
#undef Get8BitWaveForm
#undef Get16BitWaveForm
#undef FilterSample
#undef MixSample
#undef MixSampleSurround
#undef RampCurrVolumeL
#undef RampCurrVolumeR
#undef RAMPSPEED
#undef RAMPCOMPENSATE
#undef UpdatePos
}
namespace sb16mmx {
#include "it2drivers/sb16mmx_m.h"
#include "it2drivers/sb16mmx_m.c"
#include "it2drivers/sb16mmx.h"
#include "it2drivers/sb16mmx.c"
#undef Get8BitWaveForm
#undef Get16BitWaveForm
#undef FilterSample
#undef MixSample
#undef MixSampleSurround
#undef RampCurrVolumeL
#undef RampCurrVolumeR
#undef RAMPSPEED
#undef RAMPCOMPENSATE
#undef UpdatePos
}
namespace wavwriter {
#include "it2drivers/wavwriter_m.h"
#include "it2drivers/wavwriter_m.c"
#include "it2drivers/wavwriter.h"
#include "it2drivers/wavwriter.c"
#undef Get8BitWaveForm
#undef Get16BitWaveForm
#undef FilterSample
#undef MixSample
#undef MixSampleSurround
#undef RampCurrVolumeL
#undef RampCurrVolumeR
#undef RAMPSPEED
#undef RAMPCOMPENSATE
#undef UpdatePos
}
using namespace hq;
using namespace sb16;
using namespace sb16mmx;
using namespace wavwriter;
#include "it_structs.c"
#include "it_d_rm.c"
#include "it_m_eff.c"
#include "it_music.c"
#include "loaders/it.c"
#include "loaders/s3m.c"

} // namespace replay::it2play::probe
