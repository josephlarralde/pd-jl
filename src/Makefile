# library name
lib.name = jl

# input source files
gbend~.class.sources = externals/gbend~.cpp dependencies/jl.cpp.lib/dsp/sampler/Gbend.cpp
stut~.class.sources = externals/stut~.cpp dependencies/jl.cpp.lib/dsp/effects/Stut.cpp

# all extra files to be included in binary distribution of the library
ABS=../abstractions
HLP=../helpfiles

datafiles = \
$(ABS)/equalgainsplit~.pd \
$(ABS)/equalgainmerge~.pd \
$(ABS)/feedfm~.pd \
$(ABS)/gdelay.unit~.pd \
$(ABS)/gdelay~.pd \
$(ABS)/gflow2~.pd \
$(ABS)/gflow4~.pd \
$(ABS)/keynote.pd \
$(ABS)/mtosf.pd \
$(ABS)/pscale.pd \
$(ABS)/q-keyboard.pd \
$(ABS)/rmetro.pd \
$(ABS)/scale.pd
# $(HLP)/equalgainsplit~-help.pd \
# $(HLP)/equalgainmerge~-help.pd \
# $(HLP)/feedfm-help.pd \
# $(HLP)/feedfm~-help.pd \
# $(HLP)/gdelay~-help.pd \
# $(HLP)/gflow2~-help.pd \
# $(HLP)/gflow4~-help.pd \
# $(HLP)/keynote-help.pd \
# $(HLP)/mtosf-help.pd \
# $(HLP)/pscale-help.pd \
# $(HLP)/q-keyboard-help.pd \
# $(HLP)/rmetro-help.pd \
# $(HLP)/scale-help.pd

# update path to reflect your environment
# PDLIBDIR="/Users/larralde/Documents/Pd/externals"
PDLIBDIR=".."

# include Makefile.pdlibbuilder from submodule directory 'pd-lib-builder'
# update path to reflect your environment
PDLIBBUILDER_DIR=dependencies/pd-lib-builder/
include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder
