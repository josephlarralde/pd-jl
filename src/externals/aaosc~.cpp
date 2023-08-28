/**
 * @file aaosc~.cpp
 * @author Joseph Larralde
 * @date 07/06/2018
 * @brief pretty much the same as pd's osc~ object
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

// pretty much the same as pd's osc~ object (but based on jl.cpp.lib)
// kept here for reference, not included in jl.pd.lib

#include "m_pd.h"
#include "../dependencies/cpp-jl/dsp/synthesis/Oscillator.h"

class PdOsc;

static t_class *aaosc_tilde_class;

typedef struct _aaosc_tilde {
  t_object x_obj;
  t_sample x_f; // this is used in setup function

  PdOsc *osc;
  t_outlet *x_out;
  // t_outlet *f_out;
} t_aaosc_tilde;

//============================================================================//

class PdOsc : public jl::WavetableOscillator {
private:
  t_aaosc_tilde *x;

public:
  PdOsc() :
  WavetableOscillator() {}

  virtual ~PdOsc() {}

  void setObject(t_aaosc_tilde *obj) {
    x = obj;
  }
};

//============================================================================//

void aaosc_tilde_setsr(t_aaosc_tilde *x, t_floatarg f) {
  x->osc->setSamplingRate(f);
}

//============================ DSP OPERATIONS ================================//

t_int *aaosc_tilde_perform(t_int *w) {
  t_aaosc_tilde *x = (t_aaosc_tilde *)(w[1]);
  t_sample *in = (t_sample *)(w[2]);
  t_sample *out = (t_sample *)(w[3]);
  int n = (int)(w[4]); // VECTOR SIZE

  x->osc->process(static_cast<jl::sample *>(in), static_cast<jl::sample *>(out), n);

  return (w + 5);
}

void aaosc_tilde_dsp(t_aaosc_tilde *x, t_signal **sp) {
  aaosc_tilde_setsr(x, sys_getsr());

  dsp_add(aaosc_tilde_perform, 4, x,
          sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);

}

//======================= CONSTRUCTOR / DESTRUCTOR ===========================//

void *aaosc_tilde_new(t_symbol *s, int argc, t_atom *argv) {
  /*
   * call the "constructor" of the parent-class
   * this will reserve enough memory to hold "t_aaosc_tilde"
   */
  t_aaosc_tilde *x = (t_aaosc_tilde *)pd_new(aaosc_tilde_class);

  x->osc = new PdOsc();
  x->osc->setObject(x);

  aaosc_tilde_setsr(x, sys_getsr());

  x->x_out = outlet_new(&x->x_obj, &s_signal);
  // x->f_out = outlet_new(&x->x_obj, &s_float);

  return (void *)x;
}

void aaosc_tilde_free(t_aaosc_tilde *x) {
  delete x->osc;
  outlet_free(x->x_out);
  // outlet_free(x->f_out);
}

//============================ SETUP FUNCTION ================================//

extern "C" {

/**
 * define the function-space of the class
 * within a single-object external the name of this function is special
 */
void aaosc_tilde_setup(void) {
  /* create a new class */
  aaosc_tilde_class = class_new(gensym("aaosc~"), /* the object's name is "osc~" */
    (t_newmethod)aaosc_tilde_new,                 /* the object's constructor is "aaosc_tilde_new()" */
    (t_method)aaosc_tilde_free,                   /* the object's destructor */
    sizeof(t_aaosc_tilde),                        /* the size of the data-space */
    CLASS_DEFAULT,                                /* a normal pd object */
    A_GIMME,                                      /* args (giv'em to me) */
    0);                                           /* no creation arguments ? */

  class_addmethod(aaosc_tilde_class, (t_method)aaosc_tilde_dsp, gensym("dsp"), A_NULL);
  class_addmethod(aaosc_tilde_class, (t_method)aaosc_tilde_setsr, gensym("setsr"), A_DEFFLOAT, 0);

  CLASS_MAINSIGNALIN(aaosc_tilde_class, t_aaosc_tilde, x_f);
}

}; /* end extern "C" */
