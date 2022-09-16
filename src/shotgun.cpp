#include "userosc.h"
#include "osc_w0f.cpp"
#include "pcg.cpp"

const uint16_t BULLETS = 32u;
const float RECEP_LN05 = -3.32192809489f;

typedef struct State {
  uint32_t seed;
  uint8_t osc;
  float oct;
  uint8_t blts;
  float phases[ BULLETS ];
  float lfo, lfoz;
  float width;
  float snap;
  uint8_t flags;
} State;

enum {
  k_flags_none = 0,
  k_flag_reset = 1 << 0,
};

static State s_state;

void OSC_INIT(uint32_t platform, uint32_t api)
{
  s_state.seed = 0u;
  s_state.oct = 0.0f;

  for ( uint16_t i = 0; i < BULLETS; i ++ ) {
    s_state.phases[ i ] = 0.0f;
  }

  s_state.lfo = s_state.lfoz = 0.0f;
  s_state.width = 0.0f;
  s_state.snap = 0.0f;
  s_state.flags = k_flags_none;
}

void OSC_CYCLE(const user_osc_param_t * const params,
               int32_t *yn,
               const uint32_t frames)
{
  const uint8_t flags = s_state.flags;
  s_state.flags = k_flags_none;

  s_pcg_state = s_state.seed;
  const float width = s_state.width;
  const float snap = s_state.snap;
  const float oct = s_state.oct;
  const uint8_t osc = s_state.osc;
  const uint8_t blts = s_state.blts;

  const float pitch = (float) params->pitch / 256.0f;

  if ( flags & k_flag_reset ) {
    for ( uint16_t i = 0; i < BULLETS; i ++ ) {
      s_state.phases[ i ] = 0.0f;
    }
  }

  const float lfo = s_state.lfo = q31_to_f32( params->shape_lfo );
  float lfoz = ( flags & k_flag_reset ) ? lfo : s_state.lfoz;
  const float lfo_inc = ( lfo - lfoz ) / frames;

  q31_t * __restrict y = (q31_t *) yn;
  const q31_t * y_e = y + frames;

  float w0p[ BULLETS ];

  for ( uint16_t i = 0; i < BULLETS; i ++ ) {
    float note = width * q31_to_f32( pcg() );
    float rounded = (float) ( (uint32_t) ( note / 12.0f + 0.5f ) ) * 12.0f;
    w0p[ i ] = osc_w0f( pitch + oct + linintf( snap, note, rounded ) );
  }

  for ( ; y != y_e; ) {
    float sig = 0.0f;

    for ( uint16_t i = 0; i < BULLETS; i ++ ) {
      if (blts <= i) { break; }

      switch (osc) {
      case 0:
        sig += osc_sinf( s_state.phases[ i ] );
        break;
      case 1:
        sig += 1.0f - 2.0f * s_state.phases[ i ];
        break;
      case 2:
        sig += ( s_state.phases[ i ] <= 0.5f ) ? 1.0f : -1.0f;
        break;
      default:
        break;
      }

      s_state.phases[ i ] += w0p[ i ];
      s_state.phases[ i ] -= (uint32_t) s_state.phases[ i ];
    }

    *( y ++ ) = f32_to_q31( sig / (float) BULLETS );

    lfoz += lfo_inc;
  }

  s_state.lfoz = lfoz;
}

void OSC_NOTEON(const user_osc_param_t * const params)
{
  s_state.flags |= k_flag_reset;
}

void OSC_NOTEOFF(const user_osc_param_t * const params)
{
  (void)params;
}

void OSC_PARAM(uint16_t index, uint16_t value)
{
  const float valf = param_val_to_f32(value);
  
  switch (index) {
  case k_user_osc_param_id1:
    s_state.osc = value;
    break;
  case k_user_osc_param_id2:
    s_state.oct = 12.0f * (float) ( value - 100 );
    break;
  case k_user_osc_param_id3:
    s_state.seed = value;
    break;
  case k_user_osc_param_id4:
    s_state.blts = value + 1u;
    break;
  case k_user_osc_param_shape:
    s_state.width = 24.0f * valf;
    break;
  case k_user_osc_param_shiftshape:
    s_state.snap = valf;
    break;
  default:
    break;
  }
}

