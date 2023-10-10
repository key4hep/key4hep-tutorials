#
# Copyright (c) 2014-2023 Key4hep-Project.
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
from Configurables import ApplicationMgr
from Configurables import k4DataSvc
from Configurables import PodioOutput
from Configurables import PodioInput

data_svc = k4DataSvc("EventDataSvc")
data_svc.input = "/home/juanmi/Downloads/ZH_40_events_ILD_DST_merged.edm4hep.root"

inp = PodioInput()
inp.collections = [
    "PandoraPFOs",
]

out = PodioOutput("out")
out.outputCommands = [
    "drop *",
    "keep Muons",
    "keep PandoraPFOs",
    "keep Z",
    "keep Higgs",
]
out.filename = "higgs_recoil_out.root"

# The collections that we don't drop will be present in the output file
# out.outputCommands = ["drop Collection1"]

# If we don't specify the values for the name parameters
# they will take the default value defined in the C++ code
muon = MuonFilter("MuonFilter",
                 InputPFOs="PandoraPFOs",
                 OutputMuons="Muons",
                 MinPt=10.0)

recoil = HiggsRecoil("HiggsRecoil",
                     InputMuons="Muons",
                     HiggsCollection="Higgs",
                     ZCollection="Z",
                     )

ApplicationMgr(TopAlg=[inp, muon, recoil, out],
               EvtSel="NONE",
               EvtMax=-1,
               ExtSvc=[data_svc],
               OutputLevel=INFO,
               )
