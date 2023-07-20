# library name
lib.name = jl

EXT=src/externals
DEP=src/dependencies
ABS=abstractions
HLP=helpfiles

# input source files
# aaosc~.class.sources = $(EXT)/aaosc~.cpp $(DEP)/jl.cpp.lib/dsp/synthesis/Oscillator.cpp
# hann~.class.sources = $(EXT)/hann~.cpp $(DEP)/jl.cpp.lib/dsp/synthesis/Oscillator.cpp
bibi~.class.sources = $(EXT)/bibi~.cpp
gbend~.class.sources = $(EXT)/gbend~.cpp $(DEP)/jl.cpp.lib/dsp/sampler/Gbend.cpp
stut~.class.sources = $(EXT)/stut~.cpp $(DEP)/jl.cpp.lib/dsp/effects/Stut.cpp
sidechain~.class.sources = $(EXT)/sidechain~.cpp $(DEP)/jl.cpp.lib/dsp/effects/Compress.cpp
flatten~.class.sources = $(EXT)/flatten~.cpp $(DEP)/jl.cpp.lib/dsp/effects/Compress.cpp
map.class.sources = $(EXT)/map.cpp
magnetize.class.sources = $(EXT)/magnetize.cpp
tonnetz.class.sources = $(EXT)/tonnetz.cpp

# all extra files to be included in binary distribution of the library
datafiles = \
$(ABS)/jl-meta.pd \
$(ABS)/jl-objects.pd \
$(HLP)/gbend~-help.pd \
$(HLP)/stut~-help.pd \
$(HLP)/sidechain~-help.pd \
$(HLP)/flatten~-help.pd \
$(HLP)/bibi~-help.pd \
$(HLP)/map-help.pd \
$(HLP)/magnetize-help.pd \
$(HLP)/tonnetz-help.pd \
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
$(ABS)/slideflute~.pd \
$(HLP)/slideflute~-help.pd \
$(ABS)/gpan-unit~.pd \
$(ABS)/gdelay-unit~.pd \
$(ABS)/gdelay~.pd \
$(HLP)/gdelay~-help.pd \
$(ABS)/gflow-unit~.pd \
$(ABS)/gflow~.pd \
$(HLP)/gflow~-help.pd \
$(ABS)/guzi-unit~.pd \
$(ABS)/dirac~.pd \
$(HLP)/dirac~-help.pd \
$(ABS)/bitcrush~.pd \
$(HLP)/bitcrush~-help.pd \
$(ABS)/decimate~.pd \
$(HLP)/decimate~-help.pd \
$(ABS)/logdelay-unit~.pd \
$(ABS)/logdelay~.pd \
$(ABS)/pulse.pd \
$(HLP)/pulse-help.pd \
$(ABS)/keynote.pd \
$(HLP)/keynote-help.pd \
$(ABS)/keyboard.pd \
$(HLP)/keyboard-help.pd \
$(ABS)/envgen.pd \
$(HLP)/envgen-help.pd \
$(ABS)/switchcontrol.pd \

# update path to reflect your environment
# PDLIBDIR="/Users/larralde/Documents/Pd/externals"

UNAME := $(shell uname -s)

ifeq ($(UNAME),Darwin)
	PDLIBDIR="./build/darwin"
endif
ifeq ($(UNAME),Linux)
	PDLIBDIR="/usr/local/lib/pd-externals"
endif
ifeq (MINGW,$(findstring MINGW,$(UNAME)))
	PDLIBDIR="./build/win32"
	PDINCLUDEDIR="/c/Program\ Files/Pd/src"
	PDBINDIR="/c/Program\ Files/Pd/bin"
endif


SRCOUT=./build/source/jl

# this is needed for initializer lists
cflags += -std=c++11

CC=gcc

ifeq ($(UNAME),Darwin)
	# this is needed for use of <vector> (!?)
	cflags += -mmacosx-version-min=10.9
endif

# include Makefile.pdlibbuilder from submodule directory 'pd-lib-builder'
# update path to reflect your environment
PDLIBBUILDER_DIR=$(DEP)/pd-lib-builder/
include $(PDLIBBUILDER_DIR)/Makefile.pdlibbuilder

# found here
# https://stackoverflow.com/questions/4822321/remove-all-git-files-from-a-directory
source:
	rm -rf "$(SRCOUT)";
	mkdir "$(SRCOUT)";
	cp Makefile "$(SRCOUT)";
	mkdir "$(SRCOUT)/patches";
	cp $(datafiles) "$(SRCOUT)/patches";

	# cp -r ./abstractions ./jl/abstractions;
	# cp -r ./helpfiles ./jl/helpfiles;

	cp -r ./src "$(SRCOUT)/src";
	(find $(SRCOUT) -name ".git" && find $(SRCOUT) -name ".gitignore" && find $(SRCOUT) -name ".gitmodules") | xargs rm -rf;
