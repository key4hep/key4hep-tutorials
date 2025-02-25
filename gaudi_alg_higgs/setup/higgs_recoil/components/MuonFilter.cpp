/*
 * Copyright (c) 2014-2024 Key4hep-Project.
 *
 * This file is part of Key4hep.
 * See https://key4hep.github.io/key4hep-doc/ for further info.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Gaudi/Property.h"

#include "edm4hep/ReconstructedParticleCollection.h"
#include "edm4hep/utils/kinematics.h"

#include "k4FWCore/Transformer.h"

#include <string>

struct MuonFilter final
  : public k4FWCore::Transformer<edm4hep::ReconstructedParticleCollection(const edm4hep::ReconstructedParticleCollection&)> {
  MuonFilter(const std::string& name, ISvcLocator* svcLoc)
      : Transformer(
            name, svcLoc,
            {KeyValues("InputPFOs", {"PandoraPFOs"})},
            {KeyValues("OutputMuons", {"Muons"})}) {
  }

  edm4hep::ReconstructedParticleCollection operator()(const edm4hep::ReconstructedParticleCollection& recoColl) const override {

    auto ret = edm4hep::ReconstructedParticleCollection();
    // Since we are creating a new collection only from elements of an already
    // existing collection we have to set the subset collection flag to true
    // Otherwise there will be errors at runtime saying that the objects are
    // already in a collection so they can't be put in another one
    ret.setSubsetCollection();

    int nMuons = 0;
    // Iterate over each ReconstructedParticle in the input collection
    for (const auto& reco : recoColl) {
      // The PDG ID of the muon is 13 or -13
      if (std::abs(reco.getPDG()) == 13) {
        // Cut on Pt
        const auto muonPt = edm4hep::utils::pt(reco);
        if (muonPt > m_minPt) {
          ret.push_back(reco);
          // The debug message is only printed if the log level is set to DEBUG
          debug() << "Muon with pt " << muonPt << " GeV "
                 << "and mass " << reco.getMass() << " GeV "
                 << "and energy " << reco.getEnergy() << " GeV "
                 << "and momentum " << reco.getMomentum()[0] << " " << reco.getMomentum()[1] << " " << reco.getMomentum()[2] << " GeV "
                 << "added to collection" << endmsg;
          nMuons++;
        } else {
          debug() << "Muon with pt " << muonPt
                  << " GeV, not considered due to minimum pT cut of "
                  << m_minPt.value() << endmsg;
        }
      }
    }
    info() << "Found " << nMuons << " muons (pT > " << m_minPt.value()
           << " GeV) in " << recoColl.size() << " reconstructed particle "
           << endmsg;

    // We return always a collection that may or may not be empty
    return ret;
  }

  Gaudi::Property<double> m_minPt{this, "MinPt", 10., "Minimum pT of muons to be considered in GeV"};
};

DECLARE_COMPONENT(MuonFilter)
