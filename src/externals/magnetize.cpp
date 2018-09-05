#include "m_pd.h"
#include "../dependencies/jl.cpp.lib/control/harmony/PatternMap.h"

class PdPatternMap;

static t_class *magnetize_class;

//----------------------------------------------------------------------------//

typedef struct _magnetize {
  t_object x_obj;
  t_float lastValueIn;

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
  x->lastValueIn = f;
  outlet_float(x->f_out, x->map->process(x->lastValueIn));
}

void magnetize_bang(t_magnetize *x) {
  outlet_float(x->f_out, x->map->process(x->lastValueIn));
}

void magnetize_root(t_magnetize *x, t_floatarg f) {
  x->map->setRoot(f);
}

void magnetize_factor(t_magnetize *x, t_floatarg f) {
  x->map->setFactor(f);
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

extern "C" {

void magnetize_setup(void) {
  magnetize_class = class_new(gensym("magnetize"),
                        (t_newmethod)magnetize_new,
                        0,
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
