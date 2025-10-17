# Replacing an existing Marlin processor with a Gaudi algorithm

In this tutorial we are going to explore how to run a (very simple) kinematic
fit inside Gaudi. The final goal is to replace an existing Marlin processor in
the full ILD reconstruction chain with a newly developed (set of) Gaudi
algorithm(s).

In the course of this tutorial you will see how to
- Run the standard simulation and reconstruction for ILD using Gaudi
- Write your own Gaudi algorithm using an existing Marlin processor as a
  template
- Run and test this algorithm in isolation
- Do some analysis and validation on the outputs of the newly developed
  algorithm
- Replace the original (wrapped) Marlin Processor with the newly developed
  algorithm

One of the key goals of this tutorial are to 
- Show how to use and combine existing SW tools from the linear collider (LC)
  stack with Gaudi
- Highlight a few of the common pitfalls that one potentially encounters when
  doing so

The tutorial is structured such that it should be possible to follow this as a
standalone exercise. It was developed originally for the [LCWS
2025](https://agenda.linearcollider.org/event/10594/) where it was also given as
an in person exercise. The estimated time for completion there was 3 hours
including some general introduction to Gaudi.

## Pre-requisites
The tutorial assumes that you are familiar with the basics of Gaudi as well as
c++ and python. You should have heard of *Functional* algorithms in this context
and at least have some vague conceptual understanding of what they are.

We also assume a basic understanding of [EDM4hep](https://edm4hep.web.cern.ch)
and [podio](https://key4hep.web.cern.ch/podio), but we try in all cases to give
at least a very basic introduction or at least some pointers to relevant
information.

We have tested the instructions for this tutorial to run against a Key4hep
software environment deployed via CVMFS (e.g. on *lxplus* or the *NAF*). It will
also work if you have CVMFS installed locally and run a supported OS for
Key4hep.

## Setup

You can follow this tutorial in two ways, either you use a CVMFS installation
(available for Ubuntu24 and Alma9, e.g. via `lxplus` or the `NAF`) or you use a
dedicated container image that we have created for this purpose.

If you haven't done so already, source a Key4hep software environment via
::::{tab-set}
:::{tab-item} CVMFS installation
:sync: cvmfs-setup

Login into your favorite system (or use a local CVMFS installation) and source
the following setup script.
```bash
source /cvmfs/sw-nightlies.hsf.org/key4hep/setup.sh
```

For the remainder of the tutorial we will also assume that you are working
within some tutorial directory. We will refer to this with the
`TUTORIAL_BASE_DIR` environment variable, something along the lines of

```bash
# cd <path/where/you/have/some/space>
mkdir tutorial_lcws
cd tutorial_lcws
export TUTORIAL_BASE_DIR=$(pwd)
```

:::
:::{tab-item} Container image (docker)
:sync: container-setup-docker

For the remainder of the tutorial we will also assume that you are working
within some tutorial directory. We will refer to this with the
`TUTORIAL_BASE_DIR` environment variable. Using the commands below this will all
be done for you and launch the container appropriately.

```bash
# cd <path/where/you/have/some/space>
mkdir tutorial_lcws
docker run -it \
    -v $(pwd)/tutorial_lcws:/resources/tutorial_lcws \
    -e TUTORIAL_BASE_DIR=/resources/tutorial_lcws \
    ghcr.io/key4hep/key4hep-sim-reco-ubuntu24:lcws-tutorial
```

This will immediately drop you into a Key4hep environment that is usable for
this tutorial.
:::

:::{tab-item} Container image (apptainer)
:sync: container-setup-apptainer

For the remainder of the tutorial we will also assume that you are working
within some tutorial directory. We will refer to this with the
`TUTORIAL_BASE_DIR` environment variable. Using the commands below this will all
be done for you and launch the container appropriately.

```bash
# cd <path/where/you/have/some/space>
mkdir tutorial_lcws
apptainer run \
    -B $(pwd)/tutorial_lcws:/resources/tutorial_lcws \
    --env TUTORIAL_BASE_DIR=/resources/tutorial_lcws \
    docker://ghcr.io/key4hep/key4hep-sim-reco-ubuntu24:lcws-tutorial
```

:::
::::

### Input data

For some of the steps it will be useful to have some input files with more
statistics. Since running full simulation and reconstruction takes a
considerable amount of time for these, we have prepared these files in advance.
We will describe the contents of these input files in more detail a bit furher
below. To easily refer to these resources we will also assume that the
`RESOURCE_DIR` environment variable points to these input files

::::{tab-set}

:::{tab-item} Locally or not on lxplus
:sync: non-lxplus

If you are running on a workgroup server other than `lxplus` (e.g. on the NAF)
or locally, e.g. using a container, you need to download some files first via

``` bash
mkdir ${TUTORIAL_BASE_DIR}/resources
cd ${TUTORIAL_BASE_DIR}/resources
export RESOURCE_DIR=$(pwd)
wget https://key4hep.web.cern.ch/tutorial/gen_tau_filtered/gen_tau_pi0_250-SetA_2f_leptonic_eL_pR.slcio
wget https://key4hep.web.cern.ch/tutorial/gen_tau_filtered/gen_tau_pi0_250-SetA_2f_leptonic_eL_pR_REC_0.edm4hep.root
```

:::
:::{tab-item} lxplus
:sync: lxplus

If you are running on `lxplus`, you should have access to the *EOS* filesystem
and you can simply use that via
```bash
export RESOURCE_DIR=/eos/project/k/key4hep/www/key4hep/tutorial/gen_tau_filtered
```

:::
::::

The `gen_tau_pi0_250-SetA_2f_leptonic_eL_pR.slcio` file contains a skim of
events from the `2f_leptonic` samples of the MC-2020 production, selecting only
events with at least one hadronically decaying tau resulting in at least one
neutral pion. Additionally, at least one of the photons from the pi0 decay has
to have an energy of more than 10 GeV.

The `gen_tau_pi0_250-SetA_2f_leptonic_eL_pR_REC_0.edm4hep.root` contains a few
hundred events simulated and reconstructed via the ILD standard chain. It only
contains a filtered down event content, which should allow you to test the
algorithms you develop with a few hundred events to also give some more
meaningful plots. More files similar to this are available by simply replacing
the `0` with numbers up to `9`. However, for the purpose of this tutorial one
file should suffice.

## Running standard ILD full simulation and reconstruction

In order to run the standard ILD full simulation and reconstruction we first
have to fetch the necessary configuration files by cloning
[ILDConfig](https://github.com/iLCSoft/ILDConfig). We also create another anchor
for later; `ILDCONFIG_DIR`

```bash
cd ${TUTORIAL_BASE_DIR}
git clone --depth=1 https://github.com/iLCSoft/ILDConfig
cd ILDConfig/StandardConfig/production
export ILDCONFIG_DIR=$(pwd)
```

### Simulation
It is now possible to run the simulation using the input file that is available
from the `RESOURCE_DIR` (see [above](#setup)).

``` bash
cd $ILDCONFIG_DIR
ddsim \
  --inputFiles ${RESOURCE_DIR}/gen_tau_pi0_250-SetA_2f_leptonic_eL_pR.slcio \
  --outputFile gen_tau_pi0_SIM.root \
  --compactFile $k4geo_DIR/ILD/compact/ILD_l5_v02/ILD_l5_v02.xml \
  --steeringFile ddsim_steer.py \
  -N 20
```

This will take roughly 2-3 minutes depending on the machine where you are
running this on. Feel free to leave this running while you read on for the next
steps.

```{note}
The input file contains around 6.8k events. However, given that it takes a few
seconds per event to fully simulate them we limit the number of events we
simulate to 20 here. Fully simulated files with more events are also available,
but for developing and some first tests of the later steps these 20 events
should suffice.
```

### Reconstruction
For reconstruction we use the standard configuration for ILD:
[`ILDReconstruction.py`](https://github.com/iLCSoft/ILDConfig/blob/master/StandardConfig/production/ILDReconstruction.py)

We will not go into too much detail of this for this tutorial. However, since it
will become relevant later we do briefly mention the basic capabilities and
principles of this configuration. The first thing to note is that this is a
fairly large configuration, not least due to the fact, that it is also fairly
configurable. 
- It can transparently handle EDM4hep and LCIO inputs as well as produce either
  (or both) of them.
- The detector geometry to run over is configurable from the command line. It
  will also automatically switch to a different set of algorithms in case a
  geometry that is targetting FCCee is found
- Different reconstruction options can be toggled as well

The algorithms that are run are almost exclusively *Marlin Processors* that are
run via the *MarlinProcessorWrapper* algorithm. We refer to the [comprehensive
documentation](https://key4hep.github.io/key4hep-doc/main/how-tos/k4marlinwrapper/doc/MarlinWrapperIntroduction.html)
for more details.

One key aspect is that not all of the algorithms are configured in the top level
`ILDReconstruction.py` file. Rather it is split into smaller logically grouped
sequences of algorithms that are dynamically imported, depending on the value of
some other configuration parameters. This is done via the
[`SequenceLoader`](https://github.com/iLCSoft/ILDConfig/blob/38860c2a63c2586845f81e78a21d0c3ddbb6afe9/StandardConfig/production/py_utils.py#L81)
helper class. The exact details are not extremely relevant for the purposes of
the rest of the tutorial. The main thing to understand is that e.g.

``` python
sequenceLoader.load("HighLevelReco/HighLevelReco")
```

will import the contents of the
[`HighLevelReco/HighLevelReco.py`](https://github.com/iLCSoft/ILDConfig/blob/master/StandardConfig/production/HighLevelReco/HighLevelReco.py)
file. As you can see this file defines and configures a few high level
reconstruction algorithms and defines their running order in the final
`HighLevelRecoSequence` list at the bottom of the file.

Now that we have a basic understanding of the structure of the ILD
reconstrucion, we can run it via

```bash
cd $ILDCONFIG_DIR
k4run ILDReconstruction.py \
  --inputFiles=gen_tau_pi0_SIM.root \
  --compactFile $k4geo_DIR/ILD/compact/ILD_l5_v02/ILD_l5_o1_v02.xml \
  --outputFileBase=gen_tau_pi0  \
  --num-events=-1
```

You should now have a few output files, something like

```console
$ ls -lh gen_tau_pi0*
-rw-r--r--. 1 madlener af-ilc 23K Oct 10 20:08 gen_tau_pi0_AIDA.root
-rw-r--r--. 1 madlener af-ilc 29K Oct 10 20:08 gen_tau_pi0_PfoAnalysis.root
-rw-r--r--. 1 madlener af-ilc 45M Oct 10 20:08 gen_tau_pi0_REC.edm4hep.root
-rw-r--r--. 1 madlener af-ilc 44M Oct 10 20:08 gen_tau_pi0_SIM.root
```

The one that we are mainly interested in for the rest of the tutorial is
`gen_tau_pi0_REC.edm4hep.root`. This is the EDM4hep output of the full
simulation. We can investigate the contents of the edm4hep files on a high level
using the [`podio-dump`
tool](https://key4hep.github.io/key4hep-doc/main/how-tos/key4hep-tutorials/edm4hep_analysis/edm4hep_api_intro.html#podio-dump)

```bash
podio-dump gen_tau_pi0_REC.edm4hep.root
```

:::{dropdown} `podio-dump` Output

```
input file: gen_tau_pi0_REC.edm4hep.root
            (written with podio version: 1.5.0)

datamodel model definitions stored in this file:
 - edm4hep (0.99.99)

Frame categories in this file:
Name                    Entries  
----------------------  -------  
metadata                1        
events                  20       
configuration_metadata  1        
################################### events: 0 ####################################
Collections:
Name                                            ValueType                                                        Size   ID        
----------------------------------------------  ---------------------------------------------------------------  -----  --------  
BCAL                                            edm4hep::CalorimeterHit                                          0      f6ebd0e3  
BeamCalCollection                               edm4hep::SimCalorimeterHit                                       0      c298a348  
BeamCalCollectionContributions                  edm4hep::CaloHitContribution                                     0      a1b6cac8  
BuildUpVertex                                   edm4hep::Vertex                                                  1      4b3ce322  
BuildUpVertex_associatedParticles               podio::Link<edm4hep::Vertex,edm4hep::ReconstructedParticle>      1      778fbf01  
BuildUpVertex_RP                                edm4hep::ReconstructedParticle                                   1      c1ce60c0  
BuildUpVertex_RP_startVertices                  podio::Link<edm4hep::Vertex,edm4hep::ReconstructedParticle>      0      b481ddb3  
BuildUpVertex_V0                                edm4hep::Vertex                                                  0      77941bb6  
BuildUpVertex_V0_associatedParticles            podio::Link<edm4hep::Vertex,edm4hep::ReconstructedParticle>      0      ff218acc  
BuildUpVertex_V0_RP                             edm4hep::ReconstructedParticle                                   0      5784a361  
BuildUpVertex_V0_RP_startVertices               podio::Link<edm4hep::Vertex,edm4hep::ReconstructedParticle>      0      cb7e0119  
ClupatraTracks                                  edm4hep::Track                                                   4      264d1493  
ClupatraTracks_dQdx                             edm4hep::RecDqdx                                                 4      5acc110a  
ClupatraTrackSegments                           edm4hep::Track                                                   0      c80f6454  
ClupatraTrackSegments_dQdx                      edm4hep::RecDqdx                                                 0      a2098c13  
ClusterMCTruthLink                              podio::Link<edm4hep::Cluster,edm4hep::MCParticle>                17     e2727d90  
DistilledPFOs                                   edm4hep::ReconstructedParticle                                   6      dfdcddaf  
EcalBarrelCollection                            edm4hep::SimCalorimeterHit                                       3076   62f19310  
EcalBarrelCollectionDigi                        edm4hep::CalorimeterHit                                          2523   37ae0fab  
EcalBarrelCollectionGapHits                     edm4hep::CalorimeterHit                                          57     0b0263f2  
EcalBarrelCollectionRec                         edm4hep::CalorimeterHit                                          2523   287ce0a9  
EcalBarrelRelationsSimDigi                      podio::Link<edm4hep::CalorimeterHit,edm4hep::SimCalorimeterHit>  2523   9d52130b  
EcalBarrelRelationsSimRec                       podio::Link<edm4hep::CalorimeterHit,edm4hep::SimCalorimeterHit>  2523   df5574a9  
ECalBarrelScHitsEvenContributions               edm4hep::CaloHitContribution                                     21239  c8d02283  
ECalBarrelScHitsOddContributions                edm4hep::CaloHitContribution                                     20998  2b8dc170  
ECalBarrelSiHitsEven                            edm4hep::SimCalorimeterHit                                       1503   4316638a  
ECalBarrelSiHitsEvenContributions               edm4hep::CaloHitContribution                                     23555  7aeff7cb  
ECalBarrelSiHitsOdd                             edm4hep::SimCalorimeterHit                                       1573   9ebccc5c  
ECalBarrelSiHitsOddContributions                edm4hep::CaloHitContribution                                     25262  dc73e57f  
EcalEndcapRingCollection                        edm4hep::SimCalorimeterHit                                       0      cdf19862  
EcalEndcapRingCollectionContributions           edm4hep::CaloHitContribution                                     0      a7ec6089  
EcalEndcapRingCollectionDigi                    edm4hep::CalorimeterHit                                          0      b142d1dc  
EcalEndcapRingCollectionRec                     edm4hep::CalorimeterHit                                          0      e2e8649e  
EcalEndcapRingRelationsSimDigi                  podio::Link<edm4hep::CalorimeterHit,edm4hep::SimCalorimeterHit>  0      93e02071  
EcalEndcapRingRelationsSimRec                   podio::Link<edm4hep::CalorimeterHit,edm4hep::SimCalorimeterHit>  0      730bad7f  
ECalEndcapScHitsEvenContributions               edm4hep::CaloHitContribution                                     37     65d77aa2  
ECalEndcapScHitsOddContributions                edm4hep::CaloHitContribution                                     15     9b0f4634  
EcalEndcapsCollection                           edm4hep::SimCalorimeterHit                                       6      d6d26d20  
EcalEndcapsCollectionDigi                       edm4hep::CalorimeterHit                                          1      5f384992  
EcalEndcapsCollectionGapHits                    edm4hep::CalorimeterHit                                          0      d6a16810  
EcalEndcapsCollectionRec                        edm4hep::CalorimeterHit                                          1      0485d5e2  
ECalEndcapSiHitsEven                            edm4hep::SimCalorimeterHit                                       3      42edbbbd  
ECalEndcapSiHitsEvenContributions               edm4hep::CaloHitContribution                                     18     9bf74d0a  
ECalEndcapSiHitsOdd                             edm4hep::SimCalorimeterHit                                       3      1c39453c  
ECalEndcapSiHitsOddContributions                edm4hep::CaloHitContribution                                     10     9e0ccf88  
EcalEndcapsRelationsSimDigi                     podio::Link<edm4hep::CalorimeterHit,edm4hep::SimCalorimeterHit>  1      b23406fa  
EcalEndcapsRelationsSimRec                      podio::Link<edm4hep::CalorimeterHit,edm4hep::SimCalorimeterHit>  1      1a339e32  
EventHeader                                     edm4hep::EventHeader                                             1      d793ab91  
ForwardTracks                                   edm4hep::Track                                                   0      b7e64675  
ForwardTracks_dQdx                              edm4hep::RecDqdx                                                 0      d0daea17  
FTD_PIXELCollection                             edm4hep::SimTrackerHit                                           0      36051c2f  
FTD_STRIPCollection                             edm4hep::SimTrackerHit                                           0      91aaa2a9  
FTDCollection                                   edm4hep::SimTrackerHit                                           0      70ad1764  
FTDPixelTrackerHitRelations                     podio::Link<edm4hep::TrackerHit,edm4hep::SimTrackerHit>          0      90ec0e5d  
FTDPixelTrackerHits                             edm4hep::TrackerHitPlane                                         0      841c4c69  
FTDSpacePointRelations                          podio::Link<edm4hep::TrackerHit,edm4hep::SimTrackerHit>          0      0ede6e72  
FTDSpacePoints                                  edm4hep::TrackerHit3D                                            0      f261d80b  
FTDStripTrackerHitRelations                     podio::Link<edm4hep::TrackerHit,edm4hep::SimTrackerHit>          0      873e37f6  
FTDStripTrackerHits                             edm4hep::TrackerHitPlane                                         0      5dc33fa1  
GammaGammaCandidateEtaPrimes                    edm4hep::ReconstructedParticle                                   0      e430425d  
GammaGammaCandidateEtaPrimes_startVertices      podio::Link<edm4hep::Vertex,edm4hep::ReconstructedParticle>      0      a9b6c1b4  
GammaGammaCandidateEtas                         edm4hep::ReconstructedParticle                                   0      7d565d48  
GammaGammaCandidateEtas_startVertices           podio::Link<edm4hep::Vertex,edm4hep::ReconstructedParticle>      0      11070356  
GammaGammaCandidatePi0s                         edm4hep::ReconstructedParticle                                   1      ea935713  
GammaGammaCandidatePi0s_startVertices           podio::Link<edm4hep::Vertex,edm4hep::ReconstructedParticle>      0      6bd61357  
GammaGammaParticles                             edm4hep::ReconstructedParticle                                   1      d36b2924  
HcalBarrelCollectionDigi                        edm4hep::CalorimeterHit                                          579    43b62520  
HcalBarrelCollectionRec                         edm4hep::CalorimeterHit                                          579    5dc52a38  
HcalBarrelRegCollection                         edm4hep::SimCalorimeterHit                                       5187   fca14938  
HcalBarrelRegCollectionContributions            edm4hep::CaloHitContribution                                     13678  1fba4d71  
HcalBarrelRelationsSimDigi                      podio::Link<edm4hep::CalorimeterHit,edm4hep::SimCalorimeterHit>  579    03c20d3e  
HcalBarrelRelationsSimRec                       podio::Link<edm4hep::CalorimeterHit,edm4hep::SimCalorimeterHit>  579    a173edda  
HCalBarrelRPCHitsContributions                  edm4hep::CaloHitContribution                                     1269   73752026  
HCalECRingRPCHitsContributions                  edm4hep::CaloHitContribution                                     0      ff9a4c31  
HcalEndcapRingCollection                        edm4hep::SimCalorimeterHit                                       1      2cf7e1be  
HcalEndcapRingCollectionContributions           edm4hep::CaloHitContribution                                     1      24751e9a  
HcalEndcapRingCollectionDigi                    edm4hep::CalorimeterHit                                          0      ce498a9f  
HcalEndcapRingCollectionRec                     edm4hep::CalorimeterHit                                          0      298b321c  
HcalEndcapRingRelationsSimDigi                  podio::Link<edm4hep::CalorimeterHit,edm4hep::SimCalorimeterHit>  0      337f38db  
HcalEndcapRingRelationsSimRec                   podio::Link<edm4hep::CalorimeterHit,edm4hep::SimCalorimeterHit>  0      9538a0c4  
HCalEndcapRPCHitsContributions                  edm4hep::CaloHitContribution                                     0      0d7a1074  
HcalEndcapsCollection                           edm4hep::SimCalorimeterHit                                       31     4cc49016  
HcalEndcapsCollectionContributions              edm4hep::CaloHitContribution                                     60     5c69a46e  
HcalEndcapsCollectionDigi                       edm4hep::CalorimeterHit                                          1      87e4cc7a  
HcalEndcapsCollectionRec                        edm4hep::CalorimeterHit                                          1      5692d758  
HcalEndcapsRelationsSimDigi                     podio::Link<edm4hep::CalorimeterHit,edm4hep::SimCalorimeterHit>  1      d81dbf2d  
HcalEndcapsRelationsSimRec                      podio::Link<edm4hep::CalorimeterHit,edm4hep::SimCalorimeterHit>  1      8e670bee  
KinkRecoParticles                               edm4hep::ReconstructedParticle                                   0      d1fed390  
KinkRecoParticles_startVertices                 podio::Link<edm4hep::Vertex,edm4hep::ReconstructedParticle>      0      bcd2f4a3  
KinkVertices                                    edm4hep::Vertex                                                  0      67154bfd  
KinkVertices_associatedParticles                podio::Link<edm4hep::Vertex,edm4hep::ReconstructedParticle>      0      c7c3d304  
LCAL                                            edm4hep::CalorimeterHit                                          0      204bcc12  
LHCAL                                           edm4hep::CalorimeterHit                                          0      82c94b29  
LHCalCollection                                 edm4hep::SimCalorimeterHit                                       0      fe7e35be  
LHCalCollectionContributions                    edm4hep::CaloHitContribution                                     0      f612984f  
LumiCalCollection                               edm4hep::SimCalorimeterHit                                       0      45759015  
LumiCalCollectionContributions                  edm4hep::CaloHitContribution                                     0      dab2a7ce  
MarlinTrkTracks                                 edm4hep::Track                                                   4      7266ea44  
MarlinTrkTracks_dQdx                            edm4hep::RecDqdx                                                 4      1e3e8bd1  
MarlinTrkTracksKaon                             edm4hep::Track                                                   4      9b4b210d  
MarlinTrkTracksKaon_dQdx                        edm4hep::RecDqdx                                                 4      bf4fdd69  
MarlinTrkTracksKaonMCP                          podio::Link<edm4hep::Track,edm4hep::MCParticle>                  0      d105452e  
MarlinTrkTracksMCTruthLink                      podio::Link<edm4hep::Track,edm4hep::MCParticle>                  4      7e7a344f  
MarlinTrkTracksProton                           edm4hep::Track                                                   4      4c3b9a42  
MarlinTrkTracksProton_dQdx                      edm4hep::RecDqdx                                                 4      c9d533d0  
MarlinTrkTracksProtonMCP                        podio::Link<edm4hep::Track,edm4hep::MCParticle>                  0      04e8d978  
MCParticles                                     edm4hep::MCParticle                                              28     a1cba250  
MCParticlesSkimmed                              edm4hep::MCParticle                                              21     16b252ac  
MCTruthClusterLink                              podio::Link<edm4hep::Cluster,edm4hep::MCParticle>                17     10793d25  
MCTruthMarlinTrkTracksLink                      podio::Link<edm4hep::Track,edm4hep::MCParticle>                  4      27787578  
MCTruthRecoLink                                 podio::Link<edm4hep::ReconstructedParticle,edm4hep::MCParticle>  17     dc1423e8  
MUON                                            edm4hep::CalorimeterHit                                          1      0f355ef3  
PandoraClusters                                 edm4hep::Cluster                                                 7      1ada7608  
PandoraPFANewStartVertices                      edm4hep::Vertex                                                  7      95bc4ba3  
PandoraPFANewStartVertices_associatedParticles  podio::Link<edm4hep::Vertex,edm4hep::ReconstructedParticle>      7      151342a3  
PandoraPFOs                                     edm4hep::ReconstructedParticle                                   7      fa28d9be  
PandoraPFOs_PID_BasicVariablePID                edm4hep::ParticleID                                              4      9cc40aa6  
PandoraPFOs_PID_dEdxPID                         edm4hep::ParticleID                                              4      e82a9284  
PandoraPFOs_PID_LikelihoodPID                   edm4hep::ParticleID                                              4      e0553960  
PandoraPFOs_PID_LowMomMuID                      edm4hep::ParticleID                                              4      c024d50d  
PandoraPFOs_PID_ShowerShapesPID                 edm4hep::ParticleID                                              4      9dd76e70  
PandoraPFOs_PID_TOFEstimators0ps                edm4hep::ParticleID                                              7      dd901b8c  
PandoraPFOs_PID_TOFEstimators100ps              edm4hep::ParticleID                                              7      71fe9f72  
PandoraPFOs_PID_TOFEstimators10ps               edm4hep::ParticleID                                              7      b3db3b27  
PandoraPFOs_PID_TOFEstimators50ps               edm4hep::ParticleID                                              7      97c82aec  
PandoraPFOs_PID_TrackLengthProcessor            edm4hep::ParticleID                                              7      f01378f7  
PandoraPFOs_startVertices                       podio::Link<edm4hep::Vertex,edm4hep::ReconstructedParticle>      3      87688bf5  
PrimaryVertex                                   edm4hep::Vertex                                                  1      d2a1d555  
PrimaryVertex_associatedParticles               podio::Link<edm4hep::Vertex,edm4hep::ReconstructedParticle>      1      427d1738  
PrimaryVertex_RP                                edm4hep::ReconstructedParticle                                   1      c8b21469  
PrimaryVertex_RP_startVertices                  podio::Link<edm4hep::Vertex,edm4hep::ReconstructedParticle>      0      68e585a8  
ProngRecoParticles                              edm4hep::ReconstructedParticle                                   0      f338aaff  
ProngRecoParticles_startVertices                podio::Link<edm4hep::Vertex,edm4hep::ReconstructedParticle>      0      f3ca8a7c  
ProngVertices                                   edm4hep::Vertex                                                  0      b8af33f8  
ProngVertices_associatedParticles               podio::Link<edm4hep::Vertex,edm4hep::ReconstructedParticle>      0      2fa78077  
RecoMCTruthLink                                 podio::Link<edm4hep::ReconstructedParticle,edm4hep::MCParticle>  17     6b81837a  
RelationBCalHit                                 podio::Link<edm4hep::CalorimeterHit,edm4hep::SimCalorimeterHit>  0      16db0c64  
RelationLcalHit                                 podio::Link<edm4hep::CalorimeterHit,edm4hep::SimCalorimeterHit>  0      385d5b37  
RelationLHcalHit                                podio::Link<edm4hep::CalorimeterHit,edm4hep::SimCalorimeterHit>  0      80e932e3  
RelationMuonHit                                 podio::Link<edm4hep::CalorimeterHit,edm4hep::SimCalorimeterHit>  1      df24625a  
SETCollection                                   edm4hep::SimTrackerHit                                           15     0426d8e5  
SETSpacePointRelations                          podio::Link<edm4hep::TrackerHit,edm4hep::SimTrackerHit>          8      ccfb6226  
SETSpacePoints                                  edm4hep::TrackerHit3D                                            4      9ae08aff  
SETTrackerHitRelations                          podio::Link<edm4hep::TrackerHit,edm4hep::SimTrackerHit>          15     6ebe4659  
SETTrackerHits                                  edm4hep::TrackerHitPlane                                         15     1bcf6c70  
SITCollection                                   edm4hep::SimTrackerHit                                           17     3d720f34  
SiTracks                                        edm4hep::Track                                                   4      82d755bd  
SiTracks_dQdx                                   edm4hep::RecDqdx                                                 4      14cea21e  
SITTrackerHitRelations                          podio::Link<edm4hep::TrackerHit,edm4hep::SimTrackerHit>          17     dc11ff9d  
SITTrackerHits                                  edm4hep::TrackerHitPlane                                         17     2127d3c9  
SplitRecoParticles                              edm4hep::ReconstructedParticle                                   0      1b27bab1  
SplitRecoParticles_startVertices                podio::Link<edm4hep::Vertex,edm4hep::ReconstructedParticle>      0      d24e6609  
SplitVertices                                   edm4hep::Vertex                                                  0      4c3daf03  
SplitVertices_associatedParticles               podio::Link<edm4hep::Vertex,edm4hep::ReconstructedParticle>      0      97a389f2  
SubsetTracks                                    edm4hep::Track                                                   4      dac1deb9  
SubsetTracks_dQdx                               edm4hep::RecDqdx                                                 4      d4c47519  
TPCCollection                                   edm4hep::SimTrackerHit                                           880    1649dae6  
TPCLowPtCollection                              edm4hep::SimTrackerHit                                           0      ea1366dc  
TPCSpacePointCollection                         edm4hep::SimTrackerHit                                           0      18f61713  
TPCTrackerHitRelations                          podio::Link<edm4hep::TrackerHit,edm4hep::SimTrackerHit>          876    efb4f127  
TPCTrackerHits                                  edm4hep::TrackerHit3D                                            876    9134c63f  
V0RecoParticles                                 edm4hep::ReconstructedParticle                                   0      827bbcbb  
V0RecoParticles_startVertices                   podio::Link<edm4hep::Vertex,edm4hep::ReconstructedParticle>      0      c7c093b6  
V0Vertices                                      edm4hep::Vertex                                                  0      e7a4abb7  
V0Vertices_associatedParticles                  podio::Link<edm4hep::Vertex,edm4hep::ReconstructedParticle>      0      57cb230e  
VXDCollection                                   edm4hep::SimTrackerHit                                           24     67202e26  
VXDTrackerHitRelations                          podio::Link<edm4hep::TrackerHit,edm4hep::SimTrackerHit>          24     178c9330  
VXDTrackerHits                                  edm4hep::TrackerHitPlane                                         24     db43b881  
YokeBarrelCollection                            edm4hep::SimCalorimeterHit                                       89     97bfada6  
YokeBarrelCollectionContributions               edm4hep::CaloHitContribution                                     205    5e0fdff3  
YokeEndcapsCollection                           edm4hep::SimCalorimeterHit                                       0      84b71e41  
YokeEndcapsCollectionContributions              edm4hep::CaloHitContribution                                     0      96acca75  

Parameters:
Name               Type         Elements  
-----------------  -----------  --------  
beamPDG1           int          1         
beamPDG2           int          1         
Event Number       int          1         
ProcessID          int          1         
Run ID             int          1         
_weight            float        1         
alphaQCD           float        1         
beamPol1           float        1         
beamPol2           float        1         
crossSection       float        1         
crossSectionError  float        1         
Energy             float        1         
scale              float        1         
sqme               float        1         
BeamSpectrum       std::string  1         
processName        std::string  1         
```
:::


## Writing a Gaudi algorithm
This is the part of the tutorial where you will have to actively start thinking
and working yourself, after everything so far was possible by just copying
commands and running them.

You will write a replacement for the
[`GammaGammaCandidateFinder`](https://github.com/iLCSoft/MarlinReco/tree/master/Analysis/GammaGammaCandidateFinder)
processor with the final aim of completely replacing the `MyPi0Finder` (and
potentially also the `MyEtaFinder` and `MyEtaPrimeFinder`) Processor(s) in the
ILDReconstruction chain. To do this we will break the full
`GammaGammaCandidateFinder` processor into smaller algorithms to cover the three
main things it does
1. Filter the input particles to only contain photons with a minimal energy
  ([see
  here](https://github.com/iLCSoft/MarlinReco/blob/7aa853773d2a6a718aee63f39c436d5af622353a/Analysis/GammaGammaCandidateFinder/src/GammaGammaCandidateFinder.cc#L169-L189))
2. Build all possible photon candidates and run a kinematic fit on the
   combination if they are close enough to a configured resonance mass and keep
   only those that have a good enough fit quality ([see
   here](https://github.com/iLCSoft/MarlinReco/blob/7aa853773d2a6a718aee63f39c436d5af622353a/Analysis/GammaGammaCandidateFinder/src/GammaGammaCandidateFinder.cc#L198-L374))
  
For the first step we will write a generic `RecoParticleFilter` algorithm that
allows us to
- Filter reconstructed particles according to their assigned `PDG` value as well
  as
- requiring a minimal energy.

For the second algorithm we will then use the outputs of the first
(appropriately configured) algorithm and setup the kinematic fit for all the
possible combinations.

### Setting up tutorial

Since writing everything from scratch will take too long (and is almost never
really necessary in any case[^1]), we have prepared some skeleton code and setup for
this part. In order to get it clone the
[`key4hep-tutorials`](https://github.com/key4hep/key4hep-tutorials) repository
and go to the `gaudi_alg_kinfit` tutorial

[^1]: If you ever are, you can use the
    [`k4-project-template`](https://github.com/key4hep/k4-project-template) to
    start from a properly structured template project with the necessary
    boilerplate configuration and some examples.

``` bash
cd $TUTORIAL_BASE_DIR
git clone https://github.com/key4hep/key4hep-tutorials
cd key4hep-tutorials/gaudi_alg_kinfit
```

```{note}
If you ever get stuck at some point you can always peak into the `.solution`
folder where we have provided a complete implementation that you can in
principle also simply copy over (if you are really lazy)
```

The tutorial follows the usual structure of a Key4hep (Gaudi) project. Most
importantly there is a `CMakeLists.txt` file that contains the build
configuration. **For the purpose of this tutorial this is already fully
configured to find all dependencies and to setup everything. You will not need
to edit this file, but it's still recommended to at least have a quick look to
see if everything makes sense.** The skeleton implementations of the two
algorithms described above live in `GaudiKinfit/components` and their build is
configured in `GaudiKinfit/CMakeLists.txt`. **You will have to minimally edit this
file in the course of the tutorial.**

### Writing the Filter algorithm
We have prepared a skeleton implementation of the `RecoParticleFilter` in
`GaudiKinfit/components/RecoParticleFilter.hpp` and
`GaudiKinfit/components/RecoParticleFilter.cpp` that you will now have to fill
with life and the desired functionality

#### Tasks
The main tasks (also outlined
in the files) are to:
- Figure out which of the Functional algorithms to use as a base class for this
  algorithm
- Add some properties to make it configurable
- Implement the actual functionality
- Install the algorithm and configure the environment for Gaudi to find it
- Debug the algorithm and validate it's working correctly. We recommend to look
  at [the section below](testing-filter-algo) on how to go about
  that **once you have an algorithm that compiles and you think should work**

As a first step we need to configure the project via the usual commands

```bash
cmake -B build -S . -DCMAKE_INSTALL_PREFIX=$(pwd)/install
```

This will fail with an error similar to:

```
-- Configuring done (4.1s)
CMake Error at <long-gaudi-prefix>/lib/cmake/Gaudi/GaudiToolbox.cmake:450 (add_library):
  No SOURCES given to target: GaudiKinfitPlugins
Call Stack (most recent call first):
  GaudiKinfit/CMakeLists.txt:6 (gaudi_add_module)


CMake Generate step failed.  Build files cannot be regenerated correctly.
```

That is because we first have to include the necessary source files into the
target. **Open the `GaudiKinfit/CMakeLists.txt` and uncomment the 3rd line**
(like this)
``` cmake
set(sources
  # components/GammaGammaCandidateFinder.cpp
  components/RecoParticleFilter.cpp
)
```

Run the `cmake` command from above again and it should succeed now and you are
able to try and build for the first time via.

``` bash
cmake --build build
```

You will now run into a build failure.

:::{dropdown} Original build failure

```console
[ 20%] Building CXX object GaudiKinfit/CMakeFiles/GaudiKinfitPlugins.dir/components/RecoParticleFilter.cpp.o
In file included from <path-to>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/RecoParticleFilter.cpp:1:
<path-to>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/RecoParticleFilter.hpp:13:47: error: 'ISvcLocator' has not been declared
   13 |   RecoParticleFilter(const std::string& name, ISvcLocator* svcLoc);
      |                                               ^~~~~~~~~~~
<path-to>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/RecoParticleFilter.hpp:17:3: error: 'edm4hep::ReconstructedParticleCollection RecoParticleFilter::operator()(const edm4hep::ReconstructedParticleCollection&) const' marked 'final', but is not virtual
   17 |   operator()(const edm4hep::ReconstructedParticleCollection& recoColl) const final;
      |   ^~~~~~~~
<path-to>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/RecoParticleFilter.cpp:6:65: error: 'ISvcLocator' has not been declared
    6 | RecoParticleFilter::RecoParticleFilter(const std::string& name, ISvcLocator* svcLoc)
      |                                                                 ^~~~~~~~~~~
<path-to>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/RecoParticleFilter.cpp: In constructor 'RecoParticleFilter::RecoParticleFilter(const std::string&, int*)':
<path-to>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/RecoParticleFilter.cpp:6:59: warning: unused parameter 'name' [-Wunused-parameter]
    6 | RecoParticleFilter::RecoParticleFilter(const std::string& name, ISvcLocator* svcLoc)
      |                                        ~~~~~~~~~~~~~~~~~~~^~~~
<path-to>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/RecoParticleFilter.cpp:6:78: warning: unused parameter 'svcLoc' [-Wunused-parameter]
    6 | RecoParticleFilter::RecoParticleFilter(const std::string& name, ISvcLocator* svcLoc)
      |                                                                 ~~~~~~~~~~~~~^~~~~~
<path-to>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/RecoParticleFilter.cpp: In member function 'edm4hep::ReconstructedParticleCollection RecoParticleFilter::operator()(const edm4hep::ReconstructedParticleCollection&) const':
<path-to>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/RecoParticleFilter.cpp:19:1: warning: no return statement in function returning non-void [-Wreturn-type]
   19 | }
      | ^
<path-to>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/RecoParticleFilter.cpp: At global scope:
<path-to>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/RecoParticleFilter.cpp:21:38: error: expected constructor, destructor, or type conversion at end of input
   21 | DECLARE_COMPONENT(RecoParticleFilter)
      |                                      ^
gmake[2]: *** [GaudiKinfit/CMakeFiles/GaudiKinfitPlugins.dir/build.make:79: GaudiKinfit/CMakeFiles/GaudiKinfitPlugins.dir/components/RecoParticleFilter.cpp.o] Error 1
gmake[1]: *** [CMakeFiles/Makefile2:148: GaudiKinfit/CMakeFiles/GaudiKinfitPlugins.dir/all] Error 2
gmake: *** [Makefile:136: all] Error 2

```

:::

In order to fix this you first have to implement at least the basics of the
algorithm:
- Choose the correct Functional base class and inherit from it (don't forget to
  include the correct header for this).
- Use the correct constructor 


Once you have correctly implemented this you should be able to build (althout
likely not without warnings yet).

(setting-up-environment)=
#### Installing and setting up the runtime environment
Once you are at a state where you want to test or use your algorithm you have to
install it and setup the runtime environment. 

```{note}
Installing your algorithm is necessary whenever you have changed it and want to
test the new changes. **Setting up the environment is only necessary once.**
```

For installing simply run

``` bash
cmake --build build --target install
```

For setting up the environment we recommend to use the `k4_local_repo` command,
but you can also do it manually. We will show you how to verify this worked in
the [testing section](testing-filter-algo).


::::{tab-set} 
:::{tab-item} k4_local_repo

Make sure to call this in the base directory of your package (most importantly
the `install` folder should be in the directory you call this from).

```bash
k4_local_repo
```

:::
:::{tab-item} Manually

This (and a bit more) is also what `k4_local_repo` is doing for you. In general
the minimal setup you have to do is to change the following variables for Gaudi
to be able to pick your plugins up

``` bash
export LD_LIBRARY_PATH=$(pwd)/install/lib:$LD_LIBRARY_PATH
export PYTHONPATH=$(pwd)/install/python:$PYTHONPATH
```

:::
::::


(hints-pitfalls-filter-build)=
#### Hints and some common pitfalls

In case you get stuck and don't want to immediately peak into the `.solution`
folder, we have also tried to collect some of the common errors and issues one
encounters when first embarking onto the *Functional journey*. We have grouped
them into several separate dropdowns to avoid a) spoilers and b) excessive
scrolling.

:::{dropdown} I don't know how to choose the Functional base class

The choice of the Functional base class is entirely defined by the number of
inputs and outputs of an algorithm. In this case these are already defined in
the signature of the `operator()`.

- How many (and which) inputs does this algorithm have?
- How many (and which) outputs does this algorithm have?
- Which Functional base class fits this?

:::

:::{dropdown} I still don't know which Functional base class to use

We have a single input (in the form of a `const
edm4hep::ReconstructedParticleCollection&`) and a single output (another
`edm4hep::ReconstructedParticleCollection`). The deciding factor in this case is
the **singular output** which (in combination with at least one input) is the
definining characteristic of the `k4FWCore::Transformer`.

:::

:::{dropdown} I can't figure out the correct syntax for defining the base class

Look at the signature of `operator()`

``` cpp
edm4hep::ReconstructedParticleCollection
operator()(const edm4hep::RecontructedParticleCollection& recoColl) const final;
```

To get to the right template argument for the `k4FWCore::Transformer` what you
have to do is effectively
- Remove the function name (i.e. `operator()` in this case)
- Remove the argument name(s) (i.e. `recoColl` in this case)
- Remove all method qualifiers (i.e. `const` and `final` in this case. `final`
  might also be `override` in some cases)
  
You then arrive at

``` cpp
edm4hep::ReconstructedParticleCollection(const edm4hep::ReconstructedParticleCollection&)
```

Which, when placed into the template argument for `k4FWCore::Transformer` gives
as minimal class definition

``` cpp
struct RecoParticleFilter final : public k4FWCore::Transformer<edm4hep::ReconstructedParticleCollection(
                                      const edm4hep::ReconstructedParticleCollection&)> {
  RecoParticleFilter(const std::string& name, ISvcLocator* svcLoc);

  edm4hep::ReconstructedParticleCollection
  operator()(const edm4hep::ReconstructedParticleCollection& recoColl) const final;
};
```

:::

:::{dropdown} Common compilation errors

```{note}
The exact paths to files will be different in your case, but you should be able
to pattern match quite a bit. Additionally, **when reading compiler errors,
always start at the top and try to identify the first error in your code.** It
is possible that the rest is simply cascading failures from there and solving
the first issue makes everything build. This is also why we only show the first 
few lines of compiler output below.
```

- **The signature of `operator()` does not agree with the `Transformer` template
  argument**
  
  This will happen if you specify the template arguments of the
  `k4FWCore::Transformer` differently to the signature of `operator()`. Common
  reasons are
  - Missing `const` qualifiers on the input
  - Missing `&` (reference) qualifiers on the input
  - Typos

```console
In file included from <tutorial-prefix>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/RecoParticleFilter.cpp:1:
<tutorial-prefix>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/RecoParticleFilter.hpp:16:3: error: 'edm4hep::ReconstructedParticleCollection RecoParticleFilter::operator()(const edm4hep::ReconstructedParticleCollection&) const' marked 'final', but is not virtual
   16 |   operator()(const edm4hep::ReconstructedParticleCollection& recoColl) const final;
      |   ^~~~~~~~
[...]
```

- **The inputs and outputs are not Collections**

All functional algorithms take only EDM4hep collections or vectors of EDM4hep
collections as inputs and can only produce EDM4hep collections (or multiple
EDM4hep collections in the form of a `std::tuple`) as outputs. Trying to
instantiate them with a signature different then this will yield the following,
fairly descriptive error message:

```console
In file included from <tutorial-prefix>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/RecoParticleFilter.hpp:5,
                 from <tutorial-prefix>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/RecoParticleFilter.cpp:1:
<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/include/k4FWCore/Transformer.h: In instantiation of 'struct k4FWCore::details::Transformer<edm4hep::ReconstructedParticle(const edm4hep::ReconstructedParticleCollection&), Gaudi::Functional::Traits::use_<> >':
<tutorial-prefix>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/RecoParticleFilter.hpp:12:24:   required from here
<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/include/k4FWCore/Transformer.h:49:89: error: static assertion failed: Transformer and Producer output types must be EDM4hep collections or vectors of collections
   49 |     static_assert((std::is_base_of_v<podio::CollectionBase, Out> || isVectorLike_v<Out> ||
      |                   ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~^~
   50 |                    std::is_same_v<podio::CollectionBase*, Out>),
      |                    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~                          
<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/include/k4FWCore/Transformer.h:49:89: note: '((((bool)std::is_base_of_v<podio::CollectionBase, edm4hep::ReconstructedParticle>) || ((bool)k4FWCore::details::isVectorLike_v<edm4hep::ReconstructedParticle>)) || ((bool)std::is_same_v<podio::CollectionBase*, edm4hep::ReconstructedParticle>))' evaluates to false

[...]
```

- **The `Transformer` constructor is not called from the algorithm constructor
  (initializer list)**

Make sure to call the constructor of the Functional in the initializer list of
  your algorithms constructor, e.g.

``` cpp
RecoParticleFilter::RecoParticleFilter(const std::string& name, ISvcLocator* svcLoc) :
  Transformer(name, svcLoc, /* inputs and outputs property names and default vlaues */) 
  {}
```
The compiler will otherwise complain something along the lines of:
  

```console
<tutorial-prefix>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/RecoParticleFilter.cpp: In constructor 'RecoParticleFilter::RecoParticleFilter(const std::string&, ISvcLocator*)':
<tutorial-prefix>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/RecoParticleFilter.cpp:6:84: error: no matching function for call to 'k4FWCore::details::Transformer<edm4hep::ReconstructedParticleCollection(const edm4hep::ReconstructedParticleCollection&), Gaudi::Functional::Traits::use_<> >::Transformer()'
    6 | RecoParticleFilter::RecoParticleFilter(const std::string& name, ISvcLocator* svcLoc)
      |                                                                                    ^
In file included from <tutorial-prefix>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/RecoParticleFilter.hpp:5,
                 from <tutorial-prefix>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/RecoParticleFilter.cpp:1:
<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/include/k4FWCore/Transformer.h:68:5: note: candidate: 'template<class IArgs, class OArgs, long unsigned int ...I, long unsigned int ...J> k4FWCore::details::Transformer<Out(const In& ...), Traits_>::Transformer(std::string, ISvcLocator*, const IArgs&, std::index_sequence<I ...>, const OArgs&, std::index_sequence<J ...>) [with OArgs = IArgs; long unsigned int ...I = OArgs; long unsigned int ...J = {I ...}; Out = edm4hep::ReconstructedParticleCollection; In = {edm4hep::ReconstructedParticleCollection}; Traits_ = Gaudi::Functional::Traits::use_<>]'
   68 |     Transformer(std::string name, ISvcLocator* locator, const IArgs& inputs, std::index_sequence<I...>,
      |     ^~~~~~~~~~~
<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/include/k4FWCore/Transformer.h:68:5: note:   template argument deduction/substitution failed:
<tutorial-prefix>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/RecoParticleFilter.cpp:6:84: note:   candidate expects 6 arguments, 0 provided
    6 | RecoParticleFilter::RecoParticleFilter(const std::string& name, ISvcLocator* svcLoc)

[...]
```


- **Trying to use `k4FWCore::Transformer` in the algorithm constructor**

Do not use the fully qualified `k4FWCore::Transformer` in the initializers list.
Instead only use the unqualified `Transformer` to forward constructor arguments.

```console
<tutorial-prefix>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/RecoParticleFilter.cpp: In constructor 'RecoParticleFilter::RecoParticleFilter(const std::string&, ISvcLocator*)':
<tutorial-prefix>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/RecoParticleFilter.cpp:7:28: error: expected class-name before '(' token
    7 |     : k4FWCore::Transformer(name, svcLoc) {}
      |                            ^
<tutorial-prefix>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/RecoParticleFilter.cpp:7:28: error: no matching function for call to 'k4FWCore::details::Transformer<edm4hep::ReconstructedParticleCollection(const edm4hep::ReconstructedParticleCollection&), Gaudi::Functional::Traits::use_<> >::Transformer()'
In file included from <tutorial-prefix>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/RecoParticleFilter.hpp:5,
                 from <tutorial-prefix>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/RecoParticleFilter.cpp:1:
<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/include/k4FWCore/Transformer.h:68:5: note: candidate: 'template<class IArgs, class OArgs, long unsigned int ...I, long unsigned int ...J> k4FWCore::details::Transformer<Out(const In& ...), Traits_>::Transformer(std::string, ISvcLocator*, const IArgs&, std::index_sequence<I ...>, const OArgs&, std::index_sequence<J ...>) [with OArgs = IArgs; long unsigned int ...I = OArgs; long unsigned int ...J = {I ...}; Out = edm4hep::ReconstructedParticleCollection; In = {edm4hep::ReconstructedParticleCollection}; Traits_ = Gaudi::Functional::Traits::use_<>]'
   68 |     Transformer(std::string name, ISvcLocator* locator, const IArgs& inputs, std::index_sequence<I...>,
      |     ^~~~~~~~~~~
<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/include/k4FWCore/Transformer.h:68:5: note:   template argument deduction/substitution failed:
<tutorial-prefix>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/RecoParticleFilter.cpp:7:28: note:   candidate expects 6 arguments, 0 provided
    7 |     : k4FWCore::Transformer(name, svcLoc) {}
      |                            ^
      
[...]

```

- **Messing up the braces and parenthesis in the `Transformer` constructor**

This is probably the most common one, as it is really easy to do and things can
become confusing quickly. Almost always this will result in an error along the
lines of *error: no matching function to call for
k4FWCore::details::Transformer<[...]>*, e.g. something like shown below. The
best way to try and fix this is to go argument by argument and to remember, that
the inputs go into separate `{}` as the outputs. It usually helps to simply
format things clearly (even if `clang-format` or your editor will potentially
destroy this again when you save)

```cpp
RecoParticleFilter::RecoParticleFilter(const std::string& name, ISvcLocator* svcLoc) :
  Transformer(name, svcLoc, 
    {KeyValues("Input", {"DefaultInputName"})},           // <-- The input config
    {KeyValues("OutputName", {"OutpuCollectionDefault"})} // <-- The output config
  ) { /* Potentially more constructor things*/ }
```


```
<tutorial-prefix>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/RecoParticleFilter.cpp: In constructor 'RecoParticleFilter::RecoParticleFilter(const std::string&, ISvcLocator*)':
<tutorial-prefix>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/RecoParticleFilter.cpp:9:112: error: no matching function for call to 'k4FWCore::details::Transformer<edm4hep::ReconstructedParticleCollection(const edm4hep::ReconstructedParticleCollection&), Gaudi::Functional::Traits::use_<> >::Transformer(const std::string&, ISvcLocator*&, <brace-enclosed initializer list>)'
    9 |           {KeyValues("InputCollection", {"PandoraPFOs"}), KeyValues("OutputCollection", {"FilteredParticles"})}) {}
      |                                                                                                                ^

```

:::

(testing-filter-algo)=
### Using / testing the Filter algorithm

Implementing the functionality in c++ is only half of the story. Obviously, you
also want to exercise what you have implemented. For Gaudi based workflows this
means that we also have to write some python configuration that we can then pass
through `k4run`. For this we have also prepared a skeleton that setups most of
the basics that are always necessary. You can find the skeleton in
`GaudiKinfit/options/runRecoParticleFilter.py`.

For running this configuration use either the output of the ILD reconstruction
of the first part or the REC file that is available from the `RESOURCE_DIR`

::::{tab-set}
:::{tab-item} ILD reconstruction

```bash
k4run GaudiKinfit/options/runRecoParticleFilter.py \
  --IOSvc.Input=$ILDCONFIG_DIR/gen_tau_pi0_REC.edm4hep.root
```


:::
:::{tab-item} RESOURCE_DIR

```bash
k4run GaudiKinfit/options/runRecoParticleFilter.py \
  --IOSvc.Input=$RESOURCE_DIR/gen_tau_pi0_250-SetA_2f_leptonic_eL_pR_REC_0.edm4hep.root
```

:::
::::

By default this should produce a `photon_candidates.root` output file that you
can inspect either via `podio-dump`, `root` or you can also poke at it
interactively using e.g. `ipython` to quickly check whether things are working
as expected.

#### Tasks
The main tasks (as also outlined in the file) are
- Configure the `ApplicationMgr` to run your algorithm
- Complete the configuration of the `photon_filter` algorithm according to the
  *Properties* (specifically their names) you have defined in the c++
  implementation.
- Configure the `iosvc` to store only the collections you want at the end (there
  are multiple ways to achieve this)

(hints-pitfalls-filter-runtime)=
#### Hints and some common pitfals

Again it's possible that you encounter some issues. For the most common ones we
have collected some hints on how to detect and subsequently fix them.

:::{dropdown} ImportError: cannot import name 'XYZ' from 'Gaudi.Configurables' (unknown location)

The most likely reason for this is that you have either
- not installed your algorithm yet,
- or that you have not yet setup the environment correctly

See [how to do that here](setting-up-environment).

```console
Traceback (most recent call last):
  File "<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/bin/k4run", line 277, in <module>
    main()
    ~~~~^^
  File "<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/bin/k4run", line 206, in main
    load_file(file)
    ~~~~~~~~~^^^^^^
  File "<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/python/k4FWCore/utils.py", line 95, in load_file
    exec(compile(code, ofile.name, "exec"), namespace)
    ~~~~^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "GaudiKinfit/options/runRecoParticleFilter.py", line 5, in <module>
    from Configurables import RecoParticleFilter, EventDataSvc
ImportError: cannot import name 'RecoParticleFilter' from 'Gaudi.Configurables' (unknown location)
```

:::

:::{dropdown} I have setup the environment and installed the algorithm, but I still see the same ImportError

When you have [setup the environment and installed your algorithm
correctly](setting-up-environment) and you still see the import error you are
most likely not yet declaring your algorihm as a Gaudi component or you have a
type somewhere. In order to declare your algorithm as a Gaudi component, put

```cpp
DECLARE_COMPONENT(RecoParticleFilter)  // <--- replace with your algorithm name
```

into the `.cpp` file to make it known as a component to Gaudi.
:::

:::{dropdown} NameError: name 'XYZ' is not defined

This almost always means that you have missed to `import` an algorithm from
`Configurables` (or that you are trying to use something else that you have not
`import`ed). Other common issues that show like this are typos.

```console
Traceback (most recent call last):
  File "<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/bin/k4run", line 277, in <module>
    main()
    ~~~~^^
  File "<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/bin/k4run", line 206, in main
    load_file(file)
    ~~~~~~~~~^^^^^^
  File "<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/python/k4FWCore/utils.py", line 95, in load_file
    exec(compile(code, ofile.name, "exec"), namespace)
    ~~~~^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "GaudiKinfit/options/runGammaGammaCandidateFinder.py", line 17, in <module>
    gamma_gamma_finder = GammaGammaCandidateFinder("Pi0Finder")
                         ^^^^^^^^^^^^^^^^^^^^^^^^^
NameError: name 'GammaGammaCandidateFinder' is not defined

```

:::


:::{dropdown} ValueError: received an instance of XYZ, but ZYX  expected for property BAR

In case you see an error like this there is a type mismatch between the expected
property value and the one you specified in the options file. The most likely
case in this tutorial is trying to specify an input or output collection with a
single string argument, which will result in an error like below

The proper way to specify (even a single) input or output collection via a list
of strings, i.e.

``` python
photon_filter.InputCollection = ["PandoraPFOs"]
photon_filter.OutputCollection = ["FilteredCollection"]
```

```console
Traceback (most recent call last):
  File "<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/bin/k4run", line 277, in <module>
    main()
    ~~~~^^
  File "<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/bin/k4run", line 206, in main
    load_file(file)
    ~~~~~~~~~^^^^^^
  File "<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/python/k4FWCore/utils.py", line 95, in load_file
    exec(compile(code, ofile.name, "exec"), namespace)
    ~~~~^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "GaudiKinfit/options/runRecoParticleFilter.py", line 10, in <module>
    photon_filter.InputCollection = "PandoraPFOs"
    ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
  File "<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/gaudi/master-wsj4hl/python/GaudiKernel/Configurable.py", line 520, in __setattr__
    super(Configurable, self).__setattr__(name, value)
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~^^^^^^^^^^^^^
  File "<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/gaudi/master-wsj4hl/python/GaudiKernel/PropertyProxy.py", line 155, in __set__
    value = _isCompatible(proptype, value, self.descr.__name__)
  File "<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/gaudi/master-wsj4hl/python/GaudiKernel/PropertyProxy.py", line 69, in _isCompatible
    raise ValueError(errmsg)
ValueError: received an instance of <class 'str'>, but <class 'list'> expected for property InputCollection

```

:::

:::::{dropdown} My configuration doesn't produce any output file
This might happen if you haven't configured the `Output` property of the
`IOSvc`. In that case the algorithms will run, but no output file will be
created. You can fix this either in the options file or from the command line

::::{tab-set}
:::{tab-item} options file

Make sure that something along the lines of the following is in your options file

``` python
iosvc = IOSvc()
iosvc.Output = "outputfile.root"
```

:::

:::{tab-item} CLI

Add the following to the arguments of `k4run`
```
--IOSvc.Output=outputfile.root
```

:::
::::

:::::

:::{dropdown} ERROR Application Manager Terminated with error code 6

Whenever your algorithm chain encounters an error at runtime you are likely to
see this as the last line of the outptu of the `k4run` command. In this case the
next step is to figure out what the error(s) that occured actually are. In order
to do this you will have to read the complete output **starting at the top** and
trying to find the first `FATAL` or `ERROR` log output. This should already tell
you which algorithm (instance) was responsible and also might give you a hint at
the problem already.

The most common causes for errors are:
- An expected input collection is not found in the TES
- an uncaught exception escapes from an algorithm (this can happen everywhere,
  including `initialize` and `finalize`)
- a `StatusCode::FAILURE` is returned (most likely from `initialize` or
  `finalize`)
  
In some cases it might be useful to increase the `OutputLevel` of an algorithm
once it has been identified which one is responsible for the error. This can
give further insights into why something is failing.

:::


:::{dropdown} ERROR Immutable exceptions can only be added to subset collections

This will occur if you do not make the output collection of the
`RecoParticleFilter` algorithm a *subset collection*. Since elements in EDM4hep
belong to a single collection exclusively, we can only store a reference to them
in a subset collection. The solution to this is to simply create your output
collection as a subset collection

```cpp
auto ret = edm4hep::ReconstructedParticleCollection{};
ret.setSubsetCollection();
```

(Shortened) example output:

```console
[...]
ApplicationMgr    SUCCESS 
====================================================================================================================================
                                                   Welcome to ApplicationMgr (GaudiCoreSvc v40r0)
                                          running on madlener-Precision-5550 on Sat Oct 11 15:02:41 2025
====================================================================================================================================
ApplicationMgr       INFO Application Manager Configured successfully
k4FWCore__Seque...   INFO Member list: Gaudi::Sequencer/k4FWCore__Algs, Writer/k4FWCore__Writer
k4FWCore__Algs       INFO Member list: Reader/k4FWCore__Reader, RecoParticleFilter/PhotonFilter
IOSvc                INFO Setting metadata frame
PhotonFilter        DEBUG Property update for OutputLevel : new value = 2
PhotonFilter        DEBUG input handles: 1
PhotonFilter        DEBUG output handles: 1
PhotonFilter        DEBUG data dependencies:
  + INPUT  'PandoraPFOs'
  + OUTPUT 'FilteredParticles'
ApplicationMgr       INFO Application Manager Initialized successfully
ApplicationMgr       INFO Application Manager Started successfully
PhotonFilter        FATAL Standard std::exception is caught in sysExecute
PhotonFilter        ERROR Immutable objects can only be added to subset collections
k4FWCore__Algs      FATAL Standard std::exception is caught in sysExecute
k4FWCore__Algs      ERROR Immutable objects can only be added to subset collections
k4FWCore__Seque...  FATAL Standard std::exception is caught in sysExecute
k4FWCore__Seque...  ERROR Immutable objects can only be added to subset collections
EventLoopMgr        FATAL .executeEvent(): Standard std::exception thrown by k4FWCore__Sequencer
EventLoopMgr        ERROR Immutable objects can only be added to subset collections
EventLoopMgr      WARNING Execution of algorithm k4FWCore__Sequencer failed
EventLoopMgr        ERROR Error processing event loop.
EventLoopMgr        ERROR Terminating event processing loop due to errors
EventLoopMgr        ERROR Terminating event processing loop due to errors
ApplicationMgr       INFO Application Manager Stopped successfully
ApplicationMgr       INFO Application Manager Finalized successfully
ApplicationMgr      ERROR Application Manager Terminated with error code 6

```

:::

(output-commands-config)=
:::{dropdown} My output file doesn't have any collections stored

This will happen if you configure the `IOSvc.outputCommands` inappropriately. In
this case we might have also lured you into a little trap, by explicitly
dropping all collections from the output.

In order to fix this there are several options. You can
- configure the `IOSvc` to keep everything via `"keep *"` (this is also the
  default if you don't do anything else)
- Make sure that your desired collections are kept, i.e.

``` python
iosvc.outputCommands = [
    "drop *", 
    "keep FilteredPhotons",  # <-- Assuming this is the output of your algorithm
]
```

The commands here are executed in the order they are specified. Hence, this will
only store your newly created collection into the output file.

:::

(algorithm-in-top-alg)=
:::::{dropdown} My algorithm doesn't seem to run

Most likely you have simply not added it to the algorithms to actually run in
the `ApplicationMgr` and simply need to add it there

```python
alg_list = [photon_filter]
ApplicationMgr(
  TopAlg=alg_list,
  # Other arguments unchanged
)
```

You can verify that your algorithm is configured to run by checking the first
line of the `k4run` output:

::::{tab-set}
:::{tab-item} Incorrectly configured

```console
[k4run - INFO] k4run.main: 
```

:::
:::{tab-item} Correctly configured

```console
[k4run - INFO] k4run.main: --> PhotonFilter
```

:::
::::

You can also check whether you see the `DEBUG` messages from your algorithm in
the output (as we have configured the `OutputLevel` accordingly in the skeleton)

``` console
[...]
====================================================================================================================================
                                                   Welcome to ApplicationMgr (GaudiCoreSvc v40r0)
                                          running on madlener-Precision-5550 on Sat Oct 11 15:36:19 2025
====================================================================================================================================
ApplicationMgr       INFO Application Manager Configured successfully
k4FWCore__Seque...   INFO Member list: Gaudi::Sequencer/k4FWCore__Algs, Writer/k4FWCore__Writer
k4FWCore__Algs       INFO Member list: Reader/k4FWCore__Reader, RecoParticleFilter/PhotonFilter
IOSvc                INFO Setting metadata frame
PhotonFilter        DEBUG Property update for OutputLevel : new value = 2
PhotonFilter        DEBUG input handles: 1
PhotonFilter        DEBUG output handles: 1
PhotonFilter        DEBUG data dependencies:
  + INPUT  'PandoraPFOs'
  + OUTPUT 'FilteredParticles'
[...]
```

:::::

:::{dropdown} My collection is not stored in the output file

After you have ensured that [you do not drop all
collections](output-commands-config) you also need to ensure that your
[algorithm is actually run](algorithm-in-top-alg).

:::

:::{dropdown} ERROR XYZ : Cannot retrieve 'FOO' from transient store

You will see this error when you try to use a collection that is not known to
the transient event store (TES) at the point in time when you want to use this
as an **input collection**. This can have several reasons, the most likely ones
are
- You have a typo in the collection name
- Another algorithm that produces this collection is only run after your algorithm
- The collection is already missing from the input

:::

:::{dropdown} My filtered collection looks "weird"

If you inspect the output file using `root` (perfectly fine way to do this) you
see that your newly created output collection looks different than other
`ReconstructedParticleCollections` (e.g. *PandoraPFOs*). Most importantly
- this collection has only one branch compared to several sub-branches for
  "normal" collections
- the branch is named `<collection-name>_objIdx`

The reason for this is that you had to make this collection a *subset
collection* (in order to keep all the relations and to not duplicate data) in
the algorithm implementation.

:::

:::{dropdown} I get a crash when I try to look at the output file

The most likely reason for this is that you **do not store the original
collection into which the subset collection points**. Hence, a fix could be to
use

``` python
iosvc.outputCommands = [
    "drop *",
    "keep FilteredPhotons", # <-- your *subset* collection
    "keep PandoraPFOs",     # <-- the collection where the photons actually live
]
```

The actual symptom for this will likely look different depending on how you try
to do this 

Depending on how you try this the overall structure of the output might differ
but there will be some mention of a `bad_function_all`. We are currently
tracking this in [podio#859](https://github.com/AIDASoft/podio/issues/859).

:::


### Writing the `GammaGammaCandidateFinder` algorithm

Now that you have had some exercise in getting the boilerplate part of
Functional algorithms right, the next step will be to implement a slightly more
involved algorithm, specifically the `GammaGammaCandidateFinder`. We have again
provided a (not yet compiling) version of the algorithm in
`GaudiKinfit/components/GammaGammaCandidateFinder.{h,c}pp`. Also for testing
there is again a skeleton options file that still needs to be completed for this
part into `GaudiKinfit/options/runGammaGammaCandidateFinder.py`.

The desired functionality of this algorithm should be to
- take a configurable resonance mass and PDG value (and optionally a selection
  window for di-photon candidates around the resonance mass),
- combine all input photons (the outputs of the filter algorithm we just
  developed),
- do a kinematic fit on all combinations using the input resonance mass as a
  mass constraint,
- and finally put all of the combinations that pass (the configurable minimal
  fit probability threshold) into the output.
  
For running this configuration use either the output of the ILD reconstruction
of the first part or the REC file that is available from the `RESOURCE_DIR`

::::{tab-set}
:::{tab-item} ILD reconstruction

```bash
k4run GaudiKinfit/options/runGammaGammaCandidateFinder.py \
  --IOSvc.Input=$ILDCONFIG_DIR/gen_tau_pi0_REC.edm4hep.root
```

:::
:::{tab-item} RESOURCE_DIR

```bash
k4run GaudiKinfit/options/runGammaGammaCandidateFinder.py \
  --IOSvc.Input=$RESOURCE_DIR/gen_tau_pi0_250-SetA_2f_leptonic_eL_pR_REC_0.edm4hep.root
```

:::
::::


#### Tasks
The skeleton provides some structure and partially implemented helper
functionality. Again, follow the *TODO* items in the files. In rough order the
recommended steps are

- Enable the compilation of the `GammaGammaCandidateFinder` algorithm by
  uncommenting the corresponding line in `GaudiKinift/CMakeLists.txt`.
- Fix the compiler error you see by properly instantiating the `recoPart` in
  `GaudiKinfit/components/GammaGammaCandidateFinder.cpp` on line 127.
- Write the main loop in `operator()` to build the photon combinations and
  create a fake `FitResult` (for now only the `fittedParticle` four momentum
  vector has to be populated). Pass this `FitResult` to the `createParticle`
  function
- Finalize the `createParticle` function
- Finish the options file by instantiating the algorithm and finishing its
  configuration.
  - Run the algorithm at this stage to check that photon combinations are indeed
    happening
- Add the necessary *Properties* for all the configurable parameters that are
  necessary.
- Finalize the implementation of the `performKinematicFit` method and call it
  from the main loop in `operator()`. Replace the fake fit result from before
  with the actual on (after checking whether the fit probability is good
  enough).

#### Hints and common pitfalls

In general most of the Gaudi related issues you will probably have already seen
above and the hints [here](hints-pitfalls-filter-build) and
[here](hints-pitfalls-filter-runtime) might still prove useful. Some more
specific ones for the actual content of this algorithm are listed here.

:::{dropdown} I can't fix the compiler error for recoPart

This likely happened because you tried to use a `edm4hep::ReconstructedParticle`
as type. The compiler will yell a lot about (see also below) *passing 'const
XYZ' as 'this' argument discards qualifiers [-fpermissive]*. The default handles
in EDM4hep do not allow for mutation. For that you have to create a dedicated
mutable handle via (check also the return type of the method!)

``` cpp
auto recoPart = edm4hep::MutableReconstructedParticle{};
```

Exerpt of compiler error

``` console
[...]
<tutorial-prefix>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/GammaGammaCandidateFinder.cpp: In member function 'edm4hep::MutableReconstructedParticle GammaGammaCandidateFinder::createParticle(const FitResult&, const edm4hep::ReconstructedParticle&, const edm4hep::ReconstructedParticle&) const':
<tutorial-prefix>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/GammaGammaCandidateFinder.cpp:175:17: error: passing 'const edm4hep::CovMatrix4f' as 'this' argument discards qualifiers [-fpermissive]
  175 |     cov.setValue(vP(0, 0), x, x);
      |     ~~~~~~~~~~~~^~~~~~~~~~~~~~~~
In file included from <spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/edm4hep/main-s2hiv4/include/edm4hep/ReconstructedParticleData.h:6,
                 from <spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/edm4hep/main-s2hiv4/include/edm4hep/ReconstructedParticleObj.h:8,
                 from <spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/edm4hep/main-s2hiv4/include/edm4hep/MutableReconstructedParticle.h:6,
                 from <spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/edm4hep/main-s2hiv4/include/edm4hep/ReconstructedParticleCollection.h:7,
                 from <tutorial-prefix>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/GammaGammaCandidateFinder.hpp:7,
                 from <tutorial-prefix>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/GammaGammaCandidateFinder.cpp:1:
<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/edm4hep/main-s2hiv4/include/edm4hep/CovMatrix4f.h:108:18: note:   in call to 'constexpr void edm4hep::CovMatrix4f::setValue(float, DimEnum, DimEnum) [with DimEnum = edm4hep::FourMomCoords]'
  108 |   constexpr void setValue(float value, DimEnum dimI, DimEnum dimJ) {
      |                  ^~~~~~~~

[...]
```

:::

:::{dropdown} I can't figure out how to construct the fake FitResult

The `FitResult` has only one member that is interesting for `createParticle`;
`fittedParticle` (a four vector). One way to populate that is

```cpp
const auto& gamma1 = edm4hep::utils::p4(photonCandidates[i], edm4hep::utils::UseEnergy);
const auto& gamma2 = edm4hep::utils::p4(photonCandidates[j], edm4hep::utils::UseEnergy);

FitResult fakeResult{};
fakeResult.fittedParticle = gamma1 + gamma2;

```

:::

:::{dropdown} I get a compiler error when trying to call convertParticle

If you get an error like below complaining about a failing conversion, you are
trying to pass in the four vectors for the photons, when instead you should be
passing the actual `edm4hep::ReconstructedParticles` here, i.e.

``` cpp
auto particle = createParticle(fitResult, photonCandidates[i], photonCandidates[j]);
```

```console
<tutorial-prefix>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/GammaGammaCandidateFinder.cpp: In member function 'virtual edm4hep::ReconstructedParticleCollection GammaGammaCandidateFinder::operator()(const edm4hep::ReconstructedParticleCollection&) const':
<tutorial-prefix>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/GammaGammaCandidateFinder.cpp:63:50: error: cannot convert 'const ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> >' to 'const edm4hep::ReconstructedParticle&'
   63 |       output.push_back(createParticle(fitResult, gamma1, gamma2));
      |                                                  ^~~~~~
      |                                                  |
      |                                                  const ROOT::Math::LorentzVector<ROOT::Math::PxPyPzE4D<double> >
In file included from <tutorial-prefix>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/GammaGammaCandidateFinder.cpp:1:
<tutorial-prefix>/key4hep-tutorials/gaudi_alg_kinfit/GaudiKinfit/components/GammaGammaCandidateFinder.hpp:48:94: note:   initializing argument 2 of 'edm4hep::MutableReconstructedParticle GammaGammaCandidateFinder::createParticle(const FitResult&, const edm4hep::ReconstructedParticle&, const edm4hep::ReconstructedParticle&) const'
   48 |                                                        const edm4hep::ReconstructedParticle& gamma1,
      |                                                        ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~^~~~~~


```

:::

:::{dropdown} I get pi0 candidates that have the same photon twice

Check your loop conditions. The nested loops can be constructed such that only
unique combinations are built

``` cpp
for (size_t i = 0; i < photonCandidates.size() - 1; ++i) {
  for (size_t j = i + 1; j < photonCandidates.size(); ++j) {
      
  }
}
```

:::

:::{dropdown} I can't figure out how to setup the kinematic fit

In this case setting up the kinematic fit requires three ingredients
- a `MassConstraint` (where we can directly use the `Property` that we introduced),
- and two fit objects, for which in this case we can use `JetFitObject`s. These
  take as inputs the photon kinematics in *E, theta, phi$* coordinates as well
  as the errors on these. The final argument is the mass (i.e. `0`) in our case.

After these have been defined they have to be added to the `fitter` using
`addConstraint` and `addFitObject` respectively. Additionally the two photons
also have to be associated to the mass constraint via `addToFOList`.

The final sequence will look something like this

``` cpp
MassConstraint mc(m_resonanceMass.value());

JetFitObject j1(gamma1.E(), gamma1.Theta(), gamma1.Phi(), 
                0.16 * std::sqrt(gamma1.E()), 0.001 / std::sqrt(gamma1.E()), 0.001 / std::sqrt(gamma1.E()), 
                0.0);
JetFitObject j2(gamma2.E(), gamma2.Theta(), gamma2.Phi(), 
                0.16 * std::sqrt(gamma2.E()), 0.001 / std::sqrt(gamma2.E()), 0.001 / std::sqrt(gamma2.E()), 
                0.0);
mc.addToFOList(j1);
mc.addToFOList(j2);

auto pfitter = createFitter();
BaseFitter& fitter = *pfitter;

fitter.addFitObject(j1);
fitter.addFitObject(j2);
fitter.addConstraint(mc);
```

:::

### Pi0 / GammaGamma analysis

In the `analysis` folder we have prepared some skeleton scripts (in various
different flavours) to make some quick plots from the output files we have just
created. They contain a minor task of adding an additional histogram, but are
runnable without any modifications and will give you a plot to look at. Again
you can find complete solutions in the `.solution/analysis` folder.

For running any of them choose your favorite version and go with that.

::::{tab-set}
:::{tab-item} C++ interface

In this case you have to build the executable first. We have added a minimal
`CMakeLists.txt` file for that. (This is generic enough to also serve as a
simple copy-paste template for your work)

``` bash
cmake -B analysis/build -S analysis
cmake --build analysis/build
```

Whenever you change `analysis/make_pi0_hists.cpp` rerun `cmake --build
analysis/build`.

To run the program do

``` bash
./analysis/build/make_pi0_hists pi0_candidates.root
```

(assuming you haven't changed the output filename in
`runGammaGammaCandidateFinder.py`).

:::
:::{tab-item} C++ interface (ROOT macro)

``` bash
root -l -q analysis/make_pi0_hists.C
```

```{note} 
You might need to change the filename that is passed to `podio::makeReader` if
you have changed it in the exercise before.
```

:::
:::{tab-item} Python interface

``` bash
python3 analysis/make_pi0_hits.py pi0_candidates.root
```

(assuming you haven't changed the output filename in
`runGammaGammaCandidateFinder.py`).

:::
::::

All of these options produce an output root file with histograms stored in them.
We also provide a helper script to save them as `pdf`s:

```bash
python analysis/make_plots.py <output-file-from-above>
```


The `.solution/analysis` folder also contains an example of how to use `uproot`
to read EDM4hep files.


### Replace wrapped processor with new algorithms

Now that we have verified that our algorithm is working as expected we can try
and exchange it in the standard ILD reconstruction. This should be rather
straight forward as the only things we have to do now is:
- Copy the necessary configuration from
  `GaudiKinfit/options/runGammaGammaCandidateFinder.py` to
  `HighLevelReco/HighLevelReco.py` in `$ILDCONFIG_DIR`.
- Make sure to insert both, the filter and the algorithm that does the kinematic
  fit into the `HighLevelRecoSequence` (at the right spot).

Thanks to the use of the `IOHandlerHelper` the necessary EDM4hep -> LCIO (and
vice versa) converters should be automatically inserted and you should be able
to run the reconstruction chain with the new algorithms immediately[^2].

[^2]: Any statement using *should work* should make you immediately alert.
    Combined it with *immediately* it's almost guaranteed to be a trap ;)


#### Hints and common pitfalls

:::{dropdown} I get a segmentation violation after replacing the processor with the algorithm(s)

```{note}
TL;DR: There might be a symbol name clash between the Marlin processor and the
Gaudi algorithm that we have just created. Both are called
`GammaGammaCandidateFinder` and the dynamic linker will simply pick one of them
and essentially ignore the other.

The fix is simple: Rename your algorithm (the actual class name!) to something
that doesn't clash.
```


**Longer explanation and example crash**

If you have not changed the name of the algorithm from the skeleton version you
will almost certainly run into a segmentation violation in the *MyEtaFinder*
(see below). But how does this make sense? The segmentation violation seems to
happen inside the *MyEtaFinder*, a Marlin processor. Looking into the stacktrace
a bit more we see that it's actually in
`MarlinProcessorWrapper::instantiateProcessor` (see *Frame #8* in the
stacktrace. How can this be? We haven't touched any of that in this tutorial.

While this can, in general, have a myriad of reasons which are usually quite
hard to debug and figure out. In this case we have purposefully set you up for a
failure that we think might happen during the transition period. Having
experienced this issue first hand will hopefully make you think back to this
tutorial and give you at least some easy to try approach to fix this issue.

The technical reason is that both Gaudi and Marlin use a plugin system and they
load libraries dynamically at runtime. In this case **Gaudi does this before
Marlin**. Since the *MarlinReco* library and the one that we just built for the
tutorial both define the same `GammaGammaCandiateFinder` symbol name only one of
them will be available. On Linux systems it will be the one that is loaded first
(i.e. Gaudi in our case). Since *k4MarlinWrapper* still uses pretty much all of
the Marlin machinery, at some point it will try to use the
`GammaGammaCandidateFinder` *as if it were a Marlin processor*. However, since
it actually is the Gaudi algorithm that we just wrote trying to instantiate it
as a Marlin processor won't work and thus we get the segmentation violation.

```{note}
This will happen if
- you have a Marlin processor and a Gaudi algorithm with the same **symbol
  name** (i.e. class name),
- **and** you try to run them in the same chain.

Nevertheless we recommend to try and avoid such naming clashes wherever possible
```


``` console
[...]
MyphotonCorrect...   INFO Init processor
MyEtaFinder          INFO Parameter values for: MyEtaFinder of type GammaGammaCandidateFinder
TUnixSystem::Di...  FATAL segmentation violation



===========================================================
There was a crash.
This is the entire stack trace of all threads:
===========================================================
#0  0x00007b293e1107d7 in __GI___wait4 (pid=849797, stat_loc=stat_loc
entry=0x7ffd4a3a81b8, options=options
entry=0, usage=usage
entry=0x0) at ../sysdeps/unix/sysv/linux/wait4.c:30
#1  0x00007b293e11091b in __GI___waitpid (pid=<optimized out>, stat_loc=stat_loc
entry=0x7ffd4a3a81b8, options=options
entry=0) at ./posix/waitpid.c:38
#2  0x00007b293e0585bb in do_system (line=<optimized out>) at ../sysdeps/posix/system.c:172
#3  0x00007b293b92416a in TUnixSystem::Exec (shellcmd=<optimized out>, this=0x56e187a221c0) at <spack-prefix>/spack-stage/spack-stage-root-6.36.02-5zde5xmrhenapxt4smmop322jkfqs5sx/spack-src/core/unix/src/TUnixSystem.cxx:2157
#4  TUnixSystem::StackTrace (this=0x56e187a221c0) at <spack-prefix>/spack-stage/spack-stage-root-6.36.02-5zde5xmrhenapxt4smmop322jkfqs5sx/spack-src/core/unix/src/TUnixSystem.cxx:2448
#5  0x00007b293b923ab4 in TUnixSystem::DispatchSignals (this=0x56e187a221c0, sig=kSigSegmentationViolation) at <spack-prefix>/spack-stage/spack-stage-root-6.36.02-5zde5xmrhenapxt4smmop322jkfqs5sx/spack-src/core/unix/src/TUnixSystem.cxx:3668
#6  <signal handler called>
#7  0x0000000000000000 in ?? ()
#8  0x00007b291cdea45a in MarlinProcessorWrapper::instantiateProcessor (this=this
entry=0x56e19a98d7a0, parameters=std::shared_ptr<marlin::StringParameters> (use count 1, weak count 0) = {...}, processorTypeStr=...) at <tutorial-prefix>/k4MarlinWrapper/k4MarlinWrapper/src/components/MarlinProcessorWrapper.cpp:142
#9  0x00007b291cdf0787 in MarlinProcessorWrapper::initialize (this=0x56e19a98d7a0) at <tutorial-prefix>/k4MarlinWrapper/k4MarlinWrapper/src/components/MarlinProcessorWrapper.cpp:199
#10 0x00007b293ce88af8 in Gaudi::Algorithm::sysInitialize (this=0x56e19a98d7a0) at <dev-prefix>/Gaudi/GaudiKernel/src/Lib/Algorithm.cpp:95
#11 0x00007b293d061ab8 in operator() (__closure=<synthetic pointer>, a=<optimized out>, b=<optimized out>) at <dev-prefix>/Gaudi/GaudiKernel/src/Lib/Sequence.cpp:24
#12 std::accumulate<__gnu_cxx::__normal_iterator<Gaudi::Algorithm**, std::vector<Gaudi::Algorithm*> >, bool, (anonymous namespace)::for_algorithms<&Gaudi::Algorithm::sysInitialize, std::vector<Gaudi::Algorithm*> >(std::vector<Gaudi::Algorithm*>&)::<lambda(bool, Gaudi::Algorithm*)> > (__binary_op=..., __init=<optimized out>, __last=..., __first=0x56e19a98d7a0) at /usr/include/c++/13/bits/stl_numeric.h:169
#13 (anonymous namespace)::for_algorithms<&Gaudi::Algorithm::sysInitialize, std::vector<Gaudi::Algorithm*> > (c=std::vector of length 63, capacity 64 = {...}) at <dev-prefix>/Gaudi/GaudiKernel/src/Lib/Sequence.cpp:23
#14 Gaudi::Sequence::initialize (this=this
entry=0x56e19a9360f0) at <dev-prefix>/Gaudi/GaudiKernel/src/Lib/Sequence.cpp:32
#15 0x00007b293d066e08 in Gaudi::Sequencer::initialize (this=0x56e19a9360f0) at <dev-prefix>/Gaudi/GaudiKernel/src/Lib/Sequencer.cpp:42
#16 0x00007b293ce88af8 in Gaudi::Algorithm::sysInitialize (this=0x56e19a9360f0) at <dev-prefix>/Gaudi/GaudiKernel/src/Lib/Algorithm.cpp:95
#17 0x00007b293d061ab8 in operator() (__closure=<synthetic pointer>, a=<optimized out>, b=<optimized out>) at <dev-prefix>/Gaudi/GaudiKernel/src/Lib/Sequence.cpp:24
#18 std::accumulate<__gnu_cxx::__normal_iterator<Gaudi::Algorithm**, std::vector<Gaudi::Algorithm*> >, bool, (anonymous namespace)::for_algorithms<&Gaudi::Algorithm::sysInitialize, std::vector<Gaudi::Algorithm*> >(std::vector<Gaudi::Algorithm*>&)::<lambda(bool, Gaudi::Algorithm*)> > (__binary_op=..., __init=<optimized out>, __last=..., __first=0x56e19a9360f0) at /usr/include/c++/13/bits/stl_numeric.h:169
#19 (anonymous namespace)::for_algorithms<&Gaudi::Algorithm::sysInitialize, std::vector<Gaudi::Algorithm*> > (c=std::vector of length 2, capacity 2 = {...}) at <dev-prefix>/Gaudi/GaudiKernel/src/Lib/Sequence.cpp:23
#20 Gaudi::Sequence::initialize (this=this
entry=0x56e189977cc0) at <dev-prefix>/Gaudi/GaudiKernel/src/Lib/Sequence.cpp:32
#21 0x00007b293d066e08 in Gaudi::Sequencer::initialize (this=0x56e189977cc0) at <dev-prefix>/Gaudi/GaudiKernel/src/Lib/Sequencer.cpp:42
#22 0x00007b293ce88af8 in Gaudi::Algorithm::sysInitialize (this=0x56e189977cc0) at <dev-prefix>/Gaudi/GaudiKernel/src/Lib/Algorithm.cpp:95
#23 0x00007b293cf626bc in MinimalEventLoopMgr::initialize (this=this
entry=0x56e1898d4ab0) at <dev-prefix>/Gaudi/GaudiKernel/src/Lib/MinimalEventLoopMgr.cpp:106
#24 0x00007b293c3f8e2a in EventLoopMgr::initialize (this=0x56e1898d4ab0) at <dev-prefix>/Gaudi/GaudiCoreSvc/src/ApplicationMgr/EventLoopMgr.cpp:35
#25 0x00007b293d06c5de in Service::sysInitialize_imp (this=0x56e1898d4ab0) at <dev-prefix>/Gaudi/GaudiKernel/src/Lib/Service.cpp:62
#26 0x00007b293e0a1ed3 in __pthread_once_slow (once_control=0x56e1898d4b40, init_routine=0x7b293c8eb420 <std::__once_proxy()>) at ./nptl/pthread_once.c:116
#27 0x00007b293d068316 in __gthread_once (__func=<optimized out>, __once=0x56e1898d4b40) at /usr/include/x86_64-linux-gnu/c++/13/bits/gthr-default.h:700
#28 std::call_once<void (Service::*)(), Service*> (__f=
0x7ffd4a3ac380: (void (Service::*)(class Service * const)) 0x7b293d06c2c0 <Service::sysInitialize_imp()>, __once=...) at /usr/include/c++/13/mutex:907
#29 Service::sysInitialize (this=0x56e1898d4ab0) at <dev-prefix>/Gaudi/GaudiKernel/src/Lib/Service.cpp:34
#30 0x00007b293c4139ef in ServiceManager::initialize (this=0x56e187589020) at <dev-prefix>/Gaudi/GaudiCoreSvc/src/ApplicationMgr/ServiceManager.cpp:246
#31 0x00007b293c3cb70d in ApplicationMgr::initialize (this=0x56e187b7aac0) at <dev-prefix>/Gaudi/GaudiCoreSvc/src/ApplicationMgr/ApplicationMgr.cpp:476
#32 0x00007b293cead6d4 in Gaudi::Application::run (this=0x56e1876d1100) at <dev-prefix>/Gaudi/GaudiKernel/src/Lib/Application.cpp:83
#33 0x00007b293e26c056 in ffi_call_unix64 () from <spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/libffi/3.4.8-4ae47v/lib/libffi.so.8
#34 0x00007b293e26a2e6 in ffi_call_int () from <spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/libffi/3.4.8-4ae47v/lib/libffi.so.8
#35 0x00007b293e26b02e in ffi_call () from <spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/libffi/3.4.8-4ae47v/lib/libffi.so.8
#36 0x00007b293e289d11 in _call_function_pointer (argtypecount=<optimized out>, argcount=<optimized out>, resmem=0x7ffd4a3acae0, restype=<optimized out>, atypes=<optimized out>, avalues=0x7ffd4a3acac0, pProc=0x7b293ceaca80 <_py_Gaudi__Application__run(Gaudi::Application*)>, flags=4353, st=0x7b293dca16f0) at ./Modules/_ctypes/callproc.c:950
#37 _ctypes_callproc (st=st
entry=0x7b293dca16f0, pProc=<optimized out>, argtuple=argtuple
entry=(95526639702272,), flags=<optimized out>, argtypes=<optimized out>, restype=<optimized out>, checker=<optimized out>) at ./Modules/_ctypes/callproc.c:1301
#38 0x00007b293e28339b in PyCFuncPtr_call (self=<optimized out>, inargs=<optimized out>, kwds=0x0) at ./Modules/_ctypes/_ctypes.c:4382
#39 0x00007b293e4fc180 in _PyObject_MakeTpCall (tstate=0x7b293e99d5f0 <_PyRuntime+283024>, callable=callable
entry=<_FuncPtr(__name__='_py_Gaudi__Application__run') at remote 0x7b293d8174d0>, args=args
entry=0x7b293eb75358, nargs=1, keywords=keywords
entry=0x0) at Objects/call.c:242
#40 0x00007b293e4fc451 in _PyObject_VectorcallTstate (kwnames=0x0, nargsf=<optimized out>, args=0x7b293eb75358, callable=<_FuncPtr(__name__='_py_Gaudi__Application__run') at remote 0x7b293d8174d0>, tstate=<optimized out>) at ./Include/internal/pycore_call.h:166
#41 0x00007b293e494e09 in _PyEval_EvalFrameDefault (tstate=<optimized out>, frame=<optimized out>, throwflag=<optimized out>) at Python/generated_cases.c.h:813
#42 0x00007b293e6551fd in _PyEval_EvalFrame (throwflag=0, frame=0x7b293eb75020, tstate=0x7b293e99d5f0 <_PyRuntime+283024>) at ./Include/internal/pycore_ceval.h:119
#43 _PyEval_Vector (args=0x0, argcount=0, kwnames=0x0, locals=<code at remote 0x7b293df1d080>, func=0x7b293def4d60, tstate=0x7b293e99d5f0 <_PyRuntime+283024>) at Python/ceval.c:1816
#44 PyEval_EvalCode (co=co
entry=<code at remote 0x7b293df1d080>, globals=globals
entry={'__name__': '__main__', '__doc__': None, '__package__': None, '__loader__': <SourceFileLoader(name='__main__', path='<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/bin/k4run') at remote 0x7b293dfa63f0>, '__spec__': None, '__annotations__': {}, '__builtins__': <module at remote 0x7b293df9d850>, '__file__': '<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/bin/k4run', '__cached__': None, 'os': <module at remote 0x7b293de9d580>, 'sys': <module at remote 0x7b293df93a10>, 'argparse': <module at remote 0x7b293dee1bc0>, 'logging': <module at remote 0x7b293dee38d0>, 'signal': <module at remote 0x7b293ddcfc40>, 'warnings': <module at remote 0x7b293ddfb4c0>, 'Path': <type at remote 0x56e1877ab7e0>, 'load_file': <function at remote 0x7b293d7dbf60>, 'get_logger': <function at remote 0x7b293d7f0540>, 'set_log_level': <function at remote 0x7b293d7f0040>, 'LOG_LEVELS': ('VERBOSE', 'DEBUG', 'INFO', 'WARNING', 'ERROR'), 'FILTER_GAUDI_PROPS': ['Ca...(truncated), locals=locals
entry={'__name__': '__main__', '__doc__': None, '__package__': None, '__loader__': <SourceFileLoader(name='__main__', path='<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/bin/k4run') at remote 0x7b293dfa63f0>, '__spec__': None, '__annotations__': {}, '__builtins__': <module at remote 0x7b293df9d850>, '__file__': '<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/bin/k4run', '__cached__': None, 'os': <module at remote 0x7b293de9d580>, 'sys': <module at remote 0x7b293df93a10>, 'argparse': <module at remote 0x7b293dee1bc0>, 'logging': <module at remote 0x7b293dee38d0>, 'signal': <module at remote 0x7b293ddcfc40>, 'warnings': <module at remote 0x7b293ddfb4c0>, 'Path': <type at remote 0x56e1877ab7e0>, 'load_file': <function at remote 0x7b293d7dbf60>, 'get_logger': <function at remote 0x7b293d7f0540>, 'set_log_level': <function at remote 0x7b293d7f0040>, 'LOG_LEVELS': ('VERBOSE', 'DEBUG', 'INFO', 'WARNING', 'ERROR'), 'FILTER_GAUDI_PROPS': ['Ca...(truncated)) at Python/ceval.c:604
#45 0x00007b293e6bd4ae in run_eval_code_obj (locals={'__name__': '__main__', '__doc__': None, '__package__': None, '__loader__': <SourceFileLoader(name='__main__', path='<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/bin/k4run') at remote 0x7b293dfa63f0>, '__spec__': None, '__annotations__': {}, '__builtins__': <module at remote 0x7b293df9d850>, '__file__': '<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/bin/k4run', '__cached__': None, 'os': <module at remote 0x7b293de9d580>, 'sys': <module at remote 0x7b293df93a10>, 'argparse': <module at remote 0x7b293dee1bc0>, 'logging': <module at remote 0x7b293dee38d0>, 'signal': <module at remote 0x7b293ddcfc40>, 'warnings': <module at remote 0x7b293ddfb4c0>, 'Path': <type at remote 0x56e1877ab7e0>, 'load_file': <function at remote 0x7b293d7dbf60>, 'get_logger': <function at remote 0x7b293d7f0540>, 'set_log_level': <function at remote 0x7b293d7f0040>, 'LOG_LEVELS': ('VERBOSE', 'DEBUG', 'INFO', 'WARNING', 'ERROR'), 'FILTER_GAUDI_PROPS': ['Ca...(truncated), globals={'__name__': '__main__', '__doc__': None, '__package__': None, '__loader__': <SourceFileLoader(name='__main__', path='<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/bin/k4run') at remote 0x7b293dfa63f0>, '__spec__': None, '__annotations__': {}, '__builtins__': <module at remote 0x7b293df9d850>, '__file__': '<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/bin/k4run', '__cached__': None, 'os': <module at remote 0x7b293de9d580>, 'sys': <module at remote 0x7b293df93a10>, 'argparse': <module at remote 0x7b293dee1bc0>, 'logging': <module at remote 0x7b293dee38d0>, 'signal': <module at remote 0x7b293ddcfc40>, 'warnings': <module at remote 0x7b293ddfb4c0>, 'Path': <type at remote 0x56e1877ab7e0>, 'load_file': <function at remote 0x7b293d7dbf60>, 'get_logger': <function at remote 0x7b293d7f0540>, 'set_log_level': <function at remote 0x7b293d7f0040>, 'LOG_LEVELS': ('VERBOSE', 'DEBUG', 'INFO', 'WARNING', 'ERROR'), 'FILTER_GAUDI_PROPS': ['Ca...(truncated), co=0x7b293df1d080, tstate=0x7b293e99d5f0 <_PyRuntime+283024>) at Python/pythonrun.c:1381
#46 run_eval_code_obj (tstate=0x7b293e99d5f0 <_PyRuntime+283024>, co=0x7b293df1d080, globals={'__name__': '__main__', '__doc__': None, '__package__': None, '__loader__': <SourceFileLoader(name='__main__', path='<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/bin/k4run') at remote 0x7b293dfa63f0>, '__spec__': None, '__annotations__': {}, '__builtins__': <module at remote 0x7b293df9d850>, '__file__': '<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/bin/k4run', '__cached__': None, 'os': <module at remote 0x7b293de9d580>, 'sys': <module at remote 0x7b293df93a10>, 'argparse': <module at remote 0x7b293dee1bc0>, 'logging': <module at remote 0x7b293dee38d0>, 'signal': <module at remote 0x7b293ddcfc40>, 'warnings': <module at remote 0x7b293ddfb4c0>, 'Path': <type at remote 0x56e1877ab7e0>, 'load_file': <function at remote 0x7b293d7dbf60>, 'get_logger': <function at remote 0x7b293d7f0540>, 'set_log_level': <function at remote 0x7b293d7f0040>, 'LOG_LEVELS': ('VERBOSE', 'DEBUG', 'INFO', 'WARNING', 'ERROR'), 'FILTER_GAUDI_PROPS': ['Ca...(truncated), locals={'__name__': '__main__', '__doc__': None, '__package__': None, '__loader__': <SourceFileLoader(name='__main__', path='<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/bin/k4run') at remote 0x7b293dfa63f0>, '__spec__': None, '__annotations__': {}, '__builtins__': <module at remote 0x7b293df9d850>, '__file__': '<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/bin/k4run', '__cached__': None, 'os': <module at remote 0x7b293de9d580>, 'sys': <module at remote 0x7b293df93a10>, 'argparse': <module at remote 0x7b293dee1bc0>, 'logging': <module at remote 0x7b293dee38d0>, 'signal': <module at remote 0x7b293ddcfc40>, 'warnings': <module at remote 0x7b293ddfb4c0>, 'Path': <type at remote 0x56e1877ab7e0>, 'load_file': <function at remote 0x7b293d7dbf60>, 'get_logger': <function at remote 0x7b293d7f0540>, 'set_log_level': <function at remote 0x7b293d7f0040>, 'LOG_LEVELS': ('VERBOSE', 'DEBUG', 'INFO', 'WARNING', 'ERROR'), 'FILTER_GAUDI_PROPS': ['Ca...(truncated)) at Python/pythonrun.c:1348
#47 0x00007b293e6bd80f in run_mod (mod=<optimized out>, filename=filename
entry='<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/bin/k4run', globals=globals
entry={'__name__': '__main__', '__doc__': None, '__package__': None, '__loader__': <SourceFileLoader(name='__main__', path='<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/bin/k4run') at remote 0x7b293dfa63f0>, '__spec__': None, '__annotations__': {}, '__builtins__': <module at remote 0x7b293df9d850>, '__file__': '<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/bin/k4run', '__cached__': None, 'os': <module at remote 0x7b293de9d580>, 'sys': <module at remote 0x7b293df93a10>, 'argparse': <module at remote 0x7b293dee1bc0>, 'logging': <module at remote 0x7b293dee38d0>, 'signal': <module at remote 0x7b293ddcfc40>, 'warnings': <module at remote 0x7b293ddfb4c0>, 'Path': <type at remote 0x56e1877ab7e0>, 'load_file': <function at remote 0x7b293d7dbf60>, 'get_logger': <function at remote 0x7b293d7f0540>, 'set_log_level': <function at remote 0x7b293d7f0040>, 'LOG_LEVELS': ('VERBOSE', 'DEBUG', 'INFO', 'WARNING', 'ERROR'), 'FILTER_GAUDI_PROPS': ['Ca...(truncated), locals=locals
entry={'__name__': '__main__', '__doc__': None, '__package__': None, '__loader__': <SourceFileLoader(name='__main__', path='<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/bin/k4run') at remote 0x7b293dfa63f0>, '__spec__': None, '__annotations__': {}, '__builtins__': <module at remote 0x7b293df9d850>, '__file__': '<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/bin/k4run', '__cached__': None, 'os': <module at remote 0x7b293de9d580>, 'sys': <module at remote 0x7b293df93a10>, 'argparse': <module at remote 0x7b293dee1bc0>, 'logging': <module at remote 0x7b293dee38d0>, 'signal': <module at remote 0x7b293ddcfc40>, 'warnings': <module at remote 0x7b293ddfb4c0>, 'Path': <type at remote 0x56e1877ab7e0>, 'load_file': <function at remote 0x7b293d7dbf60>, 'get_logger': <function at remote 0x7b293d7f0540>, 'set_log_level': <function at remote 0x7b293d7f0040>, 'LOG_LEVELS': ('VERBOSE', 'DEBUG', 'INFO', 'WARNING', 'ERROR'), 'FILTER_GAUDI_PROPS': ['Ca...(truncated), flags=flags
entry=0x7ffd4a3ad188, arena=arena
entry=0x7b293df198f0, interactive_src=0x0, generate_new_source=0) at Python/pythonrun.c:1466
#48 0x00007b293e6bf9c8 in pyrun_file (flags=0x7ffd4a3ad188, closeit=1, locals={'__name__': '__main__', '__doc__': None, '__package__': None, '__loader__': <SourceFileLoader(name='__main__', path='<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/bin/k4run') at remote 0x7b293dfa63f0>, '__spec__': None, '__annotations__': {}, '__builtins__': <module at remote 0x7b293df9d850>, '__file__': '<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/bin/k4run', '__cached__': None, 'os': <module at remote 0x7b293de9d580>, 'sys': <module at remote 0x7b293df93a10>, 'argparse': <module at remote 0x7b293dee1bc0>, 'logging': <module at remote 0x7b293dee38d0>, 'signal': <module at remote 0x7b293ddcfc40>, 'warnings': <module at remote 0x7b293ddfb4c0>, 'Path': <type at remote 0x56e1877ab7e0>, 'load_file': <function at remote 0x7b293d7dbf60>, 'get_logger': <function at remote 0x7b293d7f0540>, 'set_log_level': <function at remote 0x7b293d7f0040>, 'LOG_LEVELS': ('VERBOSE', 'DEBUG', 'INFO', 'WARNING', 'ERROR'), 'FILTER_GAUDI_PROPS': ['Ca...(truncated), globals={'__name__': '__main__', '__doc__': None, '__package__': None, '__loader__': <SourceFileLoader(name='__main__', path='<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/bin/k4run') at remote 0x7b293dfa63f0>, '__spec__': None, '__annotations__': {}, '__builtins__': <module at remote 0x7b293df9d850>, '__file__': '<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/bin/k4run', '__cached__': None, 'os': <module at remote 0x7b293de9d580>, 'sys': <module at remote 0x7b293df93a10>, 'argparse': <module at remote 0x7b293dee1bc0>, 'logging': <module at remote 0x7b293dee38d0>, 'signal': <module at remote 0x7b293ddcfc40>, 'warnings': <module at remote 0x7b293ddfb4c0>, 'Path': <type at remote 0x56e1877ab7e0>, 'load_file': <function at remote 0x7b293d7dbf60>, 'get_logger': <function at remote 0x7b293d7f0540>, 'set_log_level': <function at remote 0x7b293d7f0040>, 'LOG_LEVELS': ('VERBOSE', 'DEBUG', 'INFO', 'WARNING', 'ERROR'), 'FILTER_GAUDI_PROPS': ['Ca...(truncated), start=257, filename='<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/bin/k4run', fp=0x7b293de91b00) at Python/pythonrun.c:1295
#49 _PyRun_SimpleFileObject (fp=fp
entry=0x56e187654640, filename=filename
entry='<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/bin/k4run', closeit=closeit
entry=1, flags=flags
entry=0x7ffd4a3ad188) at Python/pythonrun.c:517
#50 0x00007b293e6bff80 in _PyRun_AnyFileObject (fp=0x56e187654640, filename=filename
entry='<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/bin/k4run', closeit=closeit
entry=1, flags=flags
entry=0x7ffd4a3ad188) at Python/pythonrun.c:77
#51 0x00007b293e6e9b11 in pymain_run_file_obj (skip_source_first_line=0, filename='<spack-prefix>/spackages/skylake-ubuntu24.04-gcc13.3.0/k4fwcore/main-i35biy/bin/k4run', program_name='python') at Modules/main.c:410
#52 pymain_run_file (config=0x7b293e96fce8 <_PyRuntime+96392>) at Modules/main.c:429
#53 pymain_run_python (exitcode=0x7ffd4a3ad178) at Modules/main.c:696
#54 Py_RunMain () at Modules/main.c:775
#55 0x00007b293e6ea2fb in pymain_main (args=0x7ffd4a3ad2a0) at Modules/main.c:805
#56 Py_BytesMain (argc=<optimized out>, argv=<optimized out>) at Modules/main.c:829
#57 0x00007b293e02a1ca in __libc_start_call_main (main=main
entry=0x56e184bd1060 <main>, argc=argc
entry=8, argv=argv
entry=0x7ffd4a3ad448) at ../sysdeps/nptl/libc_start_call_main.h:58
#58 0x00007b293e02a28b in __libc_start_main_impl (main=0x56e184bd1060 <main>, argc=8, argv=0x7ffd4a3ad448, init=<optimized out>, fini=<optimized out>, rtld_fini=<optimized out>, stack_end=0x7ffd4a3ad438) at ../csu/libc-start.c:360
#59 0x000056e184bd1095 in _start ()
===========================================================

[...]
```
:::
