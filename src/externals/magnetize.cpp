/**
 * @file magnetize.cpp
 * @author Joseph Larralde
 * @date 07/06/2018
 * @brief map an incoming float on a scale defined by a repeating pattern
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
#include "../dependencies/jl.cpp.lib/control/harmony/PatternMap.h"

class PdPatternMap;

static t_class *magnetize_class;

//----------------------------------------------------------------------------//

typedef struct _magnetize {
  t_object x_obj;
  float lastValueIn;

  PdPatternMap *map;
  unsigned int pattern[JL_MAX_PATTERN_MAP_LENGTH];  
  t_outlet *f_out;
} t_magnetize;

//----------------------------------------------------------------------------//

class PdPatternMap : public jl::PatternMap<float> {
private:
  t_magnetize *x;

public:
  PdPatternMap() : PatternMap<float>() {}
  virtual ~PdPatternMap() {}

  void setObject(t_magnetize *obj) {
    x = obj;
  }
};

//----------------------------------------------------------------------------//

void magnetize_float(t_magnetize *x, t_floatarg f) {
  x->lastValueIn = static_cast<float>(f);
  outlet_float(x->f_out, x->map->process(x->lastValueIn));
}

void magnetize_bang(t_magnetize *x) {
  outlet_float(x->f_out, x->map->process(x->lastValueIn));
}

void magnetize_root(t_magnetize *x, t_floatarg f) {
  x->map->setRoot(static_cast<float>(f));
}

void magnetize_factor(t_magnetize *x, t_floatarg f) {
  x->map->setFactor(static_cast<float>(f));
}

void magnetize_pattern(t_magnetize *x, t_symbol *s, int argc, t_atom *argv) {
  if (s == gensym("pattern")) {
    for (int i = 0; i < argc; ++i) {
      x->pattern[i] = static_cast<unsigned int>(atom_getfloat(argv + i));
    }
  }

  x->map->setPattern(&(x->pattern[0]), argc);
}

//----------------------------------------------------------------------------//

void *magnetize_new(t_symbol *s, int argc, t_atom *argv) {
  t_magnetize *x = (t_magnetize *)pd_new(magnetize_class);

  x->lastValueIn = 0;

  x->map = new PdPatternMap();
  x->map->setObject(x);

  x->f_out = outlet_new(&x->x_obj, &s_float);

  return (void *)x;
}

void magnetize_free(t_magnetize *x) {
  delete x->map;
  // outlet_free(x->f_out);
}

extern "C" {

void magnetize_setup(void) {
  magnetize_class = class_new(gensym("magnetize"),
                        (t_newmethod)magnetize_new,
                        (t_method)magnetize_free,
                        sizeof(t_magnetize),
                        CLASS_DEFAULT,
                        A_GIMME,
                        0);

  class_addbang(magnetize_class, magnetize_bang);
  class_addfloat(magnetize_class, magnetize_float);
  class_addmethod(magnetize_class, (t_method)magnetize_root, gensym("root"), A_DEFFLOAT, 0);
  class_addmethod(magnetize_class, (t_method)magnetize_factor, gensym("magnetism"), A_DEFFLOAT, 0);
  class_addanything(magnetize_class, magnetize_pattern);
}

}; /* end extern "C" */
