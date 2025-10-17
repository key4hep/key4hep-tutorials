#!/usr/bin/env python3

from Gaudi.Configuration import INFO, DEBUG
from k4FWCore import ApplicationMgr, IOSvc
from Configurables import (
    RecoParticleFilter,
    EventDataSvc,
)

iosvc = IOSvc()

# Configure the RecoParticleFilter to filter photons
photon_filter = RecoParticleFilter("PhotonFilter")
photon_filter.selectPDG = 22  # Photon PDG ID
photon_filter.minEnergy = 0.5  # Minimum energy in GeV
photon_filter.InputCollection = ["PandoraPFOs"]
photon_filter.OutputCollection = ["FilteredPhotons"]

# Configure the GammaGammaCandidateFinder
# TODO: Instantiate the
# GammaGammaCandidateFinder and configure the inpus and outputs

gamma_gamma_finder.OutputLevel = DEBUG
gamma_gamma_finder.ResonancePDG = 111
gamma_gamma_finder.ResonanceMass = 0.1349766
gamma_gamma_finder.MaxDeltaM = 0.04
gamma_gamma_finder.MinFitProbability = 0.001


iosvc.Output = "pi0_candidates.root"
# TODO: Make sure to also keep your GammaGamma collection
iosvc.outputCommands = [
    "drop *",
    "keep PandoraPFOs",  # Keep the underlying data for the photons
    "keep GammaGamma*",  # Keep the original Marlin GammaGamma candidates
    "keep FilteredPhotons",
    "keep MCParticles",
    # Remove some collections that also match the GammaGamma* from above
    "drop *_startVertices",
    "drop *Eta*",
]


# Configure the application manager
app_mgr = ApplicationMgr(
    TopAlg=[photon_filter, gamma_gamma_finder],
    EvtSel="NONE",
    EvtMax=-1,
    ExtSvc=[EventDataSvc()],
    OutputLevel=INFO,
)
