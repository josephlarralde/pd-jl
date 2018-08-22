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


#include "m_pd.h"
#include "../dependencies/jl.cpp.lib/dsp/sampler/Gbend.h"

class PdGbend;

static t_class *gbend_tilde_class;

typedef struct _gbend_tilde {

  //*********** AS IN TABREAD4 *********//

  t_object x_obj;
  t_symbol *x_arrayname;
  float x_arraysr;

  int x_npoints; // samples in buffer
  float *x_vec; // ref on the actual buffer (null if 0)

  // trying to handle 2 buffers in the same object
  int x_next_npoints;
  float *x_next_vec;

  float x_f; // this is used in setup function for signal inlet

  //************** ADDED ***************//

  PdGbend *player;
  t_outlet *x_out;
  t_outlet *f_out;

} t_gbend_tilde;

//============================================================================//

// example of class derived from Gbend to use the callbacks in pure data :

class PdGbend : public jl::Gbend {
private:
  t_gbend_tilde *x;

public:
  PdGbend(unsigned int c = 1) :
  Gbend(c) {}

  virtual ~PdGbend() {}

  void setObject(t_gbend_tilde *obj) {
    x = obj;
  }

  void endReachCallback(int endReachType) {
    outlet_float(x->f_out, endReachType);
  }

  void bufUpdatedCallback() {
    x->x_npoints = x->x_next_npoints;
    x->x_vec = x->x_next_vec;
  }
};

//============================================================================//

void gbend_tilde_set(t_gbend_tilde *x, t_symbol *s, t_floatarg f) {
  x->x_arrayname = s;
  x->x_arraysr = (f > 0) ? f : sys_getsr();
  t_garray *a;

  if (!(a = (t_garray *)pd_findbyclass(x->x_arrayname, garray_class))) {
    if (*s->s_name) {
      pd_error(x, "gbend~: %s: no such array", x->x_arrayname->s_name);
    }
    x->x_next_vec = 0;
  } else if (!garray_getfloatarray(a, &x->x_next_npoints, &x->x_next_vec)) {
    pd_error(x, "%s: bad template for gbend~", x->x_arrayname->s_name);
    x->x_next_vec = 0;
  } else {
    garray_usedindsp(a);
    x->player->setBuffer(x->x_next_vec, x->x_next_npoints, x->x_arraysr);
  }
}


void gbend_tilde_bang(t_gbend_tilde *x) {
  x->player->start();
}

void gbend_tilde_stop(t_gbend_tilde *x) {
  x->player->stop();
}

//=========================== PARAMETER SETTERS ==============================//

void gbend_tilde_pitch(t_gbend_tilde *x, t_floatarg f) {
  x->player->setPitch(f);
}

void gbend_tilde_fade(t_gbend_tilde *x, t_floatarg f) {
  x->player->setFades(f);
}

void gbend_tilde_fadi(t_gbend_tilde *x, t_floatarg f) {
  x->player->setFadeIn(f);
}

void gbend_tilde_fado(t_gbend_tilde *x, t_floatarg f) {
  x->player->setFadeOut(f);
}

void gbend_tilde_interrupt(t_gbend_tilde *x, t_floatarg f) {
  x->player->setInterrupt(f);
}

void gbend_tilde_beg(t_gbend_tilde *x, t_floatarg f) {
  x->player->setBegin(f);
}

void gbend_tilde_end(t_gbend_tilde *x, t_floatarg f) {
  x->player->setEnd(f);
}

void gbend_tilde_loop(t_gbend_tilde *x, t_floatarg f) {
  x->player->setLoop(f != 0);
}

void gbend_tilde_rvs(t_gbend_tilde *x, t_floatarg f) {
  x->player->setRvs(f != 0);
}

void gbend_tilde_setsr(t_gbend_tilde *x, t_floatarg f) {
  x->player->setSamplingRate(f);
}

//============================ DSP OPERATIONS ================================//

t_int *gbend_tilde_perform(t_int *w) {
  t_gbend_tilde *x = (t_gbend_tilde *)(w[1]);
  t_sample *in = (t_sample *)(w[2]);
  t_sample *out = (t_sample *)(w[3]);
  int n = (int)(w[4]); // VECTOR SIZE

  float **outs = &out;

  x->player->process((float *)in, outs, n);

  return (w + 5);
}

void gbend_tilde_dsp(t_gbend_tilde *x, t_signal **sp) {
  
  gbend_tilde_setsr(x, sys_getsr());
  gbend_tilde_set(x, x->x_arrayname, x->x_arraysr);

  dsp_add(gbend_tilde_perform, 4, x,
          sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);

}

//======================= CONSTRUCTOR / DESTRUCTOR ===========================//

void *gbend_tilde_new(t_symbol *s) {
  /*
   * call the "constructor" of the parent-class
   * this will reserve enough memory to hold "t_gbend_tilde"
   */
  t_gbend_tilde *x = (t_gbend_tilde *)pd_new(gbend_tilde_class);

  x->x_arrayname = s;
  x->x_arraysr = sys_getsr();
  //if(x->x_arrayname == gensym("")) post("no table affected");

  x->x_vec = 0;
  // x->x_f = 0;

  x->player = new PdGbend(1);
  x->player->setObject(x);

  gbend_tilde_setsr(x, sys_getsr());
  gbend_tilde_set(x, x->x_arrayname, x->x_arraysr);

  x->x_out = outlet_new(&x->x_obj, &s_signal);
  x->f_out = outlet_new(&x->x_obj, &s_float);

  return (void *)x;
}

void gbend_tilde_free(t_gbend_tilde *x) {
  delete x->player;
  outlet_free(x->x_out);
  outlet_free(x->f_out);
}

//============================ SETUP FUNCTION ================================//

extern "C" {

/**
 * define the function-space of the class
 * within a single-object external the name of this function is special
 */
void gbend_tilde_setup(void) {
  /* create a new class */
  gbend_tilde_class = class_new(gensym("gbend~"), /* the object's name is "gbend~" */
    (t_newmethod)gbend_tilde_new,                 /* the object's constructor is "gbend_tilde_new()" */
    (t_method)gbend_tilde_free,                   /* the object's destructor */
    sizeof(t_gbend_tilde),                        /* the size of the data-space */
    CLASS_DEFAULT,                                /* a normal pd object */
    A_DEFSYM,                                     /* arg type (default associated table name) */
    0);                                           /* no creation arguments ? */

  class_addbang(gbend_tilde_class, gbend_tilde_bang);
  class_addmethod(gbend_tilde_class, (t_method)gbend_tilde_dsp, gensym("dsp"), A_NULL);
  class_addmethod(gbend_tilde_class, (t_method)gbend_tilde_set, gensym("set"), A_DEFSYM, A_DEFFLOAT, 0);
  class_addmethod(gbend_tilde_class, (t_method)gbend_tilde_stop, gensym("stop"), A_NULL);
  class_addmethod(gbend_tilde_class, (t_method)gbend_tilde_pitch, gensym("pitch"), A_DEFFLOAT, 0);
  class_addmethod(gbend_tilde_class, (t_method)gbend_tilde_fade, gensym("fade"), A_DEFFLOAT, 0);
  class_addmethod(gbend_tilde_class, (t_method)gbend_tilde_fadi, gensym("fadi"), A_DEFFLOAT, 0);
  class_addmethod(gbend_tilde_class, (t_method)gbend_tilde_fado, gensym("fado"), A_DEFFLOAT, 0);
  class_addmethod(gbend_tilde_class, (t_method)gbend_tilde_interrupt, gensym("interrupt"), A_DEFFLOAT, 0);
  class_addmethod(gbend_tilde_class, (t_method)gbend_tilde_beg, gensym("beg"), A_DEFFLOAT, 0);
  class_addmethod(gbend_tilde_class, (t_method)gbend_tilde_end, gensym("end"), A_DEFFLOAT, 0);
  class_addmethod(gbend_tilde_class, (t_method)gbend_tilde_rvs, gensym("rvs"), A_DEFFLOAT, 0);
  class_addmethod(gbend_tilde_class, (t_method)gbend_tilde_loop, gensym("loop"), A_DEFFLOAT, 0);
  class_addmethod(gbend_tilde_class, (t_method)gbend_tilde_setsr, gensym("setsr"), A_DEFFLOAT, 0);

  CLASS_MAINSIGNALIN(gbend_tilde_class, t_gbend_tilde, x_f);
}

}; /* end extern "C" */
