# Introduction
In this tutorial, we are going to build a Gaudi algorithm to find the Higgs mass
in a Higgs recoil process:

```math
e^+ e^-\rightarrow Z^*\rightarrow HZ\rightarrow H\mu^+\mu^-
```

which means

```math
p_{e^+ e^-}=p_H+p_{\mu^+}+p_{\mu^-}
```

We will select final states with two muons. To illustrate a typical case, we'll
divide the process in two steps. The first step is to iterate through the events
and select only the muons the pass a cut on Pt. A new collection with those
muons will be added and used in the main `HiggsRecoil.cpp` to select only
events with two muons and get the mass. Note that it's possible (and fine) to
do this in a single algorithm without intermediate steps, or without saving the
intermediate collection with the muons.

# Setup

First, we will need a key4hep setup, typically obtained by sourcing scripts from
cvmfs, see the instructions
[here](https://key4hep.github.io/key4hep-doc/main/getting_started/setup.html).
After sourcing, check that the environment variable `KEY4HEP_STACK` is set to
make sure the stack has been loaded correctly:

``` bash
echo $KEY4HEP_STACK
```

should report a path like `/cvmfs/sw-nightlies.hsf.org/key4hep/releases/2023-10-06/...`

In order to have the necessary code you also have to clone this repository and
go into the `setup` directory of this exercise

```bash
git clone https://github.com/key4hep/key4hep-tutorials
cd key4hep-tutorials/gaudi_alg_higgs/setup
```

# Running our Gaudi algorithm

We'll work from the `setup` directory in this folder. Once there, run

```
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=../install
cmake --build . -j $(nproc) -t install 
cd ../install
export PATH=$PWD/bin:$PATH
export LD_LIBRARY_PATH=$PWD/lib:$PWD/lib64:$LD_LIBRARY_PATH
export ROOT_INCLUDE_PATH=$PWD/include:$ROOT_INCLUDE_PATH
export PYTHONPATH=$PWD/python:$PYTHONPATH
export CMAKE_PREFIX_PATH=$PWD:$CMAKE_PREFIX_PATH
cd ../
```

Now we should have compiled our code. Two C++ files will have been compiled:
`HiggsRecoil.cpp` and `MuonFilter.cpp`, creating two Gaudi plugins that we are
now able to import from python. Now we can get a reconstructed file with the
following command:

``` bash
wget https://key4hep.web.cern.ch/tutorial/zh_mumu_ild_dst/rv02-02.sv02-02.mILD_l5_o1_v02.E250-SetA.I402004.Pe2e2h.eR.pL.n000.d_dstm_15090_0.edm4hep.root
```

**If you are running on lxplus the file is also available on EOS:**

```
/eos/project-k/key4hep/www/key4hep/tutorial/zh_mumu_ild_dst/rv02-02.sv02-02.mILD_l5_o1_v02.E250-SetA.I402004.Pe2e2h.eR.pL.n000.d_dstm_15090_0.edm4hep.root
```

To run the Higgs recoil process change the location of the input data in the
following file and then run:

``` bash
k4run higgs_recoil/options/runHiggsRecoil.py
```

This file `runHiggsRecoil.py` is called a "steering file" because it has the
instructions for what algorithms we want to run, what parameters we want to pass
them and other configuration like logging.

There are several options that can be changed in the steering file that may be important:
- The name of the input file is passed to the `PodioInput` plugin
- The name of the output file is passed to the `PodioOutput` plugin
- The number of events to process is passed to the `ApplicationMgr`. Choose `-1`
  not to limit it (all the events in the input file will be processed) or any
  other number to put a limit (sometimes useful for testing or debugging)
- The level of output that we'll get while running. This is passed to the
  `ApplicationMgr` and can be set to `INFO`, `WARNING`, `DEBUG` and other values. Make
  sure the actual value is imported in the steering file or python will complain
  that it isn't defined.

After the processing has ran, we'll have an output file. We can inspect the file
with `podio-dump` to see which collections it has. By default it will be a list of collections and parameters metadata:

```txt
input file: higgs_recoil_out.root
            (written with podio version: 1.2.99)

datamodel model definitions stored in this file: 
 - edm4hep (0.99.99)

Frame categories in this file:
Name                      Entries
----------------------  ---------
metadata                        1
events                       9400
configuration_metadata          1
################################### events: 0 ####################################
Collections:
Name         ValueType                         Size  ID
-----------  ------------------------------  ------  --------
Higgs        edm4hep::ReconstructedParticle       1  88d34b01
Muons        edm4hep::ReconstructedParticle       2  aa8128c8
PandoraPFOs  edm4hep::ReconstructedParticle      59  fa28d9be
Z            edm4hep::ReconstructedParticle       1  3dbac09d

Parameters:
Name                            Type           Elements
------------------------------  -----------  ----------
_weight                         float                 1
alphaQCD                        float                 1
beam_particle1                  std::string           1
beam_particle2                  std::string           1
beamPDG1                        int                   1
beamPDG2                        int                   1
beamPol1                        float                 1
beamPol2                        float                 1
BeamSpectrum                    std::string           1
...
```

You can execute `podio-dump` using the input file and compare the results to confirm that all metadata is preserved in the output file.

`podio-dump` has additional options (run with `-h` to see the complete list) to
inspect more events and even to get the values of every member of every element
of the collections.

# Plotting in python

A simple script to make histograms in python is provided. Run:

``` bash
python higgs_recoil/plotting/plot.py
```

If the input file is found (it will be the output file that we have obtained
before), this script will produce a `histogram.pdf` file that we can open to
find the two histograms. If you are running remotely, it may be better to copy
it locally by using `scp`.

There are other possibilities from python: it's also possible to make plots
using ROOT from python.

# Running with multithreading

By using `Gaudi::Functional` and the custom histograms from Gaudi we are ready
to run with multithreading. That means that we only have to make changes in our
steering file to tell it to run with multithreading. This more advanced topic is
reserved for a future tutorial.

