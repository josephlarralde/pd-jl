/*
 * HOWTO write an External for Pure data
 * (c) 2001-2006 IOhannes m zmölnig zmoelnig[AT]iem.at
 *
 * this is the source-code for the first example in the HOWTO
 * it creates an object that prints "Hello world!" whenever it
 * gets banged.
 *
 * for legal issues please see the file LICENSE.txt
 */

// WORK IN PROGRESS ...

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

class PdStut : public Stut {
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
  x->stutter->setSliceDuration(f);
}

void stut_tilde_slices(t_stut_tilde *x, t_floatarg f) {
  x->stutter->setSlices((long) f);
}

void stut_tilde_fade(t_stut_tilde *x, t_floatarg f) {
  x->stutter->setFades(f);
}

void stut_tilde_fadi(t_stut_tilde *x, t_floatarg f) {
  x->stutter->setFadeIn(f);
}

void stut_tilde_fado(t_stut_tilde *x, t_floatarg f) {
  x->stutter->setFadeOut(f);
}

void stut_tilde_interrupt(t_stut_tilde *x, t_floatarg f) {
  x->stutter->setInterrupt(f);
}

void stut_tilde_release(t_stut_tilde *x, t_floatarg f) {
  x->stutter->setRelease(f);
}

void stut_tilde_setsr(t_stut_tilde *x, t_floatarg f) {
  x->stutter->setSamplingRate(f);
}

//============================ DSP OPERATIONS ================================//

t_int *stut_tilde_perform(t_int *w) {
  t_stut_tilde *x = (t_stut_tilde *)(w[1]);
  t_sample *in = (t_sample *)(w[2]);
  t_sample *out = (t_sample *)(w[3]);
  int n = (int)(w[4]);

  float **ins = &in;
  float **outs = &out;

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
