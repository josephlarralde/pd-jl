# jl.pd.lib

#### a collection of externals and abstractions for pure-data vanilla

To install the library on your machine, first clone this repository :

`git clone --recursive https://github.com/josephlarralde/jl.pd.lib.git`

Then run `make install`, and eventually `make clean`

By default this creates a `jl` directory at the root of this folder that you
can copy somewhere and add to pd's paths list.
Or you can specify your own install location by changing the `PDLIBDIR` var in
`Makefile`.

This library should be available soon via pd's integrated package manager `deken`.

Note : `make source` creates a `jl` directory containing a copy of `abstractions/`,
`helpfiles/`, `src/` and `Makefile` with all git related files and folders removed
(used to generate a source-only package with `deken`).