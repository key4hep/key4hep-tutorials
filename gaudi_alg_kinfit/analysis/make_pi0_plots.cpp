#include <TFile.h>
#include <TH1D.h>
#include <TLorentzVector.h>

#include "podio/Frame.h"
#include "podio/Reader.h"

#include "edm4hep/ReconstructedParticleCollection.h"
#include "edm4hep/utils/kinematics.h"

#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <inputfile> <outputfile>" << std::endl;
    return 1;
  }

  // Open input file
  auto reader = podio::makeReader(argv[1]);

  // Create output file and histograms
  auto histfile = std::make_unique<TFile>(argv[2], "recreate");

  auto pi0_mass = TH1D("m_pi0_mass", ";M_{#pi^{0}};Entries", 100, 0.130, 0.139);
  auto pi0_mass_p4 = TH1D("m_pi0_p4", ";M_{#gamma#gamma};Entries", 100, 0.130, 0.139);
  auto pi0_mass_prefit = TH1D("m_pi0_mass_prefit", ";M_{#gamma#gamma} (prefit);Entries", 100, 0.1, 0.19);
  auto fit_delta_m =
      TH1D("fit_delta_m", ";M_{#gamma#gamma} (postfit) - M_{#gamma#gamma} (prefit);Entries", 100, -0.1, 0.1);

  // Process events
  unsigned int nEvents = reader.getEntries("events");
  for (unsigned int i = 0; i < nEvents; ++i) {
    auto event = reader.readEvent(i);

    const auto& pi0s = event.get<edm4hep::ReconstructedParticleCollection>("Pi0s_New");

    for (const auto& pi0 : pi0s) {
      pi0_mass.Fill(pi0.getMass());

      auto pi0_p4 = edm4hep::utils::p4(pi0, edm4hep::utils::UseEnergy);
      pi0_mass_p4.Fill(pi0_p4.M());

      auto particles = pi0.getParticles();
      auto gamma1_p4 = edm4hep::utils::p4(particles[0], edm4hep::utils::UseEnergy);
      auto gamma2_p4 = edm4hep::utils::p4(particles[1], edm4hep::utils::UseEnergy);
      auto prefit_pi0 = gamma1_p4 + gamma2_p4;

      pi0_mass_prefit.Fill(prefit_pi0.M());
      fit_delta_m.Fill(pi0_p4.M() - prefit_pi0.M());
    }
  }

  // Write histograms
  pi0_mass.Write();
  pi0_mass_p4.Write();
  pi0_mass_prefit.Write();
  fit_delta_m.Write();

  histfile->Close();

  std::cout << "Analysis complete. Output written to " << argv[2] << std::endl;

  return 0;
}
