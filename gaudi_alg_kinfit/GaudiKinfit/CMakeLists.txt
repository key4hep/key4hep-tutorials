set(sources
  # components/GammaGammaCandidateFinder.cpp
  # components/RecoParticleFilter.cpp
)

gaudi_add_module(GaudiKinfitPlugins
 SOURCES ${sources}
 LINK
   Gaudi::GaudiKernel
   k4FWCore::k4FWCore
   EDM4HEP::edm4hep
   MarlinKinfit::MarlinKinfit
   Eigen3::Eigen
)

install(TARGETS GaudiKinfitPlugins
  EXPORT GaudiKinfitTargets
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
)
