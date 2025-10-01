#!/usr/bin/env python3

from Gaudi.Configuration import INFO
from k4FWCore import ApplicationMgr
from Configurables import RecoParticleFilter, GammaGammaCandidateFinder, EventDataSvc


# Configure the RecoParticleFilter to filter photons
photon_filter = RecoParticleFilter("PhotonFilter")
photon_filter.PDG = 22  # Photon PDG ID
photon_filter.MinPt = 5.0  # Minimum pT in GeV
photon_filter.InputCollection = ["ReconstructedParticles"]
photon_filter.OutputCollection = ["FilteredPhotons"]

# Configure the GammaGammaCandidateFinder
gamma_gamma_finder = GammaGammaCandidateFinder("GammaGammaFinder")
gamma_gamma_finder.InputCollection = photon_filter.OutputCollection
gamma_gamma_finder.OutputCollection = "GammaGammaCandidates"
gamma_gamma_finder.ResonancePDG = 111
gamma_gamma_finder.ResonanceMass = 0.135
gamma_gamma_finder.MaxDeltaM = 0.04
gamma_gamma_finder.MinFitProbability = 0.001
gamma_gamma_finder.Fitter = "OPALFitter"

# Configure the application manager
ApplicationMgr(
    TopAlg=[photon_filter, gamma_gamma_finder],
    EvtSel="NONE",
    EvtMax=-1,
    ExtSvc=[EventDataSvc("EventDataSvc")],
    OutputLevel=INFO,
)
