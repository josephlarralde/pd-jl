# library name
lib.name = jl

EXT=src/externals
DEP=src/dependencies
ABS=abstractions
HLP=helpfiles

# input source files
# aaosc~.class.sources = $(EXT)/aaosc~.cpp $(DEP)/jl.cpp.lib/dsp/synthesis/Oscillator.cpp
# hann~.class.sources = $(EXT)/hann~.cpp $(DEP)/jl.cpp.lib/dsp/synthesis/Oscillator.cpp
gbend~.class.sources = $(EXT)/gbend~.cpp $(DEP)/jl.cpp.lib/dsp/sampler/Gbend.cpp
stut~.class.sources = $(EXT)/stut~.cpp $(DEP)/jl.cpp.lib/dsp/effects/Stut.cpp
sidechain~.class.sources = $(EXT)/sidechain~.cpp $(DEP)/jl.cpp.lib/dsp/effects/Compress.cpp
flatten~.class.sources = $(EXT)/flatten~.cpp $(DEP)/jl.cpp.lib/dsp/effects/Compress.cpp
map.class.sources = $(EXT)/map.cpp
magnetize.class.sources = $(EXT)/magnetize.cpp

# all extra files to be included in binary distribution of the library
datafiles = \
$(ABS)/jl-objects.pd \
$(HLP)/gbend~-help.pd \
$(HLP)/stut~-help.pd \
$(HLP)/sidechain~-help.pd \
$(HLP)/flatten~-help.pd \
$(HLP)/map-help.pd \
$(HLP)/magnetize-help.pd \
$(ABS)/split~.pd \
$(HLP)/split~-help.pd \
$(ABS)/merge~.pd \
$(HLP)/merge~-help.pd \
$(ABS)/feedfm~.pd \
$(HLP)/feedfm~-help.pd \
$(ABS)/rmetro.pd \
$(HLP)/rmetro-help.pd \
$(ABS)/lr2ms~.pd \
$(HLP)/lr2ms~-help.pd \
$(ABS)/ms2lr~.pd \
$(HLP)/ms2lr~-help.pd \
$(ABS)/mtosf.pd \
$(HLP)/mtosf-help.pd \
$(ABS)/gpan-unit~.pd \
$(ABS)/gdelay-unit~.pd \
$(ABS)/gdelay~.pd \
$(HLP)/gdelay~-help.pd \
$(ABS)/gflow-unit~.pd \
$(ABS)/gflow~.pd \
$(HLP)/gflow~-help.pd \
# $(ABS)/gflow2~.pd \
# $(ABS)/gflow4~.pd \
# $(HLP)/gdelay~-help.pd \
# $(HLP)/gflow2~-help.pd \
# $(HLP)/gflow4~-help.pd \

# $(ABS)/keynote.pd \
# $(HLP)/keynote-help.pd \
# $(ABS)/q-keyboard.pd \
# $(HLP)/q-keyboard-help.pd \

# update path to reflect your environment
# PDLIBDIR="/Users/larralde/Documents/Pd/externals"
PDLIBDIR="./build"
# PDLIBDIR="."

# include Makefile.pdlibbuilder from submodule directory 'pd-lib-builder'
# update path to reflect your environment
PDLIBBUILDER_DIR=$(DEP)/pd-lib-builder/
include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder

# found here
# https://stackoverflow.com/questions/4822321/remove-all-git-files-from-a-directory
source:
	rm -rf ./jl;
	mkdir jl;
	cp Makefile ./jl;
	cp -r ./abstractions ./jl/abstractions;
	cp -r ./helpfiles ./jl/helpfiles;
	cp -r ./src ./jl/src;
	cd jl;
	(find . -name ".git" && find . -name ".gitignore" && find . -name ".gitmodules") | xargs rm -rf;
