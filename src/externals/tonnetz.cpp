/**
 * @file tonnetz.cpp
 * @author Joseph Larralde
 * @date 07/06/2018
 * @brief generate scale patterns following tonnetz transitions
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
#include "../dependencies/jl.cpp.lib/control/harmony/Tonnetz.h"
#include <vector>

class PdTonnetz;

static t_class *tonnetz_class;

//----------------------------------------------------------------------------//

typedef struct _tonnetz {
  t_object x_obj;

  PdTonnetz *tonnetz;
  t_outlet *m_out;
} t_tonnetz;

//----------------------------------------------------------------------------//

class PdTonnetz : public jl::Tonnetz {
private:
  t_tonnetz *x;

public:
  PdTonnetz() : Tonnetz() {}
  virtual ~PdTonnetz() {}

  void setObject(t_tonnetz *obj) {
    x = obj;
  }
};

//----------------------------------------------------------------------------//

void tonnetz_bang(t_tonnetz *x) {
  int root = x->tonnetz->getRoot();
  std::vector<unsigned int> &pattern = x->tonnetz->getPattern();

  unsigned int patternLength = pattern.size();
  t_atom outv[patternLength];

  SETFLOAT((t_atom *)(&outv[0]), root);
  outlet_anything(x->m_out, gensym("root"), 1, (t_atom *)(&outv));

  for (unsigned int i = 0; i < patternLength; i++) {
    SETFLOAT((t_atom *)(&outv[i]), pattern[i]);
  }
  outlet_anything(x->m_out, gensym("pattern"), patternLength, (t_atom *)(&outv));
}

void tonnetz_root(t_tonnetz *x, t_floatarg f) {
  x->tonnetz->setRoot(static_cast<int>(f));
  tonnetz_bang(x);
}

void tonnetz_major(t_tonnetz *x, t_floatarg f) {
  x->tonnetz->setMajor(f != 0);
  tonnetz_bang(x);
}

void tonnetz_interval(t_tonnetz *x, t_floatarg f) {
  x->tonnetz->setInterval(static_cast<unsigned int>(f));
  tonnetz_bang(x);
}

void tonnetz_transpose(t_tonnetz *x, t_floatarg f) {
  x->tonnetz->transpose(static_cast<int>(f));
  tonnetz_bang(x);
}

void tonnetz_mode(t_tonnetz *x, t_symbol *s) {
  if (s == gensym("duad")) {
    x->tonnetz->setTonnetzMode(jl::TonnetzModeDuad);
  } else if (s == gensym("triad")) {
    x->tonnetz->setTonnetzMode(jl::TonnetzModeTriad);
  } else if (s == gensym("quad")) {
    x->tonnetz->setTonnetzMode(jl::TonnetzModeQuad);
  } else if (s == gensym("penta")) {
    x->tonnetz->setTonnetzMode(jl::TonnetzModePenta);
  } else if (s == gensym("scale")) {
    x->tonnetz->setTonnetzMode(jl::TonnetzModeScale);
  } else if (s == gensym("interval")) {
    x->tonnetz->setTonnetzMode(jl::TonnetzModeInterval);
  }
  tonnetz_bang(x);
}

void tonnetz_transition(t_tonnetz *x, t_symbol *s) {
  if (s == gensym("P")) {
    x->tonnetz->applyTransition(jl::TonnetzTransitionP);
  } else if (s == gensym("R")) {
    x->tonnetz->applyTransition(jl::TonnetzTransitionR);
  } else if (s == gensym("L")) {
    x->tonnetz->applyTransition(jl::TonnetzTransitionL);
  }
  tonnetz_bang(x);
}

void tonnetz_direction(t_tonnetz *x, t_symbol *s) {
  if (s == gensym("left")) {
    x->tonnetz->applyDirection(jl::TonnetzDirectionLeft);
  } else if (s == gensym("right")) {
    x->tonnetz->applyDirection(jl::TonnetzDirectionRight);
  } else if (s == gensym("backwards")) {
    x->tonnetz->applyDirection(jl::TonnetzDirectionBackwards);
  }
  tonnetz_bang(x);
}

//----------------------------------------------------------------------------//

void *tonnetz_new(t_symbol *s, int argc, t_atom *argv) {
  t_tonnetz *x = (t_tonnetz *)pd_new(tonnetz_class);

  x->tonnetz = new PdTonnetz();
  x->tonnetz->setObject(x);

  x->m_out = outlet_new(&x->x_obj, &s_anything);

  return (void *)x;
}

void tonnetz_free(t_tonnetz *x) {
  delete x->tonnetz;
}

extern "C" {

void tonnetz_setup(void) {
  tonnetz_class = class_new(gensym("tonnetz"),
                        (t_newmethod)tonnetz_new,
                        (t_method)tonnetz_free,
                        sizeof(t_tonnetz),
                        CLASS_DEFAULT,
                        A_GIMME,
                        0);

  class_addbang(tonnetz_class, tonnetz_bang);
  class_addmethod(tonnetz_class, (t_method)tonnetz_root, gensym("root"), A_DEFFLOAT, 0);
  class_addmethod(tonnetz_class, (t_method)tonnetz_major, gensym("major"), A_DEFFLOAT, 0);
  class_addmethod(tonnetz_class, (t_method)tonnetz_interval, gensym("interval"), A_DEFFLOAT, 0);
  class_addmethod(tonnetz_class, (t_method)tonnetz_transpose, gensym("transpose"), A_DEFFLOAT, 0);
  class_addmethod(tonnetz_class, (t_method)tonnetz_mode, gensym("mode"), A_DEFSYM, 0);
  class_addmethod(tonnetz_class, (t_method)tonnetz_transition, gensym("transition"), A_DEFSYM, 0);
  class_addmethod(tonnetz_class, (t_method)tonnetz_direction, gensym("direction"), A_DEFSYM, 0);
}

}; /* end extern "C" */
