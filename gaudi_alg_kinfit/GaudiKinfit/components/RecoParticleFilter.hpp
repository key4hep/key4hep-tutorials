#pragma once

#include <edm4hep/ReconstructedParticleCollection.h>

// TODO: include the correct k4FWCore header for the necessary base class

#include <Gaudi/Property.h>

#include <string>

struct RecoParticleFilter // : TODO: Figure out the right k4FWCore base class to inherit from
{
  RecoParticleFilter(const std::string& name, ISvcLocator* svcLoc);

  // TODO: Finalize the operator() signature
  edm4hep::ReconstructedParticleCollection
  operator()(const edm4hep::ReconstructedParticleCollection& recoColl) const final;

  // TODO: Add a property to make the (aboslute) PDG value configurable
  // TODO: Add a property to make the minimal energy cut configurable
};
