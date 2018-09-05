// pretty much the same as pd's osc~ object (but based on jl.cpp.lib)
// kept here for reference, not included in jl.pd.lib

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
