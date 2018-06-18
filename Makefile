# library name
lib.name = jl

EXT=src/externals
DEP=src/dependencies
ABS=abstractions
HLP=helpfiles

# input source files
gbend~.class.sources = $(EXT)/gbend~.cpp $(DEP)/jl.cpp.lib/dsp/sampler/Gbend.cpp
stut~.class.sources = $(EXT)/stut~.cpp $(DEP)/jl.cpp.lib/dsp/effects/Stut.cpp

# all extra files to be included in binary distribution of the library
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
PDLIBDIR="."

# include Makefile.pdlibbuilder from submodule directory 'pd-lib-builder'
# update path to reflect your environment
PDLIBBUILDER_DIR=$(DEP)/pd-lib-builder/
include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder

source:
	rm -rf ./jl;
	mkdir jl;
	cp Makefile ./jl;
	cp -r ./abstractions ./jl/abstractions;
	cp -r ./helpfiles ./jl/helpfiles;
	cp -r ./src ./jl/src;
	cd jl;
	(find . -name ".git" && find . -name ".gitignore" && find . -name ".gitmodules") | xargs rm -rf;