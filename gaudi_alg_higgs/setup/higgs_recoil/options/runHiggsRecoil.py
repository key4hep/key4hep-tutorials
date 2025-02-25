#
# Copyright (c) 2014-2024 Key4hep-Project.
#
# This file is part of Key4hep.
# See https://key4hep.github.io/key4hep-doc/ for further info.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

from Gaudi.Configuration import INFO
from Configurables import HiggsRecoil, MuonFilter
from k4FWCore import ApplicationMgr, IOSvc

io_svc = IOSvc()
io_svc.Input = "rv02-02.sv02-02.mILD_l5_o1_v02.E250-SetA.I402004.Pe2e2h.eR.pL.n000.d_dstm_15090_0.edm4hep.root"

io_svc.CollectionNames = [
    "PandoraPFOs",
]

io_svc.outputCommands = [
    "drop *",
    "keep Muons",
    "keep PandoraPFOs",
    "keep Z",
    "keep Higgs",
]
io_svc.Output = "higgs_recoil_out.root"

# The collections that we don't drop will be present in the output file
# io_svc.outputCommands = ["drop Collection1"]

# If we don't specify the values for the name parameters
# they will take the default value defined in the C++ code
muon = MuonFilter("MuonFilter",
                 InputPFOs=["PandoraPFOs"],
                 OutputMuons=["Muons"],
                 MinPt=10.0)

recoil = HiggsRecoil("HiggsRecoil",
                     InputMuons=["Muons"],
                     HiggsCollection=["Higgs"],
                     ZCollection=["Z"],
                     )

ApplicationMgr(TopAlg=[muon, recoil],
               EvtSel="NONE",
               EvtMax=-1,
               ExtSvc=[],
               OutputLevel=INFO,
               )
