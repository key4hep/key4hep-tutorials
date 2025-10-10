#include "RecoParticleFilter.hpp"

#include <fmt/format.h>

RecoParticleFilter::RecoParticleFilter(const std::string& name, ISvcLocator* svcLoc)
    : Transformer(name, svcLoc, {KeyValues("InputCollection", {"PandoraPFOs"})},
                  {KeyValues("OutputCollection", {"FilteredParticles"})}) {}

edm4hep::ReconstructedParticleCollection
RecoParticleFilter::operator()(const edm4hep::ReconstructedParticleCollection& recoColl) const {

  auto ret = edm4hep::ReconstructedParticleCollection();
  // Since we are creating a new collection only from elements of an already
  // existing collection we have to set the subset collection flag to true
  // Otherwise there will be errors at runtime saying that the objects are
  // already in a collection so they can't be put in another one
  ret.setSubsetCollection();

  int nParticles = 0;
  for (const auto& reco : recoColl) {
    verbose()
        << fmt::format(
               "Checking particle: PDG={}, mass={:.4f} GeV, energy={:.4f} GeV, momentum=({:.4f}, {:.4f}, {:.4f}) GeV",
               reco.getPDG(), reco.getMass(), reco.getEnergy(), reco.getMomentum().x, reco.getMomentum().y,
               reco.getMomentum().z)
        << endmsg;

    if (std::abs(reco.getPDG()) == std::abs(m_pdgId)) {
      const auto particlePt = edm4hep::utils::pt(reco);
      const auto particleE = reco.getEnergy();
      if (particlePt > m_minPt && particleE > m_minE) {
        ret.push_back(reco);
        nParticles++;
      }
    }
  }
  debug() << fmt::format(
                 "Found {} particles with PDG {} and pT > {} GeV and E > {} GeV in {} input reconstructed particles",
                 nParticles, m_pdgId.value(), m_minPt.value(), m_minE.value(), recoColl.size())
          << endmsg;

  return ret;
}

DECLARE_COMPONENT(RecoParticleFilter)
