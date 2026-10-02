// SPDX-License-Identifier: BSD-3-Clause AND CC-PDM-1.0
// SPDX-AI-Disclosure: ai-assisted
#include "st3play.h"

namespace replay::st3play::probe {
using namespace replay::st3play;
bool renderToWavFlag = false; // defined by upstream's example main
#include "mixer/sinc.c"
namespace sbpro {
#include "mixer/sbpro.c"
}
namespace gus {
#include "mixer/gus_gf1.c"
}
namespace opl2 {
#include "opl2/opl2.c"
}
#include "digdata.c"
#include "digadl.c"
#include "digamg.c"
#include "digcmd.c"
#include "digread.c"
#include "dig_gus.c"
#include "dig.c"
#include "load.c"
} // namespace replay::st3play::probe
