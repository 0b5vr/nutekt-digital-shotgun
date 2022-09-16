#include "osc_api.h"

/**
 * @param note Note in [0.0 - 151.996] range.
 * @return Corresponding 0-1 phase increment in floating point.
 */
__fast_inline float osc_w0f( float note ) {
  const uint32_t n0 = (uint32_t) note;
  const float f0 = osc_notehzf( n0 );
  const float f1 = osc_notehzf( n0 + 1u );

  const float mod = note - n0;
  const float f = clipmaxf( linintf( mod, f0, f1 ), k_note_max_hz );

  return f * k_samplerate_recipf;
}
