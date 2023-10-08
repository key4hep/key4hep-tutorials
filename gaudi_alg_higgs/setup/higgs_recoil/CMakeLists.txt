

set(sources components/HiggsRecoil.cpp
            components/MuonFilter.cpp
)

gaudi_add_module(tutorial
                 SOURCES ${sources}
                 LINK Gaudi::GaudiKernel
                      Gaudi::GaudiAlgLib
                      k4FWCore::k4FWCore
                      EDM4HEP::edm4hep
                      EDM4HEP::utils
                      )

install(TARGETS tutorial
  RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}" COMPONENT bin
  LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}" COMPONENT shlib
  COMPONENT dev)
