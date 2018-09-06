/**
 * @file hann~.cpp
 * @author Joseph Larralde
 * @date 07/06/2018
 * @brief hann wavetable
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

// kept here for reference, not included in jl.pd.lib
// as the same result can be achieved using vanilla objects

#include "m_pd.h"
#include "../dependencies/jl.cpp.lib/dsp/synthesis/Oscillator.h"

class PdHann;

static t_class *hann_tilde_class;

typedef struct _hann_tilde {
  t_object x_obj;
  t_sample x_f; // this is used in setup function

  PdHann *osc;
  t_outlet *x_out;
  // t_outlet *f_out;
} t_hann_tilde;

//============================================================================//

class PdHann : public jl::WavetableOscillator {
private:
  t_hann_tilde *x;

public:
  PdHann() :
  WavetableOscillator(jl::OscShapeHann, jl::OscControlPhase) {}

  virtual ~PdHann() {}

  void setObject(t_hann_tilde *obj) {
    x = obj;
  }
};

//============================================================================//

void hann_tilde_setsr(t_hann_tilde *x, t_floatarg f) {
  x->osc->setSamplingRate(f);
}

//============================ DSP OPERATIONS ================================//

t_int *hann_tilde_perform(t_int *w) {
  t_hann_tilde *x = (t_hann_tilde *)(w[1]);
  t_sample *in = (t_sample *)(w[2]);
  t_sample *out = (t_sample *)(w[3]);
  int n = (int)(w[4]); // VECTOR SIZE

  x->osc->process(static_cast<jl::sample *>(in), static_cast<jl::sample *>(out), n);

  return (w + 5);
}

void hann_tilde_dsp(t_hann_tilde *x, t_signal **sp) {
  hann_tilde_setsr(x, sys_getsr());

  dsp_add(hann_tilde_perform, 4, x,
          sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);

}

//======================= CONSTRUCTOR / DESTRUCTOR ===========================//

void *hann_tilde_new(t_symbol *s, int argc, t_atom *argv) {
  /*
   * call the "constructor" of the parent-class
   * this will reserve enough memory to hold "t_hann_tilde"
   */
  t_hann_tilde *x = (t_hann_tilde *)pd_new(hann_tilde_class);

  x->osc = new PdHann();
  x->osc->setObject(x);

  hann_tilde_setsr(x, sys_getsr());

  x->x_out = outlet_new(&x->x_obj, &s_signal);
  // x->f_out = outlet_new(&x->x_obj, &s_float);

  return (void *)x;
}

void hann_tilde_free(t_hann_tilde *x) {
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
void hann_tilde_setup(void) {
  /* create a new class */
  hann_tilde_class = class_new(gensym("hann~"),   /* the object's name is "hann~" */
    (t_newmethod)hann_tilde_new,                  /* the object's constructor is "hann_tilde_new()" */
    (t_method)hann_tilde_free,                    /* the object's destructor */
    sizeof(t_hann_tilde),                         /* the size of the data-space */
    CLASS_DEFAULT,                                /* a normal pd object */
    A_GIMME,                                      /* args (giv'em to me) */
    0);                                           /* no creation arguments ? */

  class_addmethod(hann_tilde_class, (t_method)hann_tilde_dsp, gensym("dsp"), A_NULL);
  class_addmethod(hann_tilde_class, (t_method)hann_tilde_setsr, gensym("setsr"), A_DEFFLOAT, 0);

  CLASS_MAINSIGNALIN(hann_tilde_class, t_hann_tilde, x_f);
}

}; /* end extern "C" */
