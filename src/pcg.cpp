#include "osc_api.h"

static uint32_t s_pcg_state;

__fast_inline uint32_t pcg()
{
  uint32_t state = s_pcg_state;

  s_pcg_state = s_pcg_state * 1145141u + 919810u;

  s_pcg_state = ( s_pcg_state >> 16u ) ^ s_pcg_state;

  return s_pcg_state;
}
