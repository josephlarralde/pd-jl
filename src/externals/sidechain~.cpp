/**
 * @file sidechain~.cpp
 * @author Joseph Larralde
 * @date 07/06/2018
 * @brief a sidechain to be used in dynamic range compression systems
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
#include "../dependencies/jl.cpp.lib/dsp/effects/Compress.h"

class PdSideChain;

static t_class *sidechain_tilde_class;

typedef struct _sidechain_tilde {

  t_object x_obj;

  // this is used in setup function to give a handle on the leftmost signal inlet
  float x_f;

  PdSideChain *sidechain;
  t_outlet *x_out;

} t_sidechain_tilde;

//============================================================================//

// This code is pretty much duplicated from the more advanced Compressor class
// in jl.cpp.lib, as it is not easy to have a variable number of signal outlets
// from an arg, need to use variadic or something with the dsp_add method ...

// NB : we could also have signal inlets for thresh, ratio, knee and makeup gain
// control. nah ?

class PdSideChain {
private:
  t_sidechain_tilde *x;

  jl::LogDomainSideChain sc;

  jl::Ramp<float, jl::sample> rMakeUp;
  jl::Ramp<float, jl::sample> rThreshold;
  jl::Ramp<float, jl::sample> rRatio;
  jl::Ramp<float, jl::sample> rKnee;

  float samplingRate;
  float rampDuration;
  unsigned long rampSamples;

public:
  PdSideChain() :
  rampDuration(10) {
    setSamplingRate(44100);
  }

  ~PdSideChain() {}

  void setObject(t_sidechain_tilde *obj) {
    x = obj;
  }

  void setSamplingRate(float sr) {
    samplingRate = sr;
    sc.setSamplingRate(samplingRate);
    rampSamples = static_cast<unsigned long>(rampDuration * samplingRate * 0.001);    
  }

  void setMakeUp(float m) {
    rMakeUp.ramp(m, rampSamples);
    // sc.setMakeUp(m);
  }

  void setThreshold(float t) {
    rThreshold.ramp(t, rampSamples);
    // sc.setThreshold(t);
  }

  void setRatio(float r) {
    rRatio.ramp(r, rampSamples);
    // sc.setRatio(r);
  }

  void setKnee(float k) {
    rKnee.ramp(k, rampSamples);
    // sc.setKnee(k);
  }

  void setAttack(float a) {
    sc.setAttack(a);
  }

  void setRelease(float r) {
    sc.setRelease(r);
  }

  void process(float *in, float *out, unsigned int blockSize) {
    jl::sample *m = rMakeUp.process(blockSize);
    jl::sample *t = rThreshold.process(blockSize);
    jl::sample *r = rRatio.process(blockSize);
    jl::sample *k = rKnee.process(blockSize);

    for (unsigned int i = 0; i < blockSize; ++i) {
      jl::sample g = sc.process(static_cast<jl::sample>(in[i]), m[i], t[i], r[i], k[i]);
      out[i] = static_cast<float>(g);
    }
  }
};

//============================================================================//
// THESE ARE THE ACTUAL OBJECT'S METHODS :
//============================================================================//

void sidechain_tilde_makeup(t_sidechain_tilde *x, t_floatarg f) {
  x->sidechain->setMakeUp(f);
}

void sidechain_tilde_threshold(t_sidechain_tilde *x, t_floatarg f) {
  x->sidechain->setThreshold(f);
}

void sidechain_tilde_ratio(t_sidechain_tilde *x, t_floatarg f) {
  x->sidechain->setRatio(f);
}

void sidechain_tilde_knee(t_sidechain_tilde *x, t_floatarg f) {
  x->sidechain->setKnee(f);
}

void sidechain_tilde_attack(t_sidechain_tilde *x, t_floatarg f) {
  x->sidechain->setAttack(f);
}

void sidechain_tilde_release(t_sidechain_tilde *x, t_floatarg f) {
  x->sidechain->setRelease(f);
}

void sidechain_tilde_setsr(t_sidechain_tilde *x, t_floatarg f) {
  x->sidechain->setSamplingRate(f);
}

//============================ DSP OPERATIONS ================================//

t_int *sidechain_tilde_perform(t_int *w) {
  t_sidechain_tilde *x = (t_sidechain_tilde *)(w[1]);
  t_sample *in = (t_sample *)(w[2]);
  t_sample *out = (t_sample *)(w[3]);
  int n = (int)(w[4]); // VECTOR SIZE

  x->sidechain->process(in, out, n);

  return (w + 5);
}

void sidechain_tilde_dsp(t_sidechain_tilde *x, t_signal **sp) {
  
  sidechain_tilde_setsr(x, sys_getsr());

  dsp_add(sidechain_tilde_perform, 4, x,
          sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);

}

//======================= CONSTRUCTOR / DESTRUCTOR ===========================//

void *sidechain_tilde_new(t_symbol *s, int argc, t_atom *argv) {
  t_sidechain_tilde *x = (t_sidechain_tilde *)pd_new(sidechain_tilde_class);

  x->sidechain = new PdSideChain();
  x->sidechain->setObject(x);

  sidechain_tilde_setsr(x, sys_getsr());

  x->x_out = outlet_new(&x->x_obj, &s_signal);

  return (void *)x;
}

void sidechain_tilde_free(t_sidechain_tilde *x) {
  delete x->sidechain;
  outlet_free(x->x_out);
}

//============================ SETUP FUNCTION ================================//

extern "C" {

/**
 * define the function-space of the class
 * within a single-object external the name of this function is special
 */
void sidechain_tilde_setup(void) {
  /* create a new class */
  sidechain_tilde_class = class_new(gensym("sidechain~"), /* the object's name is "compress~" */
    (t_newmethod)sidechain_tilde_new,                 /* the object's constructor is "sidechain_tilde_new()" */
    (t_method)sidechain_tilde_free,                   /* the object's destructor */
    sizeof(t_sidechain_tilde),                        /* the size of the data-space */
    CLASS_DEFAULT,                                   /* a normal pd object */
    A_GIMME,                                         /* creation arg(s) type(s) */
    0);                                              /* end creation arguments */

  // class_addbang(sidechain_tilde_class, sidechain_tilde_bang);
  class_addmethod(sidechain_tilde_class, (t_method)sidechain_tilde_dsp, gensym("dsp"), A_NULL);
  class_addmethod(sidechain_tilde_class, (t_method)sidechain_tilde_makeup, gensym("makeup"), A_DEFFLOAT, 0);
  class_addmethod(sidechain_tilde_class, (t_method)sidechain_tilde_threshold, gensym("threshold"), A_DEFFLOAT, 0);
  class_addmethod(sidechain_tilde_class, (t_method)sidechain_tilde_ratio, gensym("ratio"), A_DEFFLOAT, 0);
  class_addmethod(sidechain_tilde_class, (t_method)sidechain_tilde_knee, gensym("knee"), A_DEFFLOAT, 0);
  class_addmethod(sidechain_tilde_class, (t_method)sidechain_tilde_attack, gensym("attack"), A_DEFFLOAT, 0);
  class_addmethod(sidechain_tilde_class, (t_method)sidechain_tilde_release, gensym("release"), A_DEFFLOAT, 0);
  class_addmethod(sidechain_tilde_class, (t_method)sidechain_tilde_setsr, gensym("setsr"), A_DEFFLOAT, 0);

  CLASS_MAINSIGNALIN(sidechain_tilde_class, t_sidechain_tilde, x_f);
}

}; /* end extern "C" */
