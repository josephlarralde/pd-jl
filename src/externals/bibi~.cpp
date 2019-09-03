/**
 * @file bibi~.cpp
 * @author Joseph Larralde
 * @date 07/06/2018
 * @brief biquad filter with lowpass, highpass, bandpass, notch and allpass modes
 *
 * @copyright
 * Copyright (C) 2018 by Joseph Larralde.
 * All rights reserved.
 *
 * License (BSD 3-clause)
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "m_pd.h"
#include "../dependencies/jl.cpp.lib/dsp/effects/Biquad.h"

class PdBiquad;

static t_class *bibi_tilde_class;

typedef struct _bibi_tilde {

  t_object x_obj;

  // this is used in setup function to give a handle on the leftmost signal inlet
  t_sample x_f;

  PdBiquad *biquad;
  t_outlet *x_out;

} t_bibi_tilde;

//============================================================================//

// This code is pretty much duplicated from the more advanced Compressor class
// in jl.cpp.lib, as it is not easy to have a variable number of signal outlets
// from an arg, need to use variadic or something with the dsp_add method ...

// NB : we could also have signal inlets for thresh, ratio, knee and makeup gain
// control. nah ?

class PdBiquad {
private:
  t_bibi_tilde *x;

  jl::Biquad b;

  jl::Ramp<float, jl::sample> rF;
  jl::Ramp<float, jl::sample> rQ;

  float samplingRate;
  float rampDuration;
  unsigned long rampSamples;

public:
  PdBiquad() :
  rampDuration(5) {
    setSamplingRate(44100);
    rF.ramp(1e-3);
    rQ.ramp(1e-3);
  }

  ~PdBiquad() {}

  void setObject(t_bibi_tilde *obj) {
    x = obj;
  }

  void setSamplingRate(float sr) {
    samplingRate = sr;
    b.setSamplingRate(samplingRate);
    rampSamples = static_cast<unsigned long>(rampDuration * samplingRate * 0.001);    
  }

  void setMode(jl::BiquadMode m) { b.setMode(m); }
  void setF(float f) { rF.ramp(JL_MAX(f, 1e-3), rampSamples); }
  void setQ(float q) { rQ.ramp(JL_MAX(q, 1e-3) , rampSamples); }

  jl::sample process(jl::sample in) {
    b.setF(*(rF.process(1)));
    b.setQ(*(rQ.process(1)));

    return b.process(in);
  }
};

void bibi_tilde_mode(t_bibi_tilde *x, t_symbol *s) {
  if (s == gensym("lowpass")) {
    // post("setting lowpass mode");
    x->biquad->setMode(jl::LowpassBiquadMode);
  } else if (s == gensym("highpass")) {
    // post("setting highpass mode");
    x->biquad->setMode(jl::HighpassBiquadMode);
  } else if (s == gensym("bandpass")) {
    // post("setting bandpass mode");
    x->biquad->setMode(jl::BandpassBiquadMode);
  } else if (s == gensym("notch")) {
    // post("setting notch mode");
    x->biquad->setMode(jl::NotchBiquadMode);
  } else if (s == gensym("allpass")) {
    // post("setting allpass mode");
    x->biquad->setMode(jl::AllpassBiquadMode);
  } else {
    // post("unknown mode");
  }
}

void bibi_tilde_cutoff(t_bibi_tilde *x, t_floatarg f) {
  x->biquad->setF(static_cast<float>(f));
}

void bibi_tilde_q(t_bibi_tilde *x, t_floatarg f) {
  x->biquad->setQ(static_cast<float>(f));
}

void bibi_tilde_reset(t_bibi_tilde *x) {
  x->biquad->reset();
}

void bibi_tilde_setsr(t_bibi_tilde *x, t_floatarg f) {
  x->biquad->setSamplingRate(static_cast<float>(f));
}

//============================ DSP OPERATIONS ================================//

t_int *bibi_tilde_perform(t_int *w) {
  t_bibi_tilde *x = (t_bibi_tilde *)(w[1]);
  t_sample *in = (t_sample *)(w[2]);
  t_sample *out = (t_sample *)(w[3]);
  int n = (int)(w[4]);

  for (int i = 0; i < n; ++i) {
    jl::sample val = static_cast<jl::sample>(*in);
    *out = x->biquad->process(val);
    in++;
    out++;
  }

  return (w + 5);
}


void bibi_tilde_dsp(t_bibi_tilde *x, t_signal **sp) {
  bibi_tilde_setsr(x, sys_getsr());

  dsp_add(bibi_tilde_perform, 4, x,
          sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

//======================= CONSTRUCTOR / DESTRUCTOR ===========================//

void *bibi_tilde_new(t_symbol *s, int argc, t_atom *argv) {
  /*
   * call the "constructor" of the parent-class
   * this will reserve enough memory to hold "t_bibi_tilde"
   */
  t_bibi_tilde *x = (t_bibi_tilde *)pd_new(bibi_tilde_class);
  // x->x_f = 0;

  x->biquad = new PdBiquad();
  x->biquad->setObject(x);

  if (argc > 0) {
    bibi_tilde_mode(x, atom_getsymbol(argv));
  }

  if (argc > 1) {
    bibi_tilde_cutoff(x, atom_getfloat(argv + 1));
  }

  if (argc > 2) {
    bibi_tilde_q(x, atom_getfloat(argv + 2));
  }

  bibi_tilde_setsr(x, sys_getsr());

  x->x_out = outlet_new(&x->x_obj, &s_signal);

  return (void *)x;
}

void bibi_tilde_free(t_bibi_tilde *x) {
  delete x->biquad;

  outlet_free(x->x_out);
}

//============================ SETUP FUNCTION ================================//

extern "C" {

/**
 * define the function-space of the class
 * within a single-object external the name of this function is special
 */
void bibi_tilde_setup(void) {
  /* create a new class */
  bibi_tilde_class = class_new(gensym("bibi~"), /* the object's name is "bibi~" */
                               (t_newmethod)bibi_tilde_new, /* the object's constructor is "bibi_tilde_new()" */
                               (t_method)bibi_tilde_free, /* the object's destructor */
                               sizeof(t_bibi_tilde), /* the size of the data-space */
                               CLASS_DEFAULT, /* a normal pd object */
                               A_GIMME, /* arg type (default associated table name) */
                               0); /* no creation arguments ? */

  class_addmethod(bibi_tilde_class, (t_method)bibi_tilde_dsp, gensym("dsp"), A_NULL);
  class_addmethod(bibi_tilde_class, (t_method)bibi_tilde_mode, gensym("mode"), A_DEFSYM, 0);
  class_addmethod(bibi_tilde_class, (t_method)bibi_tilde_cutoff, gensym("cutoff"), A_DEFFLOAT, 0);
  class_addmethod(bibi_tilde_class, (t_method)bibi_tilde_q, gensym("Q"), A_DEFFLOAT, 0);
  class_addmethod(bibi_tilde_class, (t_method)bibi_tilde_reset, gensym("reset"), A_NULL);  
  class_addmethod(bibi_tilde_class, (t_method)bibi_tilde_setsr, gensym("setsr"), A_DEFFLOAT, 0);

  CLASS_MAINSIGNALIN(bibi_tilde_class, t_bibi_tilde, x_f);
}

}; /* end extern "C" */

