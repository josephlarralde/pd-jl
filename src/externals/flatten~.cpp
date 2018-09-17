/**
 * @file flatten~.cpp
 * @author Joseph Larralde
 * @date 07/06/2018
 * @brief sidechain with threshold driven by the amplitude of a control signal
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

class PdFlattener;

static t_class *flatten_tilde_class;

typedef struct _flatten_tilde {

  t_object x_obj;

  // this is used in setup function to give a handle on the leftmost signal inlet
  float x_f;

  PdFlattener *flattener;
  t_inlet *x_in2;
  t_outlet *x_out;

} t_flatten_tilde;

//============================================================================//

// This code is pretty much duplicated from the more advanced Compressor class
// in jl.cpp.lib, as it is not easy to have a variable number of signal outlets
// from an arg, need to use variadic or something with the dsp_add method ...

// NB : we could also have signal inlets for thresh, ratio, knee and makeup gain
// control. nah ?

class PdFlattener {
private:
  t_flatten_tilde *x;

  jl::LogDomainFlattener flattener;

  jl::Ramp<float, jl::sample> rMakeUp;
  jl::Ramp<float, jl::sample> rRatio;
  jl::Ramp<float, jl::sample> rKnee;

  float samplingRate;
  float rampDuration;
  unsigned long rampSamples;

public:
  PdFlattener() :
  rampDuration(10) {
    setSamplingRate(44100);
  }

  ~PdFlattener() {}

  void setObject(t_flatten_tilde *obj) {
    x = obj;
  }

  void setSamplingRate(float sr) {
    samplingRate = sr;
    flattener.setSamplingRate(samplingRate);
    rampSamples = static_cast<unsigned long>(rampDuration * samplingRate * 0.001);    
  }

  void setMakeUp(float m) { rMakeUp.ramp(m, rampSamples); }
  void setRatio(float r) { rRatio.ramp(r, rampSamples); }
  void setKnee(float k) { rKnee.ramp(k, rampSamples); }
  void setAttack(float a) { flattener.setAttack(a); }
  void setRelease(float r) { flattener.setRelease(r); }

  void process(float *in1, float *in2, float *out, unsigned int blockSize) {
    jl::sample *m = rMakeUp.process(blockSize);
    jl::sample *r = rRatio.process(blockSize);
    jl::sample *k = rKnee.process(blockSize);

    for (unsigned int i = 0; i < blockSize; ++i) {
      jl::sample g = flattener.process(
        static_cast<jl::sample>(in1[i]),
        static_cast<jl::sample>(in2[i]),
        m[i], r[i], k[i]
      );
      out[i] = static_cast<float>(g);
    }
  }
};

//============================================================================//
// THESE ARE THE ACTUAL OBJECT'S METHODS :
//============================================================================//

void flatten_tilde_makeup(t_flatten_tilde *x, t_floatarg f) {
  x->flattener->setMakeUp(f);
}

void flatten_tilde_ratio(t_flatten_tilde *x, t_floatarg f) {
  x->flattener->setRatio(f);
}

void flatten_tilde_knee(t_flatten_tilde *x, t_floatarg f) {
  x->flattener->setKnee(f);
}

void flatten_tilde_attack(t_flatten_tilde *x, t_floatarg f) {
  x->flattener->setAttack(f);
}

void flatten_tilde_release(t_flatten_tilde *x, t_floatarg f) {
  x->flattener->setRelease(f);
}

void flatten_tilde_setsr(t_flatten_tilde *x, t_floatarg f) {
  x->flattener->setSamplingRate(f);
}

//============================ DSP OPERATIONS ================================//

t_int *flatten_tilde_perform(t_int *w) {
  t_flatten_tilde *x = (t_flatten_tilde *)(w[1]);
  t_sample *in1 = (t_sample *)(w[2]);
  t_sample *in2 = (t_sample *)(w[3]);
  t_sample *out = (t_sample *)(w[4]);
  int n = (int)(w[5]); // VECTOR SIZE

  x->flattener->process(in1, in2, out, n);

  return (w + 6);
}

void flatten_tilde_dsp(t_flatten_tilde *x, t_signal **sp) {
  
  flatten_tilde_setsr(x, sys_getsr());

  dsp_add(flatten_tilde_perform, 5, x,
          sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);

}

//======================= CONSTRUCTOR / DESTRUCTOR ===========================//

void *flatten_tilde_new(t_symbol *s, int argc, t_atom *argv) {
  /*
   * call the "constructor" of the parent-class
   * this will reserve enough memory to hold "t_flatten_tilde"
   */
  t_flatten_tilde *x = (t_flatten_tilde *)pd_new(flatten_tilde_class);

  x->flattener = new PdFlattener();
  x->flattener->setObject(x);

  flatten_tilde_setsr(x, sys_getsr());

  x->x_in2 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  x->x_out = outlet_new(&x->x_obj, &s_signal);

  return (void *)x;
}

void flatten_tilde_free(t_flatten_tilde *x) {
  delete x->flattener;
  inlet_free(x->x_in2);
  outlet_free(x->x_out);
}

//============================ SETUP FUNCTION ================================//

extern "C" {

/**
 * define the function-space of the class
 * within a single-object external the name of this function is special
 */
void flatten_tilde_setup(void) {
  /* create a new class */
  flatten_tilde_class = class_new(gensym("flatten~"), /* the object's name is "flatten~" */
    (t_newmethod)flatten_tilde_new,                 /* the object's constructor is "flatten_tilde_new()" */
    (t_method)flatten_tilde_free,                   /* the object's destructor */
    sizeof(t_flatten_tilde),                        /* the size of the data-space */
    CLASS_DEFAULT,                                   /* a normal pd object */
    A_GIMME,                                         /* creation arg(s) type(s) */
    0);                                              /* end creation args */

  // class_addbang(flatten_tilde_class, flatten_tilde_bang);
  class_addmethod(flatten_tilde_class, (t_method)flatten_tilde_dsp, gensym("dsp"), A_NULL);
  class_addmethod(flatten_tilde_class, (t_method)flatten_tilde_makeup, gensym("makeup"), A_DEFFLOAT, 0);
  class_addmethod(flatten_tilde_class, (t_method)flatten_tilde_ratio, gensym("ratio"), A_DEFFLOAT, 0);
  class_addmethod(flatten_tilde_class, (t_method)flatten_tilde_knee, gensym("knee"), A_DEFFLOAT, 0);
  class_addmethod(flatten_tilde_class, (t_method)flatten_tilde_attack, gensym("attack"), A_DEFFLOAT, 0);
  class_addmethod(flatten_tilde_class, (t_method)flatten_tilde_release, gensym("release"), A_DEFFLOAT, 0);
  class_addmethod(flatten_tilde_class, (t_method)flatten_tilde_setsr, gensym("setsr"), A_DEFFLOAT, 0);

  CLASS_MAINSIGNALIN(flatten_tilde_class, t_flatten_tilde, x_f);
}

}; /* end extern "C" */
