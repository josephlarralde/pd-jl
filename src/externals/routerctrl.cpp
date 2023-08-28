/**
 * @file routerctrl.cpp
 * @author Joseph Larralde
 * @date 03/08/2023
 * @brief centralize matrix connections state to avoid feedback loops
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

#include "m_pd.h"
#include "../dependencies/cpp-jl/src/control/utilities/routing.h"

static t_class *routerctrl_class;

//----------------------------------------------------------------------------//

typedef struct _routerctrl {
  t_object x_obj;
  unsigned int sends;
  unsigned int inputs;
  unsigned int outputs;
  std::vector<bool> sendsConnections;
  std::vector<bool> allConnections;
  t_outlet *f_out;
} t_routerctrl;

void routerctrl_dump_connection(t_routerctrl* x, unsigned int* connection) {
  t_atom outatoms[3];
  SETFLOAT(outatoms, *connection);
  SETFLOAT(outatoms + 1, *(connection + 1));
  SETFLOAT(outatoms + 2, *(connection + 2));
  outlet_list(x->f_out, &s_list, 3, outatoms);
}

void routerctrl_dump(t_routerctrl* x) {
  t_atom outatoms[3];

  for (auto i = 0; i < x->sends + x->inputs; ++i) {
    SETFLOAT(outatoms, i);
    for (auto j = 0; j < x->sends + x->outputs; ++j) {
      SETFLOAT(outatoms + 1, j);
      SETFLOAT(outatoms + 2, x->allConnections[i + j * (x->sends + x->inputs)] ? 1 : 0);
      outlet_list(x->f_out, &s_list, 3, outatoms);
    }
  }
}

void routerctrl_list(t_routerctrl* x, t_symbol* s, int argc, t_atom* argv, bool dump = true) {
  unsigned int connection[3];

  if (argc > 2) {
    for (auto i = 0; i < 3; ++i) {
      if ((argv + i)->a_type != A_FLOAT) {
        post("bad argument type");
        return;
      }

      connection[i] = static_cast<unsigned int>((argv + i)->a_w.w_float);
    }

    if (connection[0] >= x->sends + x->inputs ||
        connection[1] >= x->sends + x->outputs) {
      // out of bounds, just ignore
      return;
    }

    unsigned int allIndex = connection[0] + connection[1] * (x->sends + x->inputs);

    if (connection[0] < x->sends && connection[1] < x->sends) {
      // prevent feedback loops
      unsigned int sendsIndex = connection[0] + connection[1] * x->sends;

      if (connection[2] != 0) {
        x->sendsConnections[sendsIndex] = !squareMatrixWillLoop(
          x->sendsConnections,
          x->sends,
          connection[0],
          connection[1]
        );

        connection[2] = x->sendsConnections[sendsIndex] ? 1 : 0;
      } else {
        x->sendsConnections[sendsIndex] = false;
      }

      x->allConnections[allIndex] = x->sendsConnections[sendsIndex];
    } else {
      x->allConnections[allIndex] = connection[2] != 0;
    }

    if (dump) {
      routerctrl_dump_connection(x, connection);
    }
  }
}

void routerctrl_anything(t_routerctrl* x, t_symbol* s, int argc, t_atom* argv) {
  // post("message input");
  if (s == gensym("clear")) {
    std::fill(x->sendsConnections.begin(), x->sendsConnections.end(), false);
    std::fill(x->allConnections.begin(), x->allConnections.end(), false);
    routerctrl_dump(x);
  } else if (s == gensym("dump")) {
    routerctrl_dump(x);
  } else if (s == gensym("set")) {
    // todo
  } else if (s == gensym("serialize")) {
    // create the list of all connection values line by line
    unsigned int index;
    t_atom outatoms[x->allConnections.size() + 1];

    for (auto i = 0; i < x->sends + x->inputs; ++i) {
      for (auto j = 0; j < x->sends + x->outputs; ++j) {
        index = i + j * (x->sends + x->inputs);
        SETFLOAT(outatoms + index, x->allConnections[index] ? 1 : 0);
      }
    }

    outlet_anything(
      x->f_out,
      gensym("serialize"),
      x->allConnections.size(),
      (t_atom *)(&outatoms)
    );
  } else if (s == gensym("deserialize")) {
    // restore from a previously serialized list
    if (argc == x->allConnections.size()) {
      unsigned int index;
      t_atom outatoms[3];

      for (auto i = 0; i < x->sends + x->inputs; ++i) {
        SETFLOAT(outatoms, i);

        for (auto j = 0; j < x->sends + x->outputs; ++j) {
          index = i + j * (x->sends + x->inputs);
          SETFLOAT(outatoms + 1, j);
          SETFLOAT(outatoms + 2, atom_getfloat(argv + index));
          routerctrl_list(x, gensym("list"), 3, outatoms, false);
        }
      }
    }
  }
}

//----------------------------------------------------------------------------//

void *routerctrl_new(t_symbol *s, int argc, t_atom *argv) {
  t_routerctrl *x = (t_routerctrl *) pd_new(routerctrl_class);

  x->sends = argc > 0 ? static_cast<unsigned int>(atom_getfloat(argv)) : 0;
  x->inputs = argc > 0 ? static_cast<unsigned int>(atom_getfloat(argv + 1)) : 0;
  x->outputs = argc > 0 ? static_cast<unsigned int>(atom_getfloat(argv + 2)) : 0;

  x->sendsConnections.resize(x->sends * x->sends, false);
  x->allConnections.resize((x->sends + x->inputs) * (x->sends + x->outputs), false);

  x->f_out = outlet_new(&x->x_obj, &s_anything);

  return (void*) x;
}

void routerctrl_free(t_routerctrl *x) {
  // delete x->map;
}

extern "C" {

void routerctrl_setup(void) {
  routerctrl_class = class_new(gensym("routerctrl"),
                                   (t_newmethod)routerctrl_new,
                                   (t_method)routerctrl_free,
                                   sizeof(t_routerctrl),
                                   CLASS_DEFAULT,
                                   A_GIMME,
                                   0);

  class_addlist(routerctrl_class, routerctrl_list);
  class_addanything(routerctrl_class, routerctrl_anything);
}

}; /* end extern "C" */
