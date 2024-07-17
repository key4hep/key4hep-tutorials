# Running ILD simulation and reconstruction

This exercise aims at showing you how to run full simulation as well as
reconstruction using `ddsim` and the Gaudi based Key4hep framework respectively.
You will
- Run `ddsim` to produce SIM level input files for the reconstruction in EDM4hep
  format
- Learn how to use the tools provided by
  [`k4MarlinWrapper`](https://github.com/key4hep/k4MarlinWrapper) that allows to
  run workflows that were originally developed for the `Marlin` in the Gaudi
  based framework of Key4hep. This includes
  - Converting a Marlin steering file to a Gaudi options file,
  - Adapting the options file to be able to read and write EDM4hep output
  - Running this Gaudi options file via `k4run`

In this particular case we are using the ILD configuration to do this but the
conceptual steps are very similar for other detector concepts that used Marlin
originally. 

## Setup
If you haven't done it yet, source a Key4hep software environment via

```bash
source /cvmfs/sw.hsf.org/key4hep/setup.sh
```

For the remainder of the tutorial we will assume that you are working within the
`key4hep_tut_ild_reco` directory, i.e. 

```bash 
mkdir key4hep_tut_ild_reco
cd key4hep_tut_ild_reco 
``` 

However, this is a minor detail and you can choose whatever directory you want.
We do suggest a clean directory though.

Next we will be using the standard simulation and reconstruction
configuration for ILD which we can get via

```bash
git clone https://github.com/iLCSoft/ILDConfig
```

For the rest of this tutorial we will be working in the
`ILDConfig/StandardConfig/production` folder

```bash
cd ILDConfig/StandardConfig/production
```

## Running the simulation

We will use the output file of [*the whizard
tutorial*](https://github.com/key4hep/key4hep-tutorials/blob/main/whizard_gen/README.md)
as generator level input. In case you have not done that exercise you can get
one via

```bash
wget https://raw.githubusercontent.com/key4hep/key4hep-tutorials/main/gaudi_ild_reco/input_files/zh_mumu.slcio
```

Simulating a few events with `ddsim` is straight forward. `ddsim` can produce
EDM4hep and LCIO format output files, and it decides which format to used based
on the name of the output file:
- Names ending on `.slcio` will result in LCIO output files
- Names ending in `edm4hep.root` will result in in EDM4hep output files

In the course of this exercise we will only need the EDM4hep format, we simply
provide both options for convenience here.

::::{tab-set}
:::{tab-item} EDM4hep
:sync: edm4hep

To run the simulation with EDM4hep output you can use the following command
```bash
ddsim --compactFile $k4geo_DIR/ILD/compact/ILD_l5_v02/ILD_l5_v02.xml \
      --steeringFile ddsim_steer.py \
      --inputFiles zh_mumu.slcio \
      --outputFile zh_mumu_SIM.edm4hep.root
```

:::
:::{tab-item} LCIO
:sync: lcio

To run the simulation with LCIO output you can use the following command
``` bash
ddsim --compactFile $k4geo_DIR/ILD/compact/ILD_l5_v02/ILD_l5_v02.xml \
      --steeringFile ddsim_steer.py \
      --inputFiles zh_mumu.slcio \
      --outputFile zh_mumu_SIM.slcio
```

:::
::::

Depending on the machine where you are running this, this will take up to a few
minutes to complete. You can start this and read on in the meantime.

## Reconstruction 

To run the reconstruction we will use the Gaudi based Key4hep framework. Note
that we can run the reconstruction just the same as within iLCSoft via `Marlin`.
However, we will not show that in this tutorial.

### Using `ILDReconstruction.py` from ILDConfig

In order to run the *standard reconstruction* simply run the following command

```bash
k4run ILDReconstruction.py \
      --detectorModel=ILD_l5_o1_v02 \
      --inputFiles=zh_mumu_SIM.edm4hep.root \
      --outputFileBase=zh_mumu
```

The `ILDReconstruction.py` configuration can handle several different ILD
detector configurations, so we have to choose one via the `--detectorModel`
argument. Make sure that this is compatible to the one that has been used for
simulation.

This will produce several new output files

- `zh_mumu_REC.edm4hep.root` - The output of the reconstruction in EDM4hep
  format
- `zh_mumu_AIDA.root` - The histograms that were produce by wrapped processors
  using the AIDA interface
- `zh_mumu_PfoAnalysis.root` - The output file of the Pandora PFO analysis
  processor

`ILDReconstruction.py` also supports reading LCIO inputs and will detect this
automatically from the input file name. It also supports producing LCIO output
files via the `--lcioOutput` flag, which can either be `off` (default), `on` or
`only` (for not producing any EMD4hep output).

Check [the introduction to EDM4hep /
podio](https://key4hep.github.io/key4hep-doc/how-tos/key4hep-tutorials/edm4hep_analysis/edm4hep_api_intro.html)
for more details on how to read and analyse this file.

### Creating a Gaudi options file

```{warning}
These instructions show you how to convert an existing Marlin steering file to a
Gaudi options file using the full ILD reconstruction as an example since it
exhibits a few of the potential issues that you might run into along the way.
However, we **strongly recommend using the existing reconstruction configuration
that is available from
[ILDConfig](https://github.com/iLCSoft/ILDConfig/tree/master/StandardConfig/production)
for actually running the reconstruction.** It has more features and also
supports running with different detector models, something that the following
steps will not achieve!.
```

The bulk of the work for creating such an options file from an existing Marlin
steering file in XML format can be done with the
`convertMarlinSteeringToGaudi.py` converter script. We will start by converting
the `MarlinStdReco.xml` steering file and then do some minor adjustments to the
converted options file. The main thing to consider for the ILD configuration is
that `MarlinStdReco.xml` makes use of several include statements to pull in more
configuration. Hence, we first have to create a Marlin steering file with these
includes resolved. We also have to provide a `DetectorModel` constant here,
since some of the includes depend on this.

```bash
Marlin -n MarlinStdReco.xml --constant.DetectorModel=ILD_l5_o1_v02
```

You should now have a `MarlinStdRecoParsed.xml` file. This is the one that we
will convert using the converter script via

```bash
convertMarlinSteeringToGaudi.py MarlinStdRecoParsed.xml MarlinStdReco.py
```

Since some parts of the Marlin steering file conversion can not be handled
automatically we have to make a few adjustments to `MarlinStdReco.py`. We
recommend to simply edit the file directly, but you can also use the `sed`
commands below to do these adjustments. The adjustments are:
- Give the `lcgeo_DIR` constant (first entry in the `CONSTANTS` dict) a
  meaningful value. The easiest way to do this is to simply get the value of the
  corresponding environment variable via `os.environ["lcgeo_DIR"]` (don't forget
  to `import os` at the top)
- Exclude the `BgOverlayWW`, `BgOverlayBB`, `BgOverlayBW`, `BgOverlayWB` and `PairBgOverlay`
  algorithms from being run, by simply commenting out the lines where these are
  appended to the `algList` (this list is populated at almost the end of the
  file).

:::{dropdown} `sed` commands for adjustments

``` bash
sed -i '1s/^/import os\n/' MarlinStdReco.py
sed -i 's/\( *.lcgeo_DIR.:\).*/\1 os.environ["lcgeo_DIR"],'/ MarlinStdReco.py
sed -i 's/algList.append(BgOverlayWW)/# algList.append(BgOverlayWW)/' MarlinStdReco.py
sed -i 's/algList.append(BgOverlayWB)/# algList.append(BgOverlayWB)/' MarlinStdReco.py
sed -i 's/algList.append(BgOverlayBW)/# algList.append(BgOverlayBW)/' MarlinStdReco.py
sed -i 's/algList.append(BgOverlayBB)/# algList.append(BgOverlayBB)/' MarlinStdReco.py
sed -i 's/algList.append(PairBgOverlay)/# algList.append(PairBgOverlay)/' MarlinStdReco.py
```

:::

With the state the options file is in now, you would be able to run it with LCIO
input.

:::{dropdown} Running the reconstruction with LCIO
To run the reconstruction with LCIO inputs and outputs we now simply need to
pass in the input file that we have created at the simulation step

```bash
k4run MarlinStdReco.py --LcioEvent.Files=zh_mumu_SIM.slcio
```

This should take somewhere between 20 seconds up to roughly a minute to run. If
you haven't changed anything else you should now have a few output files:

```bash
ls StandardReco_*.*
```

should now show a `REC` and `DST` file, as well as a `PfoAnalysis` and an `AIDA`
file. You can change the names of these files by adjusting the `OutputBaseName`,
resp. The corresponding filename constants values in `CONSTANTS`.

:::

### Adapting the options file for EDM4hep

It is necessary to adapt the Gaudi options file a bit further:
- Replace the `LcioEvent` algorithm with the `PodioInput` algorithm 
  - Make sure to replace the `Files` option with the `collections` option and to
    populate this option with the list of collections you want to read (see
    below)
- Replace the `EventDataSvc` with the `k4DataSvc` (remember to instantiate it
  with `"EventDataSvc"` as name)
- Add a `PodioOutput` algorithm to write EDM4hep output (don't forget to add it
  to the `algList` at the very end)
  - (For the sake of this exercise) configure this to only write the
    `MCParticlesSkimmed`, `PandoraPFOs` and the `RecoMCTruthLink` collections
- Attach the necessary in-memory on-the-fly converters between EDM4hep and LCIO
  (and vice versa)
  - For the conversion of the EDM4hep inputs to LCIO instantiate a
    `EDM4hep2LcioTool` and attach it to the first wrapped processor that is run
    (`MyAIDAProcessor`). See detailed description below.
  - For the conversion of the LCIO outputs to EDM4hep instantiate a
    `Lcio2EDM4hepTool` and attach it to the last wrapped processor that is run
    before the `PodioOutput` algorithm that you just added (`MyPfoAnalysis`).
    Also see below.
    
**For all of these steps make sure that you `import` all the necessary tools and
algorithms from `Configurables`!**
  
The top of your file should now look something like this

```python
from Configurables import (
    PodioInput, PodioOutput, k4DataSvc, MarlinProcessorWrapper,
    EDM4hep2LcioTool, Lcio2EDM4hepTool
    )
from k4MarlinWrapper.parseConstants import *
algList = []
evtsvc = k4DataSvc("EventDataSvc")
```

while the configuration for the input reader and the `EDM4hep2LcioTool` should
look like this

```python
read = PodioInput()
read.OutputLevel = INFO
read.collections = [
    # ... list of collection names
]
algList.append(read)

edm4hep2LcioConv = EDM4hep2LcioTool()
edm4hep2LcioConv.collNameMapping = {
    "MCParticles": "MCParticle"
}

# ... Unchanged config of MyAIDAProcessor

MyAIDAProcessor.EDM4hep2LcioTool = edm4hep2LcioConv
```

:::{dropdown} list of collection names

The list of collections that is populated by standard configuration of ILD for
simulation looks like this. You can simply copy this into the options file

```python
read.collections = [
     "BeamCalCollection",
     "BeamCalCollectionContributions",
     "ECalBarrelScHitsEven",
     "ECalBarrelScHitsEvenContributions",
     "ECalBarrelScHitsOdd",
     "ECalBarrelScHitsOddContributions",
     "ECalBarrelSiHitsEven",
     "ECalBarrelSiHitsEvenContributions",
     "ECalBarrelSiHitsOdd",
     "ECalBarrelSiHitsOddContributions",
     "EcalEndcapRingCollection",
     "EcalEndcapRingCollectionContributions",
     "ECalEndcapScHitsEven",
     "ECalEndcapScHitsEvenContributions",
     "ECalEndcapScHitsOdd",
     "ECalEndcapScHitsOddContributions",
     "ECalEndcapSiHitsEven",
     "ECalEndcapSiHitsEvenContributions",
     "ECalEndcapSiHitsOdd",
     "ECalEndcapSiHitsOddContributions",
     "EventHeader",
     "FTDCollection",
     "HcalBarrelRegCollection",
     "HcalBarrelRegCollectionContributions",
     "HCalBarrelRPCHits",
     "HCalBarrelRPCHitsContributions",
     "HCalECRingRPCHits",
     "HCalECRingRPCHitsContributions",
     "HcalEndcapRingCollection",
     "HcalEndcapRingCollectionContributions",
     "HCalEndcapRPCHits",
     "HCalEndcapRPCHitsContributions",
     "HcalEndcapsCollection",
     "HcalEndcapsCollectionContributions",
     "LHCalCollection",
     "LHCalCollectionContributions",
     "LumiCalCollection",
     "LumiCalCollectionContributions",
     "MCParticles",
     "SETCollection",
     "SITCollection",
     "TPCCollection",
     "TPCLowPtCollection",
     "TPCSpacePointCollection",
     "VXDCollection",
     "YokeBarrelCollection",
     "YokeBarrelCollectionContributions",
     "YokeEndcapsCollection",
     "YokeEndcapsCollectionContributions",
]
```

:::

Finally, the `PodioOutput` algorithm and the `Lcio2EDM4hepTool` can be
configuration should look something like this

```python
# ... MyPfoAnalysis configuration unchanged

lcio2edm4hepConv = Lcio2EDM4hepTool()
lcio2edm4hepConv.collNameMapping = {
    "MCParticle": "MCParticles"
}
MyPfoAnalysis.Lcio2EDM4hepTool = lcio2edm4hepConv

edm4hepOutput = PodioOutput()
edm4hepOutput.filename = "zh_mumu_reco.edm4hep.root"
edm4hepOutput.outputCommands = [
    "drop *",
    "keep MCParticlesSkimmed",
    "keep PandoraPFOs",
    "keep RecoMCTruthLink",
]

# ... the complete algList
algList.append(edm4hepOutput)

# ... ApplicationMgr config
```

### Running the reconstruction with `k4run`

After all these adaptions it is now possible to run the full reconstruction
chain on the previously simulated input with `k4run`

```bash
k4run MarlinStdReco.py --num-events=3 --EventDataSvc.input=zh_mumu_SIM.edm4hep.root
```

Here we are again using the command line to specify the input file, we could
have just as well used the `input` option of the `evtsvc` in the options file.
Note also that we explicitly pass in the number of events, this is a workaround
for [this issue](https://github.com/key4hep/k4MarlinWrapper/issues/94).

You should now have a `zh_mumu_reco.edm4hep.root` file that contains the
complete events in all their glory. For a more practical output you can tweak
the `edm4hepOutput.outputCommands` option in order to keep only "interesting"
collections. Also note that the REC and DST LCIO output files are still
produced. Can you reproduce these data tiers for EDM4hep?
