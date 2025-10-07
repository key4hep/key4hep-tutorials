#!/usr/bin/env python3

from Gaudi.Configuration import INFO
from k4FWCore import ApplicationMgr, IOSvc
from Configurables import RecoParticleFilter, GammaGammaCandidateFinder, EventDataSvc

iosvc = IOSvc()

# Configure the RecoParticleFilter to filter photons
photon_filter = RecoParticleFilter("PhotonFilter")
photon_filter.PDG = 22  # Photon PDG ID
photon_filter.MinPt = 0.5  # Minimum pT in GeV
photon_filter.InputCollection = ["PandoraPFOs"]
photon_filter.OutputCollection = ["FilteredPhotons"]

# Configure the GammaGammaCandidateFinder
gamma_gamma_finder = GammaGammaCandidateFinder("GammaGammaFinder")
gamma_gamma_finder.InputCollection = photon_filter.OutputCollection
gamma_gamma_finder.OutputCollection = ["GammaGammaCandidates_Pi0_New"]
gamma_gamma_finder.ResonancePDG = 111
gamma_gamma_finder.ResonanceMass = 0.1349766
gamma_gamma_finder.MaxDeltaM = 0.04
gamma_gamma_finder.MinFitProbability = 0.001
gamma_gamma_finder.Fitter = "OPALFitter"

pi0_filter = RecoParticleFilter("Pi0Filter")
pi0_filter.PDG = 111
pi0_filter.MinPt = 1.0
pi0_filter.InputCollection = gamma_gamma_finder.OutputCollection
pi0_filter.OutputCollection = ["Pi0s_New"]

iosvc.outputCommands = [
    "drop *",
    "keep PandoraPFOs",
    "keep GammaGamma*",
    "keep FilteredPhotons",
    "keep *_New",
    "keep MCParticles",
]

# Configure the application manager
ApplicationMgr(
    TopAlg=[photon_filter, gamma_gamma_finder, pi0_filter],
    EvtSel="NONE",
    EvtMax=-1,
    ExtSvc=[EventDataSvc("EventDataSvc")],
    OutputLevel=INFO,
)
