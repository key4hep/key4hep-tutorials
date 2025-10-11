#!/usr/bin/env python3

from Gaudi.Configuration import DEBUG, INFO
from k4FWCore import ApplicationMgr, IOSvc
from Configurables import RecoParticleFilter, EventDataSvc

iosvc = IOSvc()
photon_filter = RecoParticleFilter("PhotonFilter")
photon_filter.OutputLevel = DEBUG  # At least for development it might be useful to know what the algorithm is doing
# TODO: Use the properties you have defined to configure this to select photons
# with a minimum energy of 5 GeV

iosvc.Output = "photon_candidates.root"
# TODO: Configure the output commands here properly to keep the collections you
# have just created in the photon_filter
iosvc.outputCommands = ["drop *"]

ApplicationMgr(
    # TODO: Configure the ApplicationMgr to run your algorithm
    EvtSel="NONE",
    EvtMax=-1,  # use --num-events for changing this from the command line
    ExtSvc=[EventDataSvc()],
    OutputLevel=INFO,  # Otherwise this is very chatty
)
