set(sources
  components/GammaGammaCandidateFinder.cpp
  components/RecoParticleFilter.cpp
)

gaudi_add_module(GaudiKinfitPlugins
 SOURCES ${sources}
 LINK
   Gaudi::GaudiKernel
   k4FWCore::k4FWCore
   EDM4HEP::edm4hep
   ${MarlinKinfit_LIBRARIES}
   Eigen3::Eigen
)

target_include_directories(GaudiKinfitPlugins PUBLIC
  ${MarlinKinfit_INCLUDE_DIRS}
)
target_compile_definitions(GaudiKinfitPlugins PUBLIC
  MARLIN_USE_ROOT  # Need to work around the fact that MarlinKinfit does not properly export this
)

install(TARGETS GaudiKinfitPlugins
  EXPORT GaudiKinfitTargets
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
)
