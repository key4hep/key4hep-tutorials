#include "RecoParticleFilter.hpp"

RecoParticleFilter::RecoParticleFilter(const std::string& name, ISvcLocator* svcLoc)
    : Transformer(name, svcLoc, {KeyValues("InputPFOs", {"PandoraPFOs"})},
                  {KeyValues("OutputParticles", {"FilteredParticles"})}) {}

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
    if (std::abs(reco.getPDG()) == std::abs(m_pdgId)) {
      const auto particlePt = edm4hep::utils::pt(reco);
      if (particlePt > m_minPt) {
        ret.push_back(reco);
        verbose() << "Particle with PDG " << reco.getPDG() << " and pt " << particlePt << " GeV "
                  << "and mass " << reco.getMass() << " GeV "
                  << "and energy " << reco.getEnergy() << " GeV "
                  << "and momentum " << reco.getMomentum()[0] << " " << reco.getMomentum()[1] << " "
                  << reco.getMomentum()[2] << " GeV "
                  << "added to collection" << endmsg;
        nParticles++;
      } else {
        verbose() << "Particle with PDG " << reco.getPDG() << " and pt " << particlePt
                  << " GeV, not considered due to minimum pT cut of " << m_minPt.value() << endmsg;
      }
    }
  }
  debug() << "Found " << nParticles << " particles with PDG " << m_pdgId.value() << " (pT > " << m_minPt.value()
          << " GeV) in " << recoColl.size() << " reconstructed particle " << endmsg;

  return ret;
}

DECLARE_COMPONENT(RecoParticleFilter)
