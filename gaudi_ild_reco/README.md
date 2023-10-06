# Runing ILD simulation and reconstruction

This exercise aims at showing you how to run full simulation as well as
reconstruction using `ddsim` and the Gaudi based Key4hep framework respectively.
You will
- Run `ddsim` to produce SIM level input files for the reconstruction in both
  LCIO and EDM4hep format
- Learn how to use the tools provided by `k4MarlinWrapper` that allows to run
  workflows that were originally developed for the `Marlin` in the Gaudi based
  framework of Key4hep. This includes
  - Converting a Marlin steering file to a Gaudi options file,
  - Running this Gaudi options file via `k4run`
  - Adapting the options file to be able to read and write EDM4hep output

In this particular case we are using the ILD configuration to do this but the
conceptual steps are very similar for other detector concepts that used Marlin
originally. 

## Setup
If you haven't done it yet, source a Key4hep software environment via

```bash
source /cvmfs/sw-nightlies.hsf.org/key4hep/setup.sh
```

For the remainder of the tutorial we will assume that you are working within the
`key4hep_tut_ild_reco` directory, i.e. 

```bash 
mkdir key4hep_tut_ild_reco
cd key4hep_tut_ild_reco 
``` 

However, this is a minor detail and you can choose whatever directory you want.
We do suggest a clean directory though.

Next we will be using the the standard simulation and reconstruction
configuration for ILD which we can get via

```bash
git clone https://github.com/iLCSoft/ILDConfig
```

## Running the simulation

We will use the output file of *the whizard tutorial* as generator level input.
In case you have not done that exercise you can use the `zh_mumu0.slcio` file we
provide in the [`input_files`](./input_files) folder. You can also get it via

```bash
wget https://raw.githubusercontent.com/key4hep/key4hep-tutorials/main/gaudi_ild_reco/input_files/zh_mumu0.slcio
```

Simulating a few events with `ddsim` is straight forward. `ddsim` can produce
EDM4hep and LCIO format output files, and it decides which format to used based
on the name of the output file:
- Names ending on `.slcio` will result in LCIO output files
- Names ending in `edm4hep.root` will resultin in EDM4hep output files

::::{tab-set}

::::

- Run simulation
``` bash
ddsim \
      --compactFile $lcgeo_DIR/ILD/compact/ILD_l5_v02/ILD_l5_v02.xml \
      --steeringFile ddsim_steer.py
      --inputFiles Examples/bbudsc_3evt/bbudsc_3evt.stdhep \
      --outputFile zh_mumu0_SIM.slcio
```

- convert steering file
```bash
cd iLCSoft/ILDConfig/StandardConfig/production
Marlin -n --constant.DetectorModel=ILD_l5_o2_v02
convertMarlinSteeringToGaudi.py MarlinStdRecoParsed.xml MarlinStdReco.py
```


- Adapt the setting of `lcgeo_DIR`

```bash
sed -i '1s/^/import os\n/' MarlinStdReco.py
sed -i 's/\( *.lcgeo_DIR.:\).*/\1 os.environ["lcgeo_DIR"],'/ MarlinStdReco.py
```

- Remove the background overlay from running by commenting the lines where the
  `BgOverlayWW`, `BgOverlayBB`, `BgOverlayBW` and `BgOverlayWB` algorithms are
  appended to the `algList`

``` bash
sed -i 's/algList.append(BgOverlayWW)/# algList.append(BgOverlayWW)/' MarlinStdReco.py
sed -i 's/algList.append(BgOverlayWB)/# algList.append(BgOverlayWB)/' MarlinStdReco.py
sed -i 's/algList.append(BgOverlayBW)/# algList.append(BgOverlayBW)/' MarlinStdReco.py
sed -i 's/algList.append(BgOverlayBB)/# algList.append(BgOverlayBB)/' MarlinStdReco.py
sed -i 's/algList.append(PairBgOverlay)/# algList.append(PairBgOverlay)/' MarlinStdReco.py
```

- Run with LCIO

```bash
k4run MarlinStdReco.py --LcioEvent.Files=zh_mumu0_SIM.slcio
```


- Replace the LCIO input with an EDM4hep / podio input. This requires some
  manual intervention.

```bash
sed -i '/Configurables/s/LcioEvent, EventDataSvc/PodioInput, k4DataSvc/' MarlinStdReco.py
sed -i 's/EventDataSvc()/k4DataSvc("EventDataSvc")/' MarlinStdReco.py
sed -i 's/read = LcioEvent()/read = PodioInput("EDM4hepReader")/' MarlinStdReco.py
sed -i 's/read.Files/read.collections/' MarlinStdReco.py
```

Edit the list of collections to read (manually). It should read

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

