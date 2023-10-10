# Simple plotting with podio and EDM4hep

This is a very simple exercise that aims to show how to use the plain podio and
EDM4hep interfaces to read data from podio files. You can do this exercise using
the c++ interface or the corresponding python bindings.

Let's have a look at [the introduction to EDM4hep and
podio](./edm4hep_api_intro.md) first (if you haven't done so yet).

## Setup

Make sure that you are in a Key4hep software environment. If you are not yet, a
simple way to get into one is to

```bash
source /cvmfs/sw-nightlies.hsf.org/key4hep/setup.sh
```

afterwards create a working directory for this exercise

``` bash
mkdir edm4hep_plotting_tutorial
cd edm4hep_plotting_tutorial
```

### Input data for the exercise

In order to have at least some statistics for plots we have prepared a few input
files for this exercise. You can find them at
`/eos/project/k/key4hep/www/key4hep/tutorial/zh_mumu_filtered/`.[^1]

[^1]:These are output files produced by the [*Gaudi algorithm
tutorial*](https://github.com/key4hep/key4hep-tutorials/blob/main/gaudi_alg_higgs/README.md).

In case you do not have access to EOS (e.g. because you are not on `lxplus`),
you can also download these files via (each file contains roughly 9k events
and has around 50MB)

``` bash
wget https://key4hep.web.cern.ch/key4hep/tutorial/zh_mumu_filtered/higgs_recoil_from_gaudi_0.edm4hep.root
wget https://key4hep.web.cern.ch/key4hep/tutorial/zh_mumu_filtered/higgs_recoil_from_gaudi_1.edm4hep.root
wget https://key4hep.web.cern.ch/key4hep/tutorial/zh_mumu_filtered/higgs_recoil_from_gaudi_2.edm4hep.root
```

## Exercise

The goal of this exercise is to plot the Higgs recoil mass as well as the $Z$
mass peaks.

You should find all the necessary information on how to do this in the
[EDM4hep/podio
documentation](https://github.com/key4hep/key4hep-tutorials/blob/main/edm4hep_analysis/edm4hep_api_intro.md).
Hence, here we will only put together a list of things to do.

- `import` or `#include` the necessary bits and pieces to read files and get
  events (in the form of `podio::Frame`s).
- Open the input files and create an event loop
- Get the `Muons` collection from the file
  - Discard all events where there aren't exactly two Muons
- Get the invariant mass of the dimuon combination (and assume that it is a $Z$)
  - Remember that there is [utility
    functionality](https://edm4hep.web.cern.ch/namespaceedm4hep_1_1utils.html)
    for getting the 4 momenta
- Calculate the recoil mass using the knowledge of the colliding beams
  - I.e. the CMS system has a 4-momentum of $(0, 0, 0, 250.0)$ (ignoring
    beam-crossing angles)
- Fill two histograms, one with the $Z$ mass, one with the recoil mass

### Solutions

You can find potential solutions in the [`.solution`](.solutions/) folder
