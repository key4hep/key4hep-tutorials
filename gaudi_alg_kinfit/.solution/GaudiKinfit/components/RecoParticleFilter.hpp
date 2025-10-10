#pragma once

#include "Gaudi/Property.h"

#include "edm4hep/ReconstructedParticleCollection.h"
#include "edm4hep/utils/kinematics.h"

#include "k4FWCore/Transformer.h"

#include <string>

struct RecoParticleFilter final : public k4FWCore::Transformer<edm4hep::ReconstructedParticleCollection(
                                      const edm4hep::ReconstructedParticleCollection&)> {
  RecoParticleFilter(const std::string& name, ISvcLocator* svcLoc);

  edm4hep::ReconstructedParticleCollection
  operator()(const edm4hep::ReconstructedParticleCollection& recoColl) const override;

  Gaudi::Property<int> m_pdgId{this, "PDG", 13,
                               "PDG ID of particles to filter (will use the absolute value for filtering)"};
  Gaudi::Property<double> m_minPt{this, "MinPt", 0., "Minimum pT of particles to be considered in GeV"};
  Gaudi::Property<double> m_minE{this, "MinE", 0., "Minimum energy of particles to be considered in GeV"};
};
