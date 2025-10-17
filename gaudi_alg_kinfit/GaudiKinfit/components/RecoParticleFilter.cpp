#include "RecoParticleFilter.hpp"

#include <edm4hep/ReconstructedParticleCollection.h>
#include <fmt/format.h>

RecoParticleFilter::RecoParticleFilter(const std::string& name, ISvcLocator* svcLoc)
// TODO: Use the constructor of the base class to initialize inputs and outputs
{}

edm4hep::ReconstructedParticleCollection
RecoParticleFilter::operator()(const edm4hep::ReconstructedParticleCollection& recoColl) const {
  // TODO:
  // - Create a **subset collection** for returning the results
  for (const auto& reco : recoColl) {
    // TODO:
    // - Check all elements in the input recoColl and if they satisfy the
    //   criteria (configured via Properties) add them to the return collection
  }
}

DECLARE_COMPONENT(RecoParticleFilter)
