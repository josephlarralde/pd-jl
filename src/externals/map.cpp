/**
 * @file map.cpp
 * @author Joseph Larralde
 * @date 07/06/2018
 * @brief interval mapper using reshaping functions
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
#include "../dependencies/cpp-jl/src/control/utilities/IntervalMap.h"

class PdIntervalMap;

static t_class *map_class;

//----------------------------------------------------------------------------//

typedef struct _map {
  t_object x_obj;
  float lastValueIn;
  PdIntervalMap *map;
  t_outlet *f_out;
} t_map;

//----------------------------------------------------------------------------//

class PdIntervalMap : public jl::IntervalMap<float> {
private:
  t_map *x;

public:
  PdIntervalMap() : IntervalMap<float>() {}
  virtual ~PdIntervalMap() {}

  void setObject(t_map *obj) {
    x = obj;
  }
};

//----------------------------------------------------------------------------//

void map_float(t_map *x, t_floatarg f) {
  x->lastValueIn = static_cast<float>(f);
  outlet_float(x->f_out, x->map->process(x->lastValueIn));
}

void map_bang(t_map *x) {
  outlet_float(x->f_out, x->map->process(x->lastValueIn));
}

void map_inputmin(t_map *x, t_floatarg f) {
  x->map->setInputMin(static_cast<float>(f));
}

void map_inputmax(t_map *x, t_floatarg f) {
  x->map->setInputMax(static_cast<float>(f));
}

void map_outputmin(t_map *x, t_floatarg f) {
  x->map->setOutputMin(static_cast<float>(f));
}

void map_outputmax(t_map *x, t_floatarg f) {
  x->map->setOutputMax(static_cast<float>(f));
}

void map_xfactor(t_map *x, t_floatarg f) {
  x->map->setXFactor(static_cast<float>(f));
}

void map_sfactor(t_map *x, t_floatarg f) {
  x->map->setSFactor(static_cast<float>(JL_MAX(f, 0)));
}

//----------------------------------------------------------------------------//

void *map_new(t_symbol *s, int argc, t_atom *argv) {
  t_map *x = (t_map *)pd_new(map_class);

  x->lastValueIn = 0;

  x->map = new PdIntervalMap();
  x->map->setObject(x);

  switch (argc) {
    case 0:
      break;
    case 1:
      x->map->setInputMax(static_cast<float>(atom_getfloat(argv)));
      break;
    case 2:
      x->map->setInputMin(static_cast<float>(atom_getfloat(argv)));
      x->map->setInputMax(static_cast<float>(atom_getfloat(argv + 1)));
      break;
    case 3:
      x->map->setInputMin(static_cast<float>(atom_getfloat(argv)));
      x->map->setInputMax(static_cast<float>(atom_getfloat(argv + 1)));
      x->map->setOutputMax(static_cast<float>(atom_getfloat(argv + 2)));
      break;

    case 6:
      x->map->setSFactor(static_cast<float>(atom_getfloat(argv + 5)));
    case 5:
      x->map->setXFactor(static_cast<float>(atom_getfloat(argv + 4)));
    case 4:
      x->map->setInputMin(static_cast<float>(atom_getfloat(argv)));
      x->map->setInputMax(static_cast<float>(atom_getfloat(argv + 1)));
      x->map->setOutputMin(static_cast<float>(atom_getfloat(argv + 2)));
      x->map->setOutputMax(static_cast<float>(atom_getfloat(argv + 3)));
      break;
  }

  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("inputmin"));
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("inputmax"));
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("outputmin"));
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("outputmax"));
  // inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("xfactor"));
  // inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("sfactor"));

  x->f_out = outlet_new(&x->x_obj, &s_float);

  return (void *)x;
}

void map_free(t_map *x) {
  delete x->map;
}

extern "C" {

void map_setup(void) {
  map_class = class_new(gensym("map"),
                        (t_newmethod)map_new,
                        (t_method)map_free,
                        sizeof(t_map),
                        CLASS_DEFAULT,
                        A_GIMME,
                        0);

  class_addbang(map_class, map_bang);
  class_addfloat(map_class, map_float);
  class_addmethod(map_class, (t_method)map_inputmin, gensym("inputmin"), A_DEFFLOAT, 0);
  class_addmethod(map_class, (t_method)map_inputmax, gensym("inputmax"), A_DEFFLOAT, 0);
  class_addmethod(map_class, (t_method)map_outputmin, gensym("outputmin"), A_DEFFLOAT, 0);
  class_addmethod(map_class, (t_method)map_outputmax, gensym("outputmax"), A_DEFFLOAT, 0);
  class_addmethod(map_class, (t_method)map_xfactor, gensym("xfactor"), A_DEFFLOAT, 0);
  class_addmethod(map_class, (t_method)map_sfactor, gensym("sfactor"), A_DEFFLOAT, 0);
}

}; /* end extern "C" */
