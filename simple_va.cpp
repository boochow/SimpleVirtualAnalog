/*
 * File: simple_va.cpp
 * a wave table sawtooth oscillator 
 */

#include "userosc.h"
#include "sawtooth_wavetable.h"

#define MAX_INDEX (w_tbl_idx_size - 1)
#define W_TBL_MASK (w_tbl_size -1 )

// if DEBUG is defined, only sine wave is used
// #define DEBUG

/* note number to wavetable index number */
__fast_inline float osc_my_idx_i(int note) {
    int idx_i;

    for(idx_i = 0; idx_i < w_tbl_idx_size; idx_i++) {
        if (note_boundary[idx_i] >= note) {
            break;
        }
    }
    if (idx_i == w_tbl_idx_size) {
        idx_i--;
    }
    return idx_i;
}

__fast_inline float osc_my_idx(float note) {
    int idx_0, idx_1;
    int note_i = (int) note;

#ifdef DEBUG
    return 1.0 * MAX_INDEX;
#else
    /* search index number */
    idx_0 = osc_my_idx_i(note);
    idx_1 = osc_my_idx_i(note + 1);
    /* fraction part */
    if (idx_0 == idx_1) {
        return idx_0;
    } else {
        return idx_0 + (note - note_i);
    }
#endif
}

__fast_inline float osc_bl2_my(float x, float idx) {
    const float p = x - (uint32_t) x;
    int idx_i = (int) idx;
    float idx_f = idx - idx_i;
    float sign0, sign1;

    // half period stored -- fold back and invert
    const float x0f = p * 2 * w_tbl_size - 0.5;
    int32_t x0 = (uint32_t) x0f;
    float k = x0f - x0;
    float *wt = (float *) w_tbl[idx_i];

    if (x0 >= w_tbl_size) {
        x0 = 2 * w_tbl_size - 1 - x0;
        sign0 = -1.f;
    } else {
        sign0 = 1.f;
    }

    int32_t x1 = x0 + sign0;
    if (x1 < 0) {
        x1 = 0;
        sign1 = 1.f;
    } else if (x1 == w_tbl_size) {
        sign1 = -sign0;
    } else {
        sign1 = sign0;
    }

    const float y0 = linintf(k, wt[x0] * sign0, wt[x1] * sign1);
    wt += w_tbl_size + 1;
    const float y1 = linintf(k, wt[x0] * sign0, wt[x1] * sign1);

    idx_f = (idx_f > 0.5) ? 1.0 : 2 * idx_f;

    return linintf(idx_f, y0, y1);
}

typedef struct State {
    float phi;  /* current phase from 0.0 to 1.0 */
    uint8_t flags;
} State;

enum {
    k_flags_none = 0,
    k_flag_reset = 1<<0,
};

static State s_osc;


void OSC_INIT(uint32_t platform, uint32_t api)
{
    s_osc.phi = 0.f;
    s_osc.flags = k_flags_none;
}

void OSC_CYCLE(const user_osc_param_t * const params,
               int32_t *yn,
               const uint32_t frames)
{
    const uint8_t flags = s_osc.flags;
    s_osc.flags = k_flags_none;
    
    const float note = (params->pitch >> 8) + (params->pitch & 0xFF)/256.0f;
    const float wt_idx = osc_my_idx(note);
    const float w0 = osc_w0f_for_note((params->pitch) >> 8, params->pitch & 0xFF);
    float phi = (flags & k_flag_reset) ? 0.f : s_osc.phi;
  
    q31_t * __restrict y = (q31_t *) yn;
    const q31_t * y_e = y + frames;
    for (; y != y_e; ) {
        float sig = osc_bl2_my(phi, wt_idx);
        *(y++) = f32_to_q31(sig);

        phi += w0;
        phi -= (uint32_t) phi;
    }
    s_osc.phi = phi;
}

void OSC_NOTEON(const user_osc_param_t * const params)
{
    s_osc.flags |= k_flag_reset;
}

void OSC_NOTEOFF(const user_osc_param_t * const params)
{
    (void) params;
}

void OSC_PARAM(uint16_t index, uint16_t value)
{
}
