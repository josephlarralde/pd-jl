/**
 * @file router~.cpp
 * @author Joseph Larralde
 * @date 05/08/2023
 * @brief something like [matrix~] from Max/MSP and Cyclone
 *
 * @copyright
 * Copyright (C) 2023 by Joseph Larralde.
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

#include <vector>
#include <cstdlib>
#include <ctime>
#include "m_pd.h"
#include "../dependencies/cpp-jl/src/dsp/utilities/Ramp.h"


static t_class *router_tilde_class;

typedef struct _router_tilde {
  t_object x_obj;

  // this can be used in setup function as a signal proxy for the default inlet
  // (using CLASS_MAINSIGNALIN macro), but we don't need it here :
  // float x_f;

  unsigned long sr;

  unsigned int nb_s_inlets;
  unsigned int nb_s_outlets;
  unsigned int nb_s_xlets;

  // in ms
  float fadi;
  float fado;

  // in samples
  unsigned long sfadi;
  unsigned long sfado;

  std::vector<t_sample*> s_inputs;
  std::vector<t_sample*> s_outputs;
  std::vector<jl::Ramp<t_sample, t_sample>> ramps;

  std::vector<std::vector<t_sample>> s_tmp_outputs;

  std::vector<t_inlet*> s_inlets;
  std::vector<t_outlet*> s_outlets;

  t_outlet* f_out; // for dumping info or whatever

} t_router_tilde;

//------------------------------ UTILITIES -----------------------------------//

void router_tilde_update_sfades(t_router_tilde* x) {
  x->sfadi = x->fadi * x->sr * 0.001;
  x->sfado = x->fado * x->sr * 0.001;
}

//--------------------------- OBJECT MESSAGES --------------------------------//

void router_tilde_list(t_router_tilde* x, t_symbol* s, int argc, t_atom* argv) {
  if (argc > 2) {
    auto i = static_cast<unsigned int>(atom_getfloat(argv));
    auto o = static_cast<unsigned int>(atom_getfloat(argv + 1));
    t_sample v = (static_cast<unsigned int>(atom_getfloat(argv + 2)) != 0) ? 1 : 0;
    unsigned long sfade = (v == 1) ? x->sfadi : x->sfado;

    auto rampIndex = i + o * x->nb_s_inlets;
    x->ramps[rampIndex].ramp(v, sfade);
  }
}

void router_tilde_anything(t_router_tilde* x, t_symbol* s, int argc, t_atom* argv) {
  if (argc > 0) {
    if (s == gensym("fadi")) {
      x->fadi = atom_getfloat(argv);
      router_tilde_update_sfades(x);
    } else if (s == gensym("fado")) {
      x->fado = atom_getfloat(argv);
      router_tilde_update_sfades(x);
    } else if (s == gensym("fade")) {
      x->fadi = x->fado = atom_getfloat(argv);
      router_tilde_update_sfades(x);
    }
  }
}

//---------------------------- DSP OPERATIONS --------------------------------//

t_int* router_tilde_perform(t_int *w) {
  t_router_tilde *x = (t_router_tilde *)(w[1]);
  int vecSize = (int)(w[2]); // VECTOR SIZE

  for (unsigned int i = 0; i < x->nb_s_outlets; ++i) {
    std::fill(x->s_tmp_outputs[i].begin(), x->s_tmp_outputs[i].end(), 0.0f);

    for (unsigned int j = 0; j < x->nb_s_inlets; ++j) {
      auto rampIndex = j + i * x->nb_s_inlets;
      auto rampBlock = x->ramps[rampIndex].process(vecSize);

      for (auto n = 0; n < vecSize; ++n) {
        x->s_tmp_outputs[i][n] += x->s_inputs[j][n] * rampBlock[n];
      }
    }
  }

  for (unsigned int i = 0; i < x->nb_s_outlets; ++i) {
    for (auto n = 0; n < vecSize; ++n) {
      // this is because we might overwrite inlet values too early if we write
      // into some outlets before we compute all the outputs in a (tmp) buffer
      x->s_outputs[i][n] = x->s_tmp_outputs[i][n];
    }
  }

  return (w + x->nb_s_xlets + 3);
}

void router_tilde_dsp(t_router_tilde *x, t_signal **sp) {
  x->sr = sys_getsr();
  router_tilde_update_sfades(x);

  for (auto i = 0; i < x->nb_s_inlets; ++i) {
    x->s_inputs[i] = sp[i]->s_vec;
  }

  for (auto i = 0; i < x->nb_s_outlets; ++i) {
    x->s_outputs[i] = sp[x->nb_s_inlets + i]->s_vec;
    x->s_tmp_outputs[i] = std::vector<t_sample>(sp[0]->s_n);
  }

  dsp_add(router_tilde_perform, x->nb_s_xlets + 2, x, sp[0]->s_n);
}

//----------------------- CONSTRUCTOR / DESTRUCTOR ---------------------------//

void* router_tilde_new(t_symbol* s, int argc, t_atom* argv) {
  t_router_tilde* x = (t_router_tilde*) pd_new(router_tilde_class);

  x->nb_s_inlets = 0;
  x->nb_s_outlets = 0;
  x->nb_s_xlets = 0;

  if (argc > 1) {
    x->nb_s_inlets = static_cast<unsigned int>(atom_getfloat(argv));
    x->nb_s_outlets = static_cast<unsigned int>(atom_getfloat(argv + 1));
    x->nb_s_xlets = x->nb_s_inlets + x->nb_s_outlets;
  }

  x->s_inputs.resize(x->nb_s_inlets);
  x->s_inlets.resize(x->nb_s_inlets);

  for (auto i = 0; i < x->nb_s_inlets; ++i) {
    x->s_inlets[i] = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  }

  x->s_outputs.resize(x->nb_s_outlets);
  x->s_outlets.resize(x->nb_s_outlets);

  for (auto i = 0; i < x->nb_s_outlets; ++i) {
    x->s_outlets[i] = outlet_new(&x->x_obj, &s_signal);
  }

  x->ramps.resize(x->nb_s_inlets * x->nb_s_outlets);
  x->s_tmp_outputs.resize(x->nb_s_outlets);

  x->f_out = outlet_new(&x->x_obj, &s_anything);

  return (void*) x;
}

void router_tilde_free(t_router_tilde* x) {
  // nothing to free for now
}

//============================ SETUP FUNCTION ================================//

extern "C" {

/**
 * define the function-space of the class
 * within a single-object external the name of this function is special
 */
void router_tilde_setup(void) {

  router_tilde_class = class_new(
    gensym("router~"),              /* the object's name is "router~" */
    (t_newmethod)router_tilde_new,  /* the object's constructor */
    (t_method)router_tilde_free,    /* the object's destructor */
    sizeof(t_router_tilde),         /* the size of the data-space */
    CLASS_DEFAULT,                  /* a normal pd object */
    A_GIMME,                        /* creation arg(s) type(s) */
    0                               /* end creation args */
  );

  class_addlist(router_tilde_class, router_tilde_list);
  class_addanything(router_tilde_class, router_tilde_anything);
  class_addmethod(router_tilde_class, (t_method)router_tilde_dsp, gensym("dsp"), A_NULL);
}

}; /* end extern "C" */
