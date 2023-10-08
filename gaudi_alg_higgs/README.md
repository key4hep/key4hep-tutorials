# Introduction
In this tutorial, we are going to build a Gaudi algorithm to find the Higgs mass
in a Higgs recoil process:

```math
e^+ e^-\rightarrow Z^*\rightarrow HZ\rightarrow H\mu^+\mu^-
```

which means

```math
p_{e^+ e^-}=p_H+p_{mu^+}+p_{mu^-}
```

We will select final states with two muons. To illustrate a typical case, we'll
divide the process in two steps. The first step is to iterate through the events
and select only the muons the pass a cut on Pt. A new collection with those
muons will be added and used in the main `HiggsRecoil.cpp` to select only
eventns with two muons and get the mass. Note that it's possible (and fine) to
do this in a single algorithm without intermediate steps, or without saving the
intermediate collection with the muons.

# Setup

First, we will need a key4hep setup, typically obtained by sourcing scripts from
cvmfs, see the instructions
[here](https://key4hep.github.io/key4hep-doc/setup-and-getting-started/README.html).
After sourcing, check that the environment variable `KEY4HEP_STACK` is set to
make sure the stack has been loaded correctly:

``` bash
echo $KEY4HEP_STACK
```

should report a path like `/cvmfs/sw-nightlies.hsf.org/key4hep/releases/2023-10-06/...`

# Running our Gaudi algorithm

We'll work from the `setup` directory in this folder. Once there, run

```
mkdir build
cd build -DCMAKE_INSTALL_PREFIX=../install
cmake ..
cmake --build . -j $(nproc) -t install 
cd ../install
export PATH=$PWD/bin:$PATH
export LD_LIBRARY_PATH=$PWD/lib:$PWD/lib64:$LD_LIBRARY_PATH
export ROOT_INCLUDE_PATH=$PWD/include:$ROOT_INCLUDE_PATH
export PYTHONPATH=$PWD/python:$PYTHONPATH
export CMAKE_PREFIX_PATH=$PWD:$CMAKE_PREFIX_PATH
cd ../
```

Now we should have compiled our code. Two C++ files will have been compiled
`HiggsRecoil.cpp` and `MuonFilter.cpp` creating two Gaudi plugins that we are
now able to import from python.

To run the Higgs recoil process run the following:

```
k4run options/runHiggsRecoil.py
```

There are several options that can be changed in the steering file that may be important:
- The name of the input file is passed to the `PodioInput` plugin
- The name of the output file is passed to the `PodioOutput` plugin
- The number of events to process is passed to the `ApplicationMgr`. Choose `-1`
  not to limit it (all the events in the input file will be processed) or any
  other number to put a limit (sometimes useful for testing or debugging)
- The level of output that we'll get while running. This is passed to the
  `ApplicationMgr` and can be set to INFO, WARNING, DEBUG and other values. Make
  sure the actual value is imported in the steering file or python will complain
  that it isn't defined.

After the processing has ran, we'll have an output file. We can inspect the file
with `podio-dump` to see which collections it has. By default it will be a long
list of collections since the collections of the input file are kept

```
$ podio-dump higgs_recoil_out.root
BCalClusters                              edm4hep::Cluster                                0  bc95eab6
BCalClusters_particleIDs                  edm4hep::ParticleID                             0  ce810ab9
BCalRecoParticle                          edm4hep::ReconstructedParticle                  0  5d2cb360
BCalRecoParticle_particleIDs              edm4hep::ParticleID                             0  2c09a289
BuildUpVertex                             edm4hep::Vertex                                 0  4b3ce322
BuildUpVertex_RP                          edm4hep::ReconstructedParticle                  0  c1ce60c0
BuildUpVertex_RP_particleIDs              edm4hep::ParticleID                             0  6bc98df2
BuildUpVertex_V0                          edm4hep::Vertex                                 0  77941bb6
BuildUpVertex_V0_RP                       edm4hep::ReconstructedParticle                  0  5784a361
BuildUpVertex_V0_RP_particleIDs           edm4hep::ParticleID                             0  0d56a0e7
ClusterMCTruthLink                        edm4hep::MCRecoClusterParticleAssociation      81  e2727d90
...
```

by comparing with the input file we can see our two new collections that we
created:
```
Higgs                                     edm4hep::ReconstructedParticle                  1  88d34b01
Z                                         edm4hep::ReconstructedParticle                  1  3dbac09d
```

# Bonus: Running with multithreading

By using `Gaudi::Functional` and the custom histograms from we are ready to run
with multithtreading. A steering file for running with multithreading is
provided, to use it run:

``` bash
k4run 
```

# Plotting in python

A simple script to make histograms in python is provided. Run:

``` bash
python setup/higgs-recoil/plotting/plot.py
```

If the input file is found (it will be the output file that we have obtained
before), this script will produce a `histogram.pdf` file that we can open to
find the two histograms. If you are running remotely, it may be better to copy
it locally by using `scp`.

There are other possibilities from python: it's also possible to make plots
using ROOT from python.
