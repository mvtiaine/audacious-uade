#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <type_traits>
#include <utility>

#include "ft2play.h"

// for endianess check
#include "config.h"

namespace replay::ft2play::probe {
using namespace replay::ft2play;
#include "pmplay.c"
#include "pmp_main.c"
#include "pmp_mix.c"
#include "snd_masm.c"
}
