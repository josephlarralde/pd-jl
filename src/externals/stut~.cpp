/**
 * @file stut~.cpp
 * @author Joseph Larralde
 * @date 07/06/2018
 * @brief stutter effect
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
#include "../dependencies/jl.cpp.lib/dsp/effects/Stut.h"

#define JL_STUT_DEFAULT_BUFFER_DURATION 1000

class PdStut;

static t_class *stut_tilde_class;

typedef struct _stut_tilde {
  t_object x_obj;
  t_sample x_f; // this is used in setup function

  PdStut *stutter;
  t_outlet *x_out;
  t_outlet *f_out;
} t_stut_tilde;

//============================================================================//

class PdStut : public jl::Stut {
private:
  t_stut_tilde *x;

public:
  PdStut(float bDuration, unsigned int c = 1) :
  Stut(bDuration, c) {}

  virtual ~PdStut() {}

  void setObject(t_stut_tilde *obj) {
    x = obj;
  }

  void endReachCallback(int endReachType) {
    outlet_float(x->f_out, endReachType);
  }
};

//============================================================================//

void stut_tilde_bang(t_stut_tilde *x) {
  x->stutter->start();
}

void stut_tilde_stop(t_stut_tilde *x) {
  x->stutter->stop();
}

//=========================== PARAMETER SETTERS ==============================//

void stut_tilde_slice_duration(t_stut_tilde *x, t_floatarg f) {
  x->stutter->setSliceDuration(static_cast<float>(f));
}

void stut_tilde_slices(t_stut_tilde *x, t_floatarg f) {
  x->stutter->setSlices((long) f);
}

void stut_tilde_fade(t_stut_tilde *x, t_floatarg f) {
  x->stutter->setFades(static_cast<float>(f));
}

void stut_tilde_fadi(t_stut_tilde *x, t_floatarg f) {
  x->stutter->setFadeIn(static_cast<float>(f));
}

void stut_tilde_fado(t_stut_tilde *x, t_floatarg f) {
  x->stutter->setFadeOut(static_cast<float>(f));
}

void stut_tilde_interrupt(t_stut_tilde *x, t_floatarg f) {
  x->stutter->setInterrupt(static_cast<float>(f));
}

void stut_tilde_release(t_stut_tilde *x, t_floatarg f) {
  x->stutter->setRelease(static_cast<float>(f));
}

void stut_tilde_setsr(t_stut_tilde *x, t_floatarg f) {
  x->stutter->setSamplingRate(static_cast<float>(f));
}

//============================ DSP OPERATIONS ================================//

t_int *stut_tilde_perform(t_int *w) {
  t_stut_tilde *x = (t_stut_tilde *)(w[1]);
  t_sample *in = (t_sample *)(w[2]);
  t_sample *out = (t_sample *)(w[3]);
  int n = (int)(w[4]);

  jl::sample **ins = (jl::sample **)(&in);
  jl::sample **outs = (jl::sample **)(&out);

  x->stutter->process(ins, outs, n);

  return (w + 5);
}


void stut_tilde_dsp(t_stut_tilde *x, t_signal **sp) {
  stut_tilde_setsr(x, sys_getsr());

  dsp_add(stut_tilde_perform, 4, x,
          sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

//======================= CONSTRUCTOR / DESTRUCTOR ===========================//

void *stut_tilde_new(t_symbol *s, int argc, t_atom *argv) {
  t_stut_tilde *x = (t_stut_tilde *)pd_new(stut_tilde_class);

  float bufferDuration = JL_STUT_DEFAULT_BUFFER_DURATION;

  if (argc > 0) {
    bufferDuration = atom_getfloat(argv);
  }

  x->stutter = new PdStut(bufferDuration, 1);
  x->stutter->setObject(x);

  stut_tilde_setsr(x, sys_getsr());

  x->x_out = outlet_new(&x->x_obj, &s_signal);
  x->f_out = outlet_new(&x->x_obj, &s_float);

  return (void *)x;
}

void stut_tilde_free(t_stut_tilde *x) {
  delete x->stutter;
  outlet_free(x->x_out);
  outlet_free(x->f_out);
}

//============================ SETUP FUNCTION ================================//

extern "C" {

void stut_tilde_setup(void) {
  stut_tilde_class = class_new(gensym("stut~"), /* the object's name is "stut~" */
   (t_newmethod)stut_tilde_new,                 /* the object's constructor is "stut_tilde_new()" */
   (t_method)stut_tilde_free,                   /* the object's destructor */
   sizeof(t_stut_tilde),                        /* the size of the data-space */
   CLASS_DEFAULT,                               /* a normal pd object */
   A_GIMME,                                     /* arg type (internal buffer duration in ms) */
   0);                                          /* no creation arguments ? */

  class_addbang(stut_tilde_class, stut_tilde_bang);
  class_addmethod(stut_tilde_class, (t_method)stut_tilde_dsp, gensym("dsp"), A_NULL);
  class_addmethod(stut_tilde_class, (t_method)stut_tilde_stop, gensym("stop"), A_NULL);
  class_addmethod(stut_tilde_class, (t_method)stut_tilde_slice_duration, gensym("duration"), A_DEFFLOAT, 0);
  class_addmethod(stut_tilde_class, (t_method)stut_tilde_slices, gensym("slices"), A_DEFFLOAT, 0);
  class_addmethod(stut_tilde_class, (t_method)stut_tilde_fade, gensym("fade"), A_DEFFLOAT, 0);
  class_addmethod(stut_tilde_class, (t_method)stut_tilde_fadi, gensym("fadi"), A_DEFFLOAT, 0);
  class_addmethod(stut_tilde_class, (t_method)stut_tilde_fado, gensym("fado"), A_DEFFLOAT, 0);
  class_addmethod(stut_tilde_class, (t_method)stut_tilde_interrupt, gensym("interrupt"), A_DEFFLOAT, 0);
  class_addmethod(stut_tilde_class, (t_method)stut_tilde_release, gensym("release"), A_DEFFLOAT, 0);
  class_addmethod(stut_tilde_class, (t_method)stut_tilde_setsr, gensym("setsr"), A_DEFFLOAT, 0);

  CLASS_MAINSIGNALIN(stut_tilde_class, t_stut_tilde, x_f);
}

}; /* end extern "C" */
