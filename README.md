# pd-jl

### a collection of externals and abstractions for pure-data vanilla

This library is oriented towards musical creation, providing various audio synths and effects, as well as a bunch of utilities to manipulate rythms and scales.

It is available as `jl` via pd vanilla's integrated package manager `deken`.

#### building

To build and install the library on your machine, first clone this repository :

`git clone --recursive https://github.com/josephlarralde/pd-jl.git`

Then run `make install`, and eventually `make clean`

By default this creates a `build/darwin/jl` directory at the root of this folder that you can copy somewhere and/or add to pd's paths list.
Or you can specify your own install location by changing the `PDLIBDIR` var in
`Makefile`.

#### notes

The `make source` command creates a `build/source/jl` directory containing a copy of all the abstractions and help files of the library, the c++ source for the externals and the `Makefile`, with all git related files and folders removed (used to generate a source-only package with `deken`).

For windows compilation, I found some useful info [here](https://github.com/pure-data/pd-lib-builder/issues/47).
